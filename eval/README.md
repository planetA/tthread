# Benchmarks

Building Benchmarks is disabled by default.
To enable it use:

```
cmake -DBENCHMARK=ON .
```

This will also download the datasets required for Phoenix and Parsec Benchmarks.

If you experience out-of-memory while running the benchmark, set `vm.overcommit_memory` to 2.

```
sysctl -w vm.overcommit_memory=2
```

Tthread will allocate a lot virtual memory, because of private memory mappings (per thread).
Linux does not give processes more virtual memory then physical memory available on the system.
The benchmarks however are safe to be used this way.

## Working Benchmarks

- [ ] blackscholes (Input file not found)
- [ ] canneal (infinite loop)
- [x] dedup
- [ ] ferret (imagemagick)
- [x] histogram
- [x] kmeans
- [x] linear_regression
- [x] matrix_multiply
- [x] pca
- [x] reverse_index
- [ ] streamcluster (bufferoverlow in user code)
    - eval/tests/streamcluster/streamcluster.cpp:1097
    - work_mem[pid*stride + K+1] = cost_of_opening_x;
- [x] string_match
- [x] swaptions
- [x] word_count
