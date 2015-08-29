handle SIGSEGV nostop
handle SIGCONT nostop
handle SIGUSR1 nostop
#catch signal SIGSEGV

# since tthread uses processes instead of threads, we need to make gdb aware of
# these. `follow-fork-mode child`, will follow new spawned processes instead of
# the parent processes
set follow-fork-mode child
