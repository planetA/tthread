include(EnableWarnings)

include_directories(tthread
  include
  include/heaplayers
  include/heaplayers/util)

FILE(GLOB_RECURSE srcFiles "source/*.cpp")
add_library(tthread SHARED ${srcFiles})
target_link_libraries(tthread ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tthread ${LIBDL_LIBRARIES})
target_compile_features(tthread PRIVATE cxx_variadic_macros cxx_static_assert cxx_auto_type)

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DDEBUG_ENABLED)
  add_definitions(-DCHECK_SCHEDULE)
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

if(HAS_BUILTIN_RETURN_ADDRESS)
  add_definitions(-DBUILTIN_RETURN_ADDRESS)
endif(HAS_BUILTIN_RETURN_ADDRESS)

# Get some characteristics about running.
#add_definitions(-DGET_CHARACTERISTICS)

if(CMAKE_COMPILER_IS_GNUCC OR __COMPILER_GNU)
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif()

set_target_properties(tthread PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(tthread PROPERTIES COMPILE_DEFINITIONS "SSE_SUPPORT;NDEBUG;LOCK_OWNERSHIP;DETERM_MEMORY_ALLOC;LOCK_OWNERSHIP")
# By forcing to load all symbols at instant, we avoid segmentation faults in
# GOT, when entries are updated in the segmentation fault handler.
set_target_properties(tthread PROPERTIES LINK_FLAGS -Wl,-z,now)
