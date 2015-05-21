// #include "xpagelog.h"
#include <assert.h>
#include <iostream>
#include <pthread.h>

#include "minunit.h"
#include "xpagelog.h"

enum {
  PAGE_SIZE = 4096,
};

MU_TEST(test_read_access) {
  // span over 2 pages, so we can trigger an access
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);
  xpagelog log = xpagelog::getInstance();

  log.reset();
  char c = heapbuf[PAGE_SIZE];
  assert(log.len() == 1);

  xpagelogentry e = log.get(0);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_read_access);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return 0;
}
