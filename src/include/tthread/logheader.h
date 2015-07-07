#ifndef __TTHREAD_LOG_HEADER_H__
#define __TTHREAD_LOG_HEADER_H__

#include "visibility.h"

namespace tthread {
typedef struct {
  // Address where global memory starts
  const void *globalStart;

  // Address where global memory ends
  const void *globalEnd;

  // Address where heap memory starts
  const void *heapStart;

  // Address where heap memory ends
  const void *heapEnd;
} memorylayout_t;

class _PUBLIC_ logheader {
public:

  enum {
    FILE_MAGIC = 0xBADC3D2,
  };

private:

  // Used to identify log file type
  unsigned int _fileMagic;

  memorylayout_t _memoryLayout;

  // Number of entries written to the log,
  // the file size might be greater.
  volatile unsigned long _logEntryCount;

public:

  // Set a new file header on a buffer
  // usage:
  // void *buf = mmap(...);
  // new(buf)tthread::logheader(globalStart, globalEnd, heapStart, heapEnd)
  // assert(((unsigned long*) buf)[0] == tthread::logheader::FILE_MAGIC)
  logheader(memorylayout_t memoryLayout)
    : _fileMagic(FILE_MAGIC),
    _memoryLayout(memoryLayout),
    _logEntryCount(0)
  {}

  inline const bool validFileMagick() const {
    return _fileMagic == FILE_MAGIC;
  }

  inline memorylayout_t getMemoryLayout() const {
    return _memoryLayout;
  }

  inline volatile unsigned long *getLogEntryCount() {
    return &_logEntryCount;
  }
};
}

#endif /* __TTHREAD_LOG_HEADER_H__ */
