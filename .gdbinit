#symbol-file src/libdthread.so
handle SIGSEGV nostop

# since tthread uses processes instead of threads, we need to make gdb aware of
# these. `follow-fork-mode child`, will follow new spawned processes instead of
# the parent processes
set follow-fork-mode child

