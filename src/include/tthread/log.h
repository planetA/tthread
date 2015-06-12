#ifndef __TTHREAD_LOG_H__
#define __TTHREAD_LOG_H__

/*
 * @file   tthread/log.h
 * @brief  Read logged memory events
 */

#include <sys/types.h>

#include "tthread/logentry.h"
#include "visibility.h"


namespace tthread {
class _PUBLIC_ log {
private:

  void openLog(int logFd);

  const off_t _logOffset;
  const size_t _logSize;
  logentry *_log;
  log(const log& l);

public:

  // include all events logged at the time of construction
  log();
  log(off_t offset);
  ~log();

  // open custom log from file handle
  log(int   logFd,
      off_t size,
      off_t offset);

  size_t length() const;

  off_t end() {
    return _logSize;
  }

  const logentry get(unsigned long i) const;
  void print() const;
};
}

#endif /* __TTHREAD_LOG_H__ */
