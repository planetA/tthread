#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

#include <algorithm>
#include <iterator>
#include <string>

#include "simdag/simdag.h"
#include "numasim.h"

static char *jedule_file;
static char *platform_file;
static char *scheduler_str;
static char *graph_file;
static char *sched_arg;
static double CCR = 10;        /* Computation-communication ratio */
static double CV = 1.0;         /* Coefficient of variation */

static int parse_args(int argc, char **argv)
{
  static struct option long_options[] = {
    {"platform", required_argument, 0, 'p'},
    {"sched",    required_argument, 0, 's'},
    {"graph",    required_argument, 0, 'g'},
    {"jedule",    required_argument, 0, 'j'},
    {"CCR",      optional_argument, 0, 'c'},
    {"CV",       optional_argument, 0, 'v'},
    {NULL, 0, NULL, 0}
  };

  int c;
  int indexptr = 0;
  printf("argc %d \n", argc);
  while ((c = getopt_long(argc, argv, "p:s:g:c:v:j:",
                          long_options, &indexptr)) != -1) {
    switch (c) {
    case 'p':
      platform_file = optarg;
      break;
    case 's':
      scheduler_str = optarg;
      break;
    case 'g':
      graph_file = optarg;
      break;
    case 'c':
      CCR = strtod(optarg, NULL);
      break;
    case 'v':
      CV = strtod(optarg, NULL);
      break;
    case 'j':
      jedule_file = optarg;
      break;
    }
  }
  return 0;
}

void print_dag(xbt_dynar_t tasklist)
{
  /* SD_task_get_amount(task); */
  /* SD_task_get_name(task); */
  SD_task_t task;
  unsigned int cpt;
  xbt_dynar_foreach(tasklist, cpt, task) {
    SD_task_dump(task);
  }
}

void init_platform()
{
  int N_processor = SD_workstation_get_number();
  const SD_workstation_t* processors = SD_workstation_get_list();
  int max_domain = 0;

  for (int i = 0; i < N_processor; i++) {
    auto domain = std::stoi(SD_workstation_get_property_value(processors[i], "domain"));
    // set access mode
    SD_workstation_set_access_mode(processors[i], SD_WORKSTATION_SHARED_ACCESS);
    printf("PE%02d: %s %s\n", i, SD_workstation_get_name(processors[i]),
           SD_workstation_get_property_value(processors[i], "domain"));
    max_domain = std::max(max_domain, domain);

    auto ws_data = new ws_data_t;
    ws_data->domain = domain;
    ws_data->numa = NULL;
    SD_workstation_set_data(processors[i], ws_data);
  }

  max_domain++;

  std::vector<numa_data_t *> domains(max_domain);

  for (int i = 0; i < max_domain; i++) {
    domains[i] = new numa_data();
  }

  for (int i = 0; i < N_processor; i++) {
    auto ws_data = static_cast<ws_data_t *>(SD_workstation_get_data(processors[i]));
    ws_data->numa = domains[ws_data->domain];
    printf("Init size %lu\n", ws_data->numa->all.size());
  }

  int N_link = SD_link_get_number();
  const SD_link_t *links = SD_link_get_list();

  for (int i = 0; i < N_link; i++) {
    auto policy = SD_link_get_sharing_policy(links[i]);

    if (policy == SD_LINK_FATPIPE) {
      // printf("Link %s\n", SD_link_get_name(links[i]));
    } else {
      xbt_assert(policy == SD_LINK_SHARED);
    }
  }
}

/*
 * Add synthetic tasks which "copy" required pages from other NUMA
 * domains. A task may run only when the synthetic ones are finished.
 */
