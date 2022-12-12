#!/bin/sh
filename=ipmbtest
toolchain=/home/keti/BMC_SDK/output/host/usr/bin/arm-linux-gcc
version=.c
obj=.o

$toolchain -o $filename $filename$version -lpthread



#/home/keti/Workspace/buildroot/source/armv6-aspeed-linux-gnueabi/bin/armv6-aspeed-linux-gnueabi-g++
#/home/keti/BMC_SDK/output/host/usr/bin/arm-linux-g++ server.cpp
