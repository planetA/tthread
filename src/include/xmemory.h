// -*- C++ -*-

/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

/*
 * @file   xmemory.h
 * @brief  Manage different kinds of memory, main entry for memory.
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @author Tongping Liu <http://www.cs.umass.edu/~tonyliu>
 * @author Charlie Curtsinger <http://www.cs.umass.edu/~charlie>
 */

#ifndef _XMEMORY_H_
#define _XMEMORY_H_

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#if !defined(_WIN32)
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <ucontext.h>
# include <unistd.h>
#endif // if !defined(_WIN32)

#include <cstddef>
#include <set>

#include "debug.h"

// Heap Layers
#include "accessedmmappages.h"
#include "heaplayers/stlallocator.h"
#include "internalheap.h"
#include "mmappage.h"
#include "objectheader.h"
#include "tthread/log.h"
#include "warpheap.h"
#include "xdefines.h"
#include "xglobals.h"
#include "xheap.h"
#include "xmmap.h"
#include "xoneheap.h"
#include "xpageentry.h"

class xlogger;
template<class SourceHeap>class xoneheap;
template<unsigned long Size>class xheap;

// Encapsulates all memory spaces (globals & heap).

class xmemory {
private:

  // instance for signal handling
  static xmemory *_instance;

  /// The globals region.
  xglobals _globals;

  /// Protected heap.
  warpheap<xdefines::NUM_HEAPS, xdefines::PROTECTEDHEAP_CHUNK,
           xoneheap<xheap<xdefines::PROTECTEDHEAP_SIZE> > >_pheap;

  /// A signal stack, for catching signals.
  stack_t _sigstk;

  int _heapid;

  xmmap _mmap;
  accessedmmappages _accessedmmappages;
  xlogger *_logger;

public:

  void initialize(xlogger& logger) {
    DEBUG("initializing xmemory");
    _logger = &logger;

    // Intercept SEGV signals (used for trapping initial reads and
    // writes to pages).
    installSignalHandler();

    // Call _pheap so that xheap.h can be initialized at first and then can work
    // normally.
    _pheap.initialize(logger);
    _globals.initialize(logger);
    _mmap.initialize(logger);
    xpageentry::getInstance().initialize();

    // Initialize the internal heap.
    InternalHeap::getInstance().initialize();

    // initialize memory protection so page access can be tracked from the
    // beginning
    setCopyOnWrite(false);
  }

  void finalize() {
    _globals.finalize();
    _pheap.finalize();
  }

  void setThreadIndex(int id) {
    _globals.setThreadIndex(id);
    _pheap.setThreadIndex(id);

    // Calculate the sub-heapid by the global thread index.
    _heapid = id % xdefines::NUM_HEAPS;
  }

  inline void *malloc(size_t sz) {
    void *ptr = _pheap.malloc(_heapid, sz);

    return ptr;
  }

  inline void *realloc(void *ptr, size_t sz) {
    size_t s = getSize(ptr);
    void *newptr = malloc(sz);

    if (newptr) {
      size_t copySz = (s < sz) ? s : sz;
      memcpy(newptr, ptr, copySz);
    }
    free(ptr);
    return newptr;
  }

  inline void free(void *ptr) {
    return _pheap.free(_heapid, ptr);
  }

  /// @return the allocated size of a dynamically-allocated object.
  inline size_t getSize(void *ptr) {
    // Just pass the pointer along to the heap.
    return _pheap.getSize(ptr);
  }

  inline tthread::memorylayout_t getLayout() {
    tthread::memorylayout_t layout;

    layout.heapStart = _pheap.getAbsoluteStart();
    layout.heapEnd = _pheap.getAbsoluteEnd();
    layout.globalStart = (void *)GLOBALS_START;
    layout.globalEnd = (void *)GLOBALS_END;
    return layout;
  }

  void setCopyOnWrite(bool copyOnWrite) {
    _globals.setCopyOnWrite(NULL, copyOnWrite);
    _pheap.setCopyOnWrite(_pheap.getend(), copyOnWrite);
  }

  void closeProtection(void) {
    _globals.closeProtection();
    _pheap.closeProtection();
  }

  inline void begin() {
    // Reset global and heap protection.
    _globals.begin();
    _pheap.begin();
  }

  void mem_write(void *dest, void *val) {
    if (_pheap.inRange(dest)) {
      _pheap.mem_write(dest, val);
    } else if (_globals.inRange(dest)) {
      _globals.mem_write(dest, val);
    }
  }

