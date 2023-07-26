#include "ipmi/chassis.hpp"
#include <ipmi/lightning_sensor.hpp>
#include <util/dbus_system.hpp>

extern int redfish_power_ctrl;
extern int bPowerGD;
/**
 * @brief chassis power 상태 정보 전달
 * @bug 현재 redfish power control action이 off로 오면 무조건 0리턴 되는 문제
 * @return uint8_t
 * @bug
 */
uint8_t Ipmichassis::get_power_status(void) {
  int ps = 0;
  log(info) << "get_power_status ";
  DBus::BusDispatcher dispatcher;
  DBus::default_dispatcher = &dispatcher;
  DBus::Connection conn_n = DBus::Connection::SystemBus();
  DBus_Sensor chassis_dbus_sensor = DBus_Sensor(conn_n,SERVER_PATH_1,SERVER_NAME_1);

  ps = chassis_dbus_sensor.lightning_sensor_read(FRU_PEB, PEB_SENSOR_ADC_P12V_PSU1);
    dispatcher.leave();

  if (ps >= 150) {
    redfish_power_ctrl = 1;
    return 1;
  } else {
    redfish_power_ctrl = 0;
    return 0;
  }
  // return bPowerGD;
}
void Ipmichassis::chassis_restart_cause(ipmi_req_t *request,
                                        ipmi_res_t *response,
                                        uint8_t *res_len) {
  uint8_t *data = &response->data[0];

  response->cc = CC_SUCCESS;

  *data++ = this->restart_cause;
  *data++ = 1;

  *res_len = data - &response->data[0];
}

void Ipmichassis::chassis_get_status(ipmi_req_t *request, ipmi_res_t *response,
                                     uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  uint8_t pwr_status = 0;
  uint8_t chassis_policy = 0;

  response->cc = CC_SUCCESS;

  if (this->restart_policy & 0x100)
    chassis_policy = 2;
  else if (this->restart_policy & 0x10)
    chassis_policy = 1;
  else if (this->restart_policy & 0x1)
    chassis_policy = 0;

  pwr_status = this->get_power_status();

  *data++ = (0x1 & pwr_status) | (chassis_policy << 5);
  *data++ = 0x0;
  *data++ = 0x40;
  *data++ = 0x0;

  *res_len = data - &response->data[0];
}

void Ipmichassis::chassis_control(ipmi_req_t *request, ipmi_res_t *response,
                                  uint8_t *res_len) {
  // uint8_t *data = &response->data[0];
  uint8_t param = request->data[0];
  response->cc = CC_SUCCESS;
  printf("params= %d \n",param);
  switch (param) {
  case PARAM_POWER_OFF:
  printf("poweroff\n");
  system("gpioset 0 122=0");
  sleep(2);
  system("gpioset 0 122=1");
    // set_value_to_SMLTR(SMLTR_POWER_OFF, 0);
    break;
  case PARAM_POWER_ON:
    this->restart_cause = 3;
    // system("gpioset 0 122=1");
    // usleep(1000);
    printf("power on\n");
    system("gpioset 0 122=0");
    usleep(1000);
    system("gpioset 0 122=1");
    usleep(1000);
    break;
  case PARAM_POWER_CYCLE:
    system("gpioset 0 120=0");
    usleep(1000);
    //usleep(1000000); 1초
    system("gpioset 0 120=1");
    this->restart_cause = 1;
    break;
  case PARAM_HARD_RESET:
    system("gpioset 0 120=0");
    usleep(1000);
    system("gpioset 0 120=1");
    this->restart_cause = 2;
    break;
  case PARAM_POWER_OFF_ORDERLY:
  system("gpioset 0 122=0");
  usleep(1000);
  system("gpioset 0 122=1");
    this->restart_cause = 0xA;
    break;
  default:
    response->cc = CC_PARAM_OUT_OF_RANGE;
    break;
  }
}

void Ipmichassis::chassis_set_policy(ipmi_req_t *request, ipmi_res_t *response,
                                     uint8_t *res_len) {
  uint8_t policy = request->data[0];
  uint8_t *data = &response->data[0];

  response->cc = CC_SUCCESS;

  switch (policy) {
  case 0x0: // Always stay powered off
    this->restart_policy = 0x1;
    *data++ = this->restart_policy;
    break;
  case 0x1:
    this->restart_policy = 0x2;
    this->restart_cause = 0x2;
    *data++ = this->restart_policy;
    break;
  case 0x2:
    this->restart_policy = 0x4;
    this->restart_cause = 0x6;
    *data++ = this->restart_policy;
    break;
  case 0x3:
    *data++ = this->restart_policy;
    break;
  default:
    response->cc = CC_PARAM_OUT_OF_RANGE;
    break;
  }

  if (response->cc == CC_SUCCESS) {
    *res_len = data - &response->data[0];
  }
}
void Ipmichassis::chassis_identify(ipmi_req_t *request, ipmi_res_t *response,
                                   uint8_t *res_len) {
  uint8_t interval = request->data[0];
  uint8_t force_on = request->data[1] & 0x1;

  if (force_on == 1) {
    printf("[Chassis] Force on Identify LED\n");
    response->cc = CC_SUCCESS;
  } else {
    if (interval == 3 || interval == 15)
      this->id_interval = 15;
    else
      this->id_interval = interval;

    response->cc = CC_SUCCESS;
  }
}

