/**
 @file app.hpp
 @author parkicheol
 @date 30 april 2021
 @brief 본 문서는 x86 서버보드 기술 개발의 KETI-IPMI 소프트웨어에 대한
 설명입니다.\n MC, System Information 소스코드 설명이 포함되어있습니다.
*/

#pragma once
#ifndef __APPS_HPP__
#define __APPS_HPP__

#define ConfigurationFileDir "/etc/ipmiwatchdog.conf"
#define IPMI_VERSION "V2.0"

#define IPMI_DAEMON "Daemon"
#define IPMI_TIMEOUT "Timeout"
#define IPMI_PRETIMEOUT "Pretimeout"
#define IPMI_INTERVAL "Interval"
#define IPMI_PRETIMEOUTINTERRUPT "INT_Pretimeout"
#define IPMI_ACTION "Action"
#define IPMI_PIDFILE "Pidfile"

// rest_show_main_call
#define ALL 0
#define EVENT_LIST 1
#define CPU1_TEMP 2
#define CPU2_TEMP 3
#define BOARD_TEMP 4
#define POWER 5
#define FANS 6
#define SYS 7
#define SUMMARY 8
#define CAPTURE 9

#pragma once
#include <condition_variable>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <mutex>
#include <stdint.h>
#include <string.h>
#include <thread>
#include <vector>

#include "ipmi/channel.hpp"
#include "ipmi/chassis.hpp"
#include "ipmi/common.hpp"
#include "ipmi/efte.hpp"
#include "ipmi/fru.hpp"
#include "ipmi/ipmi.hpp"
#include "ipmi/network.hpp"
#include "ipmi/sdr.hpp"
#include "ipmi/sel.hpp"
#include "ipmi/sensor_define.hpp"
#include "ipmi/session.hpp"
#include "ipmi/setting_service.hpp"
#include "ipmi/user.hpp"
#include <ipmi/dcmi.hpp>
// #include "util/dbus_system.hpp"

using namespace web;
using namespace utility;

#define WAITSECONDS 100





typedef struct {
  uint8_t mc_device_id;
  uint8_t mc_device_rev;
  uint8_t mc_fw_version[2];
  uint8_t mc_ipmi_version;
  uint8_t mc_additional_dev;
  uint8_t mc_mnfr_id[3];
  uint8_t mc_prod_id[2];
  uint8_t mc_aux_fw_version[4];
} mc_device_t;

typedef struct {
  uint8_t channel_number;
#if WORDS_BIGENDIAN
  uint8_t v20_data_available : 1; /* IPMI v2.0 data is available */
  uint8_t __reserved1 : 1;
  uint8_t enabled_auth_types : 6; /* IPMI v1.5 enabled auth types */
#else
  uint8_t enabled_auth_types : 6; /* IPMI v1.5 enabled auth types */
  uint8_t __reserved1 : 1;
  uint8_t v20_data_available : 1; /* IPMI v2.0 data is available */
#endif
#if WORDS_BIGENDIAN
  uint8_t __reserved2 : 2;
  uint8_t kg_status : 1;          /* two-key login status */
  uint8_t per_message_auth : 1;   /* per-message authentication status */
  uint8_t user_level_auth : 1;    /* user-level authentication status */
  uint8_t non_null_usernames : 1; /* one or more non-null users exist */
  uint8_t null_usernames : 1;     /* one or more null usernames non-null pwds */
  uint8_t anon_login_enabled : 1; /* a null-named, null-pwd user exists */
#else
  uint8_t anon_login_enabled : 1; /* a null-named, null-pwd user exists */
  uint8_t null_usernames : 1;     /* one or more null usernames non-null pwds */
  uint8_t non_null_usernames : 1; /* one or more non-null users exist */
  uint8_t user_level_auth : 1;    /* user-level authentication status */
  uint8_t per_message_auth : 1;   /* per-message authentication status */
  uint8_t kg_status : 1;          /* two-key login status */
  uint8_t __reserved2 : 2;
#endif
#if WORDS_BIGENDIAN
  uint8_t __reserved3 : 6;
  uint8_t ipmiv20_support : 1; /* channel supports IPMI v2.0 connections */
  uint8_t ipmiv15_support : 1; /* channel supports IPMI v1.5 connections */
#else
  uint8_t ipmiv15_support : 1; /* channel supports IPMI v1.5 connections */
  uint8_t ipmiv20_support : 1; /* channel supports IPMI v2.0 connections */
  uint8_t __reserved3 : 6;
#endif
  uint8_t oem_id[3];    /* IANA enterprise number for auth type */
  uint8_t oem_aux_data; /* Additional OEM specific data for oem auths */
} channel_auth_cap_t;

