SET ( CMAKE_CXX_FLAGS "-std=gnu++11" CACHE STRING "compile flags" FORCE)

include_directories("../include")

file(GLOB_RECURSE schasFiles "*.cpp")
add_executable(schas ${schasFiles})
#target_compile_features(schas PRIVATE cxx_range_for)

install(TARGETS schas
  DESTINATION bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
