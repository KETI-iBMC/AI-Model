#!/bin/sh

ip=10.0.6.109
id=root
password=ketilinux
path=/firmware

#ssh-keygen -f "/root/.ssh/known_hosts" -R "$ip"
#ssh-keyscan -t rsa $ip >>~/.ssh/known_hosts


Ibmc=output/bin/KETI-Ibmc
kvm=video_old/KETI-KVM

#Ibmc build
cmake CMakeLists.txt
make -j 30 

#kvm build
cd video_old
cmake CMakeLists.txt
make
cd ../

#wdt build
cd KETI-d
cmake CMakeLists.txt
make
cd ../





echo " overlay copy ..."
cp -f ./output/bin/* ../target_sys/firmware/
echo "scp ..."

# sshpass -p $password scp $Ibmc $rest $kvm $smltr $id@$ip:$path
#sshpass -p $password scp $Ibmc $kvm $smltr $id@$ip:$path
#sshpass -p $password scp $Ibmc $id@$ip:$path

