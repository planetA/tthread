add_benchmark(linear_regression
  ARGS ${DATASET_HOME}/linear_regression/key_file_500MB.txt
  FILES linear_regression-pthread.c
  LIBS phoenix)
