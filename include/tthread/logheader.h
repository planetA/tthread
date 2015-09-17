#ifndef __TTHREAD_LOG_HEADER_H__
#define __TTHREAD_LOG_HEADER_H__

#include "visibility.h"
#include <stdint.h>

namespace tthread {
#pragma pack(push, 1)
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
    FILE_MAGIC = 0xC3D2C3D2,
    HEADER_SIZE = 4096,
    VERSION = 1
  };

private:

  // Used to identify log file type
  uint32_t _fileMagic;

  // Log format version
  uint32_t _version;

  // Header size in bytes
  uint64_t _headerSize;

  // Number of entries written to the log,
  // the file size might be greater.
  volatile uint64_t _eventCount;

  memorylayout_t _memoryLayout;

public:

  // Set a new file header on a buffer
  // usage:
  // void *buf = mmap(...);
  // new(buf)tthread::logheader(globalStart, globalEnd, heapStart, heapEnd)
  // assert(((unsigned long*) buf)[0] == tthread::logheader::FILE_MAGIC)
  logheader(memorylayout_t memoryLayout) :
    _fileMagic(FILE_MAGIC),
    _version(VERSION),
    _headerSize(HEADER_SIZE),
    _eventCount(0),
    _memoryLayout(memoryLayout)
  {}

  inline bool validFileMagick() {
    return _fileMagic == FILE_MAGIC;
  }

  inline memorylayout_t getMemoryLayout() {
    return _memoryLayout;
  }

  inline volatile uint64_t *getEventCount() {
    return &_eventCount;
  }

  inline uint32_t getVersion() {
    return _version;
  }
};
#pragma pack(pop)
}

#endif /* __TTHREAD_LOG_HEADER_H__ */
