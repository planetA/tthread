To access th log use the `log` class in tthread from
within the programm linked against pthread.

```c
#include <stdio.h>
#include <tthread/log.h>
#include <tthread/logevent.h>

int global_var = 0;

enum { PageSize = 4096UL };
enum { PAGE_SIZE_MASK = (PageSize - 1) };

#define PAGE_ALIGN_DOWN(x) (((size_t)(x)) & ~PAGE_SIZE_MASK)

int main(int argc, char **argv) {
  // read-only log all current logged events
  tthread::log log;

  global_var = 1;

  // the size of an log instance will be not changed after instantiation
  // to get all events happend after `log` was created, use:
  // (in this case global_var will logged)
  tthread::log log2(log.end());

  unsigned int llen = log2.length();

  for (unsigned long i = 0; i < llen; i++) {
    tthread::logevent e = log2.get(i);

    const tthread::EventData data = e.getData();

    switch (e.getType()) {
    case tthread::logevent::READ:
    case tthread::logevent::WRITE:
    {
      const char *access = e.getType() ==
                           tthread::logevent::READ ? "read" : "write";
      fprintf(stderr,
              "[%s] threadIndex: %d, address: %p, pageStart: %p, issued at: [%p]\n",
              access,
              data.memory.threadId,
              data.memory.address,
              ((void *)PAGE_ALIGN_DOWN(
                 data.memory.address)),
              e.getReturnAddress());
      break;
    }

    case tthread::logevent::THUNK:
    {
      fprintf(stderr,
              "[thunk] %d, issued at [%p]\n",
              data.thunk.id,
              e.getReturnAddress());
      break;
    }

    case tthread::logevent::INVALID:
      fprintf(stderr, "[invalid entry]\n");

    default:
      printf("foo\n");
    }
  }
  return 0;
}
```
