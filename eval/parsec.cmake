DownloadDataset(http://parsec.cs.princeton.edu/download/2.0/parsec-2.0.tar.gz
  parsec-2.0.tar.gz
  ${CMAKE_CURRENT_SOURCE_DIR})

file(WRITE parsec-2.0/config/gcc.bldconf "
# Generated configuration file
export LIBS=''
export EXTRA_LIBS=''
export CC='${CMAKE_C_COMPILER}'
export CXX='${CMAKE_CXX_COMPILER}'
export LD='${CMAKE_LINKER}'
export RANLIB='${CMAKE_RANLIB}'
export STRIP='${CMAKE_STRIP}'
export AR='${CMAKE_AR}'
export CFLAGS='-O3 -funroll-loops -fprefetch-loop-arrays'
export CXXFLAGS='-O3 -funroll-loops -fprefetch-loop-arrays -fpermissive -fno-exceptions'
export CPPFLAGS=''
export LDFLAGS='${CMAKE_MODULE_LINKER_FLAGS} -L${CMAKE_CURRENT_SOURCE_DIR}/../src -ltthread'
export CC_ver=$($CC --version)
export CXX_ver=$($CXX --version)
export LD_ver=$($LD --version)
export LD_LIBRARY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/../src
export MAKE=make
export M4=m4
")
