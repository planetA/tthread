Tthreads: Tracing Multithreading library
----------------------------------------

tthread is based on [dthread](https://github.com/emeryberger/dthreads)

### Building Tthreads ###

[CMake](http://www.cmake.org/) is required to build Tthreads:

```
$ cmake .
$ make
```

This will build the Tthread library (`libtthread.so`).

### Using Tthreads ###

Tthreads currently only supports Linux/x86\_64 platforms.

1. Compile your program to object files (here, we use just one, `target.o`).

2. Link to the tthreads library. There are two options (neither of which
   is particular to tthreads).

  (a) Dynamic linking: this approach requires no environment variables,
      but the tthreads library needs to be in a fixed, known location.
      Place the tthreads library in a directory (`TTHREAD_DIR`).
      Then compile your program as follows:

```
% g++ target.o -rdynamic <TTHREAD_DIR>/libtthread.so -ldl -o target
```

  (b) Ordinary dynamic linking: this approach is more flexible (you can
      change the location of the tthreads library), but you must also
      set the `LD_LIBRARY_PATH` environment variable.

```
% g++ target.o -L<TTHREAD_DIR> -ltthread -dl -o target
% export LD_LIBRARY_PATH=<TTHREAD_DIR>:$LD_LIBRARY_PATH
```