typedef struct {
  uint8_t set_in_prog;
  std::string sysfw_ver;
  std::string sys_name;
  std::string pri_os_name;
  std::string present_os_name;
  std::string present_os_ver;
  std::string bmc_url;
  std::string os_hv_url;
} sys_info_param_t;

enum {
  SYS_INFO_PARAM_SET_IN_PROG,
  SYS_INFO_PARAM_SYSFW_VER,
  SYS_INFO_PARAM_SYS_NAME,
  SYS_INFO_PARAM_PRI_OS_NAME,
  SYS_INFO_PARAM_PRESENT_OS_NAME,
  SYS_INFO_PARAM_PRESENT_OS_VER,
  SYS_INFO_PARAM_BMC_URL,
  SYS_INFO_PARAM_OS_HV_URL,
};

/**
 * @brief Transport Command Codes (IPMI/Table H-1)
 *
 */
enum {
  CMD_TRANSPORT_SET_LAN_CONFIG = 0x01,
  CMD_TRANSPORT_GET_LAN_CONFIG = 0x02,
  CMD_TRANSPORT_SET_SOL_CONFIG_PARAMETERS = 0x21,
  CMD_TRANSPORT_GET_SOL_CONFIG_PARAMETERS = 0x22,
};

/**
 * @brief Storage Command Codes (IPMI/Table H-1)
 *
 */
enum {
  CMD_STORAGE_GET_FRUID_INFO = 0x10,
  CMD_STORAGE_READ_FRUID_DATA = 0x11,
  CMD_STORAGE_WRITE_FRUID_DATA = 0x12,
  CMD_STORAGE_GET_SDR_INFO = 0x20,
  CMD_STORAGE_RSV_SDR = 0x22,
  CMD_STORAGE_GET_SDR = 0x23,
  CMD_STORAGE_CLR_SDR_REPO = 0x27,
  CMD_STORAGE_GET_SEL_INFO = 0x40,
  CMD_STORAGE_RSV_SEL = 0x42,
  CMD_STORAGE_GET_SEL = 0x43,
  CMD_STORAGE_ADD_SEL = 0x44,
  CMD_STORAGE_DEL_SEL_ENTRY = 0x46,
  CMD_STORAGE_CLR_SEL = 0x47,
  CMD_STORAGE_GET_SEL_TIME = 0x48,
  CMD_STORAGE_SET_SEL_TIME = 0x49,
  CMD_STORAGE_GET_SEL_UTC = 0x5C,

};

/**
 * @brief Sensor Command Codes (IPMI/Table H-1)
 *
 */
enum {
  CMD_SENSOR_SET_EVENT_RECEIVER = 0x00,
  CMD_SENSOR_GET_EVENT_RECEIVER = 0x01,
  CMD_SENSOR_PLATFORM_EVENT = 0x02,
  CMD_GET_PEF_CAPABILITIES = 0x10,
  CMD_SET_PEF_CONFIG_PARMS = 0x12,
  CMD_GET_PEF_CONFIG_PARMS = 0x13,
  CMD_GET_LAST_PROCESSED_EVT_ID = 0x15,
  CMD_SENSOR_GET_DEVICE_SDR_INFO = 0x20,
  CMD_SENSOR_GET_DEVICE_SDR = 0x21,
  CMD_SENSOR_RES_DEVICE_SDR_REPOSITORY = 0x22,
  CMD_SENSOR_GET_SENSOR_HYSTERESIS = 0x25,
  CMD_SENSOR_SET_SENSOR_THRESHOLD = 0x26,
  CMD_SENSOR_GET_SENSOR_THRESHOLD = 0x27,
  CMD_SENSOR_SET_SENSOR_EVENT_ENABLE = 0x28,
  CMD_SENSOR_GET_SENSOR_EVENT_ENABLE = 0x29,
  CMD_SENSOR_GET_SENSOR_EVENT_STATUS = 0x2b,
  CMD_SENSOR_GET_SENSOR_READING = 0x2d,
};

/**
 * @brief Application Command Codes (IPMI/Table H-1)
 *
 */
