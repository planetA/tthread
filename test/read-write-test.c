#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "minunit.h"

// object header access first page, so allocate 2 pages
#define BUF_SIZE (getpagesize() * 2)

MU_TEST(test_read) {
  // explicitly leak memory,
  // so page faults on access are produced every time
  void *buf = malloc(BUF_SIZE);

  mu_check(buf != NULL);

  int f = open("/dev/zero", O_RDONLY);
  mu_check(f > 0);

  ssize_t n = read(f, buf, BUF_SIZE);
  mu_check(n == BUF_SIZE);
  mu_check(((char *)buf)[0] == 0
           && ((char *)buf)[BUF_SIZE - 1] == 0);

  close(f);
}

MU_TEST(test_fread) {
  void *buf = malloc(BUF_SIZE);

  mu_check(buf != NULL);

  FILE *f = fopen("/dev/zero", "r");
  mu_check(f != NULL);

  ssize_t n = fread(buf, sizeof(char), BUF_SIZE, f);
  mu_check(n == BUF_SIZE);
  mu_check(((char *)buf)[0] == 0
           && ((char *)buf)[BUF_SIZE - 1] == 0);

  fclose(f);
}

MU_TEST(test_write) {
  void *buf = malloc(BUF_SIZE);

  mu_check(buf != NULL);

  int f = open("/dev/null", O_WRONLY);
  mu_check(f > 0);

  ssize_t n = write(f, buf, BUF_SIZE);
  mu_check(n == BUF_SIZE);

  close(f);
}
MU_TEST(test_fwrite) {
  void *buf = malloc(BUF_SIZE);

  mu_check(buf != NULL);

  FILE *f = fopen("/dev/null", "w");
  mu_check(f != NULL);

  ssize_t n = fwrite(buf, sizeof(char), BUF_SIZE, f);
  mu_check(n == BUF_SIZE);

  close(f);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_read);
  MU_RUN_TEST(test_fread);
  MU_RUN_TEST(test_write);
  MU_RUN_TEST(test_fwrite);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
