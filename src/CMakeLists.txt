include_directories(dthread include include/heaplayers include/heaplayers/util)
FILE(GLOB srcFiles "source/*.cpp")
add_library(dthread SHARED ${srcFiles})
target_link_libraries(dthread ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(dthread ${LIBDL_LIBRARIES})

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

# Check deterministic schedule
#add_definitions(-DCHECK_SCHEDULE)

# Get some characteristics about running.
#add_definitions(-DGET_CHARACTERISTICS)

set_target_properties(dthread PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(dthread PROPERTIES COMPILE_DEFINITIONS "SSE_SUPPORT;NDEBUG;LOCK_OWNERSHIP;DETERM_MEMORY_ALLOC;LOCK_OWNERSHIP")
