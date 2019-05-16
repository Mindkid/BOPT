#!/bin/bash
TESTDIR=./tests/*.bopl
TESTOPTANE=./testsOptane/*.bopl
TESTHASHMAP=./tests/m:2_*.bopl

help()
{
  echo "Use the commands:"
  echo "    -a for all tests"
  echo "    -o for optane"
  echo "    -r for ram"
  echo "    -s for ssd"
  exit 1;
}

runRam()
{
  echo "RAM - TESTING"
  for f in $TESTDIR
  do
    echo "Executing: $f"
    rm -f ../ramdisk/*
    ./boplFileReader.o -r $f
    rm -f ../ramdisk/*
    ./boplFileReaderSTTRAM.o -r $f
  done
  rm -f ../ramdisk/*
}

runSSD()
{
  echo "SSD - TESTING"
  for t in $TESTDIR
  do
    echo "Executing: $t"
    rm -f ../ramdisk/*
    ./boplFileReaderSSD.o -r $t
  done
  rm -f ../ramdisk/*
}

runOptane()
{
  echo "OPTANE - TESTING"
  for t in $TESTOPTANE
  do
    echo "Executing: $t"
    rm -f /mnt/optane/pmartins/*
    ./boplFileReaderOPTANE.o -r $t
  done
  rm -f /mnt/optane/pmartins/*
}

testHahMap()
{
  echo "BUCKET - TESTING"
  for t in $TESTHASHMAP
  do
    echo "Executing: $t"
    rm -f ../ramdisk/*
    ./hashmap$1.o -r $t 
  done
}

while getopts ":aorsh" o; do
    case "${o}" in
        a)
            make
            runRam
            runOptane
            ;;
        o)
            make
            runOptane
            ;;
        r)
            make
            runRam
            ;;
        s)
            make
            runSSD
            ;;
        h)
            testHahMap 10
            testHahMap 50
            testHahMap 100
            testHahMap 500
            testHahMap 1000
            testHahMap 5000
            ;;
        *)
            help
            ;;
    esac
done
