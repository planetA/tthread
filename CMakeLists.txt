cmake_minimum_required(VERSION 3.1)
enable_testing()
project(tthread)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_THREAD_PREFER_PTHREAD)
find_package(Threads)
if(NOT CMAKE_USE_PTHREADS_INIT)
  message(SEND_ERROR "Pthreads required")
endif(NOT CMAKE_USE_PTHREADS_INIT)

find_package(LibDL)
if(NOT LIBDL_FOUND)
  message(SEND_ERROR "libdl required")
endif(NOT LIBDL_FOUND)

INCLUDE (CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES(
"
int main()
{
__builtin_return_address(0);
}
" HAS_BUILTIN_RETURN_ADDRESS)

if(LIB_INSTALL_DIR)
else()
  set(LIB_INSTALL_DIR lib)
endif()

option (BENCHMARK OFF)
if (BENCHMARK)
  if (NOT BENCHMARK STREQUAL "OFF")
    add_subdirectory(eval)
  endif()
endif (BENCHMARK)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(test)