enum {
  CMD_APP_GET_DEVICE_ID = 0x01,
  CMD_APP_COLD_RESET = 0x02,
  CMD_APP_WARM_RESET = 0x03,
  CMD_APP_GET_SELFTEST_RESULTS = 0x04,
  CMD_APP_GET_DEVICE_GUID = 0x08,
  CMD_APP_RESET_WDT = 0x22,
  CMD_APP_SET_WDT = 0x24,
  CMD_APP_GET_WDT = 0x25,
  CMD_APP_SET_GLOBAL_ENABLES = 0x2E,
  CMD_APP_GET_GLOBAL_ENABLES = 0x2F,
  CMD_APP_CLEAR_MESSAGE_FLAGS = 0x30,
  CMD_APP_GET_SYSTEM_GUID = 0x37,
  CMD_APP_GET_CHANNEL_AUTH_CAP = 0x38,
  CMD_APP_GET_SESS_CHALLENGE = 0x39,
  CMD_APP_ACTIVATE_SESSION = 0x3a,
  CMD_APP_SET_SESSION_PRIV = 0x3b,
  CMD_APP_CLOSE_SESSION = 0x3c,
  CMD_APP_GET_SESSION_INFO = 0x3d,
  CMD_APP_SET_CHANNEL_ACCESS = 0x40,
  CMD_APP_GET_CHANNEL_ACCESS = 0x41,
  CMD_APP_GET_CHANNEL_INFO = 0x42,
  CMD_APP_GET_USER_NAME = 0x46,
  CMD_APP_GET_USER_ACCESS = 0x44,
  CMD_APP_SET_USER_NAME = 0x45,
  CMD_APP_SET_USER_ACCESS = 0x43,
  CMD_APP_SET_USER_PASSWD = 0x47,
  CMD_APP_SOL_ACTIVE_PAYLOAD = 0x48,
  CMD_APP_SOL_DEACTIVATE_PAYLOAD = 0x49,
  CMD_APP_GET_CIPHER_SUITES = 0x54,
  CMD_APP_SET_SYS_INFO_PARAMS = 0x58,
  CMD_APP_GET_SYS_INFO_PARAMS = 0x59
};

class Ipmiapplication {
private:
  mc_device_t g_mc_device;
  channel_auth_cap_t g_authrsp[4];
  sys_info_param_t g_sysinfo;

public:
  string g_host_name;
  string g_kernel_version;
  Ipmiapplication();

  void app_get_sysinfo_param(uint8_t *request, uint8_t *response,
                             uint8_t *res_len);
  void app_set_sysinfo_param(uint8_t *request, uint8_t *response,
                             uint8_t *res_len);

  void app_set_user_passwd(uint8_t *request, uint8_t *response,
                           uint8_t *res_len);
  void app_set_useraccess(uint8_t *request, uint8_t *response,
                          uint8_t *res_len);
  void app_set_username(uint8_t *request, uint8_t *response, uint8_t *res_len);
  void app_get_useraccess(uint8_t *request, uint8_t *response,
                          uint8_t *res_len);
  void app_get_username(uint8_t *request, uint8_t *response, uint8_t *res_len);

  void app_get_session_info(uint8_t *request, uint8_t *response,
                            uint8_t *res_len);

  void app_bmc_selftest(uint8_t *response, uint8_t *res_len);
  void app_clear_message_flag(uint8_t *request, uint8_t *response,
                              uint8_t *res_len);
  void app_cold_reset(uint8_t *response, uint8_t *res_len);
  void app_warm_reset(uint8_t *response, uint8_t *res_len);
  void app_set_global_enables(uint8_t *request, uint8_t *response,
                              uint8_t *res_len);
  void app_get_global_enables(uint8_t *response, uint8_t *res_len);
  void app_get_device_guid(uint8_t *response, uint8_t *res_len);

  void app_close_session(uint8_t *request, uint8_t *response, uint8_t *res_len);
  void app_set_session_priv(uint8_t *request, uint8_t *response,
                            uint8_t *res_len);
  void app_set_channel_access(uint8_t *request, uint8_t *response,
                              uint8_t *res_len);
  void app_get_channel_access(uint8_t *request, uint8_t *response,
                              uint8_t *res_len);
  void app_get_channel_info(uint8_t *request, uint8_t *response,
                            uint8_t *res_len);
  void app_get_channel_auth_cap(uint8_t *request, uint8_t *response,
                                uint8_t *res_len);
  void app_get_device_id(uint8_t *response, uint8_t *res_len);
  void ipmi_handle_app(uint8_t *request, uint8_t *response, uint8_t *res_len);
};
/**
 * @brief rest 요청을 받는 구조체
 */

int app_getUserPriv(std::string _username, uint8_t index);
std::string app_getUserpassword(uint8_t index);
int app_getUserindex(std::string _username);
uint8_t ipmi_handle(uint8_t bRMCP, uint8_t *request, uint8_t req_len,
                    uint8_t *response, uint8_t *res_len);
