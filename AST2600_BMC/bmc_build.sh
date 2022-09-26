#!/bin/sh

ip=10.0.6.109
id=root
password=ketilinux
path=/firmware

ssh-keygen -f "/root/.ssh/known_hosts" -R "$ip"
ssh-keyscan -t rsa $ip >>~/.ssh/known_hosts


edge=output/bin/KETI-Edge
# rest=output/bin/KETI-REST
psu=output/bin/KETI-PSU
WDT=output/bin/KETI-WDT
SERIAL=output/bin/KETI-Serial
smltr=smltr/smltr
kvm=kvm/KETI-KVM

#edge build
cmake CMakeLists.txt
make -j 30 

#kvm build
#cd kvm
#cmake CMakeLists.txt
#make
#cd ../

#wdt build
#cd KETI-WDT
#cmake CMakeLists.txt
#make
#cd ../





echo " overlay copy ..."
#cp ./output/bin/* ../target_sys/firmware/
cp $edge ../target_sys/firmware/
# cp $rest ../target_sys/firmware/
cp $kvm ../target_sys/firmware/
cp $WDT ../target_sys/firmware/
# cp $SERIAL ../target_sys/firmware/
#cp $psu ../overlay/usr/sbin/

echo "scp ..."

# sshpass -p $password scp $edge $rest $kvm $smltr $id@$ip:$path
#sshpass -p $password scp $edge $kvm $smltr $id@$ip:$path
sshpass -p $password scp $edge $id@$ip:$path

