add_benchmark(string_match
  ARGS ${DATASET_HOME}/string_match_datafiles/key_file_500MB.txt
  FILES string_match-pthread.c
  LIBS phoenix)
