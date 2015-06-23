AddBenchmark(linear_regression
  ARGS ${DATASET_HOME}/linear_regression_datafiles/key_file_500MB.txt
  FILES linear_regression-pthread.c
  INCLUDES ../../include
  LIBS phoenix)
