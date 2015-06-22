macro(AddBenchmark benchmark)
  set(multiValueArgs DEFINITIONS RENAME)
  cmake_parse_arguments(AddBenchmark
    ""
    ""
    "ARGS;DEFINITIONS;LIBS;INCLUDES;FILES"
    ${ARGN})

  include_directories(../../include include . ${INCLUDES})
  add_definitions(${AddBenchmark_DEFINITIONS})

  add_executable(${benchmark}-pthread ${AddBenchmark_FILES})
  target_link_libraries(${benchmark}-pthread ${CMAKE_THREAD_LIBS_INIT}
    ${AddBenchmark_LIBS})
  add_custom_target(bench-${benchmark}-pthread
    COMMAND ${benchmark}-pthread ${AddBenchmark_ARGS})

  add_executable(${benchmark}-tthread ${AddBenchmark_FILES})
  target_link_libraries(${benchmark}-tthread LINK_PUBLIC tthread
    ${AddBenchmark_LIBS})
  add_custom_target(bench-${benchmark}-tthread
    COMMAND ${benchmark}-tthread ${AddBenchmark_ARGS})
endmacro(AddBenchmark)
