// -*- C++ -*-

/*
   Author: Emery Berger, http://www.cs.umass.edu/~emery

   Copyright (c) 2007-8 Emery Berger, University of Massachusetts Amherst.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include <cstddef>
#include <errno.h>
#include <new>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "debug.h"
#include "prof.h"
#include "real.h"
#include "tthread/log.h"
#include "visibility.h"
#include "xdefines.h"
#include "xlogger.h"
#include "xmemory.h"
#include "xrun.h"

#ifndef BUILTIN_RETURN_ADDRESS
# include <execinfo.h>
#endif // ifndef BUILTIN_RETURN_ADDRESS

#if defined(__GNUG__)
void initialize() __attribute__((constructor));

void finalize() __attribute__((destructor));
#endif // if defined(__GNUG__)

// points to cross process shared data
runtime_data_t *global_data;

// By locating all static memory of this library in one place,
// we can later easily exclude it from memory protection
static bool initialized = false;
static char xrunbuf[sizeof(xrun)];
static char xmemorybuf[sizeof(xmemory)];
#ifdef DEBUG
static char logbuf[sizeof(tthread::log)];
#endif // ifdef DEBUG
static xrun *run;
static xmemory *memory;

// store size of malloc() calls during library initialisation
// this is neither thread-safe nor fast
typedef struct {
  void *ptr;
  size_t size;
} mmap_allocation_t;
enum {
  MAX_MMAP_ALLOCATIONS = 1024
};
static mmap_allocation_t preinit_mmap_allocations[MAX_MMAP_ALLOCATIONS] = {};

namespace tthread {
xlogger *logger;
}

void initialize() {
  DEBUG("intializing libtthread");


  init_real_functions();

  global_data = (runtime_data_t *)WRAP(mmap)(NULL,
                                             xdefines::PageSize,
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED | MAP_ANONYMOUS,
                                             -1,
                                             0);
  global_data->thread_index = 1;

#ifndef BUILTIN_RETURN_ADDRESS

  // In glibc backtrace() invokes _dl_init() on the first call, which triggers
  // malloc. To avoid pagefaults, call backtrace once on startup
  void *array[1];
  backtrace(array, 1);
#endif // ifdef BUILTIN_RETURN_ADDRESS

  DEBUG("after mapping global data structure");
  memory = new(xmemorybuf)xmemory;
  tthread::logger = xlogger::allocate(global_data->xlogger,
                                      memory->getLayout());

  run = new(xrunbuf)xrun(*memory, *tthread::logger);
  initialized = true;
}

#ifdef BUILTIN_RETURN_ADDRESS
# define CALLER (__builtin_return_address(0))
#else // ifdef BUILTIN_RETURN_ADDRESS
// Get instruction pointer of caller
// this function is implemented as a macro, because
// it would otherwise appear in the backtrace
//
// glibc's backtrace uses libgcc,
// which uses pthread_mutex_lock to obtain backtrace
// to avoid infinite recursion (because pthread_mutex_lock),
// because our pthread_mutex_lock also calls this macro, disable
// the lock during this function call
# define CALLER ({                      \
    bool was_initialized = initialized; \
    initialized = false;                \
    void *array[2];                     \
    int size = backtrace(array, 2);     \
    assert(size == 2);                  \
    initialized = was_initialized;      \
    array[1];                           \
  })
#endif // ifdef BUILTIN_RETURN_ADDRESS

void finalize() {
  DEBUG("finalizing libtthread");

  memory->closeProtection();
  initialized = false;

  #ifdef DEBUG_ENABLED
  fprintf(stderr, "\nStatistics information:\n");
  PRINT_TIMER(serial);
  PRINT_COUNTER(commit);
  PRINT_COUNTER(twinpage);
  PRINT_COUNTER(suspectpage);
  PRINT_COUNTER(slowpage);
  PRINT_COUNTER(dirtypage);
  PRINT_COUNTER(lazypage);
  PRINT_COUNTER(shorttrans);

  tthread::log *log = new(logbuf)tthread::log;
  log->print();
  #endif // DEBUG_ENABLED
}

extern "C" {
_PUBLIC_ void printLog() {
  tthread::log log;

  log.print();
}

_PUBLIC_ void *malloc(size_t sz) __THROW {
  void *ptr;

  if (initialized) {
    ptr = run->malloc(sz);
  } else {
    DEBUG("Pre-initialization malloc call forwarded to mmap");

    for (int i = 0; i < MAX_MMAP_ALLOCATIONS; i++) {
      mmap_allocation_t *alloc = &preinit_mmap_allocations[i];

      if (alloc->ptr == NULL) {
        ptr = mmap(NULL,
                   sz,
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS,
                   -1,
                   0);
        alloc->ptr = ptr;
        alloc->size = sz;

        if (ptr == MAP_FAILED) {
          return NULL;
        }
        return ptr;
      }
    }
    fprintf(stderr, "Too much pre-initialisation mmaps");
    abort();
  }

  if (ptr == NULL) {
    fprintf(stderr, "%d: Out of memory!\n", getpid());
    ::abort();
  }
  return ptr;
}

_PUBLIC_ void *calloc(size_t nmemb, size_t sz) throw() {
  void *ptr;

  if (initialized) {
    ptr = run->calloc(nmemb, sz);
  } else {
    DEBUG("Pre-initialization calloc call forwarded to mmap");

    // actually mmap see malloc impl above
    ptr = malloc(nmemb * sz);
    memset(ptr, 0, sz * nmemb);
  }

  if (ptr == NULL) {
    fprintf(stderr, "%d: Out of memory!\n", getpid());
    ::abort();
  }

  memset(ptr, 0, sz * nmemb);

  return ptr;
}

_PUBLIC_ void free(void *ptr) __THROW {
  if (initialized) {
    run->free(ptr);
  } else {
    DEBUG("Pre-initialization free forwarded to munmap");

    for (int i; i < MAX_MMAP_ALLOCATIONS; i++) {
      mmap_allocation_t *alloc = &preinit_mmap_allocations[i];

      if (alloc->ptr == ptr) {
        assert(munmap(ptr, alloc->size) == 0);
        alloc->ptr = NULL;
        alloc->size = 0;
        return;
      }
    }
  }
}

_PUBLIC_ void *memalign(size_t boundary, size_t size) throw() {
  DEBUG("memalign is not supported");
  return NULL;
}

_PUBLIC_ size_t malloc_usable_size(void *ptr) __THROW {
  if (initialized) {
    return run->getSize(ptr);
  } else {
    DEBUG("Pre-initialization malloc_usable_size");

    for (int i; i < MAX_MMAP_ALLOCATIONS; i++) {
      mmap_allocation_t *alloc = &preinit_mmap_allocations[i];

      if (alloc->ptr == ptr) {
        return alloc->size;
      }
    }
  }
  return 0;
}

_PUBLIC_ void *realloc(void *ptr, size_t sz) throw() {
  if (initialized) {
    return run->realloc(ptr, sz);
  } else {
    DEBUG("Pre-initialization realloc forwared to mremap.");

    for (int i; i < MAX_MMAP_ALLOCATIONS; i++) {
      mmap_allocation_t *alloc = &preinit_mmap_allocations[i];

      if (alloc->ptr == ptr) {
        void *newPtr = mremap(ptr, alloc->size, sz, 0);

        if (newPtr == MAP_FAILED) {
          return NULL;
        }
        alloc->ptr = newPtr;
        alloc->size = sz;
        return newPtr;
      }
    }
  }
  return NULL;
}

_PUBLIC_ pid_t getpid(void) __THROW {
  if (initialized) {
    return run->id();
  }
  return 0;
}

_PUBLIC_ int sched_yield(void) __THROW {
  return 0;
}

_PUBLIC_ void pthread_exit(void *value_ptr) {
  if (initialized) {
    run->threadDeregister();
  }
  _exit(0);
}

_PUBLIC_ int pthread_cancel(pthread_t thread) {
  if (initialized) {
    run->cancel(CALLER, (void *)thread);
  }
  return 0;
}

_PUBLIC_ int pthread_setconcurrency(int) __THROW {
  return 0;
}

_PUBLIC_ int pthread_attr_init(pthread_attr_t *) __THROW {
  return 0;
}

_PUBLIC_ int pthread_attr_destroy(pthread_attr_t *) throw() {
  return 0;
}

_PUBLIC_ pthread_t pthread_self(void) __THROW {
  if (initialized) {
    return (pthread_t)run->id();
  }
  return 0;
}

_PUBLIC_ int pthread_kill(pthread_t thread, int sig) __THROW {
  DEBUG("pthread_kill is not supported");
  return 0;
}

_PUBLIC_ int sigwait(const sigset_t *set, int *sig) {
  return run->sig_wait(CALLER, set, sig);
}

_PUBLIC_ int pthread_mutex_init(pthread_mutex_t *mutex,
                                const pthread_mutexattr_t *) throw() {
  if (initialized) {
    return run->mutex_init(mutex);
  }
  return 0;
}

_PUBLIC_ int pthread_mutex_lock(pthread_mutex_t *mutex) throw() {
  if (initialized) {
    run->mutex_lock(CALLER, mutex);
  }
  return 0;
}

_PUBLIC_ int pthread_mutex_trylock(pthread_mutex_t *mutex) throw() {
  DEBUG("pthread_mutex_trylock is not supported");
  return 0;
}

_PUBLIC_ int pthread_mutex_unlock(pthread_mutex_t *mutex) throw() {
  if (initialized) {
    run->mutex_unlock(CALLER, mutex);
  }
  return 0;
}

_PUBLIC_ int pthread_mutex_destory(pthread_mutex_t *mutex) {
  if (initialized) {
    return run->mutex_destroy(mutex);
  }
  return 0;
}

_PUBLIC_ int pthread_attr_getstacksize(const pthread_attr_t *,
                                       size_t *s) throw() {
  *s = 1048576UL; // really? FIX ME
  return 0;
}

_PUBLIC_ int pthread_mutexattr_destroy(pthread_mutexattr_t *) throw() {
  return 0;
}

_PUBLIC_ int pthread_mutexattr_init(pthread_mutexattr_t *) throw() {
  return 0;
}

_PUBLIC_ int pthread_mutexattr_settype(pthread_mutexattr_t *, int) throw() {
  return 0;
}

_PUBLIC_ int pthread_mutexattr_gettype(const pthread_mutexattr_t *,
                                       int *) throw() {
  return 0;
}

_PUBLIC_ int pthread_attr_setstacksize(pthread_attr_t *, size_t) throw() {
  return 0;
}

_PUBLIC_ int pthread_create(pthread_t            *tid,
                            const pthread_attr_t *attr,
                            void *(*fn)(
                              void *),
                            void                 *arg) throw() {
  if (!initialized) {
    return EAGAIN;
  }

  void *res = run->spawn(CALLER, fn, arg);

  if (res == NULL) {
    return EAGAIN;
  }
  *tid = (pthread_t)res;
  return 0;
}

_PUBLIC_ int pthread_join(pthread_t tid, void **val) {
  if (initialized) {
    run->join(CALLER, (void *)tid, val);
  }
  return 0;
}

_PUBLIC_ int pthread_cond_init(pthread_cond_t           *cond,
                               const pthread_condattr_t *attr) throw() {
  if (initialized) {
    run->cond_init((void *)cond);
  }
  return 0;
}

_PUBLIC_ int pthread_cond_broadcast(pthread_cond_t *cond) throw() {
  if (initialized) {
    run->cond_broadcast(CALLER, (void *)cond);
  }
  return 0;
}

_PUBLIC_ int pthread_cond_signal(pthread_cond_t *cond) throw() {
  if (initialized) {
    run->cond_signal(CALLER, (void *)cond);
  }
  return 0;
}

_PUBLIC_ int pthread_cond_wait(pthread_cond_t  *cond,
                               pthread_mutex_t *mutex)
{
  if (initialized) {
    run->cond_wait(CALLER, (void *)cond, (void *)mutex);
  }
  return 0;
}

_PUBLIC_ int pthread_cond_destroy(pthread_cond_t *cond) throw() {
  if (initialized) {
    run->cond_destroy(cond);
  }
  return 0;
}

// Add support for barrier functions
_PUBLIC_ int pthread_barrier_init(pthread_barrier_t           *barrier,
                                  const pthread_barrierattr_t *attr,
                                  unsigned int                count) throw() {
  if (initialized) {
    return run->barrier_init(barrier, count);
  }
  return 0;
}

_PUBLIC_ int pthread_barrier_destroy(pthread_barrier_t *barrier) throw() {
  if (initialized) {
    return run->barrier_destroy(barrier);
  }
  return 0;
}

_PUBLIC_ int pthread_barrier_wait(pthread_barrier_t *barrier) throw() {
  if (initialized) {
    return run->barrier_wait(barrier);
  }
  return 0;
}

_PUBLIC_ ssize_t write(int fd, const void *buf, size_t count) {
  // Produce read pagefault to buffer in advance to avoid EFAULT errno
  uint8_t *start = (uint8_t *)buf;

  volatile int temp;

  for (size_t i = 0; i < count; i += xdefines::PageSize) {
    temp = start[i];
  }

  temp = start[count - 1];

  return WRAP(write)(fd, buf, count);
}

_PUBLIC_ ssize_t read(int fd, void *buf, size_t count) {
  // Produce write pagefault to buffer in advance to avoid EFAULT errno
  uint8_t *start = (uint8_t *)buf;

  for (size_t i = 0; i < count; i += xdefines::PageSize) {
    start[i] = 0;
  }

  start[count - 1] = 0;

  return WRAP(read)(fd, buf, count);
}

// DISABLED
#if 0
void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
  int newflags = flags;

  if ((initialized == true)
      && (flags & MAP_PRIVATE)) {
    //		newflags = (flags & ~MAP_PRIVATE) | MAP_SHARED;
    printf("flags %x and newflags %x\n", flags, newflags);
  }
  return WRAP(mmap)(addr, length, prot, newflags, fd, offset);
}

#endif // if 0
}
