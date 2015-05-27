#include <pthread.h>
#include <stdlib.h>

#include "minunit.h"
#include "objectheader.h"

enum {
  PAGE_SIZE = 4096,
};

char global;

void *write_from_child(void *data) {
  ((char *)data)[0] = 'b';
  global = 'b';

  return NULL;
}

MU_TEST(test_write_from_child) {
  char *heap = (char *)malloc(4096);

  heap[0] = 'a';
  global = 'a';

  mu_check(heap[0] == 'a');
  mu_check(global == 'a');

  pthread_t thread;

  pthread_create(&thread, NULL, write_from_child, heap);
  pthread_join(thread, NULL);

  mu_check(heap[0] == 'b');
  mu_check(global == 'b');

  fprintf(stderr, "l: %d\n", __LINE__);

  // FIXME Bug, when freeing this buffer
  // free(heap);
}

pthread_mutex_t mutex[2];
void *read_after_lock(void *data) {
  *((char *)data) = 'b';
  fprintf(stderr, "set b\n");
  pthread_mutex_unlock(&mutex[0]); // unlock parent

  pthread_mutex_lock(&mutex[1]);   // wait for parent to check
  return NULL;
}

MU_TEST(test_read_after_lock) {
  char *heap = (char *)malloc(sizeof(char));

  heap[0] = 'a';
  pthread_t thread;
  pthread_mutex_init(&mutex[0], NULL);
  pthread_mutex_init(&mutex[1], NULL);

  pthread_mutex_lock(&mutex[0]);
  pthread_mutex_lock(&mutex[1]);

  pthread_create(&thread, NULL, read_after_lock, heap);

  fprintf(stderr, "before %d\n", __LINE__);
  pthread_mutex_lock(&mutex[0]); // wait for child to unlock
  fprintf(stderr, "after %d\n", __LINE__);

  mu_check(*heap == 'b');

  pthread_mutex_unlock(&mutex[1]); // unlock child
  pthread_join(thread, NULL);
  free(heap);
}


MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(test_write_from_child);

  MU_RUN_TEST(test_read_after_lock);
}

int main(int argc, char **argv) {
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return 0;
}
