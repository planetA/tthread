add_benchmark(matrix_multiply
  ARGS 2000 2000
  FILES matrix_multiply.c matrix_multiply-pthread.c
  LIBS phoenix)
