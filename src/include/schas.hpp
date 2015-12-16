#pragma once

#include <iostream>

namespace schas
{

struct MemoryRange
{
  off_t start;
  off_t end;
  int cpu;
  int flags;

  MemoryRange() {};

  MemoryRange(off_t start, off_t end, int cpu, int flags)
    : start(start), end(end), cpu(cpu), flags(flags)
  {
  }

  friend std::ostream& operator<< (std::ostream &out, MemoryRange &range)
  {
    // Since operator<< is a friend of the Point class, we can access
    // Point's members directly.
    out << "[" << range.start << ", " << range.end << "] "
        << range.cpu << "  " << range.flags;
    return out;
  }
} __attribute__((packed));

struct MemoryHeader
{
  int thread;
  int thunks;
  int space;
} __attribute__((packed));

struct MemoryThunk
{
  int id;
  int ranges;
} __attribute__((packed));

enum PAGE_FLAGS {
  READ  = 0x1,
  WRITE = 0x2,
};

}
