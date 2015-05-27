#include "minunit.h"
#include "xpagelog.h"
#include <execinfo.h>

enum {
  PAGE_SIZE = 4096,
};


pthread_mutex_t mutex;

void *access_mutex(void *data) {
  char *buf = (char *)data;
  xpagelog log = xpagelog::getInstance();

  pthread_mutex_lock(&mutex);
  buf[PAGE_SIZE] = 'a';
  pthread_mutex_unlock(&mutex);
  buf[PAGE_SIZE] = 'b';

  return NULL;
}

MU_TEST(test_after_mutex) {
  char *heapbuf = (char *)malloc(PAGE_SIZE * 2);
  xpagelog log = xpagelog::getInstance();

  pthread_t thread;

  pthread_mutex_init(&mutex, NULL);

  pthread_create(&thread, NULL, access_mutex, &heapbuf);

  pthread_join(thread, NULL);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_after_mutex);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return 0;
}
