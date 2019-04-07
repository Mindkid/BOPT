# BOPT

Run the script run.sh on the root folder, this script will create the following folders:
  -> ramdisk : folder that contain all map files.
  -> source/tests : folder that contain all the tests for NVRAM.
  -> source/testsOptane : folder that contain all the tests for Optane.
  -> source/graphs : folder that contain a csv with the data of the tests
  
The Source folder has the code of the program, it has an Makefile.
To create all the executables and execute tests run:

    make  OR make all

To run all the tests run:

    make tests

if there are no tests it will print on the command prompt:

    There are no tests to run, please create them using the benchmark executable.
