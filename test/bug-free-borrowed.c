#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "minunit.h"

pthread_barrier_t malloc_barrier;
pthread_barrier_t free_barrier;

void *failed = (void *)1;

void *malloc_buf(void *arg_) {
  void **arg = (void **)arg_;
  void *temp = malloc(getpagesize());

  *arg = malloc(getpagesize() / 3);

  pthread_barrier_wait(&malloc_barrier);

  free(temp);
  pthread_barrier_wait(&free_barrier);
  return NULL;
}

void *free_borrowed(void *arg_) {
  void **arg = (void **)arg_;

  free(*arg);
  pthread_barrier_wait(&free_barrier);
  return NULL;
}

MU_TEST(test_free_borrowed) {
  void *buf;
  void *arg = &buf;
  pthread_t malloc_thread, free_thread;

  mu_check(pthread_barrier_init(&malloc_barrier, NULL, 2) == 0);
  mu_check(pthread_barrier_init(&free_barrier, NULL, 2) == 0);

  pthread_create(&malloc_thread, 0, malloc_buf, buf);
  pthread_barrier_wait(&malloc_barrier);
  mu_check(buf != NULL);

  pthread_create(&free_thread, 0, free_borrowed, buf);

  void *status = 0;
  pthread_join(malloc_thread, &status);
  mu_check(status == NULL);
  pthread_join(free_thread, &status);
  mu_check(status == NULL);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_free_borrowed);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return minunit_fail;
}
