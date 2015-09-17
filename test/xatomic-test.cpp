#include "minunit.h"
#include "xatomic.h"

MU_TEST(test_increment_and_return) {
  unsigned long i = 0;
  unsigned long result = xatomic::increment_and_return(&i);
  unsigned long result2 = xatomic::increment_and_return(&i);

  mu_check(result == 0);
  mu_check(result2 == 1);
  mu_check(i == 2);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_increment_and_return);
}

int main() {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
