set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")

include(ProcessorCount)
ProcessorCount(CORES)
add_definitions(-DCORES=${CORES})

foreach(testprog bug-atom bug-deadlock bug-deadlock1 bug-lockuse bug-order bug-race parallel-sum lockowner)
  add_executable (${testprog} ${testprog}.c)
  target_link_libraries (${testprog} ${CMAKE_THREAD_LIBS_INIT})
  add_executable (${testprog}-det ${testprog}.c)
  target_link_libraries (${testprog}-det LINK_PUBLIC dthread)
  add_test(${testprog} ${testprog}-det)
endforeach(testprog)

add_executable (bug-init bug-init-32bit.cpp)
target_link_libraries (bug-init LINK_PUBLIC dthread)
add_test(bug-init bug-init)

set(CMAKE_C_FLAGS "-std=c99")
