// #include "xpagelog.h"
#include <assert.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "minunit.h"
#include "xpagelog.h"

enum {
  PAGE_SIZE = 4096,
};

MU_TEST(test_read_after_read) {
  // span over 3 pages, so we can trigger an access
  // explicitly leak memory, so we get fresh pages every time
  char *heapbuf = (char *)malloc(PAGE_SIZE * 3);
  xpagelog log = xpagelog::getInstance();

  log.reset();
  char c = heapbuf[PAGE_SIZE];
  mu_check(log.len() == 1);
  mu_check(log.get(0).getAccess() == xpagelogentry::READ);
  mu_check(log.get(0).getThreadId() == getpid());
  mu_check(log.get(0).getPageNo() > 0);

  char c2 = heapbuf[PAGE_SIZE + 1];
  mu_check(log.len() == 1);

  char c3 = heapbuf[PAGE_SIZE * 2];
  mu_check(log.len() == 2);
}

MU_TEST(test_read_after_write) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);
  xpagelog log = xpagelog::getInstance();

  log.reset();
  heapbuf[PAGE_SIZE] = 1;
  mu_check(log.len() == 1);
  mu_check(log.get(0).getAccess() == xpagelogentry::WRITE);

  // read after write will not appear in the log
  mu_check(heapbuf[PAGE_SIZE] == 1);
  mu_check(log.len() == 1);
}

MU_TEST(test_write_after_read) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);
  xpagelog log = xpagelog::getInstance();
}

MU_TEST(test_write_after_write) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 3);
  xpagelog log = xpagelog::getInstance();

  log.reset();
  heapbuf[PAGE_SIZE] = 1;
  mu_check(log.len() == 1);
  mu_check(log.get(0).getAccess() == xpagelogentry::WRITE);

  heapbuf[PAGE_SIZE + 2] = 1;
  mu_check(log.len() == 1);

  heapbuf[PAGE_SIZE * 2] = 1;
  mu_check(log.len() == 2);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_read_after_read);
  MU_RUN_TEST(test_write_after_write);
  MU_RUN_TEST(test_read_after_write);
  MU_RUN_TEST(test_write_after_read);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return 0;
}