void mutex_init(void);
void mutex_destroy(void);
void plat_ipmi_init(void);
void ipmi_handle_rest(rest_req_t *req, uint8_t *response, int *res_len);
int authenticate_ipmi(std::string _username, std::string _password);
int authenticate_ldap(std::string _username, std::string _password);
int authenticate_radius(std::string _username, std::string _password);
int authenticate_ad(std::string _username, std::string _password);
int rest_show_main(int menu, char *ret);
int find_sensor_name(uint8_t sType, uint8_t sNum, string &msg);
/**
 * @brief
 * BMC Kernel Build time을 불러오는 함수입니다.\n
 *
 * @param unsigned char * build_time - BMC Build time 버전 정보를 반환받을
 * 문자열 포인터
 * @return NULL
 */
void get_build_time(unsigned char *build_time);
int new_ipmi_user(string user_name, string password, bool enabled,int priv,int index);

/**
 * @brief REST 요청으로 현재 서버보드 전원 상태를 반환하는 함수\n
 * @param unsigned char *res - JSON 문자열 포인터
 * @return strlen(res) - JSON 문자열 길이
 */
int rest_get_power_status(unsigned char *res);
/**
 * @brief get system info, return json format string & size
 * @param unsigned char *res - JSON 문자열 포인터
 * @date 21.05.21
 * @author doyoung
 */
int rest_get_sysinfo_config(char *res);
/**
 * @brief DNS 설정 정보를 JSON 문자열로 반환하는 함수\n
 * @param unsigned char *res - JSON 문자열 포인터
 * @return strlen(res) - JSON 문자열 길이
 */
int rest_get_ddns_info(unsigned char *res);
/**
 * @brief DNS Hostname 정보를 가져오는 함수 싱크문제없음
 * @param unsigned char *host_name - DNS Hostname
 * @return void
 */
void get_ddns_host_name(unsigned char *host_name);
/**
 * @brief DNS Domain Name 정보를 가져오는 함수 싱크문제없음
 * @param unsigned char *domain_name - DNS Domain Name
 * @return void
 */
void get_ddns_domain_name(unsigned char *domain_name);
/**
 * @brief DNS Nameserver 정보를 가져오는 함수\n
 * @param int flag - Network Interface Flag
 * @param unsigned char *nameserver - DNS Nameserver
 * @return void
 */
void get_ddns_nameserver(int flag, unsigned char *nameserver);
/**
 * @brief DNS Hostname을 설정하는 함수\n
 * @param unsigned char *host_name - DNS Hostname
 */
int set_ddns_host_name(unsigned char *host_name);
/**
 * @brief DNS Domain Name을 설정하는 함수\n
 * @param int flag - Network Interface Flag
 * @param unsigned char *domain_name - DNS Domain name 주소
 * @return 실패시 0\n
 */
int set_ddns_domain_name(int flag, unsigned char *domain_name);
/**
 * @brief DNS Nameserver를 설정하는 함수\n
 * @param int flag - Network Interface Flag
 * @param unsigned char *nameserver - DNS Nameserver 주소
 * @return 실패시 0\n
 */
int set_ddns_nameserver(int flag, unsigned char *nameserver);
/**
 * @brief BMC Network 정보를 JSON 문자열 형태로 반환하는 함수\n
 * @param unsigned char *res - JSON 문자열 포인터
 * @return strlen(res) - JSON 문자열 길이
 */
int rest_get_lan_config(char *res);

int parse_ntp_conf_file(char *res);
int set_ntp_conf_auto(char *server);
/** @brief flag에 따라 REST 요청 처리, Primary receiver, Secondary receiver
 * 주소를 반환하는 함수\n
 *
 * 본 함수는 입력된 flag에 따라 REST 요청을 처리하거나 Primary / Secondary
 * receiver Address를 반환합니다.\n
 *
 * @param char *res - 반환할 메세지를 받을 char형 포인터
 * @param int flag\n
 * 0 : JSON String을 생성하여 REST 요청 처리\n
 * 1 : Primary Server Address를 char *res에 반환\n
 * 2 : Secondary Server Address를 char *res에 반환\n
 * @return int - 0 : Success / 1 : Failed\n
 */
int rest_get_smtp_json(char *res, int flag);
/** @brief SMTP Sender 설정 함수
 *
 * 본 함수는 SMTP Sender를 설정하는 함수입니다.\n
 * SMTP_BIN에 설정정보를 fwrite로 저장합니다.\n
 *
 * @param char *sender - Sender Name\n
 * @param char *machine - Machine Name\n
 * @return 실패시 0\n
 */
