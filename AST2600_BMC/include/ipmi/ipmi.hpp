#ifndef APP_UNIQUE_NAME_HERE
#define APP_UNIQUE_NAME_HERE

#include <arpa/inet.h>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <condition_variable>
#include <cpprest/json.h>
#include <ctime>
#include <errno.h>
#include <iostream>
#include <ipmi/KETI_stdx.hpp>
#include <map>
#include <mutex>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <vector>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
using namespace logging::trivial;

extern src::severity_logger<severity_level> g_logger;
/**
 * @brief Logging macro
 */
#define log(level) BOOST_LOG_SEV(g_logger, level)

using namespace web;
using namespace std;
#define SDR_RECORDS_MAX 128
#define SENSOR_DISC_MAX 8

#define MU_ACCOUNT 0
#define MU_CHASSIS 1
#define MU_SYSTEM 2
#define MU_MANAGER 3
#define MU_SESSION 4
#define SENSOR_MGMT_MAX 1
// network
#define BUFF_SIZE 1024
#define DEV_NAME_SHARED "eth0"
#define DEV_NAME_DEDI "eth1"

// ntp
#define NTPFILE "/etc/ntp.conf"

// ssl
#define SSL_BIN "/conf/ipmi/ssl.bin"

// user
#define MAX_USER 10
#define MAX_PASSWD 40
#define MAX_USERNAME 17
#define MAX_ID 10

//충돌문제발쌩
// /** @brief Lan 변경 횟수 플래그 변수
//  */
// int lc_flag;
// int l_interrupt;
#define THRESH_NOT_AVAILABLE 0xDF
#define BMC_SLAVE_ADDR 0x20
#define SIZE_TIME_STAMP 4
#define SENSOR_STR_SIZE 20
#define secs_from_1970_1996 820454400
#define IPMI_FRU_PRODUCT_PATH "/conf/ipmi/fru_product.bin"
#define IPMI_FRU_CHASSIS_PATH "/conf/ipmi/fru_chassis.bin"
#define IPMI_FRU_BOARD_PATH "/conf/ipmi/fru_board.bin"
#define IPMI_FRU_HEADER_PATH "/conf/ipmi/fru_header.bin"
#define POLICY_FILE "/conf/ipmi/policy.bin"
#define ALERT_JSON "/conf/ipmi/alert.json"

#define EVENT_FILTER_TABLE_ELEMS_MAX 0x7f
#define EVENT_FILTER_TABLE_INDEX_MAX (EVENT_FILTER_TABLE_ELEMS_MAX - 1)
#define BM(valM,valL) 			(((valM&0xC0)<<2)|valL)
#define ABBM(val) 				(val>>9)==1 ? (int16_t)(val|0xFC00) : val
#define ABEXP(exp) 				(exp>>3)==1 ? (int8_t)(exp|0xF0) : exp
#define REXP(exp) 				(exp>>4)
#define BEXP(exp) 				(exp&0xF)
#define IS_UCH_MASKED(mask) 	(mask[1]&0x02) // bit 9
#define IS_UNCH_MASKED(mask) 	(mask[0]&0x80) // bit 7
#define IS_LNCL_MASKED(mask) 	(mask[0]&0x01) // bit 0
#define IS_LCL_MASKED(mask) 	(mask[0]&0x04) // bit 2
# define __TO_M(mtol)       (int16_t)(tos32((((mtol & 0xff00) >> 8) | ((mtol & 0xc0) << 2)), 10))
# define __TO_B(bacc)       (int32_t)(tos32((((bacc & 0xff000000) >> 24) | ((bacc & 0xc00000) >> 14)), 10))
# define __TO_R_EXP(bacc)   (int32_t)(tos32(((bacc & 0xf0) >> 4), 4))
# define __TO_B_EXP(bacc)   (int32_t)(tos32((bacc & 0xf), 4))
#define tos32(val, bits)    ((val & ((1<<((bits)-1)))) ? (-((val) & (1<<((bits)-1))) | (val)) : (val))

/**
 * @brief 로그레벨 : 비활성화
 *
 */
#define LOG_LVL_DISABLE 0
/**
 * @brief 로그레벨 : Trace
 *
 */
#define LOG_LVL_TRACE 1
/**
 * @brief 로그레벨 : Information
 *
 */
#define LOG_LVL_INFO 2
/**
 * @brief 로그레벨 : Warning
 *
 */
