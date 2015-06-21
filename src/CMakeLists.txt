include(EnableWarnings)

include_directories(tthread
  include
  include/heaplayers
  include/heaplayers/util)

FILE(GLOB_RECURSE srcFiles "source/*.cpp")
add_library(tthread SHARED ${srcFiles})
target_link_libraries(tthread ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tthread ${LIBDL_LIBRARIES})
target_compile_features(tthread PRIVATE cxx_variadic_macros cxx_static_assert)

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DDEBUG)
  add_definitions(-DCHECK_SCHEDULE)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

if(HAS_BUILTIN_RETURN_ADDRESS)
  add_definitions(-DBUILTIN_RETURN_ADDRESS)
endif(HAS_BUILTIN_RETURN_ADDRESS)

# Check deterministic schedule
#add_definitions(-DCHECK_SCHEDULE)

# Get some characteristics about running.
#add_definitions(-DGET_CHARACTERISTICS)

if(CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  message(STATUS "Found g++, enabling -fvisibility=hidden")
endif()

set_target_properties(tthread PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(tthread PROPERTIES COMPILE_DEFINITIONS "SSE_SUPPORT;NDEBUG;LOCK_OWNERSHIP;DETERM_MEMORY_ALLOC;LOCK_OWNERSHIP")
