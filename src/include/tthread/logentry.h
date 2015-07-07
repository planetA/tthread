#ifndef __TTHREAD_LOG_ENTRY_H__
#define __TTHREAD_LOG_ENTRY_H__

/*
 * @file   tthread/logentry.h
 * @brief  a memory event
 */

#include "visibility.h"

class xthread;

namespace tthread {
class _PUBLIC_ logentry {
public:

  enum accessType {
    WRITE = 0,
    READ = 1
  };

private:

  // how memory was access (read/write)
  int access;

  // memory address at which the first page fault was triggered
  const void *firstAccessedAddress;

  // page within the memory location
  void *pageStart;

  // process id, which accessed the memory
  int threadId;

  // synchronisation point within on thread
  int thunkId;

  // return address, where the junk starts
  // the end, is the next thunk with id + 1
  const void *thunkStart;

  // return address, which issued the first page fault on this page
  const void *firstIssuerAddress;

public:

  logentry(const void *address,
           void       *pageStart,
           accessType access,
           const void *issuerAddress)
    : access(access),
    firstAccessedAddress(address),
    pageStart(pageStart),
    firstIssuerAddress(issuerAddress)
  {}

  void setThread(xthread thread);

  inline void *getPageStart() const {
    return pageStart;
  }

  inline int getThreadId() const {
    return threadId;
  }

  inline int getThunkId() const {
    return thunkId;
  }

  inline const void *getThunkStart() const {
    return thunkStart;
  }

  inline accessType getAccess() const {
    return (accessType)access;
  }

  inline const void *getFirstAccessedAddress() const {
    return firstAccessedAddress;
  }

  inline const void *getFirstIssuerAddress() const {
    return firstIssuerAddress;
  }
};
}

#endif /* __TTHREAD_LOG_ENTRY_H__ */
