add_benchmark(word_count
  ARGS ${DATASET_HOME}/word_count/word_100MB.txt
  FILES word_count-pthread.c sort-pthread.c
  LIBS phoenix)
