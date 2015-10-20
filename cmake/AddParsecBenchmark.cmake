macro(AddParsecBenchmark benchmark)
  set(multiValueArgs DEFINITIONS RENAME)
  cmake_parse_arguments(bench
    ""
    ""
    "ARGS;PATH;ARCHIVE;ENV;EXE"
    ${ARGN})

  set(destination ${CMAKE_CURRENT_SOURCE_DIR}/tests/${benchmark})
  set(executable ${destination}/${benchmark})

  add_custom_target(bench-${benchmark})
  add_dependencies(bench-${benchmark} parsec)

  if ("x${bench_EXE}x" STREQUAL "xx")
    set(bench_EXE ${benchmark})
  endif()

  add_custom_command(TARGET bench-${benchmark} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy ${PARSEC_APP_PATH}/${bench_PATH}/inst/amd64-linux.gcc-pthreads/bin/${bench_EXE}
    ${executable})

  if (NOT "x${bench_ARCHIVE}x" STREQUAL "xx")
    add_custom_command(TARGET bench-${benchmark} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E
      tar xzf ${PARSEC_APP_PATH}/${bench_PATH}/inputs/${bench_ARCHIVE}
      WORKING_DIRECTORY ${destination})
  endif()

  foreach(type pthread tthread)
    if (${type} EQUAL tthread)
      set(bench_ENV ${bench_ENV} "LD_PRELOAD=${PROJECT_SOURCE_DIR}/src/libtthread.so")
    endif()
    add_custom_target(bench-${benchmark}-${type}
      COMMAND ${CMAKE_COMMAND} -E
      time ${CMAKE_COMMAND} -E
      env ${bench_ENV}
      ${executable} ${bench_ARGS}
      DEPENDS bench-${benchmark}
      WORKING_DIRECTORY ${destination})
  endforeach()
endmacro()
