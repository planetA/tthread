#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include "simdag/simdag.h"
#include "numasim.h"

static int read_num(FILE *file, int line, param_t *i)
{
  int ret = fscanf(file, "%lld\n", i);
  if (ret < 0) {
    fprintf(stderr, "Failed to read integer number in line %d\n", line);
    exit(EXIT_FAILURE);
  }
  return ret;
}

static int read_2num(FILE *file, int line, param_t *i1, param_t *i2)
{
  int ret = fscanf(file, "%lld %lld\n", i1, i2);
  if (ret < 0) {
    fprintf(stderr, "Failed to read integer number in line %d\n", line);
    exit(EXIT_FAILURE);
  }
  return ret;
}

static struct task_data *task_data_new()
{
  struct task_data *task_data = xbt_new0(struct task_data, 1);

  task_data->cpu = 0;
  task_data->write_set = new page_set();
  task_data->read_set = new page_set();
  task_data->access_set = new page_set();

  return task_data;
}

static void task_data_destroy(struct task_data* data)
{
  delete (data->write_set);
  delete data->read_set;
  delete data->access_set;
  xbt_free(data);
}

static void dot_task_p_free(void *task) {
  SD_task_t *t = static_cast<SD_task_t *>(task);
  task_data_destroy(static_cast<struct task_data *>(SD_task_get_data(*t)));
  SD_task_destroy(*t);
}

void init_state(struct load_state *state)
{
  state->id = 0;
  state->concurrent = xbt_dynar_new(sizeof(unsigned), NULL);
  state->cur = xbt_dict_new();
  state->map = xbt_dict_new();
  state->finished = xbt_dict_new();

  struct task_data *task_data = task_data_new();

  state->order = xbt_dynar_new(sizeof(SD_task_t), dot_task_p_free);
  state->all = xbt_dynar_new(sizeof(SD_task_t), NULL);
  state->start = SD_task_create_comp_seq("task[0:0]", task_data, 0);

  int thread = 0;
  xbt_dict_set_ext(state->cur, (char *)&thread, sizeof(thread), (void *) state->start, NULL);

  unsigned tid = make_tid(thread, 0);
  task_data->tid = tid;
  xbt_dict_set_ext(state->cur, (char *)&tid, sizeof(tid), (void *) state->start, NULL);
  xbt_dynar_push(state->order, &state->start);
}

void free_state(struct load_state *state)
{
  xbt_dynar_free(&state->concurrent);
  xbt_dynar_free(&state->order);
  xbt_dynar_free(&state->all);

  xbt_dict_free(&state->cur);
  xbt_dict_free(&state->map);
  xbt_dict_free(&state->finished);
}

static SD_task_t task_by_tid(struct load_state *state, unsigned tid)
{
  return static_cast<SD_task_t>(xbt_dict_get_ext(state->map, (char *) &tid, sizeof(tid)));
}


static void add_thunk(struct load_state *state, unsigned tid, param_t cpu)
{
  int thread;
  int thunk;

  thread = get_thread(tid);
  thunk = get_thunk(tid);

  char name[namelen];
  snprintf(name, namelen, "task[%d:%d]", thread, thunk);

  struct task_data *task_data = task_data_new();
  task_data->cpu = cpu;
  task_data->tid = tid;

  SD_task_t task = SD_task_create_comp_seq(name, task_data, 0);

  SD_task_t prev_thunk = static_cast<SD_task_t>(xbt_dict_get_or_null_ext(state->cur,
                                                                         (char *)&thread, sizeof(thread)));
  if (prev_thunk) {
    xbt_dict_set_ext(state->finished, (char *)&thread, sizeof(thread), prev_thunk, NULL);
  }

  xbt_dict_cursor_t cursor;
  int *key;
  SD_task_t cur_task = NULL;

  xbt_dict_foreach(state->finished, cursor, key, cur_task) {
    char dep_name[namelen];
    snprintf(dep_name, namelen, "%s-%s",
             SD_task_get_name(cur_task), SD_task_get_name(task));
    SD_task_dependency_add (dep_name, NULL, cur_task, task);
  }

  xbt_dict_set_ext(state->cur, (char *)&thread, sizeof(thread), task, NULL);
  xbt_dict_set_ext(state->map, (char *)&tid, sizeof(tid), task, NULL);
  xbt_dynar_push(state->order, &task);
  xbt_dynar_push(state->all, &task);
  /* printf("Adding %x %s\n", ) */
}

