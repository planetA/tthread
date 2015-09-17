#include <pthread.h>
#include <sys/mman.h>

#include "minunit.h"
#include "tthread/log.h"

MU_TEST(test_mmap) {
  const size_t size = sizeof(char) * getpagesize() * 2;
  char *buf = (char *)mmap(NULL,
                           size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANON,
                           -1,
                           0);

  mu_check(buf != MAP_FAILED);
  tthread::log log;
  buf[0] = 1;
  mu_check(buf[0] == 1);
  mu_check(buf[getpagesize()] == 0);
  tthread::log log2(log.end());

  mu_check(log2.length() == 2);
  mu_check(log2.get(0).getType() == tthread::logevent::WRITE);
  mu_check(log2.get(1).getType() == tthread::logevent::READ);
  mu_check(munmap(buf, size) == 0);
}

void *access_mmap(void *buf) {
  ((char *)buf)[1] = 2;
  return NULL;
}

MU_TEST(test_reset_mmap_access) {
  const size_t size = sizeof(char) * getpagesize();
  char *buf = (char *)mmap(NULL,
                           size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANON,
                           -1,
                           0);

  mu_check(buf != MAP_FAILED);
  tthread::log log;
  buf[0] = 1;

  tthread::log log2(log.end());
  mu_check(log2.length() == 1);

  pthread_t thread;
  pthread_create(&thread, NULL, access_mmap, buf);
  pthread_join(thread, NULL);
  tthread::log log3(log2.end());
  bool found = false;

  for (unsigned int i = 0; i < log3.length(); i++) {
    auto ev = log3.get(i);
    found |= ev.getThreadId() != (int)pthread_self()
             && ev.getData().memory.address == &buf[1];
  }
  mu_check(found); // other thread has accessed &buf[1]
  tthread::log log4(log3.end());
  buf[0] = 1;
  tthread::log log5(log4.end());
  mu_check(log5.length() == 1);
}

MU_TEST(test_partial_unmap) {
  const size_t page = sizeof(char) * getpagesize();
  char *buf1 = (char *)mmap(NULL,
                            page * 2,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANON,
                            -1,
                            0);
  char *buf2 = &buf1[page];

  mu_check(munmap(buf1, page) == 0);
  buf2[0] = 1;
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_mmap);
  MU_RUN_TEST(test_reset_mmap_access);
  MU_RUN_TEST(test_partial_unmap);
}

int main() {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