int set_smtp_sender_machine(char *sender, char *machine);
/** @brief SMTP Primary 설정 함수
 *
 * 본 함수는 SMTP Primary를 설정정보를 저장하는 함수입니다.\n
 * Primary Receiver의 Server Address, ID, Password를 설정합니다.\n
 *
 * @param char *server - Primary SMTP Server Address\n
 * @param char *id -  Primary Sender ID\n
 * @param char *pwd - Primary Sender Password\n
 * @return 실패시 0\n
 */
int set_smtp_primary_receiver(char *server, char *id, char *pwd);
/** @brief SMTP Secondary 설정 함수
 *
 * 본 함수는 SMTP Secondary 설정정보를 저장하는 함수입니다.\n
 * Secondary SMTP Receiver의 Server Address, ID, Password를 설정합니다.
 *
 * @param char *server - Secondary SvMTP Server Address\n
 * @param char *id -  Secondary Sender ID\n
 * @param char *pwd - Secondary Sender Password\n
 * @return 실패시 0\n
 */
int set_smtp_secondary_receiver(char *server, char *id, char *pwd);
/** @brief SMTP 설정정보를 SMTP Config 파일에 저장하는 함수\n
 *
 * 본 함수는 SMTP 설정정보를 SMTP Config파일인 .msmtprc 파일에 저장하는
 * 함수입니다.\n .msmtp 파일에는 다음과 같은 내용이 포함되어있습니다.\n
 *
 * Machine Name\n
 * default Sender\n
 * Primary Server Address, Account, Password\n
 * Secondary Server Address, Account, Password\n
 *
 * @param void\n
 * @return 실패시 0\n
 */

int write_smtp_config_to_file();
typedef struct {
    unsigned char status;
    unsigned char msg[50];
    unsigned char trigger;
} smtp_arg_t;

int send_alert_with_SMTP(int smtp_cnt, smtp_arg_t* smtp_arr, char* receiver1);

void  prepare_SMTP(void* unused);
typedef struct {
  /// sender : 보내는 사람 이메일 주소
  char sender[32];
  /// machine : machine name
  char machine[32];
  /// server1 : Primary Server Address
  char server1[20];
  /// id1 : Primary Sender 이메일 주소
  char id1[20];
  /// pwd1 : Primary Sender Password
  char pwd1[24];
  /// server2 : Secondary Server Address
  char server2[20];
  /// id2 : Secondary Sender 이메일 주소
  char id2[20];
  /// pwd2 : Secondary Sender Password
  char pwd2[24];
} smtp_config_t;

/**
 * @brief SSL 인증서 정보를 JSON 문자열로 반환하는 함수.\n
 * 본 함수는 SSL 인증서 정보를 파일에서 불러와 JSON 문자열 형태로 반환\n
 * 파일이 존재하지 않거나 내용이 없을 경우 빈 내용을 JSON 문자열 형태로 생성하여
 * 반환\n 저장된 파일 정보는 KETI-Web 에서 클라이언트로부터 SSL 인증서 정보 요청
 * 시 시각화 하여 보여줌\n
 * @param char *res - SSL JSON 문자열을 반환받을 문자열 포인터\n
 * @return strlen(res) - JSON 문자열 길이\n
 */
int parse_ssl_conf_file(unsigned char *res);
/**
 * @brief SSL 인증서 정보를 파일에 저장하는 함수.\n
 * 본 함수는 SSL 인증서 정보를 파일에 저장하는 함수로 입력된 매개변수로부터
 * 정보를 받아 SSL_BIN 파일에 저장\n 저장된 파일 정보는 KETI-Web 에서
 * 클라이언트로부터 SSL 인증서 정보 요청 시 JSON 문자열 형태로 반환.\n
 * @param char *keylen - SSL 인증서 키 길이\n
 * @param char *country - SSL 인증서 국가 코드\n
 * @param char *state_province - SSL 인증서 지역명\n
 * @param char *city_locality - SSL 인증서 도시명\n
 * @param char *organ - SSL 인증서 기관명\n
 * @param char *valid_for - SSL 인증서 VALID 정보\n
 * @return 실패시 0\n
 */
int set_ssl_1(char *keylen, char *country, char *state_province,
              char *city_locality, char *organ, int valid_for);
