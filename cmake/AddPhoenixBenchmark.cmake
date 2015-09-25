macro(AddPhoenixBenchmark benchmark)
  set(multiValueArgs DEFINITIONS RENAME)
  cmake_parse_arguments(bench
    ""
    ""
    "ARGS;DEFINITIONS;LIBS;FILES"
    ${ARGN})

  foreach(type pthread tthread)
    set(target ${benchmark}-${type})
    add_executable(${target} ${bench_FILES})

    set(libs ${bench_LIBS})
    if(${type} EQUAL pthread)
      set(libs "${CMAKE_THREAD_LIBS_INIT};${bench_LIBS}")
    else()
      set(libs "tthread;${bench_LIBS}")
    endif()

    set(src ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${target} PROPERTIES
      COMPILE_FLAGS "-march=native -mtune=native -O3 -pipe"
      COMPILE_DEFINITIONS "${bench_DEFINITIONS}"
      INCLUDE_DIRECTORIES "${PHOENIX_INCLUDE};${src};${bench_INCLUDES}"
      COMPILE_FEATURES cxx_variadic_macros cxx_static_assert cxx_auto_type
      LINK_LIBRARIES "${libs}"
    )

    add_custom_target(bench-${benchmark}-${type}
      COMMAND ${CMAKE_COMMAND} -E
      time ./${benchmark}-${type} ${bench_ARGS})
  endforeach()
endmacro(AddPhoenixBenchmark)
