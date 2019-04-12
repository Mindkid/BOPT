#!/bin/bash
TESTDIR=./tests/*.bopl
TESTOPTANE=./testsOptane/*.bopl

help()
{
  echo "Use the commands:"
  echo "    -a for all tests"
  echo "    -o for optane"
  echo "    -r for ram"
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
}

runOptane()
{
  echo "OPTANE - TESTING"
  for f in $TESTOPTANE
  do
    echo "Executing: $f"
    rm -f ../ramdisk/*
    ./boplFileReaderOPTANE.o -r $f
  done
}



while getopts ":aor" o; do
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
        *)
            help
            ;;
    esac
done

rm -f ../ramdisk/*
