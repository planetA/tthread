add_benchmark(string_match
  ARGS ${DATASET_HOME}/string_match/key_file_500MB.txt
  FILES string_match.c string_match-pthread.c
  LIBS phoenix)
