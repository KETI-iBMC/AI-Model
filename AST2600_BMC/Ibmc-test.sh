#!/bin/sh

ip=10.0.6.104
id=root
password=ketilinux
path=/firmware
# path=/tmp

ssh-keygen -f "/root/.ssh/known_hosts" -R "$ip"
ssh-keyscan -t rsa $ip >>~/.ssh/known_hosts


Ibmc=output/bin/KETI-Ibmc
test=output/bin/KETI-Ibmc-Test

cmake CMakeLists.txt
make -j 30 

# mv $Ibmc $test 
echo " overlay copy ..."

# cp $Ibmc ../target_sys/firmware/


echo "scp ..."

# sshpass -p $password scp $test $id@$ip:$path
sshpass -p $password scp $Ibmc $id@$ip:$path
