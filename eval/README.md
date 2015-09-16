# Benchmarks

Building Benchmarks is disabled by default.
To enable it use:

```
cmake -DBENCHMARK=ON .
```

This will also download the datasets required for Phoenix and Parsec Benchmarks.

If you experience out-of-memory while running the benchmark, set `vm.overcommit_memory` to 1.

```
sysctl -w vm.overcommit_memory=1
```

Tthread will allocate a lot virtual memory, because of private memory mappings (per thread).
Linux does not give processes more virtual memory then physical memory available on the system.
The benchmarks however are safe to be used this way.

## Working Benchmarks

- phoenix:
  - [x] histogram
  - [x] linear_regression
  - [x] reverse_index
  - [x] string_match
  - [x] word_count
  - [x] kmeans
  - [x] matrix_multiply
  - [x] pca
- parsec:
  - [x] blackscholes
  - [x] canneal
  - [x] dedup
  - [ ] ferret (builds now, not reviewed yet)
  - [x] streamcluster
      - eval/tests/streamcluster/streamcluster.cpp:1097
      - work_mem[pid*stride + K+1] = cost_of_opening_x;
  - [ ] vips (mutex_trylock not implemented -> infinite loop?)
  - [ ] raytrace (Own task scheduler, No commit on sched_yield -> infinite loop?)
  - [x] swaptions
