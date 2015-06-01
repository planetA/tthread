#ifndef __TTHREAD_LOG_H__
#define __TTHREAD_LOG_H__

/*
 * @file   tthread/log.h
 * @brief  Log page read/write access by threads to build a data graph
 */
#include <algorithm>
#include <execinfo.h>
#include <stdexcept>

#include "tthread/logentry.h"
#include "xatomic.h"

namespace tthread {
class log {
private:

  logentry *_log;
  volatile long unsigned *_next_entry;
  xthread *_thread;

public:

  enum {
    SIZE_LIMIT = 4096
  };

  log() {
    // TODO increase memory on demand
    void *buf = mmap(NULL, sizeof(logentry) * SIZE_LIMIT,
                     PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (buf == MAP_FAILED) {
      fprintf(stderr, "tthread::log: mmap error with %s\n", strerror(errno));
      ::abort();
    }
    _next_entry = (long unsigned *)buf;
    _log = (logentry *)(_next_entry + 1);
    *_next_entry = 0;
  }

  void setThread(xthread& thread) {
    _thread = &thread;
  }

  // allocate shared log between all threads
  static log *newInstance() {
    void *buf = mmap(NULL,
                     sizeof(tthread::log),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS,
                     -1,
                     0);

    return new(buf)log();
  }

  void add(logentry e) {
    int next = xatomic::increment_and_return(_next_entry);

    // not initialized yet, happens at startup
    assert(_thread != NULL);
    e.setThread(*_thread);

    if ((next - 1) > SIZE_LIMIT) {
      fprintf(stderr, "pagelog size limit reached\n");
      ::abort();
    }
    _log[next] = e;
  }

  // for testing purpose only
  void reset() {
    *_next_entry = 0;
  }

  unsigned long len() {
    return *_next_entry;
  }

  // for testing purpose only
  logentry get(unsigned long i) {
    if ((i >= SIZE_LIMIT)
        || (*_next_entry == 0)) {
      throw std::out_of_range("not in range of log");
    }
    return _log[i];
  }

  void print() {
    fprintf(stderr, "______Page Access Result_______\n");

    long unsigned int llen, size = *_next_entry;

    if (SIZE_LIMIT < size) {
      llen = SIZE_LIMIT;
    } else {
      llen = size;
    }

    for (int i = 0; i < llen; i++) {
      logentry e = _log[i];

      fprintf(stderr,
              "threadIndex: %d, thunkId: %d, address: %p, pageStart: %p, access: %s, issued at: ",
              e.getThreadId(),
              e.getThunkId(),
              e.getFirstAccessedAddress(),
              e.getPageStart(),
              e.getAccess() == logentry::READ ? "read" : "write");

      // hacky solution to print backtrace without using malloc
      void *issuerAddress = const_cast<void *>(e.getFirstIssuerAddress());
      backtrace_symbols_fd(&issuerAddress, 1, fileno(stderr));

      void *thunkStart = const_cast<void *>(e.getThunkStart());
      fprintf(stderr, "\tthunk_start: ");
      backtrace_symbols_fd(&thunkStart, 1, fileno(stderr));
    }
  }
};
}

#endif /* __TTHREAD_LOG_H__ */
