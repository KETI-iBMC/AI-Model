#------------ Global Variables -------------
arg_i=2 # interfaces options
arg_c="" # static IP Address
arg_g="" # Static Gateway IP Address
arg_n="" # Static Netmask Address

copy_config()
{

        case $arg_i in
                0) # DHCP IP Options
            cp ./source/target_sys/interfaces/interfaces_dhcp ./source/overlay/etc/network/interfaces
                        ;;
                1) # Static IP Options
                        sed -i "11s/.*/     address $arg_c/g" /etc/network/interfaces
                        sed -i "12s/.*/     netmask $arg_n/g" /etc/network/interfaces
                        sed -i "13s/.*/     gateway $arg_g/g" /etc/network/interfaces
                ;;
        esac;
}

echo "\033[34mGet Build Options...\033[0m"

while getopts c:g:i:n: opt
do
        case $opt in
                c)
                        arg_c=$OPTARG
                ;;
                n)
                        arg_n=$OPTARG
                ;;
                g)
                        arg_g=$OPTARG
                ;;
                i)
                        arg_i=$OPTARG
                ;;
        esac
done
echo "\033[34mCopy Configs and overlay ...\033[0m"
copy_config;
echo "\033[34mDone ...\033[0m"
/etc/init.d/S40network restart
