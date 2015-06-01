set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic")

include(ProcessorCount)
ProcessorCount(CORES)
add_definitions(-DCORES=${CORES})
include_directories(tthread ../src/include ../src/include/heaplayers ../src/include/heaplayers/util)

foreach(testprog bug-atom bug-deadlock bug-deadlock1 bug-lockuse bug-order bug-race parallel-sum lockowner)
  add_executable(${testprog} ${testprog}.c)
  target_link_libraries(${testprog} ${CMAKE_THREAD_LIBS_INIT})
  add_executable(${testprog}-det ${testprog}.c)
  target_link_libraries(${testprog}-det LINK_PUBLIC tthread)
  add_test(${testprog} ${testprog}-det)
endforeach(testprog)

# Check for clock_gettime()
# is included in newer versions of glibc directly in libc,
# older version requires to link against librt
include(CheckFunctionExists)
set(CMAKE_EXTRA_INCLUDE_FILES time.h)
CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
if(NOT HAVE_CLOCK_GETTIME)
find_library(LIBRT_LIBRARIES rt)
if(NOT LIBRT_LIBRARIES)
message(FATAL_ERROR "librt not found")
endif(NOT LIBRT_LIBRARIES)
endif(NOT HAVE_CLOCK_GETTIME)

foreach(testprog bug-init-32bit pageaccesslog-test thunkcounter-test persistence-test)
add_executable(${testprog} ${testprog}.cpp)
target_link_libraries(${testprog} LINK_PUBLIC tthread)
if(LIBRT_LIBRARIES)
  target_link_libraries(${testprog} LINK_PUBLIC ${LIBRT_LIBRARIES})
endif(LIBRT_LIBRARIES)
add_test(${testprog} ${testprog})
endforeach(testprog)

set(CMAKE_C_FLAGS "-std=c99")
add_definitions(-D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE)
