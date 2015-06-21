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
  _logSize(tthread::logger->getLogSize()),
  _log(NULL)
{
  openLog(tthread::logger->getLogFd());
}

// all events since offset
log::log(off_t offset) :
  _logOffset(offset),
  _logSize(tthread::logger->getLogSize()),
  _log(NULL)
{
  openLog(tthread::logger->getLogFd());
}

log::log(int logFd, off_t size, off_t offset) :
  _logOffset(offset),
  _logSize(size),
  _log(NULL)
{
  openLog(tthread::logger->getLogFd());
}

log::~log() {
  munmap(_log, _logSize);
}

size_t log::length() const {
  return (_logSize - _logOffset) / sizeof(logentry);
}

const logentry log::get(unsigned long i) const {
  if (i > length()) {
    throw std::out_of_range("not in range of log");
  }
  return _log[i];
}

void log::print() const {
  fprintf(stderr, "______Page Access Result_______\n");

  unsigned int llen = length();

  for (unsigned long i = 0; i < llen; i++) {
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

void log::openLog(int logFd) {
  assert(logFile >= 0);

  if (_logSize == 0) {
    // zero entries logged yet, skip opening the log -> _log == NULL
    return;
  }

  size_t alignedOffset = PAGE_ALIGN_DOWN(_logOffset);
  size_t diff = _logOffset - alignedOffset;
  size_t alignedSize = _logSize + diff;
  char *buf = (char *)mmap(NULL,
                           alignedSize,
                           PROT_READ,
                           MAP_SHARED,
                           logFd,
                           alignedOffset);

  if (buf == MAP_FAILED) {
    fprintf(stderr, "tthread::log: mmap error: %s\n", strerror(errno));
    ::abort();
  }
  _log = (logentry *)(buf + diff);
}
}