void Ipmichassis::chassis_get_poh(ipmi_req_t *request, ipmi_res_t *response,
                                  uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  uint8_t mins_per_count = 60;
  long count = 180;
  struct sysinfo info;

  sysinfo(&info);
  count = info.uptime / mins_per_count;

  *data++ = 1;
  *data++ = count & 0xff;
  *data++ = (count & 0xff00) >> 8;
  *data++ = (count & 0xff0000) >> 16;
  *data++ = (count & 0xff000000) >> 24;

  response->cc = CC_SUCCESS;

  *res_len = data - &response->data[0];
}

void Ipmichassis::chassis_get_boot_options(ipmi_req_t *request,
                                           ipmi_res_t *response,
                                           uint8_t *res_len) {
  uint8_t param = request->data[0];
  uint8_t *data = &response->data[0];

  response->cc = CC_SUCCESS;

  *data++ = 0x01; // Parameter Version
  *data++ = request->data[0];

  switch (param) {
  case PARAM_SET_IN_PROG:
    *data++ = this->g_chassis_bootp.set_in_prog;
    break;
  case PARAM_SVC_PART_SELECT:
    *data++ = this->g_chassis_bootp.svc_part_select;
    break;
  case PARAM_SVC_PART_SCAN:
    *data++ = this->g_chassis_bootp.svc_part_scan;
    break;
  case PARAM_BOOT_FLAG_CLR:
    *data++ = this->g_chassis_bootp.boot_flag_clr;
    break;
  case PARAM_BOOT_INFO_ACK:
    std::copy(this->g_chassis_bootp.boot_info_ack.begin(),
              this->g_chassis_bootp.boot_info_ack.end(), data);
    data += BOOT_INFO_ACK_SIZE; // ACK Size
    break;
  case PARAM_BOOT_FLAGS:
    std::copy(this->g_chassis_bootp.boot_flags.begin(),
              this->g_chassis_bootp.boot_flags.end(), data);
    data += BOOT_FLAGS_SIZE; // Boot Flag Size
    break;
  case PARAM_BOOT_INIT_INFO:
    std::copy(this->g_chassis_bootp.boot_init_info.begin(),
              this->g_chassis_bootp.boot_flags.end(), data);
    data += BOOT_INIT_INFO_SIZE; // Boot init Size
    break;
  default:
    response->cc = CC_PARAM_OUT_OF_RANGE;
    break;
  }

  if (response->cc == CC_SUCCESS)
    *res_len = data - &response->data[0];
}

void Ipmichassis::chassis_set_boot_options(ipmi_req_t *request,
                                           ipmi_res_t *response,
                                           uint8_t *res_len) {
  uint8_t param = request->data[0];
  uint8_t *data = &response->data[0];

  response->cc = CC_SUCCESS;

  switch (param) {
  case PARAM_SET_IN_PROG:
    this->g_chassis_bootp.set_in_prog = request->data[1];
    break;
  case PARAM_SVC_PART_SELECT:
    this->g_chassis_bootp.svc_part_select = request->data[1];
    break;
  case PARAM_SVC_PART_SCAN:
    this->g_chassis_bootp.svc_part_scan = request->data[1];
    break;
  case PARAM_BOOT_FLAG_CLR:
    this->g_chassis_bootp.boot_flag_clr = request->data[1];
    break;
  case PARAM_BOOT_INFO_ACK:
    this->g_chassis_bootp.boot_info_ack.assign(
        request->data + 1, request->data + 1 + BOOT_INFO_ACK_SIZE);
    break;
  case PARAM_BOOT_FLAGS:
    this->g_chassis_bootp.boot_flags.assign(
        request->data + 1, request->data + 1 + BOOT_FLAGS_SIZE);
    break;
  case PARAM_BOOT_INIT_INFO:
    this->g_chassis_bootp.boot_init_info.assign(
        request->data + 1, request->data + 1 + BOOT_INIT_INFO_SIZE);
    break;
  default:
    response->cc = CC_PARAM_OUT_OF_RANGE;
    break;
  }

  if (response->cc == CC_SUCCESS)
    *res_len = data - &response->data[0];
}

Ipmichassis::Ipmichassis() {}