/**
 * @brief SSL 인증서 정보를 파일에 저장하는 함수.\n
 * 본 함수는 SSL 인증서 정보를 파일에 저장하는 함수로 입력된 매개변수로부터
 * 정보를 받아 SSL_BIN 파일에 저장\n 저장된 파일 정보는 KETI-Web 에서
 * 클라이언트로부터 SSL 인증서 정보 요청 시 JSON 문자열 형태로 반환\n
 * @param char *organ_unit - SSL 인증서 기관 조직명\n
 * @param char *common - SSL 인증서 Common Name\n
 * @param char *email - SSL 인증서 전자메일 주소\n
 * @return 실패시 0\n
 */
int set_ssl_2(char *organ_unit, char *common, char *email);

/**
 * @brief SSL Config 구조체
 *
 */
typedef struct {
  /// 국가명
  unsigned char country[3];
  /// 지역명
  unsigned char state_province[16];
  /// 도시명
  unsigned char city_locality[16];
  /// 기관명
  unsigned char organ[16];
  /// 부서명
  unsigned char organ_unit[16];
  /// Common String
  unsigned char common[16];
  /// Email 주소
  unsigned char email[32];
  /// SSL Key 길이
  unsigned char keylen[6];
  /// Valid from
  unsigned char valid_from[16];
  unsigned char valid_to[16];
  int valid_for;
} ssl_config_t;

/**
 * @brief 현재 날짜와 인증서 만료 날짜를 문자열로 반환하는 함수.\n
 * 본 함수는 localtime 함수를 사용하여 현재 날짜로부터 인증서 만료 날짜를
 * 생성하여 문자열로 반환\n
 * @param int days - 만료일\n
 * @param char *today - 반환할 현재 날짜\n
 * @param char *expire_day - 반환할 인증서 만료 날짜\n
 * @return void
 */
void get_expire_day(int days, char *today, char *expire_day);
/**
 * @brief Active Directory 정보를 파일에서 가져와 JSON 문자열로 생성하는 함수\n
 * 본 함수는 Active Directory 및 LDAP 설정 정보 요청 시 JSON 문자열 형태로
 * 생성하여 반환하는 함수이다.\n get_ad_ldap_info 함수를 호출하여 설정 정보를
 * 불러와 JSON 문자열로 생성한다.\n
 * @param char *res - Active Directory JSON 문자열을 반환받을 문자열 포인터\n
 * @return strlen(res) - JSON 문자열 길이\n
 */
int parse_ad_conf_file(char *res);
/**
 * @brief Active Directory를 활성화 / 비활성화 하는 함수\n
 * 본 함수는 Active Directory 설정을 반영할 때 Active Directory를 활성화 /
 * 비활성화 하는 함수이다.\n 본 함수를 호출하면 LDAP, RADIUS 기능은 비활성화된다
 * (KETI-REST Function Call에 의해 반영)\n
 * @param int enable - Active Directory 활성화 (1) / 비활성화(0)\n
 * @return 실패시 0\n
 */
int set_ad_enable(int enable);
/**
 * @brief Active Directory 설정 정보 (AD 서버 IP, 비밀번호)를 파일에 저장하는
 * 함수\n 본 함수는 Active Directory 설정 정보를 파일에 저장하는 함수이다.\n
 * 입력된 설정 정보는 AD_BIN 파일에 저장되며 클라이언트가 AD 설정정보 요청 시
 * 해당 파일에서 정보를 불러와 반환한다.\n
 * @param char *ip - Active Directory 서버 IP주소\n
 * @param char *pwd - Active Directory 서버 비밀번호\n
 * @return 실패시 0\n
 */
int set_ad_conf_file_ip_pwd(char *ip, char *pwd);
/**
 * @brief Active Directory 설정 정보 (Active Directory Domain)를 파일에 저장하는
 * 함수\n 본 함수는 Active Directory 설정 정보를 파일에 저장하는 함수이다.\n
 * 입력된 설정 정보는 AD_BIN 파일에 저장되며 클라이언트가 AD 설정정보 요청 시
 * 해당 파일에서 정보를 불러와 반환한다.\n
 * @param char *domain - Active Directory 서버 Domain\n
 * @return 실패시 0\n
 */
int set_ad_conf_file_domain(char *domain);
/**
 * @brief Active Directory 설정 정보 (Active Directory 서버 사용자 계정)를
 * 파일에 저장하는 함수\n 본 함수는 Active Directory 설정 정보를 파일에 저장하는
 * 함수이다.\n 입력된 설정 정보는 AD_BIN 파일에 저장되며 클라이언트가 AD
 * 설정정보 요청 시 해당 파일에서 정보를 불러와 반환한다.\n
 * @param char *s_username - Active Directory 서버 사용자 계정\n
 * @return 실패시 0\n
 */
