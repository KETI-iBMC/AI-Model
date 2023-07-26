#pragma once
#define KETI_DEBUG 0
#define HOST_POWER_BTN PIN_GPIOD3
#define HOST_RESET_BTN PIN_GPIOD4
#define SDR_LOG_FILE "/conf/ipmi/sdr.bin"
#define SDR_HEADER_FILE "/conf/ipmi/sdr_hdr.bin"
#define IPMI_FRU_PRODUCT_PATH "/conf/ipmi/fru_product.bin"
#define IPMI_FRU_CHASSIS_PATH "/conf/ipmi/fru_chassis.bin"
#define IPMI_FRU_BOARD_PATH "/conf/ipmi/fru_board.bin"
#define IPMI_FRU_HEADER_PATH "/conf/ipmi/fru_header.bin"
#define FIRST_BOOT_FILE "/conf/first_boot"
#define POLICY_FILE "/conf/ipmi/policy.bin"
#define ALERT_JSON "/conf/ipmi/alert.json"
#define EVENT_FILTER_TABLE_FILE "/conf/ipmi/eft.bin"
#define SOCK_PATH_IPMI "/tmp/ipmi_socket"
#define SET_FILE "/conf/ipmi/service_setting.bin"
#define DCMI_MANDATORY_CAPA_PATH "/conf/dcmi/dcmi_manda.bin"
#define DCMI_OPTION_CAPA_PATH "/conf/dcmi/dcmi_option.bin"
#define DCMI_MGNT_CAPA_PATH "/conf/dcmi/dcmi_mgnt.bin"
#define DCMI_CAPA_PATH "/conf/dcmi/dcmi_capa.bin"
#define DCMI_POWER_LIMIT_PATH "/conf/dcmi/dcmi_power_lmt.bin"
#define DCMI_ASSET_MNGCTRL_PATH "/conf/dcmi/dcmi_asset_mngctrl.bin"
#define DCMI_CONF_PARAM_PATH "/conf/dcmi/dcmi_conf.bin"
#define IPMI_FRU_PATH "/conf/ipmi/fru.bin"
#define IPMI_FRU_JSON_PATH "/conf/ipmi/fru.json"
#define IPMI_LAN_ALERT_PATH "/conf/ipmi/lan_alert.bin"
#define IPMI_LAN_ALERT_DEDI_PATH "/conf/ipmi/lan_alert_dedi.bin"
#define IPMI_USER_PATH "/conf/ipmi/user.bin"
#define IPMI_SYSINFO_PATH "/conf/ipmi/sysinfo.bin"
#define OEM_FAN_PATH "/conf/ipmi/auto_fan.bin"
#define NETWORK_PRIORITY "/conf/ipmi/eth_priority"
#define IPMI_SENSOR_THRESH_PATH "/conf/ipmi/sensor_thresh.bin"
#define SOL_CONFIG_PATH "/conf/ipmi/sol_config.bin"
#define IPMI_GLOBAL_EN_PATH "/conf/ipmi/global_enable.bin"
#define AUTO_FAN_PATH "/conf/ipmi/auto_fan.bin"
#define NETWORK_MAC_ADDR_SHARED "/conf/ipmi/network_mac.bin"
#define NETWORK_MAC_ADDR_DEDI "/conf/ipmi/network_mac_dedi.bin"
#define IPMI_SEL_PATH "/conf/ipmi/sel.bin"
#define SMTPCONF "/etc/msmtprc"
#define SMTP_BIN "/conf/ipmi/smtp.bin"
#define ALERT_SMTP_PATH "/conf/content.txt"
#define RAD_BIN "/conf/ipmi/rad.bin"
#define KVM_PORT_BIN "/conf/ipmi/kvm.bin"
#define ALERT_PORT_BIN "/conf/ipmi/alert_port.bin"
#define SSH_SERVICE_BIN "/conf/ipmi/ssh_port.bin"
#define WEB_PORT_BIN "/conf/ipmi/web_port.bin"
#define NSLCD_FILE "/etc/nslcd.conf"
#define LDAP_BIN "/conf/ipmi/ldap.bin"
#define AD_BIN "/conf/ipmi/ad.bin"
#define S_LAN_STR_BIN "/conf/ipmi/network_shared.bin"
#define D_LAN_STR_BIN "/conf/ipmi/network_dedi.bin"
#define POWER_USAGE_DB "/conf/ipmi/power.db"
#define INTERFACE_FILE "/etc/network/interfaces"
#define DNSSERVER_FILE "/etc/resolv.conf"

#define HOSTNAME_FILE "/etc/hostname"
#define DOMAINNAME_FILE "/etc/hosts"
#define FAIL "\"CODE\":\"400\", \"MESSAGE\":\"FAIL\"}"
#define GET_WEB_PORT                                                           \
  "head -n 35 /etc/nginx/nginx.conf | tail -1 | awk \'{print $2}\' | awk -F "  \
  "\';\' \'{print $1}\'"

#include "ipmi/ipmi.hpp"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int init_ssh_service();
int load_g_setting();
int get_nginx_port();
int get_alert_status();
int init_setting_service();
int init_kvm_service();
int init_alert_service();
int init_web_service();
int set_smtp_port(string port);
int set_ssh_port(string port);
int get_setting_service(char *res);
int set_setting_service(char flag, char *str);
