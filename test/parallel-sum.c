#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE (4096)
#define NUM_BYTES ((int)(PAGE_SIZE * 1e3)) /*kb*/
#define NUM_THREADS 4

typedef struct {
  char *buf;
  int size;
} job_t;

volatile unsigned int sum = 0;
pthread_mutex_t mutexsum;
pthread_t threads[NUM_THREADS];
job_t jobs[NUM_THREADS];

int setup_test_input()
{
  const char template[] = "tracetest.XXXXXX";
  char fname[PATH_MAX];

  strcpy(fname, template);
  int testfd = mkstemp(fname);

  if (testfd < 0) {
    perror("Could not create tempfile");
    return -1;
  }

  int rc = ftruncate(testfd, NUM_BYTES);

  if (rc < 0) {
    perror("Failed to resize tempfile");
    goto cleanup;
  }

  void *buf = mmap(0, NUM_BYTES, PROT_WRITE, MAP_SHARED, testfd, 0);

  if (buf == MAP_FAILED) {
    perror("Failed to create mapping");
    goto cleanup;
  }

  memset(buf, 1, NUM_BYTES);
  rc = munmap(buf, NUM_BYTES);

  if (rc < 0) {
    perror("Failed to unmap file");
    goto cleanup;
  }

  close(testfd);

  int testfd2 = open(fname, O_RDONLY);
  unlink(fname);

  if (testfd2 < 0) {
    perror("Fail to reopen testfile");
  }

  return testfd2;

  cleanup:
  unlink(fname);
  close(testfd);
  return -1;
}

void *sumfile(void *arg)
{
  job_t *job = (job_t *)arg;
  unsigned int local_sum = 0;
  char *buf = job->buf;
  void *testbuf = malloc(job->size);

  memcpy(testbuf, buf, sizeof(job->size));

  for (int i = 0; i < job->size; i++) {
    local_sum += buf[i];
  }

  pthread_mutex_lock(&mutexsum);
  sum += local_sum;
  pthread_mutex_unlock(&mutexsum);

  pthread_exit((void *)0);
  free(testbuf);
}

int main(int argc, const char *argv[])
{
  int fd = setup_test_input();

  if (fd < 0) {
    return 1;
  }
  void *buf = mmap(0, NUM_BYTES, PROT_READ, MAP_SHARED, fd, 0);

  if (buf == MAP_FAILED) {
    perror("Failed to create mapping");
    close(fd);
    return -1;
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_mutex_init(&mutexsum, NULL);

  const int per_thread = (NUM_BYTES / NUM_THREADS);

  for (int i = 0; i < NUM_THREADS; i++) {
    char *segment = buf + (per_thread * i);
    int size = per_thread;

    if ((i + 1) >= NUM_THREADS) {
      size += NUM_BYTES - (per_thread * NUM_THREADS);
    }
    job_t job = {
      .buf = segment,
      .size = size,
    };
    jobs[i] = job;
    pthread_create(&threads[i], &attr, sumfile, &jobs[i]);
  }
  pthread_attr_destroy(&attr);

  void *status;

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], &status);
  }
  int rc = 0;

  if (sum != NUM_BYTES) {
    printf("Expected sum to equal %d, got: %d\n", NUM_BYTES, sum);
    rc = 1;
  }

  close(fd);
  pthread_mutex_destroy(&mutexsum);
  return rc;
}
