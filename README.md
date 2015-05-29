Tthread: Tracing Multithreading library
----------------------------------------

tthread is based on [dthreads](https://github.com/emeryberger/dthreads)

### Building Tthread ###

[CMake](http://www.cmake.org/) is required to build Tthread:

```
$ cmake .
$ make
```

This will build the Tthread library (`libtthread.so`).

### Using Tthread ###

Tthread currently only supports Linux/x86\_64 platforms.

1. Compile your program to object files (here, we use just one, `target.o`).

2. Link to the tthread library. There are two options (neither of which
   is particular to tthread).

  (a) Dynamic linking: this approach requires no environment variables,
      but the tthread library needs to be in a fixed, known location.
      Place the tthread library in a directory (`TTHREAD_DIR`).
      Then compile your program as follows:

```
% g++ target.o -rdynamic <TTHREAD_DIR>/libtthread.so -ldl -o target
```

  (b) Ordinary dynamic linking: this approach is more flexible (you can
      change the location of the tthread library), but you must also
      set the `LD_LIBRARY_PATH` environment variable.

```
% g++ target.o -L<TTHREAD_DIR> -ltthread -dl -o target
% export LD_LIBRARY_PATH=<TTHREAD_DIR>:$LD_LIBRARY_PATH
```
