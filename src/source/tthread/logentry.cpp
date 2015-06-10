#include "tthread/logentry.h"
#include "xthread.h"

namespace tthread {
void logentry::setThread(xthread thread) {
  threadId = thread.getId();
  thunkId = thread.getThunkId();
  thunkStart = thread.getThunkStart();
}
}
