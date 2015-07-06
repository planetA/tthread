#include "tthread/logentry.h"
#include "xthread.h"

extern char _start;

namespace tthread {
void logentry::setThread(xthread thread) {
  threadId = thread.getId();
  thunkId = thread.getThunkId();
  thunkStart = thread.getThunkStart();

  if (thunkStart == NULL) {
    thunkStart = (const void *)&_start;
  }
}
}
