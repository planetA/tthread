add_benchmark(pca
  ARGS -r 4000 -c 4000 -s 100
  FILES pca.c pca-pthread.c
  LIBS phoenix)
