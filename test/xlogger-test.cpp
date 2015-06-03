#include "minunit.h"
#include "tthread/log.h"
#include "xlogger.h"
#include <unique.h>

MU_TEST(test_grow_log) {
  struct xlogger_shared_data data = { 0, 0 };
  xlogger logger(data);
  tthread::logentry dummy((void *)1,
                          (void *)1,
                          tthread::logentry::READ,
                          (void *)1);

  // force new page
  int overflow = (xlogger::REQUEST_SIZE / sizeof(dummy) + 1);

  for (int i = 0; i < overflow; i++) {
    logger.add(dummy);
  }
  write(fileno(stdout), "foo\n", 4);
  tthread::log log(logger.getLogFd(), logger.getLogSize(), 0);

  mu_check(log.get(0).getAccess() == tthread::logentry::READ);
  mu_check(log.get(0).getFirstAccessedAddress() == (void *)1);
  mu_check(log.length() == overflow);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_grow_log);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return 0;
}