#define LOG_LVL_WARNING 3
/**
 * @brief 로그레벨 : All Information
 *
 */
#define LOG_LVL_ALL_INFO 4

typedef struct {
	unsigned char
					state:3,
					count:3,    // count for successive reading fail
					event:1,
					rsvd:1;
}host_status_t ;

#define SMLTR_POWER_OFF 0xB0
#define SMLTR_POWER_ON 0xB1
#define SMLTR_POWER_CYCLE 0xB2
#define SMLTR_POWER_RESET 0xB3
#define SMLTR_SET_FAN 0xB4



// SEL
#define SENSOR_THRESH_MAX (SDR_RECORDS_MAX-(SENSOR_MGMT_MAX+SENSOR_DISC_MAX)) 
#define SEL_RECORDS_MAX 255
#define SEL_ELEMS_MAX (SEL_RECORDS_MAX + 1)
#define SEL_INDEX_MAX SEL_RECORDS_MAX
// host_status_t host_state[SDR_RECORDS_MAX];
// host_status_t dcmi_state[4];
/**
 * @brief REST 요청시 받은 데이터 및 명령어
 *
 */
struct rest_req_t {
  uint8_t netfn_lun;
  uint8_t cmd;
  uint8_t data[128];
};

typedef struct {
  long data_type;
  uint8_t sensor_num;
  uint8_t value;
} smltr_data_t;

typedef struct {
  uint8_t netfn_lun;
  uint8_t cmd;
  uint8_t cc;
  uint8_t data[];
} ipmi_res_t;

typedef struct {
  uint8_t netfn_lun;
  uint8_t cmd;
  uint8_t data[];
} ipmi_req_t;

struct wdt_msq_t{
  long type;
  int flag = 0;
};

typedef struct {
  unsigned char owner;
  unsigned char lun;
  unsigned char sensor_num;
  unsigned char ent_id;
  unsigned char ent_inst;
  unsigned char sensor_init;
  unsigned char sensor_caps;
  unsigned char sensor_type;
  unsigned char evt_read_type;
  unsigned char lt_read_mask[2];
  unsigned char ut_read_mask[2];
  unsigned char set_thresh_mask[2];
  unsigned char sensor_units1;
  unsigned char sensor_units2;
  unsigned char sensor_units3;
  unsigned char linear; /* 70h=non linear, 71h-7Fh=non linear, OEM */
  unsigned char m_val; //__TO_M(sensor->mtol);
  unsigned char m_tolerance;
  unsigned char b_val; //__TO_B(sensor->bacc);
  unsigned char b_accuracy;
  unsigned char accuracy_dir;
  unsigned char rb_exp;
  unsigned char analog_flags;
  unsigned char nominal;
  unsigned char normal_max;
  unsigned char normal_min;
  unsigned char max_reading;
  unsigned char min_reading;
  unsigned char unr_thresh;
  unsigned char uc_thresh;
  unsigned char unc_thresh;
  unsigned char lnr_thresh;
  unsigned char lc_thresh;
  unsigned char lnc_thresh;
  unsigned char pos_hyst;
  unsigned char neg_hyst;
  unsigned char oem;
  unsigned char str_type_len;
  char str[SENSOR_STR_SIZE]; /* name of the sensor */
} sensor_thresh_t;

class IpmiInterface {
public:
  uint8_t netfn;
  uint8_t cmd;
  uint8_t c_code;
  std::vector<uint8_t> data;

  IpmiInterface() {}
};
struct ipmi_rq {
  struct {
    uint8_t netfn : 6;
    uint8_t lun : 2;
    uint8_t cmd;
    uint8_t target_cmd;
    uint16_t data_len;
    uint8_t *data;
  } msg;
};

class IpmiEnvironment {
private:
};

/////////////rest
/*----------REST-----------------*/

#define CMD_GET_LANINFO 0x01
#define CMD_GET_SYSINFO 0x02
#define CMD_GET_DDNSINFO 0x03
#define CMD_GET_FRUINFO 0x04
#define CMD_GET_SENSOR 0x05
#define CMD_GET_EVENT 0x06
#define CMD_GET_BMCINFO 0x07
#define CMD_GET_SYSGUID 0x08
#define CMD_GET_ASSETTAG 0x09

