#include "minunit.h"
#include <errno.h>
#include <malloc.h>
#include <stdint.h>

#define MIN_ALIGN_SIZE sizeof(void *) * 2

MU_TEST(test_memalign) {
  uintptr_t p1 = (uintptr_t)memalign(3,      1);

  mu_check(p1 == 0);
  mu_check(errno == EINVAL); // not a power of 2

  uintptr_t p2 = (uintptr_t)memalign(MIN_ALIGN_SIZE << 1, 1);
  mu_check(p2 != 0);
  mu_check(p2 % MIN_ALIGN_SIZE == 0);

  uintptr_t p3 = (uintptr_t)memalign(MIN_ALIGN_SIZE * 4, 1);
  mu_check(p3 != 0);
  mu_check(((uintptr_t)p3) % (MIN_ALIGN_SIZE * 4) == 0);

  free((void *)p3);
  free((void *)p2);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_memalign);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
