#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <unistd.h>
#include <papi.h>
#include <pthread.h>
#include <sched.h>

#include "xsched.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE);   \
  } while (0)

int xsched::get_affinity(int thunk)
{
  // Pay attention that thunk numeration starts from 1, not from 0
  if (_thread_schedule[thunk*2] != thunk) {
    fprintf(stderr, "Unexpected thunk affinity [%d/%d]=%d\n",
            _thread.getThreadIndex(), thunk, _thread_schedule[thunk*2 + 1]);
    ::abort();
  }
  return _thread_schedule[thunk*2 + 1];
}

void xsched::trigger()
{
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(get_affinity(_thread.getThunkId()), &set);
  if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
    errExit("sched_setaffinity");
}

int xsched::getCPU()
{
  return sched_getcpu();
  // return get_affinity(_thread->getThunkId());
}

void xsched::updateThread()
{
  // Schedule format:
  // threadId thunks thunkid cpu thunkid cpu ... threadId thunks thunkid cpu ...
  if (!_thunk_schedule)
    return;

  int tid = _thread.getThreadIndex();
  for (int *cur_tid = _thunk_schedule; cur_tid < _thunk_schedule + _sched_size;) {
    if (*cur_tid < tid) {
      // cur_tid + 1 number of thunks
      // * 2 size of thread schedule: thunkid cpu thunkid cpu ...
      // + 2 size of header of a thread schedule: threadId thunks
      cur_tid += (*(cur_tid + 1)) * 2 + 2;
      continue;
    }
    if (*cur_tid != tid) {
      fprintf(stderr, "Expected schedule for thread %d, found %d\n", tid, *cur_tid);
      ::abort();
    }
    _thread_schedule = cur_tid;

    break;
  }
}

xsched::xsched(xthread &thread)
  : _thread(thread),
    _thunk_schedule(NULL)
{
   char *sched_file = getenv("TTHREAD_SCHED");

   if (sched_file) {
     int fd;

     fprintf(stderr, "Loading sched: %s\n", sched_file);
     fd = WRAP(open)(sched_file, O_RDONLY);

     int ret = WRAP(lseek)(fd, 0, SEEK_END);
     if (ret < 0) {
       fprintf(stderr, "Failed to get size of a file\n");
       ::abort();
     }

     _thunk_schedule = (int *)WRAP(mmap)(NULL, ret, PROT_WRITE, MAP_PRIVATE, fd, 0);
     if (_thunk_schedule == MAP_FAILED)
       fprintf(stderr, "MMAP failed\n");

     _sched_size = ret / sizeof(int);
   }
}
