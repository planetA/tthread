#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "minunit.h"
#include "tthread/tthread.h"

enum {
  PAGE_SIZE = 4096,
};

MU_TEST(test_read_after_read) {
  // span over 3 pages, so we can trigger an access
  // explicitly leak memory, so we get fresh pages every time
  char *heapbuf = (char *)malloc(PAGE_SIZE * 3);
  tthread::log& log = tthread::getLog();

  log.reset();
  char c = heapbuf[PAGE_SIZE];
  mu_check(log.len() == 1);
  mu_check(log.get(0).getAccess() == tthread::logentry::READ);
  mu_check(log.get(0).getThreadId() == getpid());
  mu_check(log.get(0).getPageStart() > 0);

  // access the same page again
  char c2 = heapbuf[PAGE_SIZE + 1];
  mu_check(log.len() == 1);

  // access the next page again
  char c3 = heapbuf[PAGE_SIZE * 2];
  mu_check(log.len() == 2);
}

MU_TEST(test_write_after_write) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 3);
  tthread::log& log = tthread::getLog();

  log.reset();
  heapbuf[PAGE_SIZE] = 1;
  mu_check(log.len() == 1);
  mu_check(log.get(0).getAccess() == tthread::logentry::WRITE);

  heapbuf[PAGE_SIZE + 2] = 1;
  mu_check(log.len() == 1);

  heapbuf[PAGE_SIZE * 2] = 1;
  mu_check(log.len() == 2);
}

MU_TEST(test_read_after_write) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);
  tthread::log& log = tthread::getLog();

  log.reset();
  heapbuf[PAGE_SIZE] = 1;
  mu_check(log.len() == 1);
  mu_check(log.get(0).getAccess() == tthread::logentry::WRITE);

  // read after write will not appear in the log,
  // because x86 mmu does not support this kind of protection
  mu_check(heapbuf[PAGE_SIZE] == 1);
  mu_check(log.len() == 1);
}

MU_TEST(test_write_after_read) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);
  tthread::log& log = tthread::getLog();

  log.reset();
  mu_check(heapbuf[PAGE_SIZE] == 0);
  mu_check(log.len() == 1);
  mu_check(log.get(0).getAccess() == tthread::logentry::READ);

  heapbuf[PAGE_SIZE] = 1;
  mu_check(log.len() == 2);
  mu_check(log.get(1).getAccess() == tthread::logentry::WRITE);
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
