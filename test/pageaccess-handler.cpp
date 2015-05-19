// #include "xpagelog.h"
#include <iostream>
#include <pthread.h>

void *test_heapaccess(void *data) {
  std::cout << "hello world" << std::endl;

  // std::string *s = (string *)data;
  // s->size();

  return NULL;
}

int main(int argc, char **argv) {
  std::string *s = new std::string("foo");

  pthread_t thread;

  pthread_create(&thread, NULL, test_heapaccess, (void *)s);

  void *status;
  pthread_join(thread, &status);

  // xpagelog log = xpagelog::getInstance();
  // log.reset();
}
