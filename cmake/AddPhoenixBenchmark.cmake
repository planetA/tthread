macro(AddPhoenixBenchmark benchmark)
  set(multiValueArgs DEFINITIONS RENAME)
  cmake_parse_arguments(bench
    ""
    ""
    "ARGS;DEFINITIONS;ENV;FILES"
    ${ARGN})

  set(destination ${CMAKE_CURRENT_SOURCE_DIR}/tests/${benchmark})

  set(src ${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(${benchmark} ${bench_FILES})
  set_target_properties(${benchmark} PROPERTIES
    COMPILE_FLAGS "-march=native -mtune=native -O3 -pipe"
    COMPILE_DEFINITIONS "${bench_DEFINITIONS}"
    OUTPUT_NAME ${destination}/${benchmark}
    INCLUDE_DIRECTORIES "${PHOENIX_INCLUDE};${src};${bench_INCLUDES}"
    COMPILE_FEATURES cxx_variadic_macros cxx_static_assert cxx_auto_type
    LINK_LIBRARIES "${CMAKE_THREAD_LIBS_INIT};phoenix")

  foreach(type pthread tthread)
    if (${type} EQUAL tthread)
      set(bench_ENV ${bench_ENV} "LD_PRELOAD=${PROJECT_SOURCE_DIR}/src/libtthread.so")
    endif()

    add_custom_target(bench-${benchmark}-${type}
      COMMAND ${CMAKE_COMMAND} -E
      env ${bench_ENV} ${CMAKE_COMMAND} -E
      time ./${benchmark} ${bench_ARGS}
      WORKING_DIRECTORY ${destination})
  endforeach()
endmacro(AddPhoenixBenchmark)