#define CMD_SET_ASSETTAG 0x0A
#define CMD_GET_MC_ID 0x0B
#define CMD_SET_MC_ID 0x0C
#define CMD_SET_POWER_CTL 0x0D
#define CMD_SENSOR_FAN_CTL 0x0E
#define CMD_SET_LAN_MAC_ADDR 0x0F
#define CMD_SET_LAN_IPV4_DHCP 0x10
#define CMD_SET_LAN_IPV4_IP_NETMASK 0x11
#define CMD_SET_LAN_IPV4_GATEWAY 0x12
#define CMD_SET_LAN_VLAN_ENABLE 0x13
#define CMD_SET_LAN_VLAN_ID 0x14
#define CMD_SET_LAN_VLAN_PRIORITY 0x15
#define CMD_SET_LAN_IPV6_ENABLE 0x16
#define CMD_SET_LAN_IPV6_DHCP 0x17
#define CMD_SET_LAN_IPV6_IP 0x18
#define CMD_SET_LAN_IPV6_PREFIX 0x19
#define CMD_SET_LAN_IPV6_GATEWAY 0x1A
#define CMD_SENSOR_SET_THRESH 0x1B

#define CMD_GET_PEF_LIST 0x1C
#define CMD_GET_PEF_POLICY 0x1D
#define CMD_ALERT_TEST 0x1E
#define CMD_SET_PEF_POLICY 0x1F
#define CMD_SET_LAN_ALERT 0x20
#define CMD_DEL_PEF_POLICY 0x21

#define CMD_GET_LAN_DESTINATION 0x22
#define CMD_GET_USER_LIST 0x24
#define CMD_SET_USER_NAME 0x25
#define CMD_SET_USER_PASSWORD 0x26
#define CMD_SET_USER_ACCESS 0x27
#define CMD_DEL_USER 0x28
#define CMD_GET_SMTPINFO 0x29
#define CMD_SET_SMTP_SENDER 0x2A
#define CMD_SET_SMTP_PRIMARY 0x2B
#define CMD_SET_SMTP_SECONDARY 0x2C
#define CMD_GET_POWER_STATUS 0x2D
#define CMD_SET_POWER_STATUS 0x2E

#define CMD_GET_LDAP 0x2F
#define CMD_SET_LDAP_ENABLE 0x30
#define CMD_SET_LDAP_IP 0x31
#define CMD_SET_LDAP_PORT 0x32
#define CMD_SET_LDAP_SEARCHBASE 0x33
#define CMD_SET_LDAP_BINDDN 0x34
#define CMD_SET_LDAP_PASSWORD 0x35
#define CMD_SET_LDAP_SSL 0x36
#define CMD_SET_LDAP_TIMELIMIT 0x37

#define CMD_GET_DNS 0x38
#define CMD_SET_DNS_DOMAIN 0x39
#define CMD_SET_DNS_HOSTNAME 0x3A
#define CMD_SET_DNS_IPV4_PREFER 0x3B
#define CMD_SET_DNS_IPV4_ALTER 0x3C
#define CMD_SET_DNS_IPV6_PREFER 0x3D
#define CMD_SET_DNS_IPV6_ALTER 0x3E

#define CMD_GET_NTP 0x3F
#define CMD_SET_NTP_AUTO 0x40

#define CMD_GET_SSL 0x41
#define CMD_SET_SSL_1 0x42
#define CMD_SET_SSL_2 0x43
#define CMD_GET_RADIUS 0x44
#define CMD_SET_RADIUS 0x45
#define CMD_GET_ACTIVE_DIRECTORY 0x46
#define CMD_SET_ACTIVE_DIRECTORY_ENABLE 0x47
#define CMD_SET_ACTIVE_DIRECTORY_IP_PWD 0x48
#define CMD_SET_ACTIVE_DIRECTORY_DOMAIN 0x49
#define CMD_SET_ACTIVE_DIRECTORY_USERNAME 0x4A

// Set FRU
#define CMD_SET_FRU_HEADER 0x4B
#define CMD_SET_FRU_BOARD 0x4C
#define CMD_SET_FRU_PRODUCT 0x4D
#define CMD_SET_FRU_CHASSIS 0x4E
#define LEN_MFG_DATE 26
#define LEN_MFG 23
#define LEN_PRODUCT 24
#define LEN_SERIAL 24
#define LEN_PART_NUM 24
#define LEN_NAME 24
#define LEN_VERSION 8
#define LEN_TYPE 3

