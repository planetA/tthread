#ifeq ($(NCORES), 8)
#	THREADS = 7
#endif
#
#ifeq ($(NCORES), 4)
#	THREADS = 3
#endif
#
#ifeq ($(NCORES), 2)
#	THREADS = 1
#endif

set(THREADS 3)

add_definitions(-DENABLE_THREADS -DPARALLEL)
AddBenchmark(canneal
  ARGS ${THREADS} 15000 2000 ${DATASET_HOME}/canneal/400000.nets 128
  FILES annealer_thread.cpp rng.cpp netlist.cpp main.cpp netlist_elem.cpp
  LIBS m stdc++
  DEFINITIONS -DENABLE_THREADS -DPARALLEL)
