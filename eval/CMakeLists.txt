set(DATASET_HOME ${CMAKE_CURRENT_SOURCE_DIR}/datasets)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native -O3 -g")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -O3 -g")

include(phoenix.cmake)
include(parsec.cmake)

include(AddBenchmark)

file(GLOB benchmarks tests/*)
foreach(entry ${benchmarks})
  if(IS_DIRECTORY ${entry})
    add_subdirectory(${entry})
  endif(IS_DIRECTORY ${entry})
endforeach(entry)
