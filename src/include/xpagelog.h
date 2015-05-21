#ifndef __XPAGELOG_H__
#define __XPAGELOG_H__

/*
 * @file   xpagelog.h
 * @brief  Log page read/write access by threads to build a data graph
 */
#include <algorithm>
#include <stdexcept>

#include "xatomic.h"
#include "xpagelogentry.h"

class xpagelog {
private:

  xpagelogentry *_log;
  volatile long unsigned *_next_entry;

public:

  enum {
    SIZE_LIMIT = 4096
  };

  void initialize(void) {
    // TODO increase memory on demand
    void *buf = mmap(NULL, sizeof(xpagelogentry) * SIZE_LIMIT,
                     PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);


    if (buf == MAP_FAILED) {
      fprintf(stderr, "xpagelog: mmap error with %s\n", strerror(errno));
      ::abort();
    }
    _next_entry = (long unsigned *)buf;
    _log = (xpagelogentry *)(_next_entry + 1);
    *_next_entry = 0;
  }

  static xpagelog& getInstance(void) {
    static xpagelog *xpagelogObject = NULL;

    if (!xpagelogObject) {
      void *buf = mmap(NULL,
                       sizeof(xpagelog),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS,
                       -1,
                       0);
      xpagelogObject = new(buf)xpagelog();
    }
    return *xpagelogObject;
  }

  void add(xpagelogentry e) {
    int page = xatomic::increment_and_return(_next_entry);

    if ((page - 1) > SIZE_LIMIT) {
      fprintf(stderr, "pagelog size limit reached\n");
      ::abort();
    }
    _log[page] = e;
  }

  // for testing purpose only
  void reset() {
    *_next_entry = 0;
  }

  unsigned long len() {
    return *_next_entry;
  }

  // for testing purpose only
  xpagelogentry get(unsigned long i) {
    if ((i >= SIZE_LIMIT)
        || (*_next_entry == 0)) {
      throw std::out_of_range("not in range of log");
    }
    return _log[i];
  }

  void print() {
    fprintf(stderr, "______RESULT_______\n");

    long unsigned int llen, size = *_next_entry;

    if (SIZE_LIMIT < size) {
      llen = SIZE_LIMIT;
    } else {
      llen = size;
    }

    for (int i = 0; i < llen; i++) {
      xpagelogentry e = _log[i];

      fprintf(stderr, "threadIndex: %d, thunkId: %d, pageNo: %d, access: %s\n",
              e.getThreadIndex(),
              e.getThunkId(),
              e.getPageNo(),
              e.getAccess() == xpagelogentry::READ ? "read" : "write");
    }
  }
};

#endif /* __XPAGELOG_H__ */
