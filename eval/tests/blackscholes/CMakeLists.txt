add_benchmark(blackscholes
  ARGS 8 ${DATASET_HOME}/blackscholes/in_1M.txt prices.txt
  FILES blackscholes-pthread.cpp
  LIBS m stdc++
  DEFINITIONS -DENABLE_THREADS -DPARALLEL -DNCO=2 -DN=960)
