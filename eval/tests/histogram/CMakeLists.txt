add_benchmark(histogram
  ARGS ${DATASET_HOME}/histogram/large.bmp
  FILES histogram-pthread.c
  LIBS phoenix)
