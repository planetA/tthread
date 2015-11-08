/*
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

/*
 * @file  xthread.cpp
 * @brief  Functions to manage thread related spawn, join.
 *
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @author Tongping Liu <http://www.cs.umass.edu/~tonyliu>
 */

#include <new>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stddef.h>
#include <stddef.h>
#include <syscall.h>
#include <unistd.h>

#include "debug.h"
#include "xrun.h"
#include "xthread.h"

void *xthread::spawn(const void     *caller,
                     threadFunction *fn,
                     void           *arg,
                     int            parent_index) {
  // Allocate an object to hold the thread's return value.
  void *buf = allocateSharedObject(4096);

  static_assert(4096 > sizeof(ThreadStatus),
                "Not enough space to hold ThreadStatus");
  ThreadStatus *t = new (buf)ThreadStatus;

  return forkSpawn(caller, fn, t, arg, parent_index);
}

/// @brief Get thread index for this thread.
int xthread::getThreadIndex(void *v) {
  ASSERT(v != NULL);

  ThreadStatus *t = (ThreadStatus *)v;
  ASSERT(_run.threadindex() == t->threadIndex);

  return t->threadIndex;
}

/// @brief Get thread index for this thread.
int xthread::getThreadIndex() {
  return _run.threadindex();
}

/// @brief Get thread tid for this thread.
int xthread::getThreadPid(void *v) {
  ASSERT(v != NULL);

  ThreadStatus *t = (ThreadStatus *)v;

  return t->tid;
}

/// @brief Do pthread_join.
void xthread::join(void *v, void **result) {
  ThreadStatus *t = (ThreadStatus *)v;

  DEBUGF("%d: Joining thread %d", getpid(), t->tid);

  // Grab the thread result from the status structure (set by the thread),
  // reclaim the memory, and return that result.
  if (result != NULL) {
    *result = t->retval;
  }

  // Free the shared object held by this thread.
  freeSharedObject(t, 4096);
}

/// @brief Cancel one thread. Send out a SIGKILL signal to that thread
int xthread::cancel(void *v) {
  ThreadStatus *t = (ThreadStatus *)v;

  int threadindex = t->threadIndex;

  kill(t->tid, SIGKILL);

  // Free the shared object held by this thread.
  freeSharedObject(t, 4096);
  return threadindex;
}

int xthread::thread_kill(void *v, int sig)
{
  int threadindex;
  ThreadStatus *t = (ThreadStatus *)v;

  threadindex = t->threadIndex;

  kill(t->tid, sig);

  freeSharedObject(t, 4096);
  return threadindex;
}

void *xthread::forkSpawn(const void     *caller,
                         threadFunction *fn,
                         ThreadStatus   *t,
                         void           *arg,
                         int            parent_index) {
  // Use fork to create the effect of a thread spawn.
  // FIXME:: For current process, we should close share.
  // children to use MAP_PRIVATE mapping. Or just let child to do that in the
  // beginning.
  int child = syscall(SYS_clone, CLONE_FS | CLONE_FILES | SIGCHLD, (void *)0);


  if (child == -1) {
    // fork failed, handle failure in parent
    return NULL;
  } else if (child) {
    // I need to wait until the child has waited on creation barrier
    // sucessfully.
    _run.waitChildRegistered();

    return (void *)t;
  } else {
    pid_t mypid = syscall(SYS_getpid);
    setId(mypid);

    int threadindex = _run.childRegister(mypid, parent_index);

    t->threadIndex = threadindex;
    t->tid = mypid;

    _run.waitParentNotify();

    _nestingLevel++;
    run_thread(caller, fn, t, arg);
    _nestingLevel--;

    _exit(0);
    return NULL;
  }
}

// @brief Execute the thread.
void xthread::run_thread(const void     *caller,
                         threadFunction *fn,
                         ThreadStatus   *t,
                         void           *arg) {
  _run.atomicBegin(caller);
  void *result = fn(arg);
  _run.threadDeregister(caller);

  t->retval = result;
}
