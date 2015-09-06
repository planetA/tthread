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
#include "tthread/logevent.h"
#include "tthread/logheader.h"
#include "xatomic.h"
#include "xdefines.h"
#include "xthread.h"

#define LOG_FD_ENV "TTHREAD_LOG_FD"

class xlogger {
private:

  /*** point to multi-process shared memory ***/

  // next free place in log
  volatile unsigned long *_next;

  // allocated file size
  volatile off_t *_fileSize;

  /* per-process private data*/

  // make expanding the log file concurrency-safe
  pthread_mutex_t *_truncateMutex;
  pthread_mutexattr_t _truncateMutexattr;


  /*** process local ***/

  xthread *_thread;

  int _logFd;

  // begin of mmap
  tthread::logevent *_log;

  off_t _mmapOffset;

public:

  enum {
    REQUEST_SIZE = 4096 * 16,
    HEADER_SIZE = PAGE_ALIGN_UP(sizeof(tthread::logheader)),
    EVENT_SIZE = sizeof(tthread::logevent)
  };

  // struct xlogger_shared_data must be located in cross-process memory
  xlogger(xlogger_shared_data   & data,
          tthread::memorylayout_t memoryLayout) :
    _fileSize(&data.fileSize),
    _truncateMutex(&data.truncateMutex),
    _thread(NULL),
    _mmapOffset(-REQUEST_SIZE)
  {
    assert(_fileSize);
    *_fileSize = 0;

    _logFd = openLog();

    if (WRAP(pthread_mutexattr_init)(&_truncateMutexattr) != 0) {
      fprintf(stderr, "tthread::log: failed initialize mutexattr: %s\n",
              strerror(errno));
      ::abort();
    }

    pthread_mutexattr_setpshared(&_truncateMutexattr, PTHREAD_PROCESS_SHARED);

    if (WRAP(pthread_mutex_init)(_truncateMutex, &_truncateMutexattr) != 0) {
      fprintf(stderr, "tthread::log: failed initialize mutex: %s\n",
              strerror(errno));
      ::abort();
    }

    tthread::logheader *header = allocateHeader(memoryLayout);
    _next = header->getEventCount();
    *_next = 0;
    growLog();
  }

  int openLog() {
    const char *fdStr = getenv(LOG_FD_ENV);

    if (fdStr) {
      unsetenv(LOG_FD_ENV);
      int fd = (int)atol(fdStr);
      int res = fcntl(fd, F_GETFD);

      if (res >= 0) {
        return fd;
      }
      DEBUGF("not a valid file descriptor was passed via %s: %s: %s",
             LOG_FD_ENV,
             fdStr,
             strerror(errno));
    }

    // Fallback to private temporary file in case no valid environment variable
    // is set
    char _name[L_tmpnam];
    sprintf(_name, "tthreadLXXXXXX");
    int fd = mkstemp(_name);

    if (fd < -1) {
      fprintf(stderr, "tthread::log: failed to open log file: %s\n",
              strerror(errno));
      ::abort();
    }
    unlink(_name);
    return fd;
  }

  static xlogger *allocate(xlogger_shared_data   & data,
                           tthread::memorylayout_t layout) {
    void *buf = WRAP(mmap)(NULL, sizeof(xlogger), PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANON, -1, 0);

    assert(buf != MAP_FAILED);
    return new(buf)xlogger(data, layout);
  }

  int getLogFd() {
    return _logFd;
  }

  void setThread(xthread *thread) {
    _thread = thread;
  }

  void add(tthread::logevent e);

private:

  tthread::logheader *allocateHeader(tthread::memorylayout_t layout) {
    if (ftruncate(_logFd, HEADER_SIZE) != 0) {
      fprintf(stderr,
              "tthread::log: failed to increase log size: ftruncate(%d, %u) %s\n",
              _logFd,
              HEADER_SIZE,
              strerror(errno));
      ::abort();
    }

    void *buf = WRAP(mmap)(NULL,
                           HEADER_SIZE,
                           PROT_WRITE,
                           MAP_SHARED,
                           _logFd,
                           0);

    if (buf == MAP_FAILED) {
      fprintf(stderr, "tthread::log: mmap error with %s\n", strerror(errno));
      ::abort();
    }
    return new(buf)tthread::logheader(layout);
  }

  void growLog() {
    off_t currentSize = *_fileSize;

    // check if someone else has resized the log -> then just mmap to new size
    if ((_mmapOffset + REQUEST_SIZE) >= currentSize) {
      WRAP(pthread_mutex_lock)(_truncateMutex);

      // test if someone else has truncated the log, while we try to get lock
      if (currentSize == *_fileSize) {
        off_t newSize = currentSize + REQUEST_SIZE;

        if (ftruncate(_logFd, newSize + HEADER_SIZE) != 0) {
          fprintf(stderr,
                  "tthread::log: failed to increase log size: ftruncate(%d, %lu) %s\n",
                  _logFd,
                  newSize,
                  strerror(errno));
          ::abort();
        }
        *_fileSize = newSize;
      }

      WRAP(pthread_mutex_unlock)(_truncateMutex);
    }

    // free old mapping, if set
    if (_log != NULL) {
      munmap(_log, REQUEST_SIZE);
    }

    _mmapOffset = *_fileSize - REQUEST_SIZE;

    char *buf = (char *)WRAP(mmap)(NULL,
                                   REQUEST_SIZE,
                                   PROT_WRITE,
                                   MAP_SHARED,
                                   _logFd,
                                   _mmapOffset + HEADER_SIZE);

    if (buf == MAP_FAILED) {
      fprintf(stderr, "xlogger: mmap error with %s\n", strerror(errno));
      ::abort();
    }
    _log = (tthread::logevent *)buf;
  }
};

#endif /* __XLOGGER_H__ */