int set_ad_conf_file_username(char *s_username);
/**
 * @brief Active Directory 설정 정보 (Active Directory 서버 사용자 계정)를
 * 파일에 저장하는 함수\n 본 함수는 Active Directory 설정 정보를 파일에 저장하는
 * 함수이다.\n 입력된 설정 정보는 AD_BIN 파일에 저장되며 전체 설정정보를 모아
 * /etc/nslcd.conf 파일에 반영한다.\n 설정이 저장되면 KETI-Web 로그인 시 Active
 * Directory 서버 인증을 통하여 로그인한다.\n
 * @param void\n
 * @return 실패시 0\n
 */
int write_ad_to_file();

typedef struct {
  int enable;
  char dc_ip[32]; // domain controller ip (server ip)
  char domain[32];
  char secret_name[64]; // binddn
  char secret_pwd[32];  // bindpw
} ad_config_t;

/**
 * @brief LDAP 설정 정보를 파일에서 불러와 JSON 문자열로 생성하는 함수\n
 * get_ad_ldap_info()함수를 호출하여 LDAP과 AD 설정 정보를 불러온다.\n
 * 불러온 정보를 기반으로 JSON 문자열로 생성하여 반환한다.\n
 * @param char *res - LDAP JSON 문자열을 반환받을 문자열 포인터\n
 * @return strlen(res) - JSON 문자열 길이\n
 */
int parse_ldap_conf_file(char *res);
/**
 * @brief LDAP 인증 서버 사용을 활성화 / 비활성화하는 함수\n
 * LDAP 서버 사용 시, AD 서버 인증 사용 시 호출되는 함수로 LDAP 서버 사용 유무를
 * 활성화 / 비활성화 한다.\n 설정 정보는 LDAP_BIN 파일에 저장되며, 추후 LDAP
 * 정보 요청 시 해당 정보를 반영하여 불러온다.\n
 * @param int enable - LDAP 활성화 (1) / 비활성화 (0)
 * @return 실패시 0\n
 */
int set_ldap_enable(int enable);
/**
 * @brief LDAP 설정 정보 (LDAP 인증 서버 IP주소)를 파일에 저장하는 함수\n
 * 본 함수는 LDAP 설정 정보를 파일에 저장하는 함수이다.\n
 * 입력된 설정 정보는 LDAP_BIN 파일에 저장되며 클라이언트가 LDAP 설정정보 요청
 * 시 해당 파일에서 정보를 불러와 반환한다.\n
 * @param char *ip - LDAP 인증 서버 IP주소\n
 * @return 실패시 0\n
 */
int set_ldap_ip(char *ip);
/**
 * @brief LDAP 설정 정보 (LDAP 인증 서버 포트번호)를 파일에 저장하는 함수\n
 * 본 함수는 LDAP 설정 정보를 파일에 저장하는 함수이다.\n
 * 입력된 설정 정보는 LDAP_BIN 파일에 저장되며 클라이언트가 LDAP 설정정보 요청
 * 시 해당 파일에서 정보를 불러와 반환한다.\n
 * @param char *port - LDAP 인증 서버 포트번호\n
 * @return 실패시 0\n
 */
int set_ldap_port(char *port);
/**
 * @brief LDAP 설정 정보 (LDAP 인증 서버 Searchbase - Base Domain name)를 파일에
 * 저장하는 함수\n 본 함수는 LDAP 설정 정보를 파일에 저장하는 함수이다.\n 입력된
 * 설정 정보는 LDAP_BIN 파일에 저장되며 클라이언트가 LDAP 설정정보 요청 시 해당
 * 파일에서 정보를 불러와 반환한다.\n
 * @param char *basedn - LDAP 인증 서버 Searchbase\n
 * @return 실패시 0\n
 */
int set_ldap_searchbase(char *basedn);
/**
 * @brief LDAP 설정 정보 (LDAP 인증 서버 Bind Domain name)를 파일에 저장하는
 * 함수\n 본 함수는 LDAP 설정 정보를 파일에 저장하는 함수이다.\n 입력된 설정
 * 정보는 LDAP_BIN 파일에 저장되며 클라이언트가 LDAP 설정정보 요청 시 해당
 * 파일에서 정보를 불러와 반환한다.\n
 * @param char *binddn - LDAP 인증 서버 Bind Domain name\n
 * @return 실패시 0\n
 */
