#include "debug.h"
#include "string.h"
#include "xlogger.h"

void xlogger::add(tthread::logentry e) {
  // FIXME later on, _thread should be logged
  // as an seperate event
  if (_thread != NULL) {
    e.setThread(*_thread);
  }

  unsigned long next = xatomic::increment_and_return(_next, 1);

  if (next * sizeof(tthread::logentry) - _mmapOffset > REQUEST_SIZE) {
    growLog();
  }

  // substract file offset from _next
  char *byte_offset = ((char *)(_log + next)) - _mmapOffset;
  *((tthread::logentry *)byte_offset)  = e;
}
