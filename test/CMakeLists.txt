set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")

foreach(testprog bug-atom bug-deadlock bug-deadlock1 bug-lockuse bug-order bug-race parallel-sum lockowner)
  add_executable (${testprog} ${testprog}.c)
  target_link_libraries (${testprog} ${CMAKE_THREAD_LIBS_INIT})
  add_executable (${testprog}-det ${testprog}.c)
  target_link_libraries (${testprog}-det LINK_PUBLIC dthread)
endforeach(testprog)

add_executable (bug-init bug-init-32bit.cxx)
target_link_libraries (bug-init ${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_C_FLAGS "-std=c99")
