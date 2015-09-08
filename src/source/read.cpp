#include <stdint.h>

#include "real.h"
#include "visibility.h"
#include "xdefines.h"

void prepare_read(void *buf, size_t count) {
  // Produce write pagefault to buffer in advance to avoid EFAULT errno
  uint8_t *start = (uint8_t *)buf;

  for (size_t i = 0; i < count; i += xdefines::PageSize) {
    start[i] = 0;
  }

  start[count - 1] = 0;
}

extern "C" {
_PUBLIC_ ssize_t read(int fd, void *buf, size_t count) {
  prepare_read(buf, count);
  return WRAP(read)(fd, buf, count);
}

_PUBLIC_ size_t fread(void *ptr, size_t size, size_t nmemb, void *stream) {
  prepare_read(ptr, size * nmemb);
  return WRAP(fread)(ptr, size, nmemb, stream);
}
}
