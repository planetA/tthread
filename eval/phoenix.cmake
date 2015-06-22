set(PHOENIX_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/phoenix/phoenix-2.0/lib/libphoenix.a)

ADD_CUSTOM_TARGET(
  build_phoenix
  COMMAND make
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/phoenix/phoenix-2.0
  DEPENDS ${PHOENIX_LIBRARY}
  COMMENT "Libphoenix makefile target")

add_library(phoenix STATIC IMPORTED)
set_property(TARGET phoenix APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(phoenix PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${PHOENIX_LIBRARY}")
add_dependencies(phoenix build_phoenix)

include(DownloadDataset)

# Phoenix benchmark data
DownloadDataset(http://csl.stanford.edu/%7Echristos/data/histogram.tar.gz
  histogram.tar.gz
  ${CMAKE_CURRENT_SOURCE_DIR}/datasets)
DownloadDataset(http://csl.stanford.edu/%7Echristos/data/linear_regression.tar.gz
  linear_regression.tar.gz
  ${CMAKE_CURRENT_SOURCE_DIR}/datasets)
DownloadDataset(http://csl.stanford.edu/%7Echristos/data/reverse_index.tar.gz
  reverse_index.tar.gz
  ${CMAKE_CURRENT_SOURCE_DIR}/datasets)
DownloadDataset(http://csl.stanford.edu/%7Echristos/data/string_match.tar.gz
  string_match.tar.gz
  ${CMAKE_CURRENT_SOURCE_DIR}/datasets)
DownloadDataset(http://csl.stanford.edu/%7Echristos/data/word_count.tar.gz
  word_count.tar.gz
  ${CMAKE_CURRENT_SOURCE_DIR}/datasets)

set(phoenix_benchmarks histogram linear_regression reverse_index string_match word_count)

foreach(bench ${phoenix_benchmarks})
  list(APPEND phoenix_pthread_benchmarks bench-${bench}-pthread)
  list(APPEND phoenix_tthread_benchmarks bench-${bench}-tthread)
endforeach(bench)

add_custom_target(phoenix-tthread DEPENDS ${phoenix_tthread_benchmarks})
add_custom_target(phoenix-pthread DEPENDS ${phoenix_pthread_benchmarks})
