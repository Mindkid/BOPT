#!/bin/bash

TESTDIR=./tests/*.bopl

make

for f in $TESTDIR
do
  echo "Executing: $f"
  rm -f ../ramdisk/*
  ./boplFileReader.o -r $f
  rm -f ../ramdisk/*
  ./boplFileReaderSTTRAM.o -r $f
done
