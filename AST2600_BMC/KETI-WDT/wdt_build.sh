#!/bin/sh

ip=10.0.6.104
id=root
password=ketilinux
path=/firmware

ssh-keygen -f "/root/.ssh/known_hosts" -R "$ip"
ssh-keyscan -t rsa $ip >>~/.ssh/known_hosts

wdt=../output/bin/KETI-WDT
cmake CMakeLists.txt
make -j 30 


echo " overlay copy ..."

cp $wdt ../../target_sys/firmware/

echo "scp ..."

#sshpass -p $password scp $wdt $id@$ip:$path
