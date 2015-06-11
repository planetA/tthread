#include "minunit.h"
#include <stdlib.h>
#include <string.h>

MU_TEST(test_malloc_free) {
  char *buf = (char *)malloc(sizeof(char) * 96);

  memset(buf, 'c', sizeof(char) * 96);

  mu_check(*buf == 'c');

  char *buf2 = (char *)malloc(sizeof(char) * 96);

  free(buf);
  free(buf2);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_malloc_free);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
