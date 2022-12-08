
#!/bin/sh
ip=10.0.6.109
id=root
password=ketilinux
file=/home/keti/BMC_SDK/source/AST2600_BMC/output/bin/KETI-Edge
path=/usr/sbin
sshpass -p $password scp output/bin/KETI-Edge root@10.0.6.110:/tmp/
