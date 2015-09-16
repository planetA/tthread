include(ExternalProject)

file(WRITE gcc.bldconf "
# Generated configuration file
export LIBS=''
export EXTRA_LIBS=''
export CC='${CMAKE_C_COMPILER}'
export CXX='${CMAKE_CXX_COMPILER}'
export LD='${CMAKE_LINKER}'
export RANLIB='${CMAKE_RANLIB}'
export STRIP='${CMAKE_STRIP}'
export AR='${CMAKE_AR}'
export CFLAGS='-g -O3 -funroll-loops -fprefetch-loop-arrays -rdynamic'
export CXXFLAGS='-g -O3 -funroll-loops -fprefetch-loop-arrays -fpermissive -fno-exceptions -rdynamic'
export CPPFLAGS=''
export LDFLAGS='${CMAKE_MODULE_LINKER_FLAGS}'
export CC_ver=$($CC --version)
export CXX_ver=$($CXX --version)
export LD_ver=$($LD --version)
export LD_LIBRARY_PATH=
export MAKE='make'
export M4=m4
")

ExternalProject_Add(parsec
  URL http://parsec.cs.princeton.edu/download/3.0/parsec-3.0.tar.gz
  PATCH_COMMAND sh -c "find . -name '*.pod' -print0 | xargs -0 sed -ie 's/=item \\([0-9]\\+\\)/=item C\\1/'; find . -name 'parsec_barrier.hpp' -type f -print0 | xargs -0 sed -ie 's!^#define ENABLE_AUTOMATIC_DROPIN!//#define ENABLE_AUTOMATIC_DROPIN!'"
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy ../gcc.bldconf config/gcc.bldconf
  SOURCE_DIR parsec-3.0
  BUILD_COMMAND bin/parsecmgmt -a build -c gcc-pthreads -p canneal -p dedup -p blackscholes -p streamcluster -p ferret -p raytrace -p swaptions -p vips
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE 1
  LOG_BUILD 1
)

set(PARSEC_APP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/parsec-3.0/pkgs)
set(TEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tests/)

function(AddParsecBenchmark benchmark)
  set(multiValueArgs DEFINITIONS RENAME)
  cmake_parse_arguments(AddParsecBenchmark
    ""
    ""
    "ARGS;PATH;ARCHIVE;ENV;EXE"
    ${ARGN})

  set(destination ${CMAKE_CURRENT_SOURCE_DIR}/tests/${benchmark})
  set(executable ${destination}/${benchmark})

  add_custom_target(bench-${benchmark})
  add_dependencies(bench-${benchmark} parsec)

  if ("x${AddParsecBenchmark_EXE}x" STREQUAL "xx")
    set(AddParsecBenchmark_EXE ${benchmark})
  endif()

  add_custom_command(TARGET bench-${benchmark} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy ${PARSEC_APP_PATH}/${AddParsecBenchmark_PATH}/inst/amd64-linux.gcc-pthreads/bin/${AddParsecBenchmark_EXE}
    ${executable})

  if (NOT "x${AddParsecBenchmark_ARCHIVE}x" STREQUAL "xx")
    add_custom_command(TARGET bench-${benchmark} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E
      tar xzf ${PARSEC_APP_PATH}/${AddParsecBenchmark_PATH}/inputs/${AddParsecBenchmark_ARCHIVE}
      WORKING_DIRECTORY ${destination})
  endif()

  add_custom_target(bench-${benchmark}-pthread
    COMMAND ${CMAKE_COMMAND} -E
    env ${AddParsecBenchmark_ENV} ${CMAKE_COMMAND} -E
    time ./${executable} ${AddParsecBenchmark_ARGS}
    DEPENDS bench-${benchmark})

  add_custom_target(bench-${benchmark}-tthread
    COMMAND ${CMAKE_COMMAND} -E
    env ${AddParsecBenchmark_ENV} "LD_PRELOAD=${PROJECT_SOURCE_DIR}/src/libtthread.so" ${CMAKE_COMMAND} -E
    time ./${executable} ${AddParsecBenchmark_ARGS}
    DEPENDS bench-${benchmark})
endfunction()

AddParsecBenchmark(blackscholes
  ARGS 8 in_10M.txt prices.txt
  PATH apps/blackscholes
  ARCHIVE input_native.tar)

if(${NCORES} EQUAL 8)
  set(CANNEAL_THREADS 7)
  set(DEDUP_THREADS 2)
elseif(${NCORES} EQUAL 4)
  set(CANNEAL_THREADS 3)
  set(DEDUP_THREADS 1)
else() # EQUAL 2
  set(CANNEAL_THREADS 1)
  set(DEDUP_THREADS 1)
endif()

add_definitions(-DENABLE_THREADS -DPARALLEL)
AddParsecBenchmark(canneal
  ARGS ${CANNEAL_THREADS} 15000 2000 ${TEST_PATH}/canneal/400000.nets 128
  PATH kernels/canneal
  ARCHIVE input_simlarge.tar)

AddParsecBenchmark(dedup
  ARGS -c -p -t ${DEDUP_THREADS} -i ${TEST_PATH}/vips/FC-6-x86_64-disc1.iso -o output.dat.ddp
  PATH kernels/dedup
  ARCHIVE input_native.tar)

AddParsecBenchmark(ferret
  ARGS ${TEST_PATH}/ferret/corel lsh ${TEST_PATH}/ferret/queries 10 20 1 output.txt
  PATH apps/ferret
  ARCHIVE input_native.tar)

AddParsecBenchmark(swaptions
  ARGS -ns 128 -sm 50000 -nt ${NCORES}
  PATH apps/swaptions)

AddParsecBenchmark(streamcluster
  ARGS 10 20 128 16384 16384 1000 none output.txt ${NCORES}
  PATH kernels/streamcluster)

AddParsecBenchmark(vips
  ARGS im_benchmark ${TEST_PATH}/vips/orion_18000x18000.v output.v
  ENV IM_CONCURRENCY=${NCORES}
  PATH apps/vips
  ARCHIVE input_native.tar)

AddParsecBenchmark(raytrace
  ARGS ${TEST_PATH}/raytrace/thai_statue.obj -automove -nthreads ${NCORES} -frames 200 -res 1920 1080
  PATH apps/raytrace
  EXE rtview
  ARCHIVE input_native.tar)

#add_custom_target(bench-raytrace-tthread
#  COMMAND ${benchmark}-tthread ${AddBenchmark_ARGS})
#
#add_custom_target(bench-${benchmark}-pthread
#  COMMAND ${benchmark}-pthread ${AddBenchmark_ARGS})
#
#AddBenchmark(raytrace
#  ARGS ${DATASET_HOME}/reverse_index_datafiles
#  FILES reverseindex-pthread.c
#  INCLUDES ../../include
#  LIBS phoenix)
