#!/bin/sh

cd kcs_bt_test
cmake .
make
cd ..
cd mctp
cmake .
make
cd ..
cd mem_utils
cmake .
make
cd ..
cd oob
cmake .
make
cd ..

cd peci
cmake .
make
cd..

cd video
cmake .
make
cd..
echo "xdma"
cd xdma
cmake .
make
cd..