#define CMD_DCMI_MC_INFO 0x50
#define CMD_DCMI_MC_GUID 0x51
#define CMD_DCMI_ASSET_TAG 0x52
#define CMD_DCMI_MNGCTRL_ID 0x53
#define CMD_DCMI_INLET_TEMP 0x54
#define CMD_DCMI_CPU_TEMP 0x55
#define CMD_DCMI_BASEBOARD_TEMP 0x56
#define CMD_DCMI_GET_SEL 0x57
#define CMD_DCMI_CLEAR_LOG 0x58
#define CMD_BMC_WARM_RESET 0x59

#define CMD_SET_LAN_PRIORITY 0x5A
#define CMD_GET_SETTING_SERVICE 0x5B
#define CMD_SET_SETTING_SERVICE 0x5C
#define CMD_SET_RADIUS_DISABLE 0x5D
#define CMD_SAVE_EVENT 0x5E
#define CMD_GET_POWER_USAGE 0x5F
#define CMD_GET_POWER_PEAK 0x60
#define CMD_GET_POWER_CONSUMPTION 0x61
#define CMD_GET_LAST_MIN_POWER_USAGE 0x62
#define CMD_GET_LAST_HOUR_POWER_USAGE 0x63
#define CMD_GET_REALTIME_POWER_USAGE 0x64
#define CMD_TRY_LOGIN 0xfe
#define CMD_SHOW_MAIN 0xff

typedef struct {
  long type;
  int next;
  int res_len;
  unsigned char data[35000];
  // QSIZE
} msq_rsp_t;

typedef struct {
  long type;
  int ccode;
} kvm_msq_t;
typedef struct {
  long type;
  int index;
  // unsigned char data[MAX_IPMI_MSG_SIZE];
  rest_req_t ipmi_msg;
} msq_req_t;
/////////////rest

enum {
  NETFN_CHASSIS_REQ = 0x00,
  NETFN_CHASSIS_RES,
  NETFN_BRIDGE_REQ,
  NETFN_BRIDGE_RES,
  NETFN_SENSOR_REQ,
  NETFN_SENSOR_RES,
  NETFN_APP_REQ,
  NETFN_APP_RES,
  NETFN_FIRMWARE_REQ,
  NETFN_FIRMWARE_RES,
  NETFN_STORAGE_REQ,
  NETFN_STORAGE_RES,
  NETFN_TRANSPORT_REQ,
  NETFN_TRANSPORT_RES,
  NETFN_DCMI_REQ = 0x2C,
  NETFN_DCMI_RES = 0x2D,
  NETFN_OEM_GROUP_REQ = 0x2E,
  NETFN_OEM_GROUP_RES = 0x2F,
  NETFN_OEM_1S_REQ = 0x38,
  NETFN_OEM_1S_RES = 0x39,
  NETFN_REST_REQ = 0x43,
  NETFN_REST_RES = 0x44
};

enum {
  CC_SUCCESS = 0x00,
  CC_INVALID_PARAM = 0x80,
  CC_SEL_ERASE_PROG = 0x81,
  CC_POWER_OOL = 0x84,
  CC_POWER_CORRECTION = 0x85,
  CC_POWER_SAMPLE_TIME = 0x89,
  CC_UNKNOWN = 0xC0,
  CC_INVALID_CMD = 0xC1,
  CC_OUT_OF_SPACE = 0xC4,
  CC_PARAM_OUT_OF_RANGE = 0xC9,
  CC_UNSPECIFIED_ERROR = 0xFF,
  CC_DISABLED = 0xd6
};

/**
 * @brief 로그를 남길 경로와 prefix로 파일 이름을 설정하여 로그파일 정보를
 * 설정하는 함수
 *
 * @param dir 저장할 로그 파일 경로
 * @param prefix 저장할 로그 파일 prefix
 * @return int 성공시 0, 실패시 -1
 */
int LOGsetInfo(const char *dir, const char *prefix);
/**
 * @brief 로그 레벨을 설정하는 함수
 *
 * @param log_lvl 설정할 로그레벨
 * LOG_LVL_DISABLE
 * LOG_LVL_TRACE
 * LOG_LVL_INFO
 * LOG_LVL_WARNING
 * LOG_LVL_ALL_INFO
 * @return int 로그 레벨을 반환
 */
int LogSetLevel(int log_lvl);
/**
 * @brief 현재 로그 레벨을 확인하는 함수
 *
 * @return int 현재 로그 레벨을 반환
 */
int LogGetLevel(void);
/**
 * @brief 로그 파일을 생성하는 함수
 *
 * @param tml 현재 시간
 * @param src_file 로그파일 prefix
 * @return int 성공시 0, 실패시 -1
 */
