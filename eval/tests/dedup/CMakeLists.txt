#ifeq ($(NCORES), 8)
#	THREADS = 2
#else
#	THREADS = 1
#endif

#add_benchmark(dedup
#  ARGS -c -p -f -t ${THREADS} -i ${DATASET_HOME}/dedup/media.dat -o output.dat.ddp
#  FILES binheap.c decoder.c dedup.c encoder.c hashtable.c queue.c rabin.c tree.c util.c
#  LIBS ssl z crypto
#  DEFINITIONS -DENABLE_THREADS -DPARALLEL -DMD32_REG_T=int -D_REENTRANT -DL_ENDIAN -DUSE_MMAP -D_XOPEN_SOURCE=600)
