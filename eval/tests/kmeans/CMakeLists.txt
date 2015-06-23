AddBenchmark(kmeans
  ARGS -d 3 -c 1000 -p 100000 -s 1000
  FILES kmeans-pthread.c
  INCLUDES ../../include
  LIBS phoenix
  )