static int LOGcreateFile(struct tm *tml, const char *src_file);
/**
 * @brief 로그 파일에 내용을 기록하는 함수
 *
 * @param log_type 로그 타입 (TRACE, INFO, WARNING, ALL_INFO)
 * @param src_file 로그를 기록할 파일명
 * @param func 로그를 호출한 함수명
 * @param line_no 라인 번호
 * @param fmt 로그 내용
 * @param ...
 * @return int 성공시 기록한 문자 개수, 실패시 -1
 */
int LOGlogging(char log_type, const char *src_file, const char *func,
               int line_no, const char *fmt, ...);
void delay(int millisec);
void time_stamp_fill(uint8_t *ts);

void test_ipmi(void);

/**
 * @brief REST WEB에서 POWER 사용량을 전달하기위한 구조체
 *
 */
typedef struct {
  int id;
  int watt;
  unsigned char dt[100];
} power_usage_t;

enum class Theshold_type {
  lnc_low = 0,
  lnc_high,
  lc_low,
  lc_high,
  lnr_low,
  lnr_high,
  unc_low,
  unc_high,
  uc_low,
  uc_high,
  unr_low,
  unr_high,

};
enum class Sensor_tpye {
  reserved = 0,
  Temperature = 1,
  Voltage,
  Current,
  Fan,
  PhysicalSecurity,
  PlatformSecurity,
  Processor,
  PowerSupply,
  PowerUnit,
  CoolingDevice,
  Other,
  Memory,
  DriveSlotBay,
  POSTMemoryResize,
  SystemFirmwares,
  EventLoggingDisabled,
  Watchdog1,
  SystemEvent,
  CriticalInterrupt,
  Button,
  ModuleBoard,
  Microcontroller,
  AddinCard,
  Chassis,
  ChipSet,
  OtherFRU,
  CableInterconnect,
  Terminator,
  SystemBootInitiated,
  BootError,
  OSBoot,
  OSCriticalStop,
  SlotConnector,
  SystemACPIPowerState,
  Watchdog2,
  PlatformAlert,
  EntityPresence,
  MonitorASIC,
  LAN,
  ManagementSubsysHealth,
  Battery,
  SessionAudit,
  VersionChange,
  FRUState,

};

/// fru

struct ipmi_fruid_info_t {
  uint8_t size_lsb;
  uint8_t size_msb;
  uint8_t bytes_words;
};

struct fru_info_t {
  string mfg;
  string prod;
  string serial;
  string serial2;
};

/**
 * @brief FRU Board Information Descriptor
 *
 */

struct fru_board_info_t {
  char *mfg_date[4];
  char *mfg[32];
  char *product[32];
  char *serial[32];
  char *part_number[32];
};

/**
 * @brief FRU Product Information Descriptor
 *
 */
struct fru_product_info_t {
  char *mfg[32];
  char *name[32];
  char *part_number[32];
  char *version[32];
  char *serial[32];
  char *asset_tag[32];
};

/**
 * @brief FRU Chassis Information Descriptor
 *
 */

struct fru_chassis_info_t {
  uint8_t type;
  char *part_number[32];
  char *serial[32];
};

/**
 * @brief FRU Header Descriptor
 *
 */
struct fru_header_t {
  uint8_t id;
  uint8_t version;
  uint8_t internal;
  uint8_t chassis;
  uint8_t board;
  uint8_t product;
  uint8_t multi;
  uint8_t pad;
  uint8_t checksum;
};
/**
 *  @brief SEL header struct to keep track of SEL Log entries
 */
struct sel_hdr {
  int magic;            /// Magic number to check validity
  int version;          /// version number of this header
  int begin;            /// index to the begining of the log
  int end;              /// index to end of the log
  std::time_t ts_add;   /// last addition time stamp
  std::time_t ts_erase; /// last erase time stamp
};

struct sel_msg_t {
  unsigned char msg[16];
};

struct sdr_rec_t {
  unsigned char rec[64];
};

struct ss_t {
  int web_enables;
  char web_port[6];
  int ssh_enables;
  char ssh_port[6];
  int alert_enables;
  char alert_port[6];
  int kvm_enables;
  char kvm_port[6];
  char kvm_proxy_port[6];
};
struct kvm_service_t {
  int kvm_enables;
  char kvm_port[6];
  char kvm_proxy_port[6];
};

struct ssh_service_t {
  int ssh_enables;
  char ssh_port[6];
};

