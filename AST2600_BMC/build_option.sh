#!/bin/sh
#set -e # bash 에러시 멈춤
#------------ Global Variables -------------
set -x #e
arg_i=2 # interfaces options
arg_c="" # static IP Address
arg_g="" # Static Gateway IP Address
arg_n="" # Static Netmask Address
arg_m=3 #ramdisk cramfs 적용

help() {
    echo "\033[35m./build_option.sh -i []\033[0m"
    echo "\033[35m-i	interfaces 적용 옵션 (0 : DHCP (Default)  / 1 : Static)\033[0m"
    echo "\033[35m-c	BMC ip address\033[0m"
	echo "\033[35m-m	mtd 옵션 (0 : 미적용(Ramdisk) / 1 : 적용(cramfs) / 2 : nfs 적용 (s, n, c 옵션 필요))\033[0m"
    echo "\033[35m-g	BMC gateway ip address\033[0m"
    echo "\033[35m-n	BMC netmask address\033[0m"
    exit 0;
}

err_check_options() 
{
    echo "i is $arg_i"
	echo "m is $arg_m"
	if [ $arg_m -ge 3 ];then
		echo "\033[31m[ERROR] Please select MTD Options\033[0m"
		help;
	fi
		if [ $arg_m -eq 2 ];then
		if [ -z $arg_s ];then
			echo "\033[31m[ERROR] Please insert BMC NFS Server IP Address\033[0m"
			help;
		fi
		if [ -z $arg_c ];then
			echo "\033[31m[ERROR] Please insert BMC NFS Client IP Address\033[0m"
			help;
		fi
		if [ -z $arg_d ];then
			echo "\033[31m[ERROR] Please insert BMC NFS Directory\033[0m"
			help;
		fi
		if [ -z $arg_n ];then
			echo "\033[31m[ERROR] Please insert BMC NFS Netmask\033[0m"
			help;
		fi
		if [ -z $arg_g ];then
			echo "\033[31m[ERROR] Please insert BMC NFS Gateway\033[0m"
			help;
		fi
	fi
	
	if [ $arg_i -eq 1 ];then
		if [ -z $arg_c ];then
			echo "\033[31m[ERROR] Please insert BMC IP Address\033[0m"
			help;
		fi
		if [ -z $arg_g ];then
			echo "\033[31m[ERROR] Please insert BMC Gateway\033[0m"
			help;
		fi
		if [ -z $arg_n ];then
			echo "\033[31m[ERROR] Please insert BMC Netmask\033[0m"
			help;
		fi
    elif [ $arg_i -ne 1 ] && [ $arg_i -ne 0 ]; then
        help;
	fi;
}

copy_config()
{
	case $arg_m in
		0) # Ramdisk File Settings
			cp -R ./source/target_sys/web/ ./source/overlay/
			cp -R ./source/target_sys/conf ./source/overlay/	
			cp -R ./source/target_sys/redfish ./source/overlay/
			cp ./source/target_sys/fstab_dir/fstab_nomtd ./source/overlay/etc/fstab
			## 기존에도 ast-g5를 사용했는지 확인할것
			cp ./source/target_sys/uboot-config/ast-g5_evb_no_mtd.h ./output/build/uboot-custom/include/configs/ast-g5_evb.h
			# cp ./source/target_sys/menuconfig/ext2_menuconfig ./.config
			# cp ./source/target_sys/uboot-ftgmac/ftgmac100_no_mtd.c ./source/u-boot-2017.01/drivers/net/ftgmac100.c
			# cp ./source/target_sys/linux-menuconfig/ext2_menuconfig ./output/build/linux-custom/.config
			 echo "Ramdisk Init"
			cp ./source/ast2600a1-ramfs.its output/images/
			cp ./source/Makefile_allbin output/images/Makefile
			cp ./source/aspeed-ast2600a1-evb.dtb output/images/
		;;
		1) # cramfs File Settings
			## fstab file system tables 설정 부팅시 자동:마운트 
			cp ./source/target_sys/fstab_dir/fstab_mtd ./source/overlay/etc/fstab # cramfs fstab file
			##ast2600 메모리맵 설정
			cp ./source/target_sys/uboot-config/ast-g5_evb_mtd.h ./output/build/uboot-custom/include/configs/ast-g5_evb.h #cramfs u-boot config file
			## 확인중
			cp ./source/target_sys/uboot-ftgmac/ftgmac100_mtd.c ./source/u-boot/drivers/net/ # ftgmac100 u-boot 파일 적용
			#설정하면안됨 설정된 BMC_SDK/.config 가 cramfs임
			#cp ./source/target_sys/menuconfig/cramfs_menuconfig ./.config # Kernel cramfs menuconfig file
			##똑같은이유
			#cp ./source/target_sys/linux-menuconfig/cramfs_menuconfig ./output/build/linux-custom/.config
		;;
		2) # NFS File Settings
		;;
	esac

	case $arg_i in
		0) # DHCP IP Options
            cp ./source/target_sys/interfaces/interfaces_dhcp ./source/overlay/etc/network/interfaces
		;;
		1) # Static IP Options
			sed -i "8s/.*/     address $arg_c/g" ./source/target_sys/interfaces/interfaces_static
		        sed -i "9s/.*/     netmask $arg_n/g" ./source/target_sys/interfaces/interfaces_static
		        sed -i "10s/.*/     gateway $arg_g/g" ./source/target_sys/interfaces/interfaces_static
            cp ./source/target_sys/interfaces/interfaces_static ./source/overlay/etc/network/interfaces
		;;
	esac;
}

