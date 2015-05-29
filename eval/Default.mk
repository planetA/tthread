TTHREAD_HOME=../../..

NCORES ?= 8

#CC = gcc -m32 -march=core2 -mtune=core2
#CXX = g++ -m32 -march=core2 -mtune=core2
CC = gcc -march=core2 -mtune=core2
CXX = g++ -march=core2 -mtune=core2
CFLAGS += -O5

CONFIGS = pthread tthread
PROGS = $(addprefix $(TEST_NAME)-, $(CONFIGS))

.PHONY: default all clean

default: all
all: $(PROGS)
clean:
	rm -f $(PROGS) obj/*

eval: $(addprefix eval-, $(CONFIGS))

############ pthread builders ############

PTHREAD_CFLAGS = $(CFLAGS)
PTHREAD_LIBS += $(LIBS) -lpthread

PTHREAD_OBJS = $(addprefix obj/, $(addsuffix -pthread.o, $(TEST_FILES)))

obj/%-pthread.o: %-pthread.c
	$(CC) $(PTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

obj/%-pthread.o: %.c
	$(CC) $(PTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

obj/%-pthread.o: %-pthread.cpp
	$(CXX) $(PTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

obj/%-pthread.o: %.cpp
	$(CXX) $(PTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

$(TEST_NAME)-pthread: $(PTHREAD_OBJS)
	$(CC) $(PTHREAD_CFLAGS) -o $@ $(PTHREAD_OBJS) $(PTHREAD_LIBS)

eval-pthread: $(TEST_NAME)-pthread
	time ./$(TEST_NAME)-pthread $(TEST_ARGS) &> /dev/null

############ tthread builders ############

TTHREAD_CFLAGS = $(CFLAGS) -DNDEBUG
TTHREAD_LIBS += $(LIBS) -rdynamic $(TTHREAD_HOME)/src/libtthread.so -ldl

TTHREAD_OBJS = $(addprefix obj/, $(addsuffix -tthread.o, $(TEST_FILES)))

obj/%-tthread.o: %-pthread.c
	$(CC) $(TTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

obj/%-tthread.o: %.c
	$(CC) $(TTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

obj/%-tthread.o: %-pthread.cpp
	$(CC) $(TTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

obj/%-tthread.o: %.cpp
	$(CXX) $(TTHREAD_CFLAGS) -c $< -o $@ -I$(HOME)/include

### FIXME, put the 
$(TEST_NAME)-tthread: $(TTHREAD_OBJS) $(TTHREAD_HOME)/src/libtthread.so
	$(CC) $(TTHREAD_CFLAGS) -o $@ $(TTHREAD_OBJS) $(TTHREAD_LIBS)

eval-tthread: $(TEST_NAME)-tthread
	time ./$(TEST_NAME)-tthread $(TEST_ARGS)
#	time ./$(TEST_NAME)-tthread $(TEST_ARGS) &> /dev/null
