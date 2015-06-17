add_benchmark(kmeans
  ARGS -d 3 -c 1000 -p 100000 -s 1000
  FILES kmeans-pthread.c
  LIBS phoenix
  )
