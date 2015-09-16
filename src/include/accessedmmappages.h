#include <sys/mman.h>
#include <vector>

#include "heaplayers/stlallocator.h"
#include "privateheap.h"
#include "xdefines.h"

class accessedmmappages {
  typedef HL::STLAllocator<void *, privateheap>privateAllocator;
  typedef std::vector<void *, privateAllocator>pages;

  pages _pages;

public:

  void add(void *addr) {
    // TODO: sorted insert and mprotect over continous memory to reduce syscalls
    _pages.push_back(addr);
  }

  void reset() {
    for (pages::const_iterator it = _pages.begin(); it != _pages.end(); ++it) {
      mprotect(*it, xdefines::PageSize, PROT_NONE);
    }
    _pages.clear();
  }
};
