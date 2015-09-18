include(EnableWarnings)

file(GLOB_RECURSE srcFiles "source/*.cpp")
add_library(tthread SHARED ${srcFiles})
target_link_libraries(tthread ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tthread ${LIBDL_LIBRARIES})
target_compile_features(tthread PRIVATE cxx_variadic_macros cxx_static_assert cxx_auto_type)
target_include_directories(tthread PRIVATE
  ../include/
  include
  include/heaplayers
  include/heaplayers/util)

set(TTHREAD_DEFINITIONS "")

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
  set(TTHREAD_DEFINITIONS "${TTHREAD_DEFINITIONS};DDEBUG_ENABLED;DCHECK_SCHEDULE")
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

if(HAS_BUILTIN_RETURN_ADDRESS)
  set(TTHREAD_DEFINITIONS "${TTHREAD_DEFINITIONS};BUILTIN_RETURN_ADDRESS")
endif(HAS_BUILTIN_RETURN_ADDRESS)

# Get some characteristics about running.
#set(TTHREAD_DEFINITIONS "${TTHREAD_DEFINITIONS};-DGET_CHARACTERISTICS")

set_target_properties(tthread PROPERTIES
  COMPILE_FLAGS "-fPIC -g -fvisibility=hidden -fvisibility-inlines-hidden"
  COMPILE_DEFINITIONS "SSE_SUPPORT;NDEBUG;LOCK_OWNERSHIP;DETERM_MEMORY_ALLOC;LOCK_OWNERSHIP;${TTHREAD_DEFINITIONS}"
  # By forcing to load all symbols at instant, we avoid segmentation faults in
  # GOT, when entries are updated in the segmentation fault handler.
  LINK_FLAGS -Wl,-z,now)

install(TARGETS tthread
  DESTINATION ${LIB_INSTALL_DIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
