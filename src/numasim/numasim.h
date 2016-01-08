#pragma once

#include <assert.h>
#include <set>

#define PAGE_COEFF 32

#define THREAD_OFFSET (2 << 20)

static inline unsigned make_tid(unsigned thread, unsigned thunk)
{
  assert(thunk < THREAD_OFFSET);
  return thread * THREAD_OFFSET + thunk;
}

static inline unsigned get_thread(unsigned tid)
{
  return tid / THREAD_OFFSET;
}

static inline unsigned get_thunk(unsigned tid)
{
  return tid % THREAD_OFFSET;
}

typedef unsigned long long int param_t;


struct load_state
{
  unsigned id;
  xbt_dynar_t concurrent;
  xbt_dict_t cur;
  xbt_dict_t map;
  xbt_dict_t finished;
  SD_task_t start;
  xbt_dynar_t order;
  xbt_dynar_t all;
};

void load_task_graph(struct load_state *state, const char *graph_file);
void free_state(struct load_state *state);
void init_state(struct load_state *state);

void print_dag(xbt_dynar_t tasklist);
void dump_jedule(char* filename, xbt_dynar_t dag);

typedef std::set<unsigned long> page_set;

struct task_data
{
  int cpu;
  page_set *read_set;
  page_set *write_set;
  page_set *access_set;
  unsigned tid;
};

typedef struct task_data task_data_t;

struct numa_data
{
  page_set all;
};

typedef struct numa_data numa_data_t;

struct ws_data
{
  int domain;
  numa_data_t *numa;
};

typedef struct ws_data ws_data_t;

static const int namelen = 80;
