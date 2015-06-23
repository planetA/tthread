AddBenchmark(reverse_index
  ARGS ${DATASET_HOME}/reverse_index_datafiles
  FILES reverseindex-pthread.c
  INCLUDES ../../include
  LIBS phoenix)