struct alert_servce_t {
  int alert_enables;
  char alert_port[6];
};

struct web_service_t {
  int web_enables;
  char web_port[6];
};

// sdr

/// SDR Header magic number
#define SDR_HDR_MAGIC 0xFBFBFBFB

#define SDR_HDR_VERSION 0x01
#define IPMI_SDR_VERSION 0x51
// SDR reservation IDs can not be 0x00 or 0xFFFF
#define SDR_RSVID_MIN 0x01
#define SDR_RSVID_MAX 0xFFFE

// SDR index to keep track
#define SDR_INDEX_MIN 0
#define SDR_INDEX_MAX (SDR_RECORDS_MAX - 1)

// Record ID can not be 0x0 (IPMI/Section 31)
#define SDR_RECID_MIN 1
#define SDR_RECID_MAX SDR_RECORDS_MAX

// Special RecID value for first and last (IPMI/Section 31)
#define SDR_RECID_FIRST 0x0000
#define SDR_RECID_LAST 0xFFFF

#define SDR_VERSION 0x51
#define SDR_LEN_MAX 64

#define SDR_FULL_TYPE 0x01
#define SDR_FRU_TYPE 0x11
#define SDR_MGMT_TYPE 0x12
#define SDR_OEM_TYPE 0xC0

#define SDR_FULL_LEN 64
#define SDR_MGMT_LEN 32
#define SDR_OEM_LEN 64


#define PSDR_ANALOG_DISABLE 3

#define SENSOREV_UPPER_NONRECOVER_THR_SETREAD_MASK (0x20)
#define SENSOREV_UPPER_CRITICAL_THR_SETREAD_MASK (0x10)
#define SENSOREV_UPPER_NONCRITICAL_THR_SETREAD_MASK (0x08)
#define SENSOREV_LOWER_NONRECOVER_THR_SETREAD_MASK (0x04)
#define SENSOREV_LOWER_CRITICAL_THR_SETREAD_MASK (0x02)
#define SENSOREV_LOWER_NONCRITICAL_THR_SETREAD_MASK (0x01)

class Ipmisdr {
public:
  uint8_t sensor_num;
  uint8_t rec_id[2];
  uint8_t ver;
  uint8_t type;
  uint8_t len;

  sensor_thresh_t rec;

public:
  Ipmisdr(int _rec_id, uint8_t _snum, sensor_thresh_t _rec) {
    this->sensor_num = _snum;
    this->rec_id[0] = _rec_id & 0xff;
    this->rec_id[1] = (_rec_id >> 8) & 0xff;
    this->ver = SDR_HDR_VERSION;
    this->type = SDR_FULL_TYPE;
    this->rec = _rec;
    this->len = SDR_FULL_LEN + (_rec.str_type_len & 0x1f) - SENSOR_STR_SIZE;
  }
  uint8_t sdr_thresh_write(int sensor_num);
  sensor_thresh_t *sdr_get_entry();
  std::vector<uint8_t> sdr_get_rec_to_vector();
  uint8_t sdr_get_sensornum();
  int sdr_thresh_write(uint8_t param, uint8_t *data);
  uint8_t sdr_get_thresh_flag();
  uint8_t sdr_get_thresh_param(uint8_t param);
  uint8_t sdr_sensor_read();
  uint8_t sdr_get_analog_flag();
  void print_sensor_info();
  /**
   * @brief 모든 샌서 정보를 가지고 있는 구조체 반환
   * @details Ipmisdr이가지고있는 sensor_thresh_t -> rec 반환
   */
  sensor_thresh_t get_sensor_thresh_t() { return this->rec; }
};

typedef struct {
  unsigned char ts[4];
} time_stamp_t;

struct sdr_hdr_t {
  int magic;             /// Magic number to check validity
  int version;           /// version number of this header
  int begin;             /// index to the first SDR entry
  int end;               /// index to the last SDR entry
  time_stamp_t ts_add;   /// last addition time stamp
  time_stamp_t ts_erase; /// last erase time stamp
};

/**
 * @brief BMC Watchdog Descriptor
 *
 */

/* Configuration file key works*/

typedef struct {
  unsigned char timer_use;
  unsigned char timer_actions;
  unsigned char pre_timeout;
  unsigned char timer_use_exp;
  unsigned char initial_countdown_lsb;
  unsigned char initial_countdown_msb;
  unsigned char present_countdown_lsb;
  unsigned char present_countdown_msb;
  unsigned char pretimeoutInterrupt;
  int *interval;
  bool Islogging = false;

} bmc_watchdog_param_t;