static void add_end(struct load_state *state, unsigned tid, param_t time)
{
  const int namelen = 80;
  char name[namelen];
  snprintf(name, namelen, "task[%d:%d]", get_thread(tid), get_thunk(tid));

  SD_task_t task = task_by_tid(state, tid);
  SD_task_set_amount(task, (double)time);



  xbt_dynar_t parents = SD_task_get_parents(task);
  SD_task_t parent;
  unsigned parent_cpt;
  xbt_dynar_foreach(parents, parent_cpt, parent) {
    struct task_data *task_data = static_cast<struct task_data *>(SD_task_get_data(parent));
    int thread = get_thread(task_data->tid);
    if (xbt_dict_get_or_null_ext(state->finished, (char *)&thread, sizeof(thread)))
      xbt_dict_remove_ext(state->finished, (char *)&thread, sizeof(thread));
  }
  xbt_dynar_free_container(&parents);

  int thread = get_thread(tid);
  xbt_dict_set_ext(state->finished, (char *)&thread, sizeof(thread), task, NULL);
}

static void add_finish(struct load_state *state, unsigned tid)
{
  return;

  int thread = get_thread(tid);
  xbt_dict_remove_ext(state->finished, (char *)&thread, sizeof(thread));
}

static void add_read(struct load_state *state, unsigned tid, param_t start, param_t end)
{
  SD_task_t cur = task_by_tid(state, tid);

  struct task_data *task_data = static_cast<struct task_data *>(SD_task_get_data(cur));

  for(unsigned long i = start; i <= end; ++i) {
    task_data->read_set->insert(i);
    task_data->access_set->insert(i);
  }
}

static void add_write(struct load_state *state, unsigned tid, param_t start, param_t end)
{
  SD_task_t cur = task_by_tid(state, tid);

  struct task_data *task_data = static_cast<struct task_data *>(SD_task_get_data(cur));

  for(unsigned long i = start; i <= end; ++i) {
    task_data->write_set->insert(i);
    task_data->access_set->insert(i);
  }
}

void load_task_graph(struct load_state *state, const char *graph_file)
{
  printf("Opening file %s\n", graph_file);
  FILE *file = fopen(graph_file, "r");
  if (!file) {
    fprintf(stderr, "Failed to open graph file: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  int line = 0;
  while (!feof(file)) {
    char command[10];
    int thread, thunk;
    int ret;

    ++line;

    /* printf("Reading line %d\n", line); */
    ret = fscanf(file, "%10s %d %d", command, &thread, &thunk);
    if (ret < 0) {
      fprintf(stderr, "Failed to read a command in line %d\n", line);
      exit(EXIT_FAILURE);
    }
    /* printf("Command: %s [%d, %d] ", command, thread, thunk); */

    unsigned tid = make_tid(thread, thunk);

    if (!strcmp("thunk", command)) {
      param_t cpu;
      read_num(file, line, &cpu);

      add_thunk(state, tid, cpu);
    } else if (!strcmp("end", command)) {
      param_t time;
      read_num(file, line, &time);
      add_end(state, tid, time);
    } else if (!strcmp("finish", command)) {
      fscanf(file, "\n");
      add_finish(state, tid);
    } else if (!strcmp("write", command)) {
      param_t start, end;
      read_2num(file, line, &start, &end);
      add_write(state, tid, start, end);
      /* printf("START: %lld END: %lld\n", start, end); */
    } else if (!strcmp("read", command)) {
      param_t start, end;
      read_2num(file, line, &start, &end);
      add_read(state, tid, start, end);
      /* printf("START: %lld END: %lld\n", start, end); */
    } else {
      fprintf(stderr, "Unrecognized command: %s\n", command);
      exit(EXIT_FAILURE);
    }
  }
}
