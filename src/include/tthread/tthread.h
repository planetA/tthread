#ifndef __TTHREAD_H__
#define __TTHREAD_H__

#include "tthread/log.h"

namespace tthread {
extern tthread::log *_log;
inline log& getLog() {
  return *_log;
}
}

#endif /* __TTHREAD_H__ */
