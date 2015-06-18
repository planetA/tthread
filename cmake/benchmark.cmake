macro(add_benchmark benchmark)
  set(multiValueArgs DEFINITIONS RENAME)
  cmake_parse_arguments(add_benchmark
    ""
    ""
    "ARGS;DEFINITIONS;LIBS;INCLUDES;FILES"
    ${ARGN})

  include_directories(../../include include . ${INCLUDES})
  if(add_benchmark_DEFINITIONS)
    add_definitions(${DEFINITIONS})
  endif(add_benchmark_DEFINITIONS)

  add_executable(${benchmark}-pthread ${add_benchmark_FILES})
  target_link_libraries(${benchmark}-pthread ${CMAKE_THREAD_LIBS_INIT}
    ${add_benchmark_LIBS})
  add_custom_target(bench-${benchmark}-pthread
    COMMAND ${benchmark}-pthread ${add_benchmark_ARGS})

  add_executable(${benchmark}-tthread ${add_benchmark_FILES})
  target_link_libraries(${benchmark}-tthread LINK_PUBLIC tthread
    ${add_benchmark_LIBS})
  add_custom_target(bench-${benchmark}-tthread
    COMMAND ${benchmark}-tthread ${add_benchmark_ARGS})
endmacro(add_benchmark)
