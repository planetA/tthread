set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-aliasing -fkeep-inline-functions")
add_benchmark(swaptions
  ARGS -ns 128 -sm 50000 -nt ${NCORES}
  FILES CumNormalInv.cpp MaxFunction.cpp RanUnif.cpp icdf.cpp
  HJM_SimPath_Forward_Blocking.cpp HJM.cpp HJM_Swaption_Blocking.cpp
  HJM_Securities.cpp nr_routines.cpp
  DEFINITIONS ENABLE_THREADS
  LIBS stdc++ m)
