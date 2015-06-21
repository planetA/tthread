#ifndef __XPAGEINFO_H__
#define __XPAGEINFO_H__

struct xpageinfo {
  int pageNo;

  unsigned int version;

  // Used to save start address for this page.
  void *pageStart;
  bool isShared;
  bool release;
};

#endif /* __XPAGEINFO_H__ */
