set(c_tests
  bug-atom
  bug-deadlock
  bug-deadlock1
  bug-lockuse
  bug-order
  bug-race
  parallel-sum
  #lockowner
  overhead
  memalign-test
  read-write-test
  #bug-free-borrowed
)

set(cpp_tests
  bug-init-32bit
  pageaccesslog-test
  usage-test
  thunkcounter-test
  persistence-test
  xatomic-test
  xlogger-test
  malloc-free-test
  mmap-test
)

include(ProcessorCount)
ProcessorCount(CORES)
set(TTHREAD_DEFINITIONS "CORES=${CORES};TEST_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

set(TTHREAD_INCLUDES
  ../include
  ../src/include
  ../src/include/heaplayers
  ../src/include/heaplayers/util
)

# Check for clock_gettime()
# is included in newer versions of glibc directly in libc,
# older version requires to link against librt
find_library(LIBRT_LIBRARIES rt)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")

foreach(testprog ${c_tests})
  add_executable(${testprog} ${testprog}.c finetime.cpp)
  target_link_libraries(${testprog} ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})
  target_compile_features(${testprog} PRIVATE cxx_variadic_macros)
  target_include_directories(${testprog} PRIVATE ${TTHREAD_INCLUDES})
  target_compile_definitions(${testprog} PRIVATE ${TTHREAD_DEFINITIONS})

  add_executable(${testprog}-det ${testprog}.c finetime.cpp)
  target_link_libraries(${testprog}-det LINK_PUBLIC tthread ${LIBRT_LIBRARIES})
  target_compile_features(${testprog}-det PRIVATE cxx_variadic_macros)
  target_include_directories(${testprog}-det PRIVATE ${TTHREAD_INCLUDES})
  target_compile_definitions(${testprog}-det PRIVATE ${TTHREAD_DEFINITIONS})

  add_test(${testprog} ${testprog}-det)
endforeach(testprog)

foreach(testprog ${cpp_tests})
  add_executable(${testprog} ${testprog}.cpp finetime.cpp)
  target_link_libraries(${testprog} LINK_PUBLIC tthread ${LIBRT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  target_compile_features(${testprog} PRIVATE cxx_auto_type cxx_variadic_macros)
  target_include_directories(${testprog} PRIVATE ${TTHREAD_INCLUDES})
  target_compile_definitions(${testprog} PRIVATE ${TTHREAD_DEFINITIONS})

  add_test(${testprog} ${testprog})
endforeach(testprog)

set(CMAKE_C_FLAGS "-std=c99")

add_definitions(-D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE)
