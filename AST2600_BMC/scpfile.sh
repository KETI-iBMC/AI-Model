
#!/bin/sh
ip=10.0.6.104
id=root
password=ketilinux
path=/usr/sbin
sshpass -p $password scp KETI-REST/KETI-REST root@10.0.6.104:/usr/sbin
