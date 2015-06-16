#include <stddef.h>

#include "tthread/logentry.h"
#include "xatomic.h"
#include "xlogger.h"

void xlogger::add(tthread::logentry e) {
  // FIXME later on, _thread should be logged
  // as an seperate event
  if (_thread != NULL) {
    e.setThread(*_thread);
  }

  unsigned long next = xatomic::increment_and_return(_next, 1);
  unsigned long required_size =
    (next + 1) * sizeof(tthread::logentry) - _mmapOffset;

  if (required_size > REQUEST_SIZE) {
    growLog();
  }

  // substract file offset from _next
  char *byte_offset = ((char *)(_log + next)) - _mmapOffset;
  *((tthread::logentry *)byte_offset)  = e;
}
