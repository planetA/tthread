AddBenchmark(histogram
  ARGS ${DATASET_HOME}/histogram_datafiles/large.bmp
  FILES histogram-pthread.c
  INCLUDES ../../include
  LIBS phoenix)
