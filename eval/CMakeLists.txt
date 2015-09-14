cmake_minimum_required(VERSION 2.8.7)
set(DATASET_HOME ${CMAKE_CURRENT_SOURCE_DIR}/datasets)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native -O3 -g")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -O3 -g")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake)

include(ProcessorCount)
ProcessorCount(AVAILABLE_CORES)
set(NCORES ${AVAILABLE_CORES} CACHE FILEPATH "number of cores in the benchmark")

include(phoenix.cmake)
include(parsec.cmake)

include(AddBenchmark)

file(GLOB benchmarks tests/*)
foreach(entry ${benchmarks})
  if(EXISTS ${entry}/CMakeLists.txt)
    add_subdirectory(${entry})
  endif()
endforeach(entry)
