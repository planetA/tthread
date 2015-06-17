add_benchmark(pca
  ARGS -r 4000 -c 4000 -s 100
  FILES pca-pthread.c
  LIBS phoenix)