void schedule_pages(struct load_state *state, SD_task_t task)
{
  using std::begin;
  using std::end;
  using std::inserter;
  using std::back_inserter;
  using std::vector;
  using std::set_intersection;
  using std::set_difference;

  const SD_workstation_t *ws_list = SD_workstation_get_list();
  int ws_max = SD_workstation_get_number();
  auto task_data = static_cast<struct task_data *>(SD_task_get_data(task));
  auto page_demand = *task_data->access_set;
  auto ws_data =
    static_cast<struct ws_data *>(SD_workstation_get_data(ws_list[task_data->cpu]));
  auto local_numa_data = ws_data->numa;
  vector<unsigned long> asked_pages;

  if (task_data->access_set->size() == 0)
    return;

  printf("Need to access %ld pages\n", task_data->access_set->size());

  for (int i = 0; i < ws_max; ++i) {
    auto ws_data = static_cast<struct ws_data *>(SD_workstation_get_data(ws_list[i]));
    auto numa_data = ws_data->numa;
    vector<unsigned long> result;
    set_intersection(numa_data->all.begin(), numa_data->all.end(),
                          page_demand.begin(), page_demand.end(),
                          back_inserter(result));
    if (result.size() > 0) {
      // We have pages to fetch from a NUMA domain
      if (numa_data != local_numa_data) {
        // We fetch pages from another NUMA domain

        // We create two additional tasks. Sequential task creates
        // dependency from remote domain. Communication task transfers
        // pages.
        char name[namelen];
        snprintf(name, namelen, "dep-%s", SD_task_get_name(task));
        SD_task_t comp = SD_task_create_comp_seq(name, NULL, 0);
        xbt_dynar_push(state->all, &comp);

        snprintf(name, namelen, "comm-%s", SD_task_get_name(task));
        SD_task_t comm = SD_task_create_comm_e2e(name, NULL, result.size() * PAGE_COEFF);
        xbt_dynar_push(state->all, &comm);

        snprintf(name, namelen, "dep-comm-%s", SD_task_get_name(task));
        SD_task_dependency_add (name, NULL, comp, comm);
        snprintf(name, namelen, "comm-task-%s", SD_task_get_name(task));
        SD_task_dependency_add (name, NULL, comm, task);

        SD_task_schedulev(comp, 1, &ws_list[i]);
        // SD_task_schedulel(comm, 2, &ws_list[i], &ws_list[task_data->cpu]);


        printf("Take %ld pages from remote domain\n", result.size());
      } else {
        // We fetch pages from local NUMA domain
        // printf("Take %ld pages from local domain\n", result.size());
      }
      asked_pages.insert(end(asked_pages), begin(result), end(result));
    }
  }

  sort(begin(asked_pages), end(asked_pages));

  auto old_size = local_numa_data->all.size();

  vector<unsigned long> rest;
  set_difference(begin(page_demand), end(page_demand),
                   begin(asked_pages), end(asked_pages),
                   inserter(local_numa_data->all, begin(local_numa_data->all)));

  if (old_size != local_numa_data->all.size())
    printf("New size of local domain is %lu (was %lu)\n",
           local_numa_data->all.size(), old_size);
}

/*
 * Commit pages from write set. Synthetic subtasks of _task_ delay
 * children of _task_.
 */
void commit_pages(struct load_state *state, SD_task_t task)
{
  using std::begin;
  using std::end;
  using std::inserter;
  using std::back_inserter;
  using std::vector;
  using std::set_intersection;
  using std::set_difference;

  const SD_workstation_t *ws_list = SD_workstation_get_list();
  int ws_max = SD_workstation_get_number();
  auto task_data = static_cast<struct task_data *>(SD_task_get_data(task));

  if (!task_data)
    return;

  auto write_set = *task_data->write_set;
  auto ws_data =
    static_cast<struct ws_data *>(SD_workstation_get_data(ws_list[task_data->cpu]));
  auto local_numa_data = ws_data->numa;
  vector<unsigned long> asked_pages;

  printf("Need to commit %ld pages\n", task_data->write_set->size());
  for (int i = 0; i < ws_max; ++i) {
    auto ws_data = static_cast<struct ws_data *>(SD_workstation_get_data(ws_list[i]));
    auto numa_data = ws_data->numa;
    vector<unsigned long> result;
    set_intersection(numa_data->all.begin(), numa_data->all.end(),
                          write_set.begin(), write_set.end(),
                          back_inserter(result));
    if (result.size() > 0) {
      // We have pages to fetch from a NUMA domain
      if (numa_data != local_numa_data) {
        // We fetch pages from another NUMA domain

        // We create two additional tasks. Sequential task creates
        // dependency from remote domain. Communication task transfers
        // pages.
        char name[namelen];
        snprintf(name, namelen, "dep-%s", SD_task_get_name(task));
        SD_task_t comp = SD_task_create_comp_seq(name, NULL, result.size() * PAGE_COEFF);
        xbt_dynar_push(state->all, &comp);

        snprintf(name, namelen, "comm-%s", SD_task_get_name(task));
        SD_task_t comm = SD_task_create_comm_e2e(name, NULL, result.size() * PAGE_COEFF);
        xbt_dynar_push(state->all, &comm);

        // snprintf(name, namelen, "dep-comm-%s", SD_task_get_name(task));
        // SD_task_dependency_add (name, NULL, task, comm);

        SD_workstation_t list[2] = {ws_list[task_data->cpu], ws_list[i]};
        SD_task_schedulev(comm, 2, list);

        snprintf(name, namelen, "comm-task-%s", SD_task_get_name(task));
        SD_task_dependency_add (name, NULL, comm, comp);
        SD_task_schedulev(comp, 1, &ws_list[i]);

        xbt_dynar_t children = SD_task_get_children(task);
        SD_task_t child;
        unsigned child_cpt;
        xbt_dynar_foreach(children, child_cpt, child) {
          snprintf(name, namelen, "dep-%s-%s", SD_task_get_name(task), SD_task_get_name(child));
          SD_task_dependency_add (name, NULL, comp, child);
        }
        xbt_dynar_free_container(&children);


        printf("Commit %ld pages from remote domain\n", result.size());
      } else {
        // We commit pages to local NUMA domain
        char name[namelen];
        snprintf(name, namelen, "dep-%s", SD_task_get_name(task));
        SD_task_t comp = SD_task_create_comp_seq(name, NULL, result.size() * PAGE_COEFF);
        xbt_dynar_push(state->all, &comp);

        SD_task_schedulev(comp, 1, &ws_list[task_data->cpu]);
        // printf("Commit %ld pages from local domain\n", result.size());
        xbt_dynar_t children = SD_task_get_children(task);
        SD_task_t child;
        unsigned child_cpt;
        xbt_dynar_foreach(children, child_cpt, child) {
          snprintf(name, namelen, "dep-%s-%s", SD_task_get_name(task), SD_task_get_name(child));
          SD_task_dependency_add (name, NULL, comp, child);
        }
        xbt_dynar_free_container(&children);
      }
      asked_pages.insert(end(asked_pages), begin(result), end(result));
    }
  }
}

