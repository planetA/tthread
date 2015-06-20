add_benchmark(reverse_index
  ARGS ${DATASET_HOME}/reverse_index_datafiles
  FILES reverseindex-pthread.c
  LIBS phoenix)