int set_ldap_binddn(char *binddn);
/**
 * @brief LDAP 설정 정보 (LDAP 인증 서버 Bind Password)를 파일에 저장하는 함수\n
 * 본 함수는 LDAP 설정 정보를 파일에 저장하는 함수이다.\n
 * 입력된 설정 정보는 LDAP_BIN 파일에 저장되며 클라이언트가 LDAP 설정정보 요청
 * 시 해당 파일에서 정보를 불러와 반환한다.\n
 * @param char *bindpw - LDAP 인증 서버 Bind Password\n
 * @return 실패시 0\n
 */
int set_ldap_bindpw(char *bindpw);
/**
 * @brief LDAP 설정 정보 (LDAP 인증 서버 SSL 사용 유무)를 파일에 저장하는 함수\n
 * 본 함수는 LDAP 설정 정보를 파일에 저장하는 함수이다.\n
 * 입력된 설정 정보는 LDAP_BIN 파일에 저장되며 클라이언트가 LDAP 설정정보 요청
 * 시 해당 파일에서 정보를 불러와 반환한다.\n
 * @param int ssl - LDAP 인증 서버 SSL 사용 유무 (1 - 활성화 / 0 - 비활성화)\n
 * @return 실패시 0\n
 */
int set_ldap_ssl(int ssl);
/**
 * @brief LDAP 설정 정보 (LDAP 인증 서버 Timeout 시간)를 파일에 저장하는 함수\n
 * 본 함수는 LDAP 설정 정보를 파일에 저장하는 함수이다.\n
 * 입력된 설정 정보는 LDAP_BIN 파일에 저장되며 클라이언트가 LDAP 설정정보 요청
 * 시 해당 파일에서 정보를 불러와 반환한다.\n
 * @param int time - LDAP 인증 서버 Timeout 시간\n
 * @return 실패시 0\n
 */
int set_ldap_timelimit(int time);
/**
 * @brief LDAP 인증서버 설정 정보를 파일에 저장하는 함수\n
 * 본 함수는 LDAP 인증서버 설정 정보를 파일에 저장하는 함수이다.\n
 * LDAP_BIN 파일에서 LDAP 설정 정보를 불러와 /etc/nslcd.conf파일에 반영한다.\n
 * 설정이 저장되면 KETI-Web 로그인 시 LDAP 서버 인증을 통하여 로그인한다.\n
 * @param void\n
 * @return 실패시 0\n
 */
int write_ldap_to_nslcd();

typedef struct {

  int enable;
  char ip[17];
  char port[6];
  char binddn[64];
  char bindpw[32];
  char basedn[32];
  int ssl;
  int timelimit;
} ldap_config_t;

#define RADFILE "/etc/raddb/server"

int parse_radius_conf_file(char *res);
int set_radius_config(char *ip, char *port, char *secret);
int set_radius_disable();

typedef struct {
  unsigned char enable;
  unsigned char ip[16];
  unsigned char port[6];
  unsigned char secret[32];
} rad_config_t;

void get_temp_cpu0(json::value &JCPU);
void get_temp_cpu1(json::value &JCPU);
void get_voltage_fan_power(json::value &JPOWER);
void get_fans(json::value &JFANS);
void get_temp_board(json::value &JBOARD_TEMP);
void get_sys_info(json::value &JSYS_INFO);
void get_health_summary(json::value &jhealth_summary);

/*
 * Editor : KICHEL
 * DATE : 2021-07-12
 * Description : Get watchdog timer functions
 */

static void app_get_watchdog_timer_params(unsigned char *request,
                                          unsigned char *response,
                                          unsigned char *res_len);

static void app_set_watchdog_timer_params(unsigned char *request,
                                          unsigned char *response,
                                          unsigned char *res_len);

static int ReadConfigurationFile(char *file);
static int WriteConfigurationFile(char *file);

/////////////////////////REST WEB 관련
#define BUFF_SIZE 64
#define WATT_ALL 0
#define WATT_TOP 1
#define WATT_MIDDLE 2
#define WATT_BOTTOM_DAY 5
#define WATT_BOTTOM_MIN 3
#define WATT_BOTTOM_HOUR 4

#define LAST_MIN_COUNT_DB 30
#define LAST_HOUR_COUNT_DB 600
#define LAST_DAY_COUNT_DB 1440
#define MAX_LAST_MIN_COUNT 30
#define MAX_LAST_HOUR_COUNT 60
#define MAX_LAST_DAY_COUNT 24

void convert_power_to_g_power(power_usage_t **power_in, int flag);
void get_min_power_usage(int menu);
void get_power_response(int menu, char *ret);
////////////////
void get_power_response(int menu, json::value &response_json);
#endif