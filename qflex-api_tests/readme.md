# Qlex-Api Tests

This is the readme file for the qflex-api (trace) tests. 
These tests are designed to traverse all the state transitions in the MESI protocol that is used by flexus.

### Getting started

To boot QEMU and flexus , use the script run.sh.

Examples provided:
  - debian.img :  An image with debian 7.70 pre-installed can be found at "/home/riyer/parsasrv3-homedir/active_qflex"
  - user_new.cfg : A sample user configuration file needed by run.sh
  - qemu_variables.cfg : A sample qemu variables file needed by run.sh
  - user_postload : A sample file to set trace parameters for flexus


Note: The provided user_postload file is configured for a single core. To run the multi-core tests the following parameters along with the corresponding parameter in the user.cfg file must reflect the number of cores being simulated

  - "-L2:CMPWidth"
  - "-bpwarm:cores"
  - "-feeder:CMPwidth"
  
### The tests
The C code for all 4 tests is provided .

Naming convention :

- sc :  Single Core
- mc :  Multi Core

The tests are designed to traverse an array whose size can be specified by the user. Based on the size,number of cores and using only loads or loads+stores we can verify all the transitions in MESI.

### How to run the tests 


The script final_test.sh runs flexus trace over all the sizes and both versions of QEMU and compares the results in a manner that is easy to read . The entries in the versions array will be dicatated by the name of the QEMU folder while those in the size array will be dictated by the names of your snapshots. To understand the given naming conventions, boot flexus once and run "info snapshots" to see the present snapshots on the provided image.

While running both the single and multi core tests , please let the initial "welcome messages" be printed before taking the snapshot. For single core tests the "welcome message" is the one that says "Array initialization done !". This is necessary to avoid the pitfalls of the cow feature of the OS.
In the multicore test, please wait until the array has been initialized and then both threads print their thread id's and the limits of their traversal before taking your snapshot.

Also, while running all the tests, please compile with an optimization level of "O3"


### Results provided


The 4 directories contain results of tests run previously. The name of the test source code and that of the directory match.


