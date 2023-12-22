#!/bin/sh

USER=root
SERVER_IP=10.0.6.16
PW=0penBmc
DATE=$(date "+%Y-%m-%d")
SAVE_DIR=/lib/firmware/



west build -b ast2600_evb samples/modules/tensorflow/hello_world --pristine
#west build -b ast2600_evb samples/modules/tensorflow/magic_wand --pristine

mv build/zephyr/zephyr.bin .
mv zephyr.bin ast2600_ssp.bin


expect <<EOF
 set timeout 3
 spawn scp -o StrictHostKeyChecking=no ast2600_ssp.bin $USER@$SERVER_IP:$SAVE_DIR
 expect "password"
 send "$PW\r"
 expect eof
EOF
#scp ast2600_ssp.bin root@10.0.4.141:/lib/firmware

