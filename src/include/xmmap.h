#include <algorithm>
#include <vector>

#include "heaplayers/stlallocator.h"
#include "mmappage.h"

class xmmap {
private:

  // state for mmap allocations are not shared amongst processes
  // because mmap is also process-local
  typedef HL::STLAllocator<mmappage, privateheap>privateAllocator;
  typedef std::vector<mmappage, privateAllocator>pages;

  pages _pages;
  xlogger *_logger;

  void add(char *start, size_t length, int fd, int prot) {
    pages::const_iterator it = std::lower_bound(_pages.begin(),
                                                _pages.end(),
                                                mmappage((void *)start));

    char *end = getEnd(start, length);
    int numberOfPages = (end - start) / xdefines::PageSize;
    pages temp;

    temp.reserve(numberOfPages);

    for (int i = 0; i < numberOfPages; i++) {
      temp.push_back(mmappage(start + i * xdefines::PageSize, fd, prot));
    }
    auto b = std::make_move_iterator(temp.begin());
    auto e = std::make_move_iterator(temp.end());
    _pages.insert(it, b, e);
  }

  void remove(char *addr, size_t length) {
    pages::iterator lower = std::lower_bound(_pages.begin(),
                                             _pages.end(),
                                             mmappage(addr));


    if ((lower != _pages.end())
        && (lower->addr() <= addr)) {
      char *end = getEnd(addr, length);
      pages::iterator upper = std::lower_bound(std::next(lower),
                                               _pages.end(),
                                               mmappage(end));
      _pages.erase(lower, upper);
    }
  }

  char *getEnd(char *start, size_t length) {
    return (char *)PAGE_ALIGN_UP(((char *)start) + length);
  }

public:

  void initialize(xlogger& logger) {
    _logger = &logger;
  }

  const mmappage *find(void *ptr) {
    void *start = (void *)PAGE_ALIGN_DOWN(ptr);
    pages::const_iterator it = std::lower_bound(_pages.begin(),
                                                _pages.end(),
                                                mmappage(start));

    if ((it != _pages.end())
        && (it->addr() <= ptr)) {
      return &*it;
    } else {
      return NULL;
    }
  }

  void *map(void *addr, size_t length, int prot, int flags, int fd,
            off_t offset) {
    int newflags = flags;

    if (flags & MAP_PRIVATE) {
      newflags = (flags & ~MAP_PRIVATE) | MAP_SHARED;
      DEBUGF("flags %x and newflags %x\n", flags, newflags);
    }

    void *res = WRAP(mmap)(addr, length, PROT_NONE, newflags, fd, offset);

    if (res != MAP_FAILED) {
      add((char *)res, length, fd, prot);
    }

    return res;
  }

  int unmap(void *addr, size_t len) {
    int res = WRAP(munmap)(addr, len);

    if (res == 0) {
      remove((char *)addr, len);
    }
    return res;
  }

#if 0
  void *remap(void   *old_address,
              size_t old_size,
              size_t new_size,
              int    flags,
              ...) {
    // void *res = WRAP(remap)(addr, length, PROT_NONE, newflags, fd, offset);
    void *res =
      WRAP(remap)(addr, old_size, new_size, flags, PROT_NONE, newflags, fd,
                  offset);

    if (res == MAP_FAILED) {
      remove((char *)addr, old_size)

      return res;
    }
  }

#endif // if 0
};
