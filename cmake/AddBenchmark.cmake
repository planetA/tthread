macro(AddBenchmark benchmark)
  set(multiValueArgs DEFINITIONS RENAME)
  cmake_parse_arguments(AddBenchmark
    ""
    ""
    "ARGS;DEFINITIONS;LIBS;INCLUDES;FILES"
    ${ARGN})

  add_executable(${benchmark}-pthread ${AddBenchmark_FILES})
  target_link_libraries(${benchmark}-pthread ${CMAKE_THREAD_LIBS_INIT}
    ${AddBenchmark_LIBS})
  add_custom_target(bench-${benchmark}-pthread
    COMMAND ${CMAKE_COMMAND} -E
    time ./${benchmark}-pthread ${AddBenchmark_ARGS})
  target_include_directories(${benchmark}-pthread PRIVATE include . ${AddBenchmark_INCLUDES})
  target_compile_definitions(${benchmark}-pthread PRIVATE ${AddBenchmark_DEFINITIONS})

  add_executable(${benchmark}-tthread ${AddBenchmark_FILES})
  target_link_libraries(${benchmark}-tthread LINK_PUBLIC tthread
    ${AddBenchmark_LIBS})
  add_custom_target(bench-${benchmark}-tthread
    COMMAND ${CMAKE_COMMAND} -E
    time ./${benchmark}-tthread ${AddBenchmark_ARGS})
  target_include_directories(${benchmark}-tthread PRIVATE include . ${AddBenchmark_INCLUDES})
  target_compile_definitions(${benchmark}-tthread PRIVATE ${AddBenchmark_DEFINITIONS})
endmacro(AddBenchmark)
