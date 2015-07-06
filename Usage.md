To access th log use the `log` class in tthread from
within the programm linked against pthread.

```c
#include <stdio.h>
#include <tthread/log.h>
#include <tthread/logentry.h>

int global_var = 0;

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
    tthread::logentry e = log2.get(i);

    const char *access = e.getAccess() ==
                         tthread::logentry::READ ? "read" : "write";
    fprintf(stderr,
            "threadIndex: %d, thunkId: %d, address: %p, pageStart: %p, "
            "access: %s, issued at %p, thunk starts at %p\n",
            e.getThreadId(),
            e.getThunkId(),
            e.getFirstAccessedAddress(),
            e.getPageStart(),
            access,
            e.getFirstIssuerAddress(),
            e.getThunkStart()
            );
  }
  return 0;
}
```
