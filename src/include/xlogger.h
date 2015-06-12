#ifndef __XLOGGER_H__
#define __XLOGGER_H__

/*
 * @file   tthread/log.h
 * @brief  Logger for memory access events to build a data graph
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "debug.h"
#include "real.h"
#include "tthread/logentry.h"
#include "xatomic.h"
#include "xdefines.h"
#include "xthread.h"

class xlogger {
private:

  /* point to multi-process shared memory */

  // next free place in log
  volatile unsigned long *_next;

  // allocated file size
  volatile size_t *_fileSize;

  /* per-process private data*/

  // make expanding the log file concurrency-safe
  pthread_mutex_t _truncateMutex;
  pthread_mutexattr_t _truncateMutexattr;

  xthread *_thread;

  int _logFd;

  // begin of mmap
  tthread::logentry *_log;

  off_t _mmapOffset;

public:

  enum {
    REQUEST_SIZE = 4096 * 16
  };

  // struct xlogger_shared_data must be located in cross-process memory
  xlogger(xlogger_shared_data& data) :
    _next(&data.next),
    _mmapOffset(-REQUEST_SIZE),
    _fileSize(&data.fileSize),
    _thread(NULL)
  {
    char _name[L_tmpnam];

    sprintf(_name, "tthreadLXXXXXX");
    _logFd = mkstemp(_name);

    if (_logFd < -1) {
      fprintf(stderr, "tthread::log: failed to open log file: %s\n",
              strerror(errno));
      ::abort();
    }
    unlink(_name);

    if (WRAP(pthread_mutexattr_init)(&_truncateMutexattr) != 0) {
      fprintf(stderr, "tthread::log: failed initialize mutexatt: %s\n",
              strerror(errno));
      ::abort();
    }

    pthread_mutexattr_setpshared(&_truncateMutexattr, PTHREAD_PROCESS_SHARED);

    if (WRAP(pthread_mutex_init)(&_truncateMutex, &_truncateMutexattr) != 0) {
      fprintf(stderr, "tthread::log: failed initialize mutex: %s\n",
              strerror(errno));
      ::abort();
    }

    *_fileSize = 0;
    *_next = 0;

    growLog();
  }

  // number of bytes written to log
  off_t getLogSize() {
    return *_next * sizeof(tthread::logentry);
  }

  int getLogFd() {
    return _logFd;
  }

  void setThread(xthread *thread) {
    _thread = thread;
  }

  void add(tthread::logentry e);

private:

  void growLog() {
    size_t currentSize = *_fileSize;

    // someone else has resized the log -> then just mmap to new size
    if ((_mmapOffset + REQUEST_SIZE) >= currentSize) {
      WRAP(pthread_mutex_lock)(&_truncateMutex);

      // test if someone else has truncated the log, while we try to access it
      if (currentSize == *_fileSize) {
        int newSize = currentSize + REQUEST_SIZE;

        if (ftruncate(_logFd, newSize) != 0) {
          fprintf(stderr,
                  "tthread::log: failed to increase log size: %s\n",
                  strerror(errno));
          ::abort();
        }
        *_fileSize = newSize;
      }

      WRAP(pthread_mutex_unlock)(&_truncateMutex);
    }

    // free old mapping, if set
    if (_log != NULL) {
      munmap(_log, REQUEST_SIZE);
    }

    _mmapOffset += REQUEST_SIZE;
    _log = (tthread::logentry *)mmap(NULL,
                                     REQUEST_SIZE,
                                     PROT_WRITE,
                                     MAP_SHARED,
                                     _logFd,
                                     _mmapOffset);


    if (_log == MAP_FAILED) {
      fprintf(stderr, "tthread::log: mmap error with %s\n", strerror(errno));
      ::abort();
    }
  }
};

#endif /* __XLOGGER_H__ */
