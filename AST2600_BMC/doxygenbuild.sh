#!/bin/bash
rm doc.tar
doxygen my_conf

tar cvf doc.tar doc
ip=10.0.0.49
id=KETI
password=ketilinux
file=doc.tar
path=C:/Users/keti/CloudStation/doxgen/
sshpass -p $password scp $file $id@$ip:$path
echo "complete scp"
#rm doc.tar
