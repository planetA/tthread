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

  int pageNo;
  int threadIndex;
  int thunkId;
  accessType access;

public:

  xpagelogentry(int pageNo, accessType access)
    : pageNo(pageNo),
    access(access)
  {
    threadIndex = xthread::getId();
    thunkId = xthread::getThunkId();
  }

  inline int getPageNo() {
    return pageNo;
  }

  inline int getThreadIndex() {
    return threadIndex;
  }

  inline int getThunkId() {
    return thunkId;
  }

  inline int getAccess() {
    return access;
  }
};

#endif /* __XPAGELOGENTRY_H__ */
