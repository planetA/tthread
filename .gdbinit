handle SIGSEGV nostop
handle SIGCONT nostop
handle SIGUSR1 nostop
#catch signal SIGSEGV

# since tthread uses processes instead of threads, we need to make gdb aware of
# these. `follow-fork-mode child`, will follow new spawned processes instead of
# the parent processes
set follow-fork-mode child

define sigcont
  signal SIGCONT
end
document sigcont
send signal SIGCONT to process
end

define a
  attach $arg0
end

define preload
  set environment LD_PRELOAD=/home/joerg/git/tthread/src/libtthread.so
end
