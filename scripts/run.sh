#!/bin/bash

#==============================================================================#
# Run Phoenix benchmarks:
#   - on large inputs
#   - use taskset -c 0-.. to limit number of CPUs
#==============================================================================#

# set -e
#set -x #echo on

ROOT=$(readlink -f $(dirname $(readlink -f $0))/../)
TESTS=$ROOT/eval/tests/
DATASETS=$ROOT/eval/datasets/

#============================== PARAMETERS ====================================#
declare -a benchmarks=( \
  "histogram" \
  "kmeans" \
  "linear_regression" \
  "matrix_multiply" \
  "pca" \
  "string_match" \
  "word_count" \
)

# histogram
# kmeans -- dont need anything
# linear
# matrix multiply (requires created files)
# pca
# string match
# word count
declare -a benchinputs=(\
  "$DATASETS/histogram_datafiles/large.bmp" \
  " " \
  "$DATASETS/linear_regression_datafiles/key_file_500MB.txt" \
  "1500"\
  "-r 3000 -c 3000"\
  "$DATASETS/string_match_datafiles/key_file_500MB.txt"\
  "$DATASETS/word_count_datafiles/word_100MB.txt"\
)

# declare -a threadsarr=(1 2 4 8 12 16)
declare -a threadsarr=(1 2)
#declare -a typesarr=("pthread" "tthread" "avxswift")
declare -a typesarr=("tthread")

#action="/usr/bin/time -p"
action="perf stat -e cycles,instructions -e branches,branch-misses"

#========================== PREPARATION =======================================#
cd $TESTS/matrix_multiply
./matrix_multiply 1500 1

#========================== EXPERIMENT SCRIPT =================================#
echo "===== Results for Phoenix benchmark ====="

# for times in seq 10 ; do
for times in 1; do

  sudo sh -c 'echo 3 >/proc/sys/vm/drop_caches'

  for bmidx in "${!benchmarks[@]}"; do
    bm="${benchmarks[$bmidx]}"
    in="${benchinputs[$bmidx]}"

    cd $ROOT
    # remake the bench

    cd $TESTS
    # dry run to load files into RAM
    echo "--- Dry run for ${bm} (input '${in}') ---"
    cd $TESTS/$bm

    for threads in "${threadsarr[@]}"; do
      for type in "${typesarr[@]}"; do

        echo "--- Running ${bm} ${threads} ${type} (input: '${in}') ---"

        # physical cores start with 0, so be compliant
        realthreads=$((threads-1))
        ${action} taskset -c 0-${realthreads} env LD_PRELOAD=$ROOT/src/libtthread.so ./${bm} ${in}

      done  # type
    done  # threads

  done # benchmarks

done # times
