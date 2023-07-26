#ifndef __CHASSIS_HPP__
#define __CHASSIS_HPP__

#include "ipmi/common.hpp"
#include "ipmi/ipmi.hpp"
#include <sys/sysinfo.h>

#define BOOT_INFO_ACK_SIZE 2
#define BOOT_FLAGS_SIZE 5
#define BOOT_INIT_INFO_SIZE 9

enum {
  CMD_CHASSIS_GET_STATUS = 0x1,
  CMD_CHASSIS_CONTROL = 0x2,
  CMD_CHASSIS_IDENTIFY = 0x4,
  CMD_CHASSIS_POWER_POLICY = 0x6,
  CMD_CHASSIS_RESTART_CAUSE = 0x7,
  CMD_CHASSIS_SET_BOOT_PARAM = 0x8,
  CMD_CHASSIS_GET_BOOT_OPTIONS = 0x9,
  CMD_CHASSIS_POH = 0xF,
};

enum {
  PARAM_SET_IN_PROG = 0x00,
  PARAM_SVC_PART_SELECT,
  PARAM_SVC_PART_SCAN,
  PARAM_BOOT_FLAG_CLR,
  PARAM_BOOT_INFO_ACK,
  PARAM_BOOT_FLAGS,
  PARAM_BOOT_INIT_INFO,
};

/**
 * @brief Chassis Power Control Command List
 *
 */
enum {
  PARAM_POWER_OFF = 0x00,
  PARAM_POWER_ON,
  PARAM_POWER_CYCLE,
  PARAM_HARD_RESET,
  PARAM_PULSE_DIAGNOSTIC_INTR,
  PARAM_SOFT_OFF_VIA_OVER_TEMP,
  PARAM_POWER_OFF_ORDERLY
};

typedef struct {
  uint8_t set_in_prog;
  uint8_t svc_part_select;
  uint8_t svc_part_scan;
  uint8_t boot_flag_clr;
  std::vector<uint8_t> boot_info_ack;  // 2 Byte
  std::vector<uint8_t> boot_flags;     // 5 Byte
  std::vector<uint8_t> boot_init_info; // 9 Byte;
} chassis_bootparam_t;

class Ipmichassis {
private:
  uint8_t restart_cause;
  uint8_t soft_off_flag;
  uint8_t id_interval;
  uint8_t restart_policy;

public:
  Ipmichassis();
  chassis_bootparam_t g_chassis_bootp;

  uint8_t get_power_status(void);
  void chassis_restart_cause(ipmi_req_t *request, ipmi_res_t *response,
                             uint8_t *res_len);
  void chassis_control(ipmi_req_t *request, ipmi_res_t *response,
                       uint8_t *res_len);
  void chassis_get_status(ipmi_req_t *request, ipmi_res_t *response,
                          uint8_t *res_len);
  void chassis_set_policy(ipmi_req_t *request, ipmi_res_t *response,
                          uint8_t *res_len);
  void chassis_identify(ipmi_req_t *request, ipmi_res_t *response,
                        uint8_t *res_len);
  void chassis_get_poh(ipmi_req_t *request, ipmi_res_t *response,
                       uint8_t *res_len);
  void chassis_set_boot_options(ipmi_req_t *request, ipmi_res_t *response,
                                uint8_t *res_len);
  void chassis_get_boot_options(ipmi_req_t *request, ipmi_res_t *response,
                                uint8_t *res_len);
};
#endif