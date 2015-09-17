#pragma once

struct xpageinfo {
  int pageNo;

  unsigned int version;

  // Used to save start address for this page.
  void *pageStart;
  bool isShared;
  bool release;
};