void run_simulation(struct load_state *state)
{
  unsigned cpt;

  const SD_workstation_t *ws_list = SD_workstation_get_list();
  int ws_max = SD_workstation_get_number();

  {
    /* Schedule the very first task */
    struct task_data *task_data = static_cast<struct task_data *>(SD_task_get_data(state->start));
    xbt_assert(task_data->cpu < ws_max);
    SD_task_schedulev(state->start, 1, &ws_list[task_data->cpu]);
    printf("Schedule %s\n", SD_task_get_name(state->start));
  }

  {
    SD_task_t cur;
    xbt_dynar_foreach(state->order, cpt, cur) {
      struct task_data *task_data = static_cast<struct task_data *>(SD_task_get_data(cur));
      xbt_assert(task_data->cpu < ws_max);
      SD_task_watch(cur, SD_SCHEDULABLE);
      printf("Watch %s\n", SD_task_get_name(cur));
    }
  }

  xbt_dynar_t executed;
  do {
    executed = SD_simulate(-1);

    if (xbt_dynar_is_empty(executed))
      break;

    unsigned cpt;
    SD_task_t tmp;
    xbt_dynar_foreach(executed, cpt, tmp){
      SD_task_dump(tmp);

      xbt_dynar_t children = SD_task_get_children(tmp);
      SD_task_t child;
      unsigned child_cpt;
      xbt_dynar_foreach(children, child_cpt, child) {
        if (SD_task_get_state(child) & SD_SCHEDULABLE) {
          struct task_data *task_data = static_cast<struct task_data *>(SD_task_get_data(child));
          xbt_assert(task_data->cpu < ws_max);
          // printf("Schedule task %s %d\n", SD_task_get_name(child), task_data->cpu);
          schedule_pages(state, child);
          SD_task_schedulev(child, 1, &ws_list[task_data->cpu]);
        }
      }
      xbt_dynar_free_container(&children);
      //commit_pages(state, tmp);
    }
    xbt_dynar_free_container(&executed);
  }
  while (1);

  printf("Finished at: %f\n", SD_get_clock());
}

int main(int argc, char *argv[])
{
  SD_init(&argc, argv);

  parse_args(argc, argv);

  ////// platform initialization

  // load platform file
  printf("Platform file: %s\n", platform_file);
  if (platform_file) {
    SD_create_environment(platform_file);
  } else {
    fprintf(stderr, "Platform file is missing\n");
    exit(EXIT_FAILURE);
  }

  printf("Workstation number %d\n", SD_workstation_get_number());
  init_platform();

  struct load_state state;
  init_state(&state);
  if (graph_file) {
    load_task_graph(&state, graph_file);
  } else {
    fprintf(stderr, "Graph file is missing\n");
    exit(EXIT_FAILURE);
  }

  /* print_dag(state.order); */

  run_simulation(&state);

  if (jedule_file)
    dump_jedule(jedule_file, state.all);
  free_state(&state);

  SD_exit();
  return 0;
}
