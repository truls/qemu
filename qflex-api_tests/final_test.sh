#!/bin/bash
#Script for automatic testing of QEMU and Flexus

Versions=("old" "new")
Sizes=("4" "8" "16" "32" "64" "128" "1024" "10240")

for size in "${Sizes[@]}"
    do
    for version  in "${Versions[@]}"
       do

          LOADVM_OPT="snapshot_${version}_${size}"
          QEMU_PATH="./qemu_${version}/aarch64-softmmu"
          STATS_FILE="stats_${version}_${size}"
          USER_CFG="user_${version}.cfg"
    
      echo  $LOADVM_OPT
     
    ./run.sh -ty=single -lo=$LOADVM_OPT -uf=$USER_CFG --trace

    ./qflex/flexus/stat-manager/stat-manager -per-node print all >stats/$STATS_FILE

      done

     STATS_FILE_OLD="./stats/stats_${Versions[0]}_${size}"
     STATS_FILE_NEW="./stats/stats_${Versions[1]}_${size}"
     diff -y $STATS_FILE_OLD $STATS_FILE_NEW >./stats/Compare_${size}
     
done