/* Used by function ReadConfigurationFile to store the parameter*/
#define CONFIG_LINE_LEN 100

#define IPMIDeviceName1 "/dev/ipmi0"
#define IPMIDeviceName2 "/dev/ipmidev/0"
#define IPMIWatchdogFile1 "/etc/init.d/ipmiwatchdog"
#define IPMIWatchdogFile2 "/etc/ipmiwatchdog.conf"
/* Configuration file key works*/

// event 관련

#define PEF_FILTER_ENTRY_COUNT 2
#define PEF_POLICY_ENTRY_COUNT 1

#define PEF_CFGPARM_ID_REVISION_ONLY_MASK 0x80
#define PEF_CFGPARM_ID_SET_IN_PROGRESS 0
#define PEF_CFGPARM_ID_PEF_CONTROL 1
#define PEF_CFGPARM_ID_PEF_ACTION 2
#define PEF_CFGPARM_ID_PEF_STARTUP_DELAY 3
#define PEF_CFGPARM_ID_PEF_ALERT_STARTUP_DELAY 4
#define PEF_CFGPARM_ID_PEF_FILTER_TABLE_SIZE 5
#define PEF_CFGPARM_ID_PEF_FILTER_TABLE_ENTRY 6
#define PEF_CFGPARM_ID_PEF_FILTER_TABLE_DATA_1 7
#define PEF_CFGPARM_ID_PEF_ALERT_POLICY_TABLE_SIZE 8
#define PEF_CFGPARM_ID_PEF_ALERT_POLICY_TABLE_ENTRY 9
#define PEF_CFGPARM_ID_SYSTEM_GUID 10
#define PEF_CFGPARM_ID_PEF_ALERT_STRING_TABLE_SIZE 11
#define PEF_CFGPARM_ID_PEF_ALERT_STRING_KEY 12
#define PEF_CFGPARM_ID_PEF_ALERT_STRING_TABLE_ENTRY 13

#define PEF_FILTER_DISABLED 0x7F
#define PEF_FILTER_ENABLED 0x80
#define PEF_FILTER_TABLE_ID_MASK 0x7F
#define PEF_CONFIG_ENABLED 0x80
#define PEF_CONFIG_PRECONFIGURED 0x40
#define PEF_ACTION_DIAGNOSTIC_INTERRUPT 0x20
#define PEF_ACTION_OEM 0x10
#define PEF_ACTION_POWER_CYCLE 0x08
#define PEF_ACTION_RESET 0x04
#define PEF_ACTION_POWER_DOWN 0x02
#define PEF_ACTION_ALERT 0x01
#define PEF_POLICY_NUMBER_MASK 0x0f
#define PEF_SEVERITY_NON_RECOVERABLE 0x20
#define PEF_SEVERITY_CRITICAL 0x10
#define PEF_SEVERITY_WARNING 0x08
#define PEF_SEVERITY_OK 0x04
#define PEF_SEVERITY_INFORMATION 0x02
#define PEF_SEVERITY_MONITOR 0x01
#define PEF_SENSOR_NUMBER_MATCH_ANY 0xff
#define PEF_EVENT_TRIGGER_UNSPECIFIED 0x0
#define PEF_EVENT_TRIGGER_THRESHOLD 0x1
#define PEF_EVENT_TRIGGER_SENSOR_SPECIFIC 0x6f
#define PEF_EVENT_TRIGGER_MATCH_ANY 0xff
#define PEF_FILTER_DISABLED 0x7F
#define PEF_FILTER_ENABLED 0x80
#define PEF_FILTER_TABLE_ID_MASK 0x7F
#define PEF_POLICY_TABLE_ID_MASK 0x7f
#define PEF_POLICY_ID_MASK 0xf0
#define PEF_POLICY_ID_SHIFT 4
#define PEF_POLICY_DISABLED 0xF7
#define PEF_POLICY_ENABLED 0x08
#define PEF_POLICY_FLAGS_MASK 0x07
#define PEF_POLICY_FLAGS_MATCH_ALWAYS 0
#define PEF_POLICY_FLAGS_PREV_OK_SKIP 1
#define PEF_POLICY_FLAGS_PREV_OK_NEXT_POLICY_SET 2
#define PEF_POLICY_FLAGS_PREV_OK_NEXT_CHANNEL_IN_SET 3
#define PEF_POLICY_FLAGS_PREV_OK_NEXT_DESTINATION_IN_SET 4
#define PEF_POLICY_CHANNEL_MASK 0xf0
#define PEF_POLICY_CHANNEL_SHIFT 4
#define PEF_POLICY_DESTINATION_MASK 0x0f
#define PEF_POLICY_EVENT_SPECIFIC 0x80
#define PEF_SYSTEM_GUID_USED_IN_PET 0x01
#define PEB_SENSOR_NUM_BASE 0x24 // PEB_SENSOR_ADC_P12V

