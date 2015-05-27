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

  // page within the memory location
  int pageNo;

  // process id, which accessed the memory
  int threadId;

  // synchronisation point within on thread
  int thunkId;

  // return address, where the junk starts
  // the end, is the next thunk with id + 1
  const void *thunkStart;

  // return address, which issued the first page fault on this page
  const void *firstIssuerAddress;

  // how memory was access (read/write)
  accessType access;

public:

  xpagelogentry(int pageNo, accessType access, const void *issuerAddress)
    : pageNo(pageNo),
    access(access),
    threadId(xthread::getId()),
    thunkId(xthread::getThunkId()),
    thunkStart(xthread::getThunkStart()),
    firstIssuerAddress(issuerAddress)
  {}

  inline int getPageNo() {
    return pageNo;
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

  inline const void *getFirstIssuerAddress() {
    return firstIssuerAddress;
  }
};

#endif /* __XPAGELOGENTRY_H__ */
