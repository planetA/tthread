include(EnableWarnings)

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
  bug-free-borrowed
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
add_definitions(-DCORES=${CORES})
include_directories(tthread ../src/include ../src/include/heaplayers ../src/include/heaplayers/util)

# Check for clock_gettime()
# is included in newer versions of glibc directly in libc,
# older version requires to link against librt
find_library(LIBRT_LIBRARIES rt)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")

foreach(testprog ${c_tests})
  add_executable(${testprog} ${testprog}.c finetime.cpp)
  target_link_libraries(${testprog} ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})

  add_executable(${testprog}-det ${testprog}.c finetime.cpp)
  target_link_libraries(${testprog}-det LINK_PUBLIC tthread ${LIBRT_LIBRARIES})

  add_test(${testprog} ${testprog}-det)
endforeach(testprog)

foreach(testprog ${cpp_tests})
  add_executable(${testprog} ${testprog}.cpp finetime.cpp)
  target_link_libraries(${testprog} LINK_PUBLIC tthread ${LIBRT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  target_compile_features(${testprog} PRIVATE cxx_auto_type cxx_variadic_macros)

  add_test(${testprog} ${testprog})
endforeach(testprog)

set(CMAKE_C_FLAGS "-std=c99")

add_definitions(-D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE)
