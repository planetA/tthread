# Benchmarks

Building Benchmarks is disabled by default.
To enable it use:

```
cmake -DBENCHMARK=ON .
```

This will also download the datasets required for Phoenix and Parsec Benchmarks.

If you experience out-of-memory while running the benchmark, set `vm.overcommit_memory` to 2:

```
sysctl -w vm.overcommit_memory=2
```

## Working Benchmarks

[ ] blackscholes (Input file not found)
[ ] canneal (infinite loop)
[✓] dedup
[ ] ferret (imagemagick)
[✓] histogram
[✓] kmeans
[✓] linear_regression
[✓] matrix_multiply
[✓] pca
[✓] reverse_index
[ ] streamcluster (bufferoverlow in user code)
    - eval/tests/streamcluster/streamcluster.cpp:1097
    - work_mem[pid*stride + K+1] = cost_of_opening_x;
[✓] string_match
[✓] swaptions
[✓] word_count
