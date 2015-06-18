set(DATASET_HOME ${CMAKE_CURRENT_SOURCE_DIR}/datasets)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -mtune=native -O3 -g")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native -O3 -g")

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

file(GLOB benchmarks tests/*)
foreach(entry ${benchmarks})
  if(IS_DIRECTORY ${entry})
    add_subdirectory(${entry})
  endif(IS_DIRECTORY ${entry})
endforeach(entry)
