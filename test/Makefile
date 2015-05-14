LIBS = -lm -lrt -ldl
CFLAGS = -std=c11 -Wall -g -O3 # -T../src/dthreads.ld
CXXFLAGS = -Wall -g -O3 # -T../src/dthreads.ld

DTHREAD_LIBS = $(LIBS) -rdynamic ../src/libdthread.so
PTHREAD_LIBS = $(LIBS) -lpthread

PTHREAD_BINARIES=bug-atom bug-deadlock bug-deadlock1 bug-lockuse bug-order bug-race parallel-sum
DTHREAD_BINARIES=$(PTHREAD_BINARIES:=-det)

all: $(PTHREAD_BINARIES) $(DTHREAD_BINARIES)

lock-dthread: lockowner.c
	$(CXX) $(CFLAGS) -o $@ $< $(DTHREAD_LIBS)

lock-pthread: lockowner.c
	$(CXX) $(CFLAGS) -o $@ $< $(PTHREAD_LIBS)

bug-init: bug-init-32bit.c
	$(CXX) $(CFLAGS) -o $@ $< $(DTHREAD_LIBS)

$(DTHREAD_BINARIES): %-det: %.o
	$(CC) $(CFLAGS) $< -o $@ $(DTHREAD_LIBS)

$(PTHREAD_BINARIES): %: %.o
	$(CC) $(CFLAGS) $< -o $@ $(PTHREAD_LIBS)

clean:
	rm -rf $(PTHREAD_BINARIES) $(DTHREAD_BINARIES)
	rm -rf lock-dthread lock-pthread bug-init
