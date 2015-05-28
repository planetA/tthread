#ifndef __XPAGELOGENTRY_H__
#define __XPAGELOGENTRY_H__

#include "xthread.h"

class xpagelogentry {
public:

  enum accessType {
    WRITE,
    READ,
  };

private:

  // memory address at which the first page fault was triggered
  const void *firstAccessedAddress;

  // page within the memory location
  void *pageStart;

  // how memory was access (read/write)
  accessType access;

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

  xpagelogentry(const void *address,
                void       *pageStart,
                accessType access,
                const void *issuerAddress)
    : firstAccessedAddress(address),
    pageStart(pageStart),
    access(access),
    threadId(xthread::getId()),
    thunkId(xthread::getThunkId()),
    thunkStart(xthread::getThunkStart()),
    firstIssuerAddress(issuerAddress)
  {}

  inline void *getPageStart() {
    return pageStart;
  }

  inline int getThreadId() {
    return threadId;
  }

  inline int getThunkId() {
    return thunkId;
  }

  inline const void *getThunkStart() {
    return thunkStart;
  }

  inline int getAccess() {
    return access;
  }

  inline const void *getFirstAccessedAddress() {
    return firstAccessedAddress;
  }

  inline const void *getFirstIssuerAddress() {
    return firstIssuerAddress;
  }
};

#endif /* __XPAGELOGENTRY_H__ */