typedef struct {
  uint8_t data1;
  uint8_t config;
  uint8_t action;
  uint8_t policy_number;
  uint8_t severity;
  uint8_t generator_ID_addr;
  uint8_t generator_ID_lun;
  uint8_t sensor_type;
  uint8_t sensor_number;
  uint8_t event_trigger;
  uint8_t event_data_1_offset_mask[2];
  uint8_t event_data_1_AND_mask;
  uint8_t event_data_1_compare_1;
  uint8_t event_data_1_compare_2;
  uint8_t event_data_2_AND_mask;
  uint8_t event_data_2_compare_1;
  uint8_t event_data_2_compare_2;
  uint8_t event_data_3_AND_mask;
  uint8_t event_data_3_compare_1;
  uint8_t event_data_3_compare_2;
} pef_table_entry_t;

/// @brief SEL header struct to keep track of SEL Log entries
typedef struct {
  int magic;             /// Magic number to check validity
  int version;           /// version number of this header
  int begin;             /// index to the begining of the log
  int end;               /// index to end of the log
  time_stamp_t ts_add;   /// last addition time stamp
  time_stamp_t ts_erase; /// last erase time stamp
} sel_hdr_t;

// event msage

struct event_msg_t {
  uint8_t gen_id;
  uint8_t evm_rev;
  uint8_t sensor_type;
  uint8_t sensor_num;
  uint8_t event_dir : 1;
  uint8_t event_type : 7;
  uint8_t event_data[3];
};

///////////////pef 관련
struct pef_policy_entry {
  uint8_t policy;
  uint8_t chan_dest;
  uint8_t alert_string_key;
};

struct pef_policy_table {
#define PEF_POLICY_TABLE_ID_MASK 0x7f
  uint8_t data1;
  struct pef_policy_entry entry;
};

enum {
  IPMI_SEL_INIT_ERASE = 0xAA,
  IPMI_SEL_ERASE_STAT = 0x00,
};
typedef enum {
  SEL_ERASE_IN_PROG = 0x00,
  SEL_ERASE_DONE = 0x01,
} sel_erase_stat_t;

/**
 * @brief 다음 index를 response의 data에 저장함. 마지막 index는 0xff로
 * 마지막임을 알려줌
 * @param unsigned char *data - 바로 response data에 작성하기 위한 인자
 * @return void
 */
void plat_sel_last_rsv_id(unsigned char *data);
int plat_sel_num_entries(void);
uint8_t plat_lan_channel_selector(uint8_t net_priority);
/**
 * @brief SNMP Alert 정보를 초기화하는 함수
 * IPMI_LAN_ALERT_PATH / IPMI_LAN_ALERT_DEDI_PATH 파일이 존재하는 경우 파일
 * 내용을 읽어 lan_config_t 구조체에 반영한다.\n 파일이 없는 경우 NULL상태로
 * 유지한다.
 */
void plat_lan_alert_init();

uint8_t parse_policy_set(uint8_t policy);

#include <queue>
class IpmiLogEvent {
public:
  string event_path;
  string msg;
  string event_time;
  string event_type;
  vector<string> message_args;
  string severity;
  string sensornumber;
  int index;
  IpmiLogEvent(){};
  IpmiLogEvent(string _msg, string event_type, string _event_path,
               string severity = "OK", string sensornumber = "",
               vector<string> message_args = vector<string>()) {
    this->msg = _msg;
    this->event_path = _event_path;
    this->event_time = currentDateTime();
    this->event_path = event_type;
    this->severity = severity;
    this->sensornumber = sensornumber;
    this->message_args = message_args;
  }

private:
  const std::string currentDateTime(void) {
    time_t now = time(0); //현재 시간을 time_t 타입으로 저장
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X",
             &tstruct); // YYYY-MM-DD.HH:mm:ss 형태의 스트링
    return buf;
  }
};

#endif