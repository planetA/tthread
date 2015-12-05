#!/usr/bin/env bash

# set -x

ROOT=$1
TESTS=$ROOT/eval/tests
DATASETS=$ROOT/eval/datasets
declare -a APPS=("blackscholes" "canneal" "dedup" "streamcluster" "swaptions")

cd $ROOT/eval/parsec-3.0

source env.sh

parsecmgmt -a build -p ${APPS[@]} -c gcc-pthreads

for i in ${APPS[@]}
do
    find . -name $i -executable -type f -print -quit | xargs cp -t $TESTS/$i/
done


# Get small inputs

parsecmgmt -a run -p ${APPS[@]} -c gcc-pthreads -i simlarge

DIRS=( $(find . -name run) )

for i in ${DIRS[@]}
do
    APP=$(basename `dirname $i`)
    mkdir $DATASETS/${APP}_datafiles
    cp $i/* $DATASETS/${APP}_datafiles
done

# Get big inputs

parsecmgmt -a run -p ${APPS[@]} -c gcc-pthreads -i native -n 4

DIRS=( $(find . -name run) )

for i in ${DIRS[@]}
do
    APP=$(basename `dirname $i`)
    cp $i/* $DATASETS/${APP}_datafiles
done
