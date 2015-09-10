#include <stdlib.h>
#include <unistd.h>

#include "minunit.h"
#include "tthread/log.h"
#include "tthread/logevent.h"

enum {
  PAGE_SIZE = 4096
};

MU_TEST(test_read_after_read) {
  // span over 3 pages, so we can trigger an access
  // explicitly leak memory, so we get fresh pages every time
  char *heapbuf = (char *)malloc(PAGE_SIZE * 3);
  tthread::log log;

  char c = heapbuf[PAGE_SIZE];
  tthread::log log2(log.end());

  mu_check(c == 0);

  log2.print();

  mu_check(log2.length() == 1);
  mu_check(log2.get(0).getType() == tthread::logevent::READ);
  tthread::EventData data = log2.get(0).getData();
  mu_check(log2.get(0).getThreadId() == getpid());
  mu_check(data.memory.address > 0);

  // access the same page again
  char c2 = heapbuf[PAGE_SIZE + 1];
  tthread::log log3(log2.end());
  mu_check(log3.length() == 0);
  mu_check(c2 == 0);

  // access the next page again
  char c3 = heapbuf[PAGE_SIZE * 2];
  tthread::log log4(log3.end());
  mu_check(log4.length() == 1);
  mu_check(c3 == 0);
}

MU_TEST(test_write_after_write) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 3);

  tthread::log log;

  heapbuf[PAGE_SIZE] = 1;
  tthread::log log2(log.end());
  mu_check(log2.length() == 1);
  mu_check(log2.get(0).getType() == tthread::logevent::WRITE);

  heapbuf[PAGE_SIZE + 2] = 1;
  tthread::log log3(log2.end());
  mu_check(log3.length() == 0);

  heapbuf[PAGE_SIZE * 2] = 1;
  tthread::log log4(log3.end());
  mu_check(log4.length() == 1);
}

MU_TEST(test_read_after_write) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);

  tthread::log log;

  heapbuf[PAGE_SIZE] = 1;
  tthread::log log2(log.end());
  mu_check(log2.length() == 1);
  mu_check(log2.get(0).getType() == tthread::logevent::WRITE);

  // read after write will not appear in the log,
  // because x86 mmu does not support this kind of protection
  mu_check(heapbuf[PAGE_SIZE] == 1);
  tthread::log log3(log2.end());
  mu_check(log3.length() == 0);
}

MU_TEST(test_write_after_read) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);

  tthread::log log;

  mu_check(heapbuf[PAGE_SIZE] == 0);
  tthread::log log2(log.end());
  mu_check(log2.length() == 1);
  mu_check(log2.get(0).getType() == tthread::logevent::READ);

  heapbuf[PAGE_SIZE] = 1;
  tthread::log log3(log2.end());
  mu_check(log3.length() == 1);
  mu_check(log3.get(0).getType() == tthread::logevent::WRITE);
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
  return minunit_fail;
}