check_directories()
{
	# Delete overlay and output directory


    if test -e ./output/images/var.jffs2
        then
                rm -r ./output/images/var.jffs2 > /dev/null 2>&1
        fi
        if test -e ./output/images/etc.jffs2
        then
                rm -r ./output/images/etc.jffs2 > /dev/null 2>&1
        fi
        if test -e ./output/images/conf.jffs2
        then
                rm -r ./output/images/conf.jffs2 > /dev/null 2>&1
        fi
        if test -e ./output/images/back_conf.jffs2
        then
                rm -r ./output/images/back_conf.jffs2 > /dev/null 2>&1
		fi
		if test -e ./output/images/redfish.jffs2
        then
                rm -r ./output/images/redfish.jffs2 > /dev/null 2>&1
		fi

    rm -r ./output/images/* > /dev/null 2>&1

#테스트기간 임시제거

	if [ ! -d ./output/build/uboot-custom ];then
		echo "\033[33m[WARNING] U-Boot is not exist. Start to build U-boot\033[0m"
		make uboot-rebuild
	fi
	if [ ! -d ./output/build/linux-custom ];then
		echo "\033[33m[WARNING] Kernel is not exist. Start to build Kernel\033[0m"
		make linux-rebuild
	fi;


}

#-----main--
echo "\033[34mGet Build Options...\033[0m"
while getopts c:d:g:i:m:n:s: opt
do
	case $opt in
		c)
			arg_c=$OPTARG
		;;
		d)
			arg_d=$OPTARG
		;;
		g)
			arg_g=$OPTARG
		;;
		i)
			arg_i=$OPTARG
		;;
		m)
			arg_m=$OPTARG
		;;
		n)
			arg_n=$OPTARG
		;;
		s)
			arg_s=$OPTARG
		;;
	esac
done

echo "\033[34mError Check ...\033[0m"
err_check_options;
echo "\033[34mCheck Directories ...\033[0m"
check_directories;
echo "\033[34mCopy Configs and overlay ...\033[0m"
copy_config;

echo "\033[34mRebuild buildroot ...\033[0m"
make uboot-rebuild 
make linux-rebuild
make -j8


echo "\033[34mCreate cross compiler directory ...\033[0m"
mkdir -p ./output/host/usr/lib > /dev/null 2>&1
mkdir -p ./output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/lib > /dev/null 2>&1
mkdir -p ./output/host/usr/arm-buildroot-linux-gnueabi/sysroot/lib > /dev/null 2>&1

#테스트 기간 임시 제거
#echo "\033[34mKETI-Edge build ....\033[0m"
#./source/AST2600_BMC/ast2600_bmc_build.sh


echo "\033[34mCopy overlay files...\033[0m"
mkdir -p ./source/overlay/etc
# 아래는 확인 필요함 kcp(2021-12-30)
# cp ./source/target_sys/localtime ./source/overlay/etc/localtime
# cp ./source/target_sys/S40network ./source/overlay/etc/init.d/S40network
#init tab이 무엇인지 확인필요
# cp ./source/target_sys/inittab ./source/overlay/etc/inittab


##테스트를 위한 임시 제거

echo "\033[34mRemove Existing skeleton and output files...\033[0m"
find ./output/build -name ".stamp_target_installed" |xargs rm -rf

echo "\033[34mRebuild buildroot ...\033[0m"
make -j8



# cp ./source/ast2600a1-ramfs.its output/images/
# cp ./source/Makefile_allbin output/images/Makefile
# cp ./source/aspeed-ast2600a1-evb.dtb output/images/
# cd ./output/images/ && make

case $arg_m in 0)
		#ramdisk
                if [ ! -f ./Makefile_allbin ]
                then
                        echo "\033[41mMakefile_allbin is missing!\033[0m"
                        exit 0
                fi
                cp Makefile_allbin ./output/images/Makefile
        ;;
        1)
		#cramfs
                if [ ! -f ./Makefile_allbin_mtd ]
                then
                        echo "\033[41mMakefile_allbin is missing!\033[0m"
                        exit 0
                fi
                cp Makefile_allbin_mtdtest ./output/images/Makefile
        ;;
        2)
		#nfs는 미구현 cramfs 부터 수행된이후에 설정할 예정
                if [ ! -f ./Makefile_allbin_nfs ]
                then
                        echo "\033[41mMakefile_allbin is missing!\033[0m"
                        exit 0
                fi
                cp Makefile_allbin_nfs ./output/images/Makefile
        ;;
esac

echo "\033[34mCopy all.bin image source files\033[0m"
cd ./output/images
# rm -f rootfs.ext2.gz


#cd ./output/images
if [ $arg_m -eq 1 ];then

    echo "\033[34mMake var, etc jffs2 images...\033[0m"
	#jffs2(파일디렉토리/ 및 변경로그 파일시스템 생성)
	#web 
   ../build/host-mtd-2.1.2/mkfs.jffs2 -l -e 0x10000 -c 0x10 -d  ../../source/target_sys/web/ -o var.jffs2
   #etc
   ../build/host-mtd-2.1.2/mkfs.jffs2 -l -e 0x10000 -c 0x10 -d ../target/etc/ -o etc.jffs2
   #conf
   ../build/host-mtd-2.1.2/mkfs.jffs2 -l -e 0x10000 -c 0x10 -d ../target/conf/ -o conf.jffs2
   #redfish jffs2 생성
   ../build/host-mtd-2.1.2/mkfs.jffs2 -l -e 0x10000 -c 0x10 -d ../target/redfish/ -o redfish.jffs2

    echo "\033[34mChecking for jffs2 image size...\033[0m"

    conf_www_size=$(du -s ../../source/target_sys/web  | awk '{print $1}')
    etc_size=$(du -s ../target/etc | awk '{print $1}')
    rootfs_size_int=$(ls -l | grep rootfs.cramfs | awk '{print $5}')

    rootfs_temp=$(printf "0x%x" $rootfs_size_int)
    rootfs_temp_and=$(($rootfs_temp & 0xffff))
    if [ $rootfs_temp_and -gt 1 ];
    	then
       		rootfs_add_value=$((0x10000 - $rootfs_temp_and))
        	rootfs_final_value=$(($rootfs_temp + $rootfs_add_value))
        	rootfs_size_hex=$(printf "%x" "$rootfs_final_value")
    	else
        	rootfs_size_hex=$(printf "%x" "$rootfs_size_int")
    fi
	echo "conf_www_size = $conf_www_size"
	if [ $conf_www_size -le 2559 ];
	then
		final_conf_www_size=2818048
    elif [ $conf_www_size -gt 2560 ] && [ $conf_www_size -le 2752 ];
    then
        final_conf_www_size=2818048
    elif [ $conf_www_size -gt 2752 ] && [ $conf_www_size -le 3072 ];
    then
        final_conf_www_size=3145728
    elif [ $conf_www_size -gt 3072 ] && [ $conf_www_size -le 3584 ];
    then
        final_conf_www_size=3670016
    elif [ $conf_www_size -gt 3584 ] && [ $conf_www_size -le 4032 ];
    then
        final_conf_www_size=4063232
    elif [ $conf_www_size -gt 4032 ] && [ $conf_www_size -le 4096 ];
    then
        final_conf_www_size=4194304
    elif [ $conf_www_size -gt 4096 ] && [ $conf_www_size -le 4618 ];
    then
        final_conf_www_size=4718592
    elif [ $conf_www_size -gt 4618 ] && [ $conf_www_size -le 5120 ];
    then
        final_conf_www_size=5242880
    elif [ $conf_www_size -gt 5120 ] && [ $conf_www_size -le 5632 ];
    then
        final_conf_www_size=5767168

	else 
		final_conf_www_size=57671680
	fi

	if [ $etc_size -le 1343 ];
	then
		final_etc_size=1507328
    elif [ $etc_size -gt 1344 ] && [ $etc_size -le 1472 ];
    then
        final_etc_size=1507328
    elif [ $etc_size -gt 1472 ] && [ $etc_size -le 2048 ];
    then
        final_etc_size=2097152
    elif [ $etc_size -gt 2048 ] && [ $etc_size -le 2560 ];
    then
        final_etc_size=2621440
    elif [ $etc_size -gt 2560 ] && [ $etc_size -le 3072 ];
    then
        final_etc_size=3145728
    elif [ $etc_size -gt 3072 ] && [ $etc_size -le 3584 ];
    then
        final_etc_size=3670016
    fi


	echo "final_etc_size  =$final_etc_size"
    conf_offset_int=4194304
    conf_offset_hex=$(printf "%x\n" $conf_offset_int)
    conf_size_int=1048576
    conf_size_hex=$(printf "%x\n" $conf_size_int)

    backconf_offset_int=$(expr $conf_offset_int + $conf_size_int)
    backconf_offset_hex=$(printf "%x\n" $backconf_offset_int)
    backconf_size_int=1048576
    backconf_size_hex=$(printf "%x\n" $backconf_size_int)

    conf_www_offset_int=$(expr $backconf_offset_int + $backconf_size_int)
    conf_www_offset_hex=$(printf "%x\n" $conf_www_offset_int)
    conf_www_size_hex=$(printf "%x\n" $final_conf_www_size)

    etc_offset_int=$(expr $conf_www_offset_int + $final_conf_www_size)
    etc_offset_hex=$(printf "%x\n" $etc_offset_int)
    etc_size_hex=$(printf "%x\n" $final_etc_size)

    rootfs_offset_int=$(expr $final_etc_size + $etc_offset_int)
    rootfs_offset_hex=$(printf "%x\n" $rootfs_offset_int)


    echo "\033[34mEdit Kernel Platform device file  size...\033[0m"

    sed -i "278s/.*/                .offset       = 0x$conf_offset_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "279s/.*/                .size       = 0x$conf_size_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "283s/.*/                .offset     = 0x$backconf_offset_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "284s/.*/                .size       = 0x$backconf_size_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "288s/.*/                .offset     = 0x$conf_www_offset_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "289s/.*/                .size       = 0x$conf_www_size_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "293s/.*/                .offset       = 0x$etc_offset_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "294s/.*/                .size       = 0x$etc_size_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "298s/.*/                .offset       = 0x$rootfs_offset_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
    sed -i "299s/.*/                .size       = 0x$rootfs_size_hex,/g" ../build/linux-custom/arch/arm/plat-aspeed/dev-spi.c
	

	cd ../../
	echo "\033[34mKernel Image rebuild process...\033[0m"
    make linux-rebuild
fi


if [ $arg_m -eq 0 ];
then
    cd ../../
fi


cd ./output/images/

kernel_size_int=$(ls -l | grep zImage | awk '{print $5}')
kernel_size_hex=$(printf "0x%x\n" $kernel_size_int)

#0x3049a6c =32mb 짜리 flash
echo "/m0:0x2000000/" > ./all.mtd #0x2000000 256 mb
echo "/m1:0x60000/" > ./uboot.mtd
echo "/m2:0x20000/" > ./env.mtd
echo "/m3:0x380000/" > ./kernel.mtd
echo "/m4:0x100000/" > ./conf.mtd
echo "/m5:0x100000/" > ./redfish.mtd
echo "/m6:0x$conf_www_size_hex/" > ./www.mtd
echo "/m7:0x$etc_size_hex/" > ./etc.mtd
echo "/m8:0x$rootfs_size_hex/" >./rootfs.mtd

dd if=/dev/zero of=mtd.bin bs=1K count=64
dd if=all.mtd of=mtd.bin bs=1 seek=0 conv=notrunc
dd if=uboot.mtd of=mtd.bin bs=1 seek=16 conv=notrunc
dd if=env.mtd of=mtd.bin bs=1 seek=32 conv=notrunc
dd if=kernel.mtd of=mtd.bin bs=1 seek=48 conv=notrunc
dd if=conf.mtd of=mtd.bin bs=1 seek=64 conv=notrunc
dd if=redfish.mtd of=mtd.bin bs=1 seek=80 conv=notrunc
dd if=www.mtd of=mtd.bin bs=1 seek=96 conv=notrunc
dd if=etc.mtd of=mtd.bin bs=1 seek=112 conv=notrunc
dd if=rootfs.mtd of=mtd.bin bs=1 seek=128 conv=notrunc

echo "\033[34mMake all.bin Image process...\033[0m"


cp ./source/ast2600a1-ramfs.its output/images/
make all

if [ -f ./all.bin ]; then
        echo "\033[34mAll job is finished!\033[0m"
        ls -hl ./all.bin
        exit 0
fi
