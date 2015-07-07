#include <stdio.h>
#include <unistd.h>

#include "minunit.h"
#include "tthread/log.h"
#include "tthread/logentry.h"
#include "xdefines.h"
#include "xlogger.h"
#include <stdlib.h>

const int PAGE_SIZE = 4096;

// FIXME: should be the same as in xlogger
const int REQUEST_SIZE = PAGE_SIZE * 16;

MU_TEST(test_grow_log) {
  // force log to expand
  unsigned int overflow = (REQUEST_SIZE / sizeof(tthread::logentry) + 1);
  char *buf = (char *)malloc(overflow * PAGE_SIZE);

  tthread::log log;

  memset(buf, 1, overflow * PAGE_SIZE);
  tthread::log log2(log.end());

  mu_check(log2.length() == overflow);
  free(buf);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_grow_log);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