  inline void handleAccess(void       *addr,
                           bool       is_write,
                           const void *issuerAddress) {
    if (_pheap.inRange(addr)) {
      _pheap.handleAccess(addr, is_write, issuerAddress);
      return;
    } else if (_globals.inRange(addr)) {
      _globals.handleAccess(addr, is_write, issuerAddress);
      return;
    }

    const mmappage *page = _mmap.find(addr);

    if (page == NULL) {
      // None of the above - something is wrong.
      fprintf(stderr, "%d: wrong faulted address at %p\n", getpid(), addr);
      ::abort();
    } else {
      page->handleAccess(*_logger, addr, is_write, issuerAddress);
      _accessedmmappages.add(addr);
    }
  }

  inline void commit() {
    _pheap.checkandcommit();
    _globals.checkandcommit();
    _accessedmmappages.reset();
  }

  inline void forceCommit(int pid) {
    _pheap.forceCommit(pid, _pheap.getend());
  }

  // Since globals will not owned by one thread, there is no need
  // to cleanup and commit.
  inline void cleanupOwnedBlocks(void) {
    _pheap.cleanupOwnedBlocks();
  }

  inline void commitOwnedPage(int page_no, bool set_shared) {
    _pheap.commitOwnedPage(page_no, set_shared);
  }

  // Commit every page owned by me.
  inline void finalcommit(bool release) {
    _pheap.finalcommit(release);
  }

  void *map(void *addr, size_t length, int prot, int flags, int fd,
            off_t offset) {
    return _mmap.map(addr, length, prot, flags, fd, offset);
  }

  int unmap(void *addr, size_t len) {
    return _mmap.unmap(addr, len);
  }

public:

  /* Signal-related functions for tracking page accesses. */

  /// @brief Signal handler to trap SEGVs.
  static void segvHandle(int signum, siginfo_t *siginfo, void *unused) {
    ASSERT(signum == SIGSEGV);
    void *addr = siginfo->si_addr; // address of access
    ucontext *context = (ucontext *)unused;
    void *pc = (void *)context->uc_mcontext.gregs[REG_RIP];

    // Check if this was a SEGV that we are supposed to trap.
    if (siginfo->si_code == SEGV_ACCERR) {
      // XXX this check is x86_64 specific
      bool is_write = ((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 0x2;
      _instance->handleAccess(addr, is_write, pc);
    } else if (siginfo->si_code == SEGV_MAPERR) {
      backtrace_symbols_fd(&pc, 1, fileno(stdin));
      fprintf(stderr, "%d : map error with addr %p!\n", getpid(), addr);
      ::abort();
    } else {
      backtrace_symbols_fd(&pc, 1, fileno(stdin));
      fprintf(stderr, "%d : other access error with addr %p.\n", getpid(),
              addr);
      ::abort();
    }
  }

  static void signalHandler(int signum, siginfo_t *siginfo, void *context) {
    UNUSED(context);
    ASSERT(signum == SIGUSR1);
    union sigval signal = siginfo->si_value;
    int page_no;

    page_no = signal.sival_int;
    _instance->commitOwnedPage(page_no, true);
  }

  /// @brief Install a handler for SEGV signals.
  void installSignalHandler() {
    xmemory::_instance = this;

#if defined(linux)

    // Set up an alternate signal stack.
    _sigstk.ss_sp = WRAP(mmap)(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    _sigstk.ss_size = SIGSTKSZ;
    _sigstk.ss_flags = 0;
    sigaltstack(&_sigstk, (stack_t *)0);
#endif // if defined(linux)

    // Now set up a signal handler for SIGSEGV events.
    struct sigaction siga;
    sigemptyset(&siga.sa_mask);

    // Set the following signals to a set
    sigaddset(&siga.sa_mask, SIGSEGV);
    sigaddset(&siga.sa_mask, SIGALRM);
    sigaddset(&siga.sa_mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &siga.sa_mask, NULL);

    // Point to the handler function.
#if defined(linux)
    siga.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESTART | SA_NODEFER;
#else // if defined(linux)
    siga.sa_flags = SA_SIGINFO | SA_RESTART;
#endif // if defined(linux)

    siga.sa_sigaction = xmemory::segvHandle;

    if (sigaction(SIGSEGV, &siga, NULL) == -1) {
      perror("sigaction(SIGSEGV)");
      exit(-1);
    }

    // We set the signal handler
    siga.sa_sigaction = xmemory::signalHandler;

    if (sigaction(SIGUSR1, &siga, NULL) == -1) {
      perror("sigaction(SIGUSR1)");
      exit(-1);
    }

    sigprocmask(SIG_UNBLOCK, &siga.sa_mask, NULL);
  }
};

#endif // ifndef _XMEMORY_H_
