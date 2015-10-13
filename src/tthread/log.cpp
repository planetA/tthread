#include "tthread/log.h"
#include <assert.h>
#include <cstddef>
#include <errno.h>
#include <execinfo.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "xglobals.h"
#include "xlogger.h"

namespace tthread {
extern xlogger *logger;

log::log() :
  _logOffset(0),
  _log(NULL)
{
  openLog(tthread::logger->getLogFd());
}

// all events since offset
log::log(off_t offset) :
  _logOffset(offset),
  _log(NULL)
{
  ASSERT(offset > 0);
  openLog(tthread::logger->getLogFd());
}

log::log(int logFd) :
  _logOffset(0),
  _log(NULL)
{
  openLog(logFd);
}

log::log(int logFd, off_t offset) :
  _logOffset(offset),
  _log(NULL)
{
  ASSERT(offset > 0);
  openLog(logFd);
}

log::~log() {
  munmap(_log, _logSize);
}

size_t log::length() const {
  return (_logSize - _logOffset) / sizeof(logevent);
}

logevent log::get(unsigned long i) {
  if (i > length()) {
    throw std::out_of_range("not in range of log");
  }
  return _log[i];
}

void log::print() const {
  fprintf(stderr, "______Page Access Result_______\n");

  unsigned int llen = length();

  for (unsigned long i = 0; i < llen; i++) {
    logevent e = _log[i];

    const tthread::EventData data = e.getData();

    switch (e.getType()) {
    case tthread::logevent::READ:
    case tthread::logevent::WRITE:
    {
      const char *access = e.getType() == logevent::READ ? "read" : "write";
      fprintf(stderr,
              "[%s] threadId: %d, address: %p, pageStart: %p, issued at: ",
              access,
              e.getThreadId(),
              data.memory.address,
              ((void *)PAGE_ALIGN_DOWN(
                 data.memory.address)));

      // hacky solution to print backtrace without using malloc
      void *issuerAddress = (void *)e.getReturnAddress();
      backtrace_symbols_fd(&issuerAddress, 1, fileno(stderr));
      break;
    }

    case tthread::logevent::THUNK:
    {
      fprintf(stderr,
              "[thunk] threadId: %d, id: %d, issued at: ",
              e.getThreadId(),
              data.thunk.id);

      void *thunkStart = (void *)e.getReturnAddress();
      backtrace_symbols_fd(&thunkStart, 1, fileno(stderr));
      break;
    }

    case tthread::logevent::INVALID:
      fprintf(stderr, "[invalid entry]\n");
      break;

    case tthread::logevent::FINISH:
      fprintf(stderr, "[finish] child %d finished\n", e.getThreadId());

    default:

      // invalid state
      ASSERT(1);
    }
  }
}

logheader *log::readHeader(int logFd) {
  char *buf = (char *)WRAP(mmap)(NULL,
                                 xlogger::HEADER_SIZE,
                                 PROT_READ,
                                 MAP_SHARED,
                                 logFd,
                                 0);

  if (buf == MAP_FAILED) {
    fprintf(stderr, "failed to read header: %s", strerror(errno));
    ::abort();
  }

  return (tthread::logheader *)buf;
}

void log::openLog(int logFd) {
  assert(logFile >= 0);
  _header = log::readHeader(logFd);
  assert(_header->checkFileMagick());
  _logSize = *_header->getEventCount() * xlogger::EVENT_SIZE;

  if (_logSize == 0) {
    // zero entries logged yet, skip opening the log -> _log == NULL
    return;
  }

  size_t alignedOffset = PAGE_ALIGN_DOWN(_logOffset);
  size_t diff = _logOffset - alignedOffset;
  size_t alignedSize = _logSize + diff;
  char *buf = (char *)WRAP(mmap)(NULL,
                                 alignedSize,
                                 PROT_READ,
                                 MAP_SHARED,
                                 logFd,
                                 alignedOffset + xlogger::HEADER_SIZE);

  if (buf == MAP_FAILED) {
    fprintf(stderr, "tthread::log: mmap error: %s\n", strerror(errno));
    ::abort();
  }
  _log = (logevent *)(buf + diff);
}
}
