#include "ipmi/sel.hpp"
#include "ipmi/user.hpp"
#include <bitset>
#include <ipmi/apps.hpp>
#include <ipmi/lightning_sensor.hpp>
#include <ipmi/sensor_define.hpp>
#include <ipmi/sol.hpp>
#include <math.h>
#include <redfish/resource.hpp>
#include <sqlite3.h>
#include <string>
extern unordered_map<string, Resource *> g_record;
#pragma once
extern sensor_thresh_t peb_sensors[PEB_SENSOR_COUNT];
extern sensor_thresh_t nva_sensors[NVA_SENSOR_COUNT];
extern sensor_thresh_t pdpb_sensors[PDPB_SENSOR_COUNT];
/**
 *
 * @namespace tos32
 */

/**
 * @namespace json사용하기 위한 web
 */

#define DEBUG

using namespace std;
#define SIZE_IPMI_RES_HDR 3

static pthread_mutex_t m_app;
static pthread_mutex_t m_storage;
static pthread_mutex_t m_transport;
static pthread_mutex_t m_oem;
static pthread_mutex_t m_chassis;

/**
 * @brief Firmware build time 문자열
 *
 */
char g_firmware_build_time[50];

extern std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;
extern std::map<uint8_t, std::map<uint8_t, Ipmifru>> fru_rec;
#define ETH_COUNT 4
std::thread t_lanMonitor[ETH_COUNT];
Ipmichassis ipmiChassis = Ipmichassis();
Ipminetwork ipmiNetwork[ETH_COUNT] = {Ipminetwork(1), Ipminetwork(6),
                                      Ipminetwork(7), Ipminetwork(8)};
extern Ipmiuser ipmiUser[MAX_USER];
Ipmichannel ipmiChannel[9] = {
    Ipmichannel(MEDIUM_IPMB, PROTOCOL_IPMB_LAN, 0, CHANNEL_SESSIONLESS),
    Ipmichannel(MEDIUM_802_3_LAN, PROTOCOL_IPMB_LAN, 1, CHANNEL_MULTI_SESSION),
    Ipmichannel(MEDIUM_SI, PROTOCOL_KCS, 2, CHANNEL_SESSIONLESS),
    Ipmichannel(3),
    Ipmichannel(4),
    Ipmichannel(5),
    Ipmichannel(MEDIUM_802_3_LAN, PROTOCOL_IPMB_LAN, 6, CHANNEL_MULTI_SESSION),
    Ipmichannel(MEDIUM_802_3_LAN, PROTOCOL_IPMB_LAN, 7, CHANNEL_MULTI_SESSION),
    Ipmichannel(MEDIUM_802_3_LAN, PROTOCOL_IPMB_LAN, 6, CHANNEL_MULTI_SESSION)};
Ipmiapplication ipmiApplication = Ipmiapplication();
extern Ipmisession ipmiSession[5];
extern char uuid_hex[16];

extern std::mutex redfish_m_mutex[5];
extern std::condition_variable redfish_m_cond[5];

/**
 * watchdog 설정
 *
 */
static bmc_watchdog_param_t g_watchdog_config;
int IPMI_Interval = 10, IPMI_Timeout = 600, IPMI_Pretimeout = 10,
    IPMI_PretimeoutInterrupt = 0, IPMI_Action = 1;
char *IPMI_Daemon = NULL, *IPMI_Pidfile = NULL, *IPMI_debug = NULL;
Ipmiapplication::Ipmiapplication() {
  cout << "Ipmiapplication" << endl;
  g_watchdog_config.interval = &IPMI_Interval;
  ReadConfigurationFile(ConfigurationFileDir);
  cout << "read WDT " << endl;
  g_watchdog_config.Islogging = true;
  g_watchdog_config.timer_actions = IPMI_Action;
  g_watchdog_config.pre_timeout = IPMI_Pretimeout;
  //*g_watchdog_config.interval = 19;
  // WriteConfigurationFile(ConfigurationFileDir);
  this->g_mc_device.mc_device_id = 0x20;
  this->g_mc_device.mc_device_rev = 0x01;
  this->g_mc_device.mc_fw_version[0] = 0x01;
  this->g_mc_device.mc_fw_version[1] = 0x12;
  this->g_mc_device.mc_ipmi_version = 0x02;
  this->g_mc_device.mc_additional_dev = 0xBF;
  this->g_mc_device.mc_mnfr_id[0] = 0x83;
  this->g_mc_device.mc_mnfr_id[1] = 0x23;
  this->g_mc_device.mc_mnfr_id[2] = 0x00;
  this->g_mc_device.mc_prod_id[0] = 0x51;
  this->g_mc_device.mc_prod_id[1] = 0x16;
  this->g_mc_device.mc_aux_fw_version[0] = 0x00;
  this->g_mc_device.mc_aux_fw_version[1] = 0x00;
  this->g_mc_device.mc_aux_fw_version[2] = 0x00;
  this->g_mc_device.mc_aux_fw_version[3] = 0x00;
  
  for (int i = 0; i < 4; i++) {
    switch (i) {
    case 0:
      this->g_authrsp[i].channel_number = 1;
      break;
    case 1:
      this->g_authrsp[i].channel_number = 6;
      break;
    case 2:
      this->g_authrsp[i].channel_number = 7;
      break;
    case 3:
      this->g_authrsp[i].channel_number = 8;
      break;
    }
    this->g_authrsp[i].enabled_auth_types = 0;
    this->g_authrsp[i].v20_data_available = 0b1; // IPMI 2.0 Support
    this->g_authrsp[i].kg_status = 0b0;
    this->g_authrsp[i].per_message_auth = 0;
    this->g_authrsp[i].user_level_auth = 0b1;
    this->g_authrsp[i].anon_login_enabled = 0b0;
    this->g_authrsp[i].non_null_usernames = 0b0;
    this->g_authrsp[i].non_null_usernames = 0b0;
    this->g_authrsp[i].ipmiv15_support = 0b1;
    this->g_authrsp[i].ipmiv20_support = 0b1;
  }

  this->g_sysinfo.set_in_prog = 0;
}

void mutex_init(void) {
  pthread_mutex_init(&m_app, NULL);
  pthread_mutex_init(&m_storage, NULL);
  pthread_mutex_init(&m_transport, NULL);
  pthread_mutex_init(&m_oem, NULL);
  pthread_mutex_init(&m_chassis, NULL);
}

void mutex_destroy(void) {
  pthread_mutex_destroy(&m_app);
  pthread_mutex_destroy(&m_storage);
  pthread_mutex_destroy(&m_transport);
  pthread_mutex_destroy(&m_oem);
  pthread_mutex_destroy(&m_chassis);

  t_lanMonitor[0].join();
}

void Ipmiapplication::app_get_sysinfo_param(uint8_t *request, uint8_t *response,
                                            uint8_t *res_len) {
  cout << "app_get_sysinfo_param" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  printf("apps:://app_get_sysinfo_param request ");
  uint8_t *data = &res->data[0];
  uint8_t rev = req->data[0] & 0x80 ? 1 : 0;
  uint8_t param = req->data[1];
  *data++ = 1;

  if (rev == 0) {
    if (param != SYS_INFO_PARAM_SET_IN_PROG)
      *data++ = 0; // ASCII + Latin 1

    switch (param) {
    case SYS_INFO_PARAM_SET_IN_PROG:
      *data++ = this->g_sysinfo.set_in_prog;
      break;
    case SYS_INFO_PARAM_SYSFW_VER:
      *data++ = this->g_sysinfo.sysfw_ver.length();
      std::copy(this->g_sysinfo.sysfw_ver.begin(),
                this->g_sysinfo.sysfw_ver.end(), data);
      break;

    case SYS_INFO_PARAM_SYS_NAME:
      *data++ = this->g_sysinfo.sys_name.length();
      std::copy(this->g_sysinfo.sys_name.begin(),
                this->g_sysinfo.sys_name.end(), data);
      break;

    case SYS_INFO_PARAM_PRI_OS_NAME:
      *data++ = this->g_sysinfo.pri_os_name.length();
      std::copy(this->g_sysinfo.pri_os_name.begin(),
                this->g_sysinfo.pri_os_name.end(), data);
      break;

    case SYS_INFO_PARAM_PRESENT_OS_NAME:
      *data++ = this->g_sysinfo.present_os_name.length();
      std::copy(this->g_sysinfo.present_os_name.begin(),
                this->g_sysinfo.present_os_name.end(), data);
      break;

    case SYS_INFO_PARAM_PRESENT_OS_VER:
      *data++ = this->g_sysinfo.present_os_ver.length();
      std::copy(this->g_sysinfo.present_os_ver.begin(),
                this->g_sysinfo.present_os_ver.end(), data);
      break;
    }
  }
}

void Ipmiapplication::app_set_sysinfo_param(uint8_t *request, uint8_t *response,
                                            uint8_t *res_len) {
  cout << "app_set_sysinfo_param" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t *data = &res->data[0];
  uint8_t param = req->data[0];
  uint8_t str_len = req->data[3];

  res->cc = CC_SUCCESS;

  switch (param) {
  case SYS_INFO_PARAM_SET_IN_PROG:
    this->g_sysinfo.set_in_prog = req->data[1];
    break;
  case SYS_INFO_PARAM_SYSFW_VER:
    this->g_sysinfo.sysfw_ver.assign(req->data + 4, req->data + 4 + str_len);
    break;
  case SYS_INFO_PARAM_SYS_NAME:
    this->g_sysinfo.sys_name.assign(req->data + 4, req->data + 4 + str_len);
    break;
  case SYS_INFO_PARAM_PRI_OS_NAME:
    this->g_sysinfo.pri_os_name.assign(req->data + 4, req->data + 4 + str_len);
    break;
  case SYS_INFO_PARAM_PRESENT_OS_NAME:
    this->g_sysinfo.present_os_name.assign(req->data + 4,
                                           req->data + 4 + str_len);
    break;
  case SYS_INFO_PARAM_PRESENT_OS_VER:
    this->g_sysinfo.present_os_ver.assign(req->data + 4,
                                          req->data + 4 + str_len);
    break;
  case SYS_INFO_PARAM_BMC_URL:
    this->g_sysinfo.bmc_url.assign(req->data + 4, req->data + 4 + str_len);
    break;
  case SYS_INFO_PARAM_OS_HV_URL:
    this->g_sysinfo.os_hv_url.assign(req->data + 4, req->data + 4 + str_len);
    break;
  default:
    res->cc = CC_INVALID_PARAM;
    break;
  }
}

void Ipmiapplication::app_set_useraccess(uint8_t *request, uint8_t *response,
                                         uint8_t *res_len) {
  cout << "app_set_useraccess" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t *data = &res->data[0];

  uint8_t id = req->data[1] & 0x3F;

  ipmiUser[id - 1].setUserAccess(req);

  res->cc = CC_SUCCESS;

  *res_len = 0;
}

void Ipmiapplication::app_set_user_passwd(uint8_t *request, uint8_t *response,
                                          uint8_t *res_len) {
  cout << "app_set_user_passwd" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  std::string temp_pass;
  uint8_t *data = &res->data[0];

  uint8_t size_pwd = (req->data[0] & 0x80) >> 7;
  uint8_t user_id = (req->data[0] & 0x1F) - 1;
  uint8_t operation = (req->data[1] & 0x03);

  if (operation == 2 || operation == 3) {
    if (size_pwd == 0)
      temp_pass.assign(req->data + 2, req->data + 16);
    else if (size_pwd == 1)
      temp_pass.assign(req->data + 2, req->data + 22);
  }

  switch (operation) {
  case 0: // disable user
    ipmiUser[user_id].setUserAccess(req);
    break;
  case 1: // enable user
    ipmiUser[user_id].setUserAccess(req);
    break;
  case 2: // set password
    ipmiUser[user_id].setUserPasswd(temp_pass);
    break;
  case 3: // test password
  {
    std::string user_pwd = ipmiUser[user_id].getUserpassword();
    if (user_pwd != temp_pass)
      res->cc = 0x81;
    else if (user_pwd.size() == temp_pass.size()) {
      if (user_pwd != temp_pass)
        res->cc = CC_INVALID_PARAM;
      else
        res->cc = CC_SUCCESS;
    }
  } break;
  }
}
void Ipmiapplication::app_set_username(uint8_t *request, uint8_t *response,
                                       uint8_t *res_len) {
  cout << "app_set_username" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  std::string username(req->data + 1, req->data + 16);
  uint8_t id = req->data[0];
  uint8_t flag = 0;
  for (int i = 0; i < MAX_ID; i++) {
    if (ipmiUser[i].getUsername() == username) {
      flag++;
    }
  }

  if (flag == 0) {
    res->cc = CC_SUCCESS;
    ipmiUser[id - 1].setUserName(username);
  }
  *res_len = 0;
}

void Ipmiapplication::app_get_username(uint8_t *request, uint8_t *response,
                                       uint8_t *res_len) {
  cout << "app_get_username" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t *data = &res->data[0];
  uint8_t id = req->data[0];
  std::string name = ipmiUser[id - 1].getUsername();

  res->cc = CC_SUCCESS;
  std::copy(name.begin(), name.end(), data);
  *res_len = 16;
}

void Ipmiapplication::app_get_useraccess(uint8_t *request, uint8_t *response,
                                         uint8_t *res_len) {
  cout << "app_get_useraccess" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t *data = &res->data[0];
  uint8_t id = req->data[1];
  uint8_t entry, flag, en_users = 0;
  std::string temp_name;

  if (id == 1) {
    flag = 2;
    res->cc = CC_SUCCESS;
    for (int i = 0; i < 10; i++) {
      if (ipmiUser[i].getUserenable() == 1)
        en_users++;
    }
    ipmiUser[id - 1].getUserAccess(flag, en_users, res, res_len);
  }

  else {
    flag = 1;
    for (int i = 0; i < 10; i++) {
      temp_name = ipmiUser[i].getUsername();
      if (temp_name.length() != 0)
        entry++;
    }
    res->cc = CC_SUCCESS;
    ipmiUser[id - 1].getUserAccess(flag, entry, res, res_len);
  }
}

void Ipmiapplication::app_get_session_info(uint8_t *request, uint8_t *response,
                                           uint8_t *res_len) {
  cout << "app_get_session_info" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t *data = &res->data[0];
  uint8_t type = req->data[0];
  std::vector<uint8_t> console_ip, console_mac;
  uint16_t console_port;
  int idx = -1;

  switch (type) {
  case 0x0:
    for (int i = 0; i < 5; i++) {
      if (ipmiSession[i].getSessionHandle() != 0) {
        idx = i;
        break;
      }
    }
    break;
  case 0xfe: {
    uint8_t handle = req->data[1];

    for (int i = 0; i < 5; i++) {
      if (handle == ipmiSession[i].getSessionHandle()) {
        idx = i;
        break;
      }
    }
    break;
  }
  case 0xff: {
    uint8_t user_id = req->data[1] & 0xff;

    for (int i = 0; i < 5; i++) {
      if (user_id == ipmiSession[i].getSessionUserId()) {
        idx = i;
        break;
      }
    }
    break;
  }
  }

  if (idx != -1) {
    uint8_t count = 0;
    for (int i = 0; i < 5; i++) {
      if (ipmiSession[i].getSessionHandle() != 0) {
        count++;
      }
    }
    for (int i = 0; i < 5; i++) {
      ipmiSession[i].setActiveSessionCount(count);
    }
    res->cc = CC_SUCCESS;
    *data++ = ipmiSession[idx].getSessionHandle();
    *data++ = ipmiSession[idx].getSessionSlotCount();
    *data++ = ipmiSession[idx].getActiveSessCount();
    *data++ = ipmiSession[idx].getSessionUserId();
    *data++ = ipmiSession[idx].getSessionPriv();
    *data++ = ipmiSession[idx].getSessionChanData();

    console_ip = ipmiSession[idx].getSessionIpAddr();
    console_mac = ipmiSession[idx].getSessionMacAddr();
    console_port = ipmiSession[idx].getSessionPort();

    memcpy(data, console_ip.data(), sizeof(uint8_t) * console_ip.size());
    data += sizeof(uint8_t) * console_ip.size();
    memcpy(data, console_mac.data(), sizeof(uint8_t) * console_mac.size());
    data += sizeof(uint8_t) * console_mac.size();
    *data++ = console_port & 0xff;
    *data++ = (console_port >> 8) & 0xff;
  } else
    res->cc = CC_UNSPECIFIED_ERROR;
  *res_len = data - &res->data[0];
}

void Ipmiapplication::app_bmc_selftest(uint8_t *response, uint8_t *res_len) {
  cout << "app_bmc_selftest" << endl;

  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];
  res->cc = CC_SUCCESS;

  *data++ = 0x55; // self-test result
  *data++ = 0x00; // Extra error info in case of failure
  *res_len = data - &res->data[0];
}

void Ipmiapplication::app_clear_message_flag(uint8_t *request,
                                             uint8_t *response,
                                             uint8_t *res_len) {
  cout << "app_clear_message_flag" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t param = req->data[0];
  uint8_t oem[3] = {((param & 0x20) >> 5), ((param & 0x40) >> 6),
                    ((param & 0x80) >> 7)};
  uint8_t f_watchdog = (param & 0x08) >> 3;
  uint8_t f_evtmsg = (param & 0x02) >> 1;
  uint8_t f_recvmsg = param & 0x1;

  res->cc = CC_SUCCESS;
  *res_len = 0;
}

void Ipmiapplication::app_cold_reset(uint8_t *response, uint8_t *res_len) {
  cout << "app_cold_reset" << endl;
  ipmi_res_t *res = (ipmi_res_t *)response;
  res->cc = CC_SUCCESS;
  // coldreset H/W

  *res_len = 0;
}

void Ipmiapplication::app_warm_reset(uint8_t *response, uint8_t *res_len) {
  cout << "app_warm_reset" << endl;
  ipmi_res_t *res = (ipmi_res_t *)response;
  res->cc = CC_SUCCESS;

  *res_len = 0;
}

void Ipmiapplication::app_set_global_enables(uint8_t *request,
                                             uint8_t *response,
                                             uint8_t *res_len) {
  cout << "app_set_global_enables" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];
  KETI_define::global_enabler = static_cast<int>(req->data[0]);
  //저장
  plat_globalenable_save();
  res->cc = CC_SUCCESS;
  *res_len = 0;
}

void Ipmiapplication::app_get_global_enables(uint8_t *response,
                                             uint8_t *res_len) {
  cout << "app_get_global_enables" << endl;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];

  res->cc = CC_SUCCESS;

  *data++ = KETI_define::global_enabler;
  ;
  *res_len = data - &res->data[0];
}

void Ipmiapplication::app_get_device_guid(uint8_t *response, uint8_t *res_len) {
  cout << "app_get_device_guid" << endl;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];

  res->cc = CC_SUCCESS;

  for (int i = 3; i >= 0; i--)
    *data++ = uuid_hex[i];
  *data++ = uuid_hex[5];
  *data++ = uuid_hex[4];
  *data++ = uuid_hex[7];
  *data++ = uuid_hex[6];

  for (int i = 8; i < 16; i++)
    *data++ = uuid_hex[i];

  *res_len = data - &res->data[0];
}

void Ipmiapplication::app_close_session(uint8_t *request, uint8_t *response,
                                        uint8_t *res_len) {
  cout << "app_close_session" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];

  std::vector<uint8_t> reqSessionId(req->data, req->data + 4);
  std::vector<uint8_t> currentSessionId;
  int index = 0;

  res->cc = CC_UNSPECIFIED_ERROR;

  for (int i = 4; i >= 0; i--) {
    currentSessionId = ipmiSession[i].getSessionMgntId();

    if (currentSessionId == reqSessionId) {
      index = i;
      res->cc = CC_SUCCESS;
      break;
    }
  }

  ipmiSession[index].closeSession();

  *res_len = 16;
}

void Ipmiapplication::app_set_session_priv(uint8_t *request, uint8_t *response,
                                           uint8_t *res_len) {
  cout << "app_set_session_priv" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];
  uint8_t req_priv = req->data[0];
  int ret = 0;
  for (int i = 4; i >= 0; i--) {
    uint8_t handle = ipmiSession[i].getSessionHandle();
    if (handle >= 32) {

      ret = ipmiSession[i].setSessionPriv(req_priv);

      if (ret == 0) {
        res->cc = CC_SUCCESS;
        *data++ = req_priv;
      } else if (ret == -1)
        res->cc = 0x80;
      else {
        res->cc = 0x81;
      }
      break;
    }
  }
  *res_len = data - &res->data[0];
}

void Ipmiapplication::app_set_channel_access(uint8_t *request,
                                             uint8_t *response,
                                             uint8_t *res_len) {
  cout << "app_set_channel_access" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t ch_num = req->data[0] & 0x0f;

  ipmiChannel[ch_num].setChannelAccess(req);

  res->cc = CC_SUCCESS;
  *res_len = 0;
}

void Ipmiapplication::app_get_channel_access(uint8_t *request,
                                             uint8_t *response,
                                             uint8_t *res_len) {
  cout << "app_get_channel_access" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t ch_num = req->data[0] & 0x0f;

  ipmiChannel[ch_num].getChannelAccess(res, res_len);
}
/**
 *
 * @author KICHEOL
 * @bug event 를 위한 channel은 14인데 현재는 0~8까지의 channel만보유 따라서
 * 임시로 ipmichannel[0]으로 수정 오류 발생예정
 */
void Ipmiapplication::app_get_channel_info(uint8_t *request, uint8_t *response,
                                           uint8_t *res_len) {
  cout << "app_get_channel_info" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];
  uint8_t param = req->data[0];
  uint8_t ch_num = param & 0x0f;
  printf("chnum : %d\n", ch_num);
  if (ch_num <= 8) {
    ipmiChannel[ch_num].getChannelInfo(res, res_len, ch_num);
    res->cc = CC_SUCCESS;
  } else {
    // ipmiChannel[0].getChannelInfo(res, res_len, ch_num);
    res->cc = CC_UNKNOWN;
  }
}

void Ipmiapplication::app_get_channel_auth_cap(uint8_t *request,
                                               uint8_t *response,
                                               uint8_t *res_len) {
  cout << "app_get_channel_auth_cap" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  // //기철 수정
  // unsigned char *data = &res->data[0];
  // res->cc = CC_SUCCESS;
  // //

  uint8_t chan = (req->data[0] % 0x10);
  uint8_t priv = (req->data[1] % 0x10);
  uint8_t lan_chan = 0;
  uint8_t flag_not_null_username = 0;
  uint8_t flag_null_username = 0;
  uint8_t flag_anon_login = 0;

  //기철 수정
  cout << "channel count =" << (int)chan << endl;
  switch (chan) {
  case 1:
    lan_chan = 0;
    break;
  case 6:
    lan_chan = 1;
    break;
  case 7:
    lan_chan = 2;
    break;
  case 8:
    lan_chan = 3;
    break;
  default:
    lan_chan = 0;
  }

  for (int i = 0; i < 10; i++) {
    if (ipmiUser[i].getUserenable() == 1) {
      if (ipmiUser[i].getUsername() != "") {
        flag_not_null_username = 1;
        break;
      }
    }
  }

  if (ipmiUser[0].getUserenable() == 1) {
    if (ipmiUser[0].getUsername() == "") {
      if (ipmiUser[0].getUserpassword() == "") {
        flag_null_username = 1;
      } else {
        flag_anon_login = 1;
      }
    }
  }

  if (flag_null_username == 0)
    this->g_authrsp[lan_chan].null_usernames = 0b0;
  else
    this->g_authrsp[lan_chan].null_usernames = 0b1;

  if (flag_not_null_username == 0)
    this->g_authrsp[lan_chan].non_null_usernames = 0b0;
  else
    this->g_authrsp[lan_chan].non_null_usernames = 0b1;

  if (flag_anon_login == 0)
    this->g_authrsp[lan_chan].anon_login_enabled = 0b0;
  else
    this->g_authrsp[lan_chan].anon_login_enabled = 0b1;

  res->cc = CC_SUCCESS;
  memcpy(res->data, &this->g_authrsp[lan_chan], sizeof(channel_auth_cap_t));

  *res_len = sizeof(channel_auth_cap_t);
}

void Ipmiapplication::app_get_device_id(uint8_t *response, uint8_t *res_len) {
  cout << "app_get_device_id" << endl;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];

  res->cc = CC_SUCCESS;

  *data++ = this->g_mc_device.mc_device_id;
  *data++ = this->g_mc_device.mc_device_rev;
  *data++ = this->g_mc_device.mc_fw_version[0];
  *data++ = this->g_mc_device.mc_fw_version[1];
  *data++ = this->g_mc_device.mc_ipmi_version;
  *data++ = this->g_mc_device.mc_additional_dev;
  *data++ = this->g_mc_device.mc_mnfr_id[0];
  *data++ = this->g_mc_device.mc_mnfr_id[1];
  *data++ = this->g_mc_device.mc_mnfr_id[2];
  *data++ = this->g_mc_device.mc_prod_id[0];
  *data++ = this->g_mc_device.mc_prod_id[1];
  *data++ = this->g_mc_device.mc_aux_fw_version[0];
  *data++ = this->g_mc_device.mc_aux_fw_version[1];
  *data++ = this->g_mc_device.mc_aux_fw_version[2];
  *data++ = this->g_mc_device.mc_aux_fw_version[3];

  cout << "Get Device Info" << endl;
  printf("mc_device_id : 0x%02x\n", this->g_mc_device.mc_device_id);
  printf("mc_device_rev : 0x%02x\n", this->g_mc_device.mc_device_rev);
  printf("mc_fw_version[0] : 0x%02x \n", this->g_mc_device.mc_fw_version[0]);
  printf("mc_fw_version[1] : 0x%02x\n", this->g_mc_device.mc_fw_version[1]);
  printf("mc_ipmi_version : 0x%02x \n", this->g_mc_device.mc_ipmi_version);
  printf("mc_additional_dev : 0x%02x \n", this->g_mc_device.mc_additional_dev);
  printf("mc_mnfr_id : 0x%02x \n", this->g_mc_device.mc_mnfr_id[0]);
  printf("mc_mnfr_id : 0x%02x \n", this->g_mc_device.mc_mnfr_id[1]);
  printf("mc_mnfr_id : 0x%02x \n", this->g_mc_device.mc_mnfr_id[2]);
  printf("mc_prod_id : 0x%02x \n", this->g_mc_device.mc_prod_id[0]);
  printf("mc_prod_id : 0x%02x \n", this->g_mc_device.mc_prod_id[1]);
  printf("mc_aux_fw_version : 0x%02x \n",
         this->g_mc_device.mc_aux_fw_version[0]);
  printf("mc_aux_fw_version : 0x%02x \n",
         this->g_mc_device.mc_aux_fw_version[1]);
  printf("mc_aux_fw_version : 0x%02x \n",
         this->g_mc_device.mc_aux_fw_version[2]);
  printf("mc_aux_fw_version : 0x%02x \n",
         this->g_mc_device.mc_aux_fw_version[3]);

  *res_len = data - &res->data[0];
}

void Ipmiapplication::ipmi_handle_app(uint8_t *request, uint8_t *response,
                                      uint8_t *res_len) {
  cout << "ipmi_handle_app" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint8_t cmd = req->cmd;
  pthread_mutex_lock(&m_app);
  switch (cmd) {
  case CMD_APP_SET_WDT:
    app_set_watchdog_timer_params(request, response, res_len);
    WriteConfigurationFile(ConfigurationFileDir);
    break;
  case CMD_APP_GET_WDT:
    app_get_watchdog_timer_params(request, response, res_len);
    break;
  case CMD_APP_GET_DEVICE_ID:
    app_get_device_id(response, res_len);
    break;
  case CMD_APP_GET_CHANNEL_AUTH_CAP:
    app_get_channel_auth_cap(request, response, res_len);
    break;
  case CMD_APP_SET_SESSION_PRIV:
    app_set_session_priv(request, response, res_len);
    break;
  case CMD_APP_CLOSE_SESSION:
    app_close_session(request, response, res_len);
    break;
  case CMD_APP_GET_SYSTEM_GUID:
    app_get_device_guid(response, res_len);
    break;
  case CMD_APP_SET_GLOBAL_ENABLES:
    app_set_global_enables(request, response, res_len);
    break;
  case CMD_APP_GET_GLOBAL_ENABLES:
    app_get_global_enables(response, res_len);
    break;
  case CMD_APP_COLD_RESET:
    app_cold_reset(response, res_len);
    break;
  case CMD_APP_WARM_RESET:
    app_warm_reset(response, res_len);
    break;
  case CMD_APP_CLEAR_MESSAGE_FLAGS:
    app_clear_message_flag(request, response, res_len);
    break;
  case CMD_APP_GET_SELFTEST_RESULTS:
    app_bmc_selftest(response, res_len);
    break;
  case CMD_APP_GET_SESSION_INFO:
    app_get_session_info(request, response, res_len);
    break;
  case CMD_APP_GET_CHANNEL_INFO:
    app_get_channel_info(request, response, res_len);
    break;
  case CMD_APP_GET_CHANNEL_ACCESS:
    app_get_channel_access(request, response, res_len);
    break;
  case CMD_APP_GET_USER_NAME:
    app_get_username(request, response, res_len);
    break;
  case CMD_APP_GET_USER_ACCESS:
    app_get_useraccess(request, response, res_len);
    break;
  case CMD_APP_SET_USER_NAME:
    app_set_username(request, response, res_len);
    break;
  case CMD_APP_SET_USER_ACCESS:
    app_set_useraccess(request, response, res_len);
    break;
  case CMD_APP_SET_USER_PASSWD:
    app_set_user_passwd(request, response, res_len);
    break;
  case CMD_APP_SET_SYS_INFO_PARAMS:
    break;
  case CMD_APP_GET_SYS_INFO_PARAMS:
    break;
  case CMD_APP_SOL_ACTIVE_PAYLOAD:
    log(info) << "start CMD_APP_SOL_ACTIVE_PAYLOAD";
    (SerialOverLan::Instance().app_active_payload(req, res, res_len));
    log(info) << "end CMD_APP_SOL_ACTIVE_PAYLOAD";
    break;
  case CMD_APP_SOL_DEACTIVATE_PAYLOAD:
    log(info) << "start CMD_APP_SOL_DEACTIVATE_PAYLOAD";
    (SerialOverLan::Instance().app_deactive_payload(req, res, res_len));
    log(info) << "end CMD_APP_SOL_DEACTIVATE_PAYLOAD";
    break;
  default:
    res->cc = 0xfe;
    break;
  }
  pthread_mutex_unlock(&m_app);
}
#define IPMI_EVENT_LOG_VIEW 1
static void ipmi_handle_sensor(ipmi_req_t *request, ipmi_res_t *response,
                               uint8_t *res_len) {
  cout << "ipmi_handle_sensor" << endl;
  uint8_t cmd = request->cmd;
  response->cc = CC_SUCCESS;

  switch (cmd) {
  case CMD_SENSOR_GET_SENSOR_READING:
    sensor_get_reading(request, response, res_len);
    break;
  case CMD_SENSOR_GET_SENSOR_THRESHOLD:
    sensor_get_threshod(request, response, res_len);
    break;
  case CMD_SENSOR_SET_SENSOR_THRESHOLD:
    sensor_set_threshold(request, response, res_len);
    break;
  case CMD_SENSOR_GET_DEVICE_SDR_INFO:
    storage_get_sdr_info(response, res_len);
    break;
  case CMD_SENSOR_RES_DEVICE_SDR_REPOSITORY:
    storage_rsv_sdr(response, res_len);
    break;
#if IPMI_EVENT_LOG_VIEW
  case CMD_SENSOR_PLATFORM_EVENT: // !!rjs : for "event [1|2|3]"
    // (sensor_event_get_num(request, response, res_len));
    IPMI_Handle_Event::ipmi_handle_sel(request,response,res_len);
    response->cc = CC_SUCCESS;
    // storage_add_sel(request, response, res_len);정상동작확인
    break;
  case CMD_GET_PEF_CAPABILITIES:
    cout << "CMD_GET_PEF_CAPABILITIES" << endl;
    (pef_capabilities_info(response, res_len));
    response->cc = CC_SUCCESS;
    break;
  case CMD_SET_PEF_CONFIG_PARMS:
    cout << "CMD_SET_PEF_CONFIG_PARMS" << endl;
    (pef_set_config_param(request, response, res_len));
    response->cc = CC_SUCCESS;
    break;
  case CMD_GET_PEF_CONFIG_PARMS:
    cout << "CMD_GET_PEF_CONFIG_PARMS" << endl;
    (pef_get_config_param(request, response, res_len));
    break;

  case CMD_GET_LAST_PROCESSED_EVT_ID:
    cout << "CMD_GET_LAST_PROCESSED_EVT_ID" << endl;
    (pef_get_status(response, res_len));
    response->cc = CC_SUCCESS;
    break;
#endif
  default:
    response->cc = CC_INVALID_CMD;
    break;
  }
}

static void ipmi_handle_storage(ipmi_req_t *request, ipmi_res_t *response,
                                uint8_t *res_len) {
  cout << "ipmi_handle_storage" << endl;
  uint8_t cmd = request->cmd;
  int s_ret = 0;
  switch (cmd) {
  case CMD_STORAGE_CLR_SDR_REPO:
    storage_clear_sdr_repository(response, res_len);
    break;
  case CMD_STORAGE_GET_SDR_INFO:
    storage_get_sdr_info(response, res_len);
    break;
  case CMD_STORAGE_RSV_SDR:
    storage_rsv_sdr(response, res_len);
    break;
  case CMD_STORAGE_GET_SDR:
    storage_get_sdr(request, response, res_len);
    break;
  // add fru fru 핸들러 추가
  case CMD_STORAGE_GET_FRUID_INFO:
    storage_get_fru_info(request, response, res_len);
    break;
  case CMD_STORAGE_READ_FRUID_DATA:
    storage_get_fru(request, response, res_len);

    break;
  case CMD_STORAGE_WRITE_FRUID_DATA:
    cout << "CMD_STORAGE_WRITE_FRUID_DATA" << endl;
    break;
  case CMD_STORAGE_GET_SEL_INFO:
    cout << "CMD_STORAGE_GET_SEL_INFO DATA" << endl;
    storage_get_sel_info(response, res_len);
    break;
    // sel 파트
  case CMD_STORAGE_RSV_SEL:
    if ((KETI_define::global_enabler & 0x8) != 0x8) {
      response->cc = CC_DISABLED;
      break;
    }
    storage_rsv_sel(response, res_len);
    break;
  case CMD_STORAGE_ADD_SEL:
    cout << "CMD_STORAGE_ADD_SEL" << endl;
    if ((KETI_define::global_enabler & 0x8) != 0x8) {
      response->cc = CC_DISABLED;
      break;
    }
    cout << "BF storage_add_sel" << endl;
    storage_add_sel(request, response, res_len);
    cout << "AF storage_add_sel" << endl;
    break;
  case CMD_STORAGE_GET_SEL:
    if ((KETI_define::global_enabler & 0x8) != 0x8) {
      response->cc = CC_DISABLED;
      break;
    }
    storage_get_sel(request, response, res_len);
    break;
  case CMD_STORAGE_DEL_SEL_ENTRY:
    if ((KETI_define::global_enabler & 0x8) != 0x8) {
      response->cc = CC_DISABLED;
      break;
    }
    storage_del_sel_entry(request, response, res_len);
    break;
  case CMD_STORAGE_CLR_SEL:
    if ((KETI_define::global_enabler & 0x8) != 0x8) {
      response->cc = CC_DISABLED;
      break;
    }
    storage_clr_sel(request, response, res_len);
    break;
    /////////////////////
  }
}

// 	CMD_STORAGE_DEL_SEL_ENTRY = 0x46,
// 	CMD_STORAGE_CLR_SEL = 0x47,
// 	CMD_STORAGE_GET_SEL_TIME = 0x48,
// 	CMD_STORAGE_SET_SEL_TIME = 0x49,
// 	CMD_STORAGE_GET_SEL_UTC = 0x5C,
static void ipmi_handle_transport(ipmi_req_t *request, ipmi_res_t *response,
                                  uint8_t *res_len) {
  cout << "ipmi_handle_transport" << endl;
  uint8_t cmd = request->cmd;
  uint8_t chan = request->data[0] & 0xf;

  uint8_t r_chan = 0;

  switch (chan) {
  case 1:
    r_chan = 0;
    break;
  case 6:
    r_chan = 1;
    break;
  case 7:
    r_chan = 2;
    break;
  case 8:
    r_chan = 3;
    break;
  }

  switch (cmd) {
  case CMD_TRANSPORT_SET_LAN_CONFIG:
    ipmiNetwork[r_chan].set_lan_config(request, response, res_len);
    break;
  case CMD_TRANSPORT_GET_LAN_CONFIG:
    ipmiNetwork[r_chan].get_lan_config(request, response, res_len);
    break;
  case CMD_TRANSPORT_GET_SOL_CONFIG_PARAMETERS: {
    log(info) << "=======CMD_TRANSPORT_GET_SOL_CONFIG_PARAMETERS ========";
    SerialOverLan::Instance().transport_get_sol_config_params(request, response,
                                                              res_len);
    log(info) << "=======end ========";
    break;
  case CMD_TRANSPORT_SET_SOL_CONFIG_PARAMETERS: {
    log(info) << "=======CMD_TRANSPORT_GET_SOL_CONFIG_PARAMETERS ========";
    SerialOverLan::Instance().transport_set_sol_config_params(request, response,
                                                              res_len);
    log(info) << "=======end ========";
    break;
  }
  }
  default:
    response->cc = CC_INVALID_CMD;
    break;
  }
  cout << "Leave apps.cpp:ipmi_handle_transport \n" << endl;
}
/**
 * @brief Web REST 요청시 처리를 위한 핸들러
 * @bug rest_req_t 없음
 */
void ipmi_handle_rest(rest_req_t *req, uint8_t *response, int *res_len) {
  // cout << "apps.cpp:ipmi_handle_rest start \n" << endl;
  unsigned char cmd = req->cmd;
  unsigned char IP_ADDRS[4];
  unsigned char NET_ADDRS[4];
  unsigned char dns_ipv4_pre[4];
  unsigned char dns_ipv4_alt[4];
  unsigned char dns_ipv6[39];
  unsigned char domain_name[50] = {0};
  unsigned char host_name[50] = {0};
  float msg_data[10];

  // cout << "\t apps cmd " << cmd << endl;
  // cout << "\t apps.cpp:ipmi_handle_rest end" << endl;
  switch (cmd) {
  case CMD_TRY_LOGIN: {
    cout << "\t========== try login call ==========" << endl;
    char username[16], pwd[16];
    memcpy(username, req->data, 16);
    memcpy(pwd, req->data + 16, 16);
    char cmd[32];
    string _username(username);
    string _pwd(pwd);

    log(info) << "[try login] username : " << _username;
    log(info) << "[try login] password : " << _pwd;

    response[0] = 0;

    if (response[0] == NULL) {
      response[0] = authenticate_ipmi(_username, _pwd);
    }
    if (response[0] == NULL) {
      response[0] = authenticate_ldap(_username, _pwd);
    }
    if (response[0] == NULL) {
      response[0] = authenticate_ad(_username, _pwd);
    }

    log(info) << "user priv : " << (int)response[0];
    sprintf(response, "%d", response[0]);
    *res_len = strlen(response);
    if (ipmiUser[0].getUsername() == _username)
      if (ipmiUser[0].getUserpassword() == _pwd)
        sprintf(response, "%d", 4);
    *res_len = strlen(response);
    cout << "\t========== try login call end ==========" << endl;
    break;
  }
  case CMD_SHOW_MAIN:
    cout << "\t========== show main call ==========" << endl;
    *res_len = rest_show_main(req->data[0], response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end main call ==========" << endl;
    break;
  case CMD_GET_FRUINFO:
    cout << "\t========== get fru info ==========" << endl;
    *res_len = rest_get_fru_config(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get fru ==========" << endl;
    break;
  case CMD_SET_FRU_HEADER: {
    cout << "\t========== set fru header ==========" << endl;
    int fru_id = (int)req->data[0];
    int hdr_board = (int)req->data[1]; // 1 or 0
    int hdr_product = (int)req->data[2];
    int hdr_chassis = (int)req->data[3];
    rest_set_fru_header(fru_id, hdr_board, hdr_chassis, hdr_product);
    cout << "\t========== end set fru header ==========" << endl;
    break;
  }
  case CMD_SET_FRU_BOARD: {
    cout << "\t========== set fru board ==========" << endl;
    int fru_id = (int)req->data[0];
    char year[5] =
        {
            0,
        },
         month[3] =
             {
                 0,
             },
         day[4], hour[3], minute[3], sec[3];
    char mfg[LEN_MFG], product[LEN_PRODUCT], serial[LEN_SERIAL],
        part_num[LEN_PART_NUM];
    unsigned char datetime[30];
    unsigned char mfg_dates[4];
    struct tm time;
    unsigned short c_year = 0;
    unsigned char c_month, c_day, c_hour, c_min, c_sec = 0;
    time_t c_time;

    memcpy(year, req->data + 1, 5);
    memcpy(month, req->data + 1 + 5, 3);
    memcpy(day, req->data + 1 + 5 + 3, 3);
    memcpy(hour, req->data + 1 + 5 + 3 + 3, 3);
    memcpy(minute, req->data + 1 + 5 + 3 + 3 + 3, 3);
    memcpy(sec, req->data + 1 + 5 + 3 + 3 + 3 + 3, 3);
    memcpy(mfg, req->data + 1 + LEN_MFG_DATE, LEN_MFG);
    memcpy(product, req->data + 1 + LEN_MFG_DATE + LEN_MFG, LEN_PRODUCT);
    memcpy(serial, req->data + 1 + LEN_MFG_DATE + LEN_MFG + LEN_PRODUCT,
           LEN_SERIAL);
    memcpy(part_num,
           req->data + 1 + LEN_MFG_DATE + LEN_MFG + LEN_PRODUCT + LEN_SERIAL,
           LEN_PART_NUM);
    c_year = atoi(year);
    c_month = atoi(month);
    c_day = atoi(day);
    c_hour = atoi(hour);
    c_min = atoi(minute);
    c_sec = atoi(sec);

    sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d", c_year, c_month, c_day,
            c_hour, c_min, c_sec);
    strptime(datetime, "%Y-%m-%d %H:%M:%S", &time);

    c_time = mktime(&time);

    // convert time_t -> char [4] (ex. 1609459200 -> 5f ee 66 00)
    mfg_dates[0] = c_time >> 24;
    mfg_dates[1] = (c_time >> 16) & 0xff;
    mfg_dates[2] = (c_time >> 8) & 0xff;
    mfg_dates[3] = c_time & 0xff;

    rest_set_fru_board(fru_id, mfg_dates, mfg, product, serial, part_num);

    cout << "\t========== end set fru board ==========" << endl;
    break;
  }
  case CMD_SET_FRU_PRODUCT: {
    cout << "\t========== set fru product ==========" << endl;
    int fru_id = (int)req->data[0];
    char name[LEN_NAME], mfg[LEN_MFG], version[LEN_VERSION], serial[LEN_SERIAL],
        part_num[LEN_PART_NUM];
    memcpy(name, req->data + 1, LEN_NAME);
    memcpy(mfg, req->data + 1 + LEN_NAME, LEN_MFG);
    memcpy(version, req->data + 1 + LEN_NAME + LEN_MFG, LEN_VERSION);
    memcpy(serial, req->data + 1 + LEN_NAME + LEN_MFG + LEN_VERSION,
           LEN_SERIAL);
    memcpy(part_num,
           req->data + 1 + LEN_NAME + LEN_MFG + LEN_VERSION + LEN_SERIAL,
           LEN_PART_NUM);

    rest_set_fru_product(fru_id, name, mfg, version, serial, part_num);

    cout << "\t========== end set fru product ==========" << endl;
    break;
  }
  case CMD_SET_FRU_CHASSIS: {
    cout << "\t========== set fru chassis ==========" << endl;
    int fru_id = (int)req->data[0];
    char type[LEN_TYPE], serial[LEN_SERIAL], part_num[LEN_PART_NUM];
    memcpy(type, req->data + 1, LEN_TYPE);
    memcpy(serial, req->data + 1 + LEN_TYPE, LEN_SERIAL);
    memcpy(part_num, req->data + 1 + LEN_TYPE + LEN_SERIAL, LEN_PART_NUM);
    unsigned char real_type = atoi(type);

    rest_set_fru_chassis(fru_id, real_type, serial, part_num);

    cout << "\t========== end set fru chassis ==========" << endl;
    break;
  }
  case CMD_GET_SETTING_SERVICE: {
    cout << "\t========== get setting ==========" << endl;
    *res_len = get_setting_service(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get setting ==========" << endl;
    break;
  }
  case CMD_SET_SETTING_SERVICE: {
    cout << "\t========== set setting ==========" << endl;
    char str[32];
    strcpy(str, req->data + 1);
    set_setting_service(req->data[0], str);

    cout << "\t========== end set setting ==========" << endl;
    break;
  }
  case CMD_GET_POWER_STATUS: {
    cout << "\t========== get power status ==========" << endl;
    *res_len = rest_get_power_status(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get power status ==========" << endl;
    break;
  }
  case CMD_SET_POWER_STATUS: {
    cout << "\t========== set power status ==========" << endl;
    ipmiChassis.chassis_control((ipmi_req_t *)req, (ipmi_res_t *)response,
                                (uint8_t *)res_len);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end set power status ==========" << endl;
    break;
  }
  case CMD_GET_SYSINFO: {
    cout << "\t========== get sysinfo ==========" << endl;
    *res_len = rest_get_sysinfo_config(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get sysinfo ==========" << endl;
    break;
  }
  case CMD_GET_SENSOR: {
    cout << "\t========== get sensor config ==========" << endl;
    *res_len = rest_get_sensor_config(response);
    // cout << "\t response=" << (char *)response << endl;
    // cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get sensor config ==========" << endl;
    break;
  }
  case CMD_SENSOR_SET_THRESH: {
    cout << "\t========== set sensor thresh ==========" << endl;
    float th_data[10] = {0};
    memcpy(th_data, req->data, sizeof(float) * 10);

    uint8_t s_num = (int)th_data[6];
    int s_index = 0;
    uint8_t sdr_idx;
    sensor_thresh_t *p_sdr;

    printf("\t\t s_num : %d\n", s_num);

    sdr_idx = plat_find_sdr_index(s_num);
    p_sdr = sdr_rec[sdr_idx].find(sdr_idx)->second.sdr_get_entry();

    for (int i = 0; i < 10; i++) {
      printf("th_data %d : %f\n", i, th_data[i]);
      // printf("convert_data %d : %f\n", i,
      // sdr_convert_sensor_value_to_raw(p_sdr, th_data[i]));
    }

    printf("\t\tdy : before sensor\n");
    sdr_rec[sdr_idx].find(sdr_idx)->second.print_sensor_info();
    if (p_sdr != NULL) {
      if (p_sdr->uc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->uc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[0]);
        printf("\t\t uc updated\n");
      }
      if (p_sdr->unc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->unc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[1]);
        printf("\t\t unc updated\n");
      }
      if (p_sdr->unr_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->unr_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[2]);
        printf("\t\t unr updated\n");
      }
      if (p_sdr->lc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->lc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[3]);
        printf("\t\t lc updated\n");
      }
      if (p_sdr->lnc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->lnc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[4]);
        printf("\t\t lnc updated\n");
      }
      if (p_sdr->lnr_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->lnr_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[5]);
        printf("\t\t lnr updated\n");
      }
      file_store_sdr_data(s_index, (sdr_rec_t *)p_sdr);
    }

    printf("\t\tdy : after sensor\n");
    sdr_rec[sdr_idx].find(sdr_idx)->second.print_sensor_info();

    cout << "\t========== end set sensor thresh ==========" << endl;
    break;
  }
  case CMD_GET_EVENT: {
    cout << "\t========== get event log () ==========" << endl;
    *res_len = rest_get_eventlog_config(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get event log ==========" << endl;
    break;
  }
  case CMD_GET_DNS: {
    cout << "\t========== get dns info ==========" << endl;
    *res_len = rest_get_ddns_info(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get dns info ==========" << endl;
    break;
  }
  case CMD_SET_DNS_HOSTNAME: {
    cout << "\t========== set dns host name ==========" << endl;
    memcpy(host_name, req->data, strlen(req->data));
    *res_len = set_ddns_host_name(host_name);
    cout << "\t========== end set dns host name ==========" << endl;
    break;
  }
  case CMD_SET_DNS_DOMAIN: {
    cout << "\t========== set dns domain name ==========" << endl;
    memcpy(domain_name, req->data, strlen(req->data));
    *res_len = set_ddns_domain_name(2, domain_name);
    cout << "\t========== end set dns domain name ==========" << endl;
    break;
  }
  case CMD_SET_DNS_IPV4_PREFER: {
    cout << "\t========== set dns ipv4 prefer ==========" << endl;
    *res_len = set_ddns_nameserver(1, req->data);
    cout << "\t========== end set dns ipv4 prefer ==========" << endl;
    break;
  }
  case CMD_SET_DNS_IPV4_ALTER: {
    cout << "\t========== set dns ipv4 alter ==========" << endl;
    *res_len = set_ddns_nameserver(2, req->data);
    cout << "\t========== end set dns ipv4 alter ==========" << endl;
    break;
  }
  case CMD_SET_DNS_IPV6_PREFER: {
    cout << "\t========== set dns ipv6 prefer ==========" << endl;
    *res_len = set_ddns_nameserver(1, req->data);
    cout << "\t========== end set dns ipv6 prefer ==========" << endl;
    break;
  }
  case CMD_GET_LANINFO: {
    cout << "\t========== get lan info ==========" << endl;
    *res_len = rest_get_lan_config(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get lan info ==========" << endl;
    break;
  }
  case CMD_SET_LAN_MAC_ADDR: {
    cout << "\t========== set lan mac addr ==========" << endl;
    printf("SET MAC Address - Ethernet : %x / MAC: %x:%x:%x:%x:%x:%x\n",
           req->data[0], req->data[1], req->data[2], req->data[3], req->data[4],
           req->data[5], req->data[6]);
    // lc_flag ++; lan changed flag
    memcpy(&(ipmiNetwork[req->data[0]].mac_addr[0]), req->data + 1, 6);
    ipmiNetwork[req->data[0]].lanConfigChanged[LAN_PARAM_MAC_ADDR] = 1;
    cout << "\t========== end set lan mac addr ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV4_DHCP: {
    cout << "\t========== set lan ipv4 dhcp ==========" << endl;
    printf("REST Set DHCP OR STATIC(1 or 0): %d\n", req->data[1]);
    // lc_flag ++; lan changed flag

    ipmiNetwork[req->data[0]].ip_src = (req->data[1] + 1);
    ipmiNetwork[req->data[0]].lanConfigChanged[LAN_PARAM_IP_SRC] = 1;
    cout << "\t========== end set lan ipv4 dhcp ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV4_IP_NETMASK: {
    cout << "\t========== set lan ipv4 ip netmask ==========" << endl;
    // lc_flag ++;
    memcpy(IP_ADDRS, req->data + 1, sizeof(unsigned char) * 4);
    memcpy(NET_ADDRS, req->data + 1 + 4, sizeof(unsigned char) * 4);
    printf("REST Set IP Address : %x / IP: %d.%d.%d.%d\n", req->data[0],
           IP_ADDRS[0], IP_ADDRS[1], IP_ADDRS[2], IP_ADDRS[3]);
    printf("REST Set Netmask Address : %x / IP: %d.%d.%d.%d\n", req->data[0],
           NET_ADDRS[0], NET_ADDRS[1], NET_ADDRS[2], NET_ADDRS[3]);

    memcpy(&(ipmiNetwork[req->data[0]].ip_addr[0]), IP_ADDRS, 4);
    memcpy(&(ipmiNetwork[req->data[0]].net_mask[0]), NET_ADDRS, 4);
    ipmiNetwork[req->data[0]].lanConfigChanged[LAN_PARAM_IP_ADDR] = 1;
    ipmiNetwork[req->data[0]].lanConfigChanged[LAN_PARAM_NET_MASK] = 1;

    cout << "\t========== end set lan ipv4 ip netmask ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV4_GATEWAY: {
    cout << "\t========== set lan ipv4 ip gateway ==========" << endl;
    printf("Set IPv4 Gateway Address : %x / GATEWAY: %d.%d.%d.%d\n",
           req->data[0], req->data[1], req->data[2], req->data[3],
           req->data[4]);
    // lc_flag ++;

    memcpy(&(ipmiNetwork[req->data[0]].df_gw_ip_addr[0]), req->data + 1, 4);
    ipmiNetwork[req->data[0]].lanConfigChanged[LAN_PARAM_DF_GW_IP_ADDR] = 1;
    cout << "\t========== end set lan ipv4 ip gateway ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV6_ENABLE: {
    cout << "\t========== set lan ipv6 enable ==========" << endl;
    ipmiNetwork[req->data[0]].set_enable_v6 = req->data[1];
    cout << "\t========== end set lan ipv6 enable ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV6_DHCP: {
    cout << "\t========== set lan ipv6 dhcp ==========" << endl;
    int ip_src_v6 = 0;
    if (req->data[1] == 1)
      ip_src_v6 = 2;

    ipmiNetwork[req->data[0]].ip_src_v6 = ip_src_v6;
    ipmiNetwork[req->data[0]].lanConfigChanged[LAN_PARAM_IPV6_SRC] = 1;
    cout << "\t========== end set lan ipv6 dhcp ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV6_IP: {
    cout << "\t========== set lan ipv6 ip ==========" << endl;
    char ip[SIZE_IP_ADDR_V6] = {
        0,
    };
    memcpy(ip, &(req->data[1]), SIZE_IP_ADDR_V6);
    ipmiNetwork[req->data[0]].ip_addr_v6.assign(ip, ip + SIZE_IP_ADDR_V6);
    cout << "\t========== end set lan ipv6 ip ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV6_PREFIX: {
    cout << "\t========== set lan ipv6 prefix ==========" << endl;
    ipmiNetwork[req->data[0]].net_mask_v6[0] = req->data[1];
    cout << "\t========== end set lan ipv6 prefix ==========" << endl;
    break;
  }
  case CMD_SET_LAN_IPV6_GATEWAY: {
    cout << "\t========== set lan ipv6 gateway ==========" << endl;
    char gw[SIZE_IP_ADDR_V6] = {
        0,
    };
    memcpy(gw, &(req->data[1]), SIZE_IP_ADDR_V6);
    ipmiNetwork[req->data[0]].df_gw_ip_addr_v6.assign(gw, gw + SIZE_IP_ADDR_V6);
    cout << "\t========== end set lan ipv6 gateway ==========" << endl;
    break;
  }
  case CMD_SET_LAN_VLAN_ENABLE: {
    cout << "\t========== set lan vlan enable ==========" << endl;
    unsigned char enable_vlan = 0x80;

    if (req->data[1] == 1) {
      ipmiNetwork[req->data[0]].vlan_enable = enable_vlan;
    } else {
      char cmds[100] = {
          0,
      };

      sprintf(cmds, "ifconfig eth0.%d down", ipmiNetwork[req->data[0]].vlan_id);
      system(cmds);
      memset(cmds, 0, sizeof(cmds));

      sprintf(cmds, "vconfig rem eth0.%d", ipmiNetwork[req->data[0]].vlan_id);
      system(cmds);

      ipmiNetwork[req->data[0]].vlan_enable = 0;
      ipmiNetwork[req->data[0]].vlan_id = 0;
    }
    cout << "\t========== end set lan vlan enable ==========" << endl;
    break;
  }
  case CMD_SET_LAN_VLAN_ID: {
    char cmds[100] = {
        0,
    };
    unsigned char enable_vlan = 0x80;

    ipmiNetwork[req->data[1]].vlan_enable = enable_vlan;
    ipmiNetwork[req->data[1]].vlan_id = req->data[0];

    sprintf(cmds, "vconfig add eth0 %d", ipmiNetwork[req->data[1]].vlan_id);
    system(cmds);
    memset(cmds, 0, sizeof(cmds));

    sprintf(cmds, "ifconfig eth0.%d %d.%d.%d.%d netmask %d.%d.%d.%d up",
            ipmiNetwork[req->data[1]].vlan_id,
            ipmiNetwork[req->data[1]].ip_addr[0],
            ipmiNetwork[req->data[1]].ip_addr[1],
            ipmiNetwork[req->data[1]].ip_addr[2],
            ipmiNetwork[req->data[1]].ip_addr[3],
            ipmiNetwork[req->data[1]].net_mask[0],
            ipmiNetwork[req->data[1]].net_mask[1],
            ipmiNetwork[req->data[1]].net_mask[2],
            ipmiNetwork[req->data[1]].net_mask[3]);
    system(cmds);
    break;
  }
  case CMD_SET_LAN_VLAN_PRIORITY: {
    cout << "\t========== set vlan priority ==========" << endl;
    ipmiNetwork[req->data[0]].vlan_priority = req->data[1];
    cout << "\t========== end set vlan priority ==========" << endl;
    break;
  }
  case CMD_SET_LAN_PRIORITY: {
    cout << "\t========== set lan priority ==========" << endl;
    set_eth_priority(req->data[0] - '0');
    cout << "\t========== end set lan priority ==========" << endl;
    break;
  }
  case CMD_GET_NTP: {
    cout << "\t========== get ntp info ==========" << endl;
    *res_len = parse_ntp_conf_file(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get ntp info ==========" << endl;
    break;
  }
  case CMD_SET_NTP_AUTO: {
    cout << "\t========== set ntp info ==========" << endl;
    *res_len = set_ntp_conf_auto(req->data);
    cout << "\t========== end set ntp info ==========" << endl;
    break;
  }
  case CMD_GET_SMTPINFO: {
    cout << "\t========== get smtp info ==========" << endl;
    *res_len = rest_get_smtp_json(response, 0);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get smtp info ==========" << endl;
    break;
  }
  case CMD_SET_SMTP_SENDER: {
    cout << "\t========== set smtp sender ==========" << endl;
    char machine[32], sender[32];
    strncpy(machine, req->data, 32);
    strncpy(sender, req->data + 32, 32);
    set_smtp_sender_machine(machine, sender);
    cout << "\t========== end set smtp sender ==========" << endl;
    break;
  }
  case CMD_SET_SMTP_PRIMARY: {
    cout << "\t========== set smtp primary ==========" << endl;
    char server[20], id[20], pwd[24];
    strncpy(server, req->data, 20);
    strncpy(id, req->data + 20, 20);
    strncpy(pwd, req->data + 40, 24);
    set_smtp_primary_receiver(server, id, pwd);
    cout << "\t========== end set smtp primary ==========" << endl;
    break;
  }
  case CMD_SET_SMTP_SECONDARY: {
    cout << "\t========== set smtp secondary ==========" << endl;
    char server[20], id[20], pwd[24];
    strncpy(server, req->data, 20);
    strncpy(id, req->data + 20, 20);
    strncpy(pwd, req->data + 40, 24);
    set_smtp_secondary_receiver(server, id, pwd);
    write_smtp_config_to_file();
    cout << "\t========== end set smtp secondary ==========" << endl;
    break;
  }
  case CMD_GET_SSL: {
    cout << "\t========== get ssl info ==========" << endl;
    *res_len = parse_ssl_conf_file(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end ssl info ==========" << endl;
    break;
  }
  case CMD_SET_SSL_1: {
    cout << "\t========== set ssl info1 ==========" << endl;
    char keylen[6], country[3], state[16], city[16], organ[16], valid[4];
    strncpy(keylen, req->data, 6);
    strncpy(country, req->data + 6, 3);
    strncpy(state, req->data + 6 + 3, 16);
    strncpy(city, req->data + 6 + 3 + 16, 16);
    strncpy(organ, req->data + 6 + 3 + 16 + 16, 16);
    strncpy(valid, req->data + 6 + 3 + 16 + 16 + 16, 4);
    set_ssl_1(keylen, country, state, city, organ, atoi(valid));
    break;
    cout << "\t========== end set ssl info1 ==========" << endl;
  }
  case CMD_SET_SSL_2: {
    cout << "\t========== set ssl info2 ==========" << endl;
    char organ_unit[16], cn[16], email[32];
    strncpy(organ_unit, req->data, 16);
    strncpy(cn, req->data + 16, 16);
    strncpy(email, req->data + 16 + 16, 32);
    set_ssl_2(organ_unit, cn, email);
    break;
    cout << "\t========== end set ssl info2 ==========" << endl;
  }
  case CMD_GET_ACTIVE_DIRECTORY: {
    cout << "\t========== get active directory info ==========" << endl;
    *res_len = parse_ad_conf_file(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get active directory info ==========" << endl;
    break;
  }

  case CMD_SET_ACTIVE_DIRECTORY_ENABLE: {
    cout << "\t========== set active directory enable ==========" << endl;
    set_ad_enable((int)req->data[0]);
    cout << "\t========== end set active directory enable ==========" << endl;
    break;
  }
  case CMD_SET_ACTIVE_DIRECTORY_IP_PWD: {
    cout << "\t========== set active directory ip, pwd ==========" << endl;
    char ip[16], pwd[64];
    memcpy(ip, req->data, 16);
    memcpy(pwd, req->data + 16, 64);
    set_ad_conf_file_ip_pwd(ip, pwd);
    cout << "\t========== end set active directory ip, pwd ==========" << endl;
    break;
  }
  case CMD_SET_ACTIVE_DIRECTORY_DOMAIN: {
    cout << "\t========== set active directory domain ==========" << endl;
    set_ad_conf_file_domain(req->data);
    cout << "\t========== end set active directory domain ==========" << endl;
    break;
  }
  case CMD_SET_ACTIVE_DIRECTORY_USERNAME: {
    cout << "\t========== set active directory username ==========" << endl;
    set_ad_conf_file_username(req->data);
    write_ad_to_file();
    cout << "\t========== end set active directory username ==========" << endl;
    break;
  }
  case CMD_GET_LDAP: {
    cout << "\t========== get ldap info ==========" << endl;
    *res_len = parse_ldap_conf_file(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get ldap info ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_ENABLE: {
    cout << "\t========== set ldap enable ==========" << endl;
    set_ldap_enable((int)req->data[0]);
    cout << "\t========== end set ldap enable ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_IP: {
    cout << "\t========== set ldap ip ==========" << endl;
    set_ldap_ip(req->data);
    cout << "\t========== end set ldap ip ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_PORT: {
    cout << "\t========== set ldap port ==========" << endl;
    set_ldap_port(req->data);
    cout << "\t========== end set ldap port ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_SEARCHBASE: {
    cout << "\t========== set ldap searchbase ==========" << endl;
    set_ldap_searchbase(req->data);
    cout << "\t========== end set ldap searchbase ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_BINDDN: {
    cout << "\t========== set ldap binddn ==========" << endl;
    set_ldap_binddn(req->data);
    cout << "\t========== end set ldap binddn ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_PASSWORD: {
    cout << "\t========== set ldap password ==========" << endl;
    set_ldap_bindpw(req->data);
    cout << "\t========== end set ldap password ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_SSL: {
    cout << "\t========== set ldap ssl ==========" << endl;
    set_ldap_ssl((int)req->data[0]);
    cout << "\t========== end set ldap ssl ==========" << endl;
    break;
  }
  case CMD_SET_LDAP_TIMELIMIT: {
    cout << "\t========== set ldap timelimit ==========" << endl;
    set_ldap_timelimit(req->data[0]);
    write_ldap_to_nslcd();
    cout << "\t========== end set ldap timelimit ==========" << endl;
    break;
  }
  case CMD_GET_RADIUS: {
    cout << "\t========== get radius info ==========" << endl;
    *res_len = parse_radius_conf_file(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get radius info ==========" << endl;
    break;
  }
  case CMD_SET_RADIUS: {
    cout << "\t========== set radius info ==========" << endl;
    char ip[16], port[6], secret[16];
    memcpy(ip, req->data, 16);
    memcpy(port, req->data + 16, 6);
    memcpy(secret, req->data + 6 + 16, 16);
    set_radius_config(ip, port, secret);
    cout << "\t========== end set radius info ==========" << endl;
    break;
  }
  case CMD_SET_RADIUS_DISABLE: {
    cout << "\t========== set radius disable ==========" << endl;
    set_radius_disable();
    cout << "\t========== end set radius disable ==========" << endl;
    break;
  }
  case CMD_GET_USER_LIST: {
    cout << "\t========== get user list ==========" << endl;
    *res_len = rest_make_user_json(response);
    cout << "\t response=" << (char *)response << endl;
    cout << "\t res_len=" << (*res_len) << endl;
    cout << "\t========== end get user list ==========" << endl;
    break;
  }
  case CMD_SET_USER_NAME: {
    cout << "\t========== set user name ==========" << endl;
    int index = req->data[0] - 1;
    char name[14] = {
        0,
    };
    memcpy(name, req->data + 1, 14);
    app_set_user_name_params(index, name);
    plat_user_save();
    cout << "\t========== end set user name ==========" << endl;
    break;
  }
  case CMD_SET_USER_PASSWORD: {
    cout << "\t========== set user password ==========" << endl;
    app_set_user_passwd_params((ipmi_req_t *)req, response);
    plat_user_save();
    cout << "\t response=" << (char *)response << endl;
    cout << "\t========== end set user password ==========" << endl;
    break;
  }
  // case CMD_SET_USER_ENABLE:{
  // 	cout << "\t========== set user enable ==========" << endl;
  // 	int index = atoi(req->data[0]) - 1;
  // 	char enable = req->data[1];
  // 	app_set_user_enable(index, enable);
  // 	cout << "\t========== end set user enable ==========" << endl;
  // 	break;
  // }
  case CMD_SET_USER_ACCESS: {
    cout << "\t========== set user access ==========" << endl;
    app_set_user_access_params((ipmi_req_t *)req);
    plat_user_save();
    cout << "\t========== end set user access ==========" << endl;
    break;
  }
  case CMD_DEL_USER: {
    cout << "\t========== delete user ==========" << endl;
    int index = req->data[0] - 1;
    app_del_user(index);
    plat_user_save();
    cout << "\t========== end delete user ==========" << endl;
    break;
  }
  case CMD_SET_POWER_CTL: {
    cout << "\t========== CMD_SET_POWER_CTL ==========" << endl;
    int param = req->data[0] - '0';
    cout << "\t\tparam =" << param << endl;
    switch (param) {
    case 1:
      // on
      req->data[0] = 1;
      ipmiChassis.chassis_control((ipmi_req_t *)req, (ipmi_res_t *)response,
                                  (uint8_t *)res_len);
      response[0] = '1';
      break;
    case 2:
      // off
      req->data[0] = 0;
      ipmiChassis.chassis_control((ipmi_req_t *)req, (ipmi_res_t *)response,
                                  (uint8_t *)res_len);
      response[0] = '0';
      break;
    case 3:
      // reset
      req->data[0] = 3;
      ipmiChassis.chassis_control((ipmi_req_t *)req, (ipmi_res_t *)response,
                                  (uint8_t *)res_len);
      response[0] = '3';
      break;
    default:
      cout << ("Unknown cmd.") << endl;
      break;
    }
    *res_len = 1;
    break;
  }
  case CMD_GET_POWER_USAGE: {

    cout << "\t========== CMD_GET_POWER_USAGE ==========" << endl;
    log(info) << "req->data[0](menu)=" << (int)req->data[0];
    req->data[0] = 0;
    log(info) << "menu change value 0" << (int)req->data[0];
    try {
      cout << "\tget_min_power_usage" << endl;
      get_min_power_usage(req->data[0]);
    } catch (const std::exception &) {

      cout << "error CMD_GET_POWER_USAGE: get_min_power_usage" << endl;
      break;
    }
    // cout << "\tget_power_response" << endl;
    get_power_response(req->data[0], response);
    // cout<<"response = "<<(char *)(response)<<endl;
    *res_len = strlen((char *)response);
    // cout<<"res_len = "<<*res_len<<endl;
    break;
  }
  default:
    break;
  }

  cout << "Leave apps.cpp:ipmi_handle_rest \n" << endl;
  return;
}

static void ipmi_handle_chassis(ipmi_req_t *request, ipmi_res_t *response,
                                uint8_t *res_len) {
  uint8_t cmd = request->cmd;
  switch (cmd) {
  case CMD_CHASSIS_POH:

    ipmiChassis.chassis_get_poh(request, response, res_len);
    break;
  case CMD_CHASSIS_GET_STATUS:
    ipmiChassis.chassis_get_status(request, response, res_len);
    break;
  case CMD_CHASSIS_CONTROL:
    ipmiChassis.chassis_control(request, response, res_len);
    redfish_m_cond[MU_CHASSIS].notify_one();
    break;
  case CMD_CHASSIS_IDENTIFY:
    ipmiChassis.chassis_identify(request, response, res_len);
    break;
  case CMD_CHASSIS_POWER_POLICY:
    ipmiChassis.chassis_set_policy(request, response, res_len);
    break;
  case CMD_CHASSIS_RESTART_CAUSE:
    ipmiChassis.chassis_restart_cause(request, response, res_len);
    break;
  case CMD_CHASSIS_SET_BOOT_PARAM:
    ipmiChassis.chassis_set_boot_options(request, response, res_len);
    break;
  case CMD_CHASSIS_GET_BOOT_OPTIONS:
    ipmiChassis.chassis_get_boot_options(request, response, res_len);
    break;
  case 99:
    cout << "error" << endl;
    break;
  default:
    response->cc = 0xfe;
    break;
  }
}

uint8_t ipmi_handle(uint8_t bRMCP, uint8_t *request, uint8_t req_len,
                    uint8_t *response, uint8_t *res_len) {
  cout << "=========ipmi_handle==========" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t netfn;
  std::vector<uint8_t> data(req->data, req->data + req_len);

  if (bRMCP)
    netfn = req->netfn_lun;
  else
    netfn = req->netfn_lun >> 2;

  res->cmd = req->cmd;
  res->cc = 0xFF;
  *res_len = 0;
  cout << "ipmi_handle:netfn = " << std::hex << (int)netfn << endl;
  cout << "ipmi_handle:req cmd = " << std::hex << (int)req->cmd << endl;
  cout << "ipmi_handle:res cmd = " << std::hex << (int)res->cmd << endl;
  // cout<<"ipmi_handle:req_t data = ";;
  // for(int i=0; i<req->netfn_lun;i++)
  // 	cout<<(char)req->data[i];
  switch (netfn) {
  case NETFN_CHASSIS_REQ:
    res->netfn_lun = NETFN_CHASSIS_RES << 2;
    ipmi_handle_chassis(req, res, res_len);
    break;
  case NETFN_STORAGE_REQ:
    res->netfn_lun = NETFN_STORAGE_RES << 2;
    ipmi_handle_storage(req, res, res_len);
    break;
  case NETFN_SENSOR_REQ:
    res->netfn_lun = NETFN_SENSOR_RES << 2;
    ipmi_handle_sensor(req, res, res_len);
    break;
  case NETFN_TRANSPORT_REQ:
    res->netfn_lun = NETFN_TRANSPORT_RES << 2;
    ipmi_handle_transport(req, res, res_len);
    break;
  case NETFN_APP_REQ:
    res->netfn_lun = NETFN_APP_RES << 2;
    ipmiApplication.ipmi_handle_app(request, response, res_len);
    break;
  case NETFN_DCMI_REQ:
    res->netfn_lun = NETFN_DCMI_RES << 2;
    ipmi_handle_dcmi(request, response, res_len);
    break;
  // case 0x05: //KCS 
  //   // res->netfn_lun = 0x0b <<2;
  //   IPMI_Event_handle::ipmi_event_handle(req, res, res_len);
  //   break;
  }
  *res_len += SIZE_IPMI_RES_HDR;

  return response[2];
}
int app_getUserPriv(std::string _username, uint8_t index) {
  cout << "app_getUserPriv" << endl;
  if (index < 10) {
    if (ipmiUser[index].getUsername() == _username)
      return ipmiUser[index].getUserPriv();
  } else
    return 0;
}

std::string app_getUserpassword(uint8_t index) {
  cout << "app_getUserpassword" << endl;
  return ipmiUser[index].getUserpassword();
}

int app_getUserindex(std::string _username) {
  cout << "app_getUserindex" << endl;
  for (int i = 0; i < 10; i++) {
    if (ipmiUser[i].getUsername() == _username) {
      if (ipmiUser[i].getUserenable() == 1) {
        return i;
      } else
        return 0;
    }
  }
  return 0;
}
/**
 *
 * @brief main of project
 * @details ipmi sensor들 초기화 NVA PDPB PEB 센서
 * @bug PDPB는 아직 핀맵이 정의되지 않아 제거됨
 */
void plat_ipmi_init(void) {
  log(info) << "plat_ipmi_init start";
  char g_host_name[20];
  char g_kernel_version[20];

  FILE *pp = fopen("/proc/sys/kernel/osrelease", "r");
  fgets(g_kernel_version, sizeof(char) * 20, pp);
  fclose(pp);
  uint8_t chan = 0;
  unsigned char buff[100] = {
      0,
  };
  FILE *fp = fopen("/etc/hostname", "r");
  fgets(buff, 100, fp);
  fclose(fp);
  strcpy(g_host_name, buff);
  g_host_name[strlen(g_host_name) - 1] = '\0';

  g_kernel_version[strlen(g_kernel_version) - 1] = '\0';
  ipmiApplication.g_host_name = (string)g_host_name;
  ipmiApplication.g_kernel_version = (string)g_kernel_version;

  get_build_time(g_firmware_build_time);
  for (int i = 0; i < 10; i++)
    ipmiUser[i] = Ipmiuser();

  log(info) << "current user num : " << user_loading();
  KETI_define::global_enabler = 8;
  // sel dcmi init !
  plat_sel_init();
  plat_dcmi_init();
  // alert_init
  plat_lan_alert_init();

  // global enable loading

  t_lanMonitor[0] = std::thread([&]() { ipmiNetwork[0].lanMonitorHandler(); });
  t_lanMonitor[1] = std::thread([&]() { ipmiNetwork[1].lanMonitorHandler(); });
  t_lanMonitor[2] = std::thread([&]() { ipmiNetwork[2].lanMonitorHandler(); });
  t_lanMonitor[3] = std::thread([&]() { ipmiNetwork[3].lanMonitorHandler(); });

  uint8_t rec_id = 0;
  std::map<uint8_t, Ipmisdr> inner;

  for (int i = 0; i < PEB_SENSOR_COUNT; i++) {
    inner.insert(std::make_pair(
        rec_id, Ipmisdr(rec_id, peb_sensors[i].sensor_num, peb_sensors[i])));
    sdr_rec.insert(std::make_pair(rec_id, inner));
    rec_id++;
  }
  //AST2600a3 포팅과정 LPC 초기화 이후 IPMB를통한 데이터 init필요
  for (int i = 0; i < PDPB_SENSOR_COUNT; i++) {
    inner.insert(std::make_pair(
        rec_id, Ipmisdr(rec_id, pdpb_sensors[i].sensor_num,
        pdpb_sensors[i])));
    sdr_rec.insert(std::make_pair(rec_id, inner));
    rec_id++;
  }
  for (int i = 0; i < NVA_SENSOR_COUNT; i++) {
    inner.insert(std::make_pair(
        rec_id, Ipmisdr(rec_id, nva_sensors[i].sensor_num, nva_sensors[i])));
    sdr_rec.insert(std::make_pair(rec_id, inner));

    rec_id++;
  }
}
/**
 * @brief 로그인을 인증시 패스워드, 유저이름이 있는지 확인
 * @param _username RESTful로 받은 json에서 USERNAME
 * @param _password RESTful로 받은 json에서 PASSWORD
 */
int authenticate_ipmi(std::string _username, std::string _password) {
  for (int i = 0; i < 10; i++) {
    // ipmiUser[i].printUserInfo();
    if (ipmiUser[i].getUsername() == _username) {
      if (ipmiUser[i].getUserpassword() == _password) {
        return ipmiUser->getUserPriv();

        // if (ipmiUser[i].getUserenable() == 1)
        // {
        // 	return i;
        // 	cout << "\t return true authenticate_ipmi end" << endl;
        // }
      }
    }
  }
  log(warning) << "[...]cannot find user";
  return 0;
}
/**
 * @brief LDAP 클라이언트 인증 연동 설정
 * @param _username RESTful로 받은 json에서 USERNAME
 * @param _password RESTful로 받은 json에서 PASSWORD
 * @return 2 or 0
 * @todo 어떤 작업을 수행하는지 파악중
 */
int authenticate_ldap(std::string _username, std::string _password) {
  puts("auth ldap");
  char cmd[64];
  sprintf(cmd, "echo %s | auth %s ldap", _password.c_str(), _username.c_str());
  if (system(cmd) == 0) {
    return 2;
  }
  return 0;
}
/**
 * @brief auth active dir
 * @param _username RESTful로 받은 json에서 USERNAME
 * @param _password RESTful로 받은 json에서 PASSWORD
 * @return 2 or 0
 * @todo 어떤 작업을 수행하는지 파악중
 */
int authenticate_ad(std::string _username, std::string _password) {
  puts("auth active dir");
  char cmd[64];
  sprintf(cmd, "echo %s | auth %s ldap", _password.c_str(), _username.c_str());
  if (system(cmd) == 0) {
    return 2;
  }
  return 0;
}
/**
 * @brief auth rad
 * @param _username RESTful로 받은 json에서 USERNAME
 * @param _password RESTful로 받은 json에서 PASSWORD
 * @return 2 or 0
 * @todo 어떤 작업을 수행하는지 파악중
 */
int authenticate_radius(std::string _username, std::string _password) {
  puts("auth rad");
  char cmd[64];
  sprintf(cmd, "echo %s | auth %s radius", _password.c_str(),
          _username.c_str());
  if (system(cmd) == 0) {
    return 2;
  }
  return 0;
}
void get_temp_cpu0(json::value &JCPU) {
  unsigned char status = 1;
  status = ipmiChassis.get_power_status();
  if (status == 1) {
    unsigned char data = 0;
    data = sdr_sensor_read(PDPB_SENSOR_TEMP_CPU0);
    JCPU["VALUE"] = json::value::string(to_string(data));
  }

  return;
}

void get_temp_cpu1(json::value &JCPU) {
  unsigned char status = 1;
  status = ipmiChassis.get_power_status();
  if (status == 1) {
    unsigned char data = 0;
    data = sdr_sensor_read(PDPB_SENSOR_TEMP_CPU0);
    JCPU["VALUE"] = json::value::string(to_string(data));
  }

  return;
}
/**
 * @brief get_voltage_fan_power
 * @bug	 psu 포팅을 하지못하여 주석처리됨 수정해야함 2020-04-08
 * @param JPOWER
 * @return * void
 */
void get_voltage_fan_power(json::value &JPOWER) {
  int pow1_vol = 300;
  int pow2_vol = 100;
  int pow1_fan = 7000;
  int pow2_fan = 12000;

  // pow1_vol = sdr_sensor_read(NVA_SENSOR_PSU1_WATT) * 10;
  // pow2_vol = sdr_sensor_read(NVA_SENSOR_PSU2_WATT) * 10;
  // pow1_fan = sdr_sensor_read(NVA_SENSOR_PSU1_FAN1) * 100;
  // pow2_fan = sdr_sensor_read(NVA_SENSOR_PSU2_FAN1) * 100;

  JPOWER["POWER1_VOLTAGE"] = json::value::string(to_string(pow1_vol));
  JPOWER["POWER2_VOLTAGE"] = json::value::string(to_string(pow2_vol));
  JPOWER["POWER1_FAN"] = json::value::string(to_string(pow1_fan));
  JPOWER["POWER2_FAN"] = json::value::string(to_string(pow2_fan));

  return;
}

void get_fans(json::value &JFANS) {
  int fan[5] = {
      0,
  };

  fan[0] = sdr_sensor_read(NVA_SYSTEM_FAN1) * 100;
  fan[1] = sdr_sensor_read(NVA_SYSTEM_FAN2) * 100;
  fan[2] = sdr_sensor_read(NVA_SYSTEM_FAN3) * 100;
  fan[3] = sdr_sensor_read(NVA_SYSTEM_FAN4) * 100;
  fan[4] = sdr_sensor_read(NVA_SYSTEM_FAN5) * 100;
  for (int i = 1; i <= 5; i++) {
    char fanstr[5] = {
        0,
    };
    sprintf(fanstr, "FAN%d", i);
    JFANS[fanstr] = json::value::string(to_string(fan[i - 1]));
  }

  return;
}
/**
 * @brief 보드 온도 측정
 * @details PDPB_SENSOR_TEMP_LEFT_REAR에서 PDPB_SENSOR_TEMP_REAR_RIGHT로 변경
 * @author 기철
 **/
void get_temp_board(json::value &JBOARD_TEMP) {
  float board_val = 33.5;
  // board_val = sdr_sensor_read(PDPB_SENSOR_TEMP_REAR_RIGHT) * 0.5;
  JBOARD_TEMP["VALUE"] = json::value::string(to_string(board_val));
  return;
}
/**
 * @brief REST  system정보 LAN정보
 * @bug  net_priority= 8로 고정되어있는 문제 특별한 문제 없으면 사용가능 kernel
 * name 에러
 *
 */
void get_sys_info(json::value &JSYS_INFO) {

  int chan = 0;
  unsigned char net_priority = 1;
  // for (int i = 0 ; i <NETWORK_COUNT;i)
  switch (net_priority) {
  case 1:
    chan = 0;
    break;
  case 6:
    chan = 1;
    break;
  case 7:
    chan = 2;
    break;
  case 8:
    chan = 3;
    break;
  default:
    chan = 0;
    break;
  }
  json::value subobj = json::value::object();
  Ipminetwork *selectchan = &ipmiNetwork[chan];
  if (selectchan->ip_src == 1)
    subobj["IPV4_NETWORK_MODE"] = json::value::string(U("Static"));
  else if (selectchan->ip_src == 2)
    subobj["IPV4_NETWORK_MODE"] = json::value::string(U("DHCP"));
  else
    subobj["IPV4_NETWORK_MODE"] = json::value::string(U("Unspecified"));

  string temp;
  temp.resize(30);
  std::vector<uint8_t> iptemp = selectchan->mac_addr;
  sprintf((char *)temp.c_str(), "%02x.%02x.%02x.%02x.%02x.%02x\0", iptemp.at(0),
          iptemp.at(1), iptemp.at(3), iptemp.at(4), iptemp.at(5));
  strcat(temp.c_str(), "\0");
  temp.shrink_to_fit();
  cout << "mac addr = " << temp << endl;

  subobj["MAC_ADDRESS"] = json::value::string(U(temp));
  temp = "";
  temp.resize(15);
  iptemp = selectchan->ip_addr;
  sprintf((char *)temp.c_str(), "%d.%d.%d.%d\0", iptemp.at(0), iptemp.at(1),
          iptemp.at(2), iptemp.at(3));
  strcat(temp.c_str(), "\0");
  temp.shrink_to_fit();
  cout << "ipv4 addr = " << temp << endl;

  subobj["IPV4_ADDRESS"] = json::value::string(U(temp));

  temp = "";
  temp.resize(60);
  iptemp = selectchan->ip_addr_v6;
  subobj["IPV6_NETWORK_MODE"] = json::value::string(U("Static"));
  for (int i = 0; i < iptemp.size(); i++) {
    temp += (iptemp.at(i));
  }
  strcat(temp.c_str(), "\0");
  temp.shrink_to_fit();
  cout << "ipv6 addr = " << temp << endl;
  subobj["IPV6_ADDRESS"] = json::value::string(U(temp));

  JSYS_INFO["LAN"] = subobj;

  subobj = json::value::object();
  subobj["IPMIFW_VERSION"] = json::value::string(U(IPMI_VERSION));
  subobj["IPMIFW_BLDTIME"] =
      json::value::string(U(std::string(g_firmware_build_time)));
  subobj["KERNAL_VERSION"] =
      json::value::string(U(std::string(ipmiApplication.g_kernel_version)));
  JSYS_INFO["MC"] = subobj;

  return;
}
#define MAX_CPU 2
#define MAX_PSU 2
#define MAX_CPU0_MEM 12
#define MAX_CPU1_MEM 12
#define MAX_FAN 9
/**
 * @brief Get the health summary object
 * @bug 2020-04-08 psu 포팅문제로 인해 주석처리
 * @param jhealth_summary
 */
void get_health_summary(json::value &jhealth_summary) {
  std::vector<json::value> JVCPU, JCPU0_MEMORY, JCPU1_MEMORY, JPOWER, JFAN;
  char buf[64];
  int snr_id, idx;
  int index = 0;
  sensor_thresh_t *p_sdr, *t_sdr;
  int sensor_index = -1, namelen = -1;
  sensor_thresh_t temp;
  string sensorname;
  //
  for (snr_id = PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0;
       snr_id <= PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2; snr_id++) {
    sensor_index = plat_find_sdr_index(snr_id);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (temp.nominal > 0)
      JCPU0_MEMORY.push_back(json::value::number(1));
    else
      JCPU0_MEMORY.push_back(json::value::number(0));
  }

  for (snr_id = PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0;
       snr_id <= PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2; snr_id++) {
    sensor_index = plat_find_sdr_index(snr_id);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (temp.nominal > 0)
      JCPU1_MEMORY.push_back(json::value::number(1));
    else
      JCPU1_MEMORY.push_back(json::value::number(0));
  }

  for (snr_id = PDPB_SENSOR_TEMP_CPU0; snr_id <= PDPB_SENSOR_TEMP_CPU1;
       snr_id++) {
    sensor_index = plat_find_sdr_index(snr_id);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (temp.nominal > 0)
      JVCPU.push_back(json::value::number(1));
    else
      JVCPU.push_back(json::value::number(0));
  }

  for (snr_id = NVA_SYSTEM_FAN1; snr_id <= NVA_SYSTEM_FAN5; snr_id++) {
    sensor_index = plat_find_sdr_index(snr_id);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (temp.nominal > 0)
      JFAN.push_back(json::value::number(1));
    else
      JFAN.push_back(json::value::number(0));
  }
  // for (snr_id = NVA_SENSOR_PSU1_WATT; snr_id <= NVA_SENSOR_PSU2_WATT;
  // snr_id++)
  // {
  // 	sensor_index = plat_find_sdr_index(snr_id);
  // 	temp =
  // sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
  // if (temp.nominal > 0) JPOWER.push_back(json::value::number(1)); 	else
  // 		JPOWER.push_back(json::value::number(0));
  // }

  // BMC_WEB 홈화면 Hardware Status 용 json 상태값 임시전달
  jhealth_summary["DIMM1_CH_B"] = json::value::number(0);
  jhealth_summary["DIMM1_CH_A"] = json::value::number(0);
  jhealth_summary["DIMM2_CH_B"] = json::value::number(0);
  jhealth_summary["DIMM2_CH_A"] = json::value::number(0);

  vector<json::value> TEMP_ARRAY;
  TEMP_ARRAY.push_back(json::value::number(1));
  TEMP_ARRAY.push_back(json::value::number(0));
  TEMP_ARRAY.push_back(json::value::number(1));
  TEMP_ARRAY.push_back(json::value::number(0));
  TEMP_ARRAY.push_back(json::value::number(1));
  
    
  jhealth_summary["Temperature"] = json::value::array(TEMP_ARRAY);

  // jhealth_summary["HEALTH_SUMMARY"] = json::value::array(JVCPU);
  // jhealth_summary["CPU0_MEMORY"] = json::value::array(JCPU0_MEMORY);
  // jhealth_summary["CPU1_MEMORY"] = json::value::array(JCPU1_MEMORY);
  // jhealth_summary["POWER"] = json::value::array(JPOWER);
  // // cout<<jhealth_summary["POWER"].as_string()<<endl;
  // jhealth_summary["FAN"] = json::value::array(JFAN);

  return;
}

int rest_show_main(int menu, char *ret) {

  char buf[30000] = {
      0,
  };
  cout << "Enter rest_show_main" << endl;
  cout << "\t selected menu = " << menu << endl;
  json::value obj = json::value::object();
  json::value main = json::value::object();
  json::value JHEALTH_SUMMARY = json::value::object();
  json::value JCPU0 = json::value::object();
  json::value JCPU1 = json::value::object();
  json::value JPOWER = json::value::object();
  json::value JFANS = json::value::object();
  json::value JBOARD_TEMP = json::value::object();
  json::value JSYS_INFO = json::value::object();
  json::value JEVENT_LIST = json::value::object();
  std::vector<json::value> VJ;

  switch (menu) {
  case ALL:
    rest_get_eventlog_config(buf);
    JEVENT_LIST = json::value::parse(buf);
    get_temp_cpu0(JCPU0);
    cout << "get_temp_cpu0" << endl;
    get_temp_cpu1(JCPU1);
    cout << "get_temp_cpu1" << endl;
    get_voltage_fan_power(JPOWER);
    cout << "get_voltage_fan_power" << endl;
    get_fans(JFANS);
    cout << "get_fans" << endl;
    get_temp_board(JBOARD_TEMP);
    cout << "get_temp_board" << endl;
    get_sys_info(JSYS_INFO);
    cout << "get_sys_info" << endl;
    get_health_summary(JHEALTH_SUMMARY);
    cout << "get_health_summary 포팅이슈" << endl;
    main["EVENT_LIST"] = JEVENT_LIST["EVENT_INFO"]["SEL"];
    main["CPU1_TEMP"] = JCPU0;
    main["CPU2_TEMP"] = JCPU1;
    main["POWER"] = JPOWER;
    main["FANS"] = JFANS;
    main["BOARD_TEMP"] = JBOARD_TEMP;
    main["SYS_INFO"] = JSYS_INFO;
    main["HEALTH_SUMMARY"] = JHEALTH_SUMMARY;
    obj["MAIN"] = main;
    break;
  case EVENT_LIST:
    rest_get_eventlog_config(buf);
    JEVENT_LIST = json::value::parse(buf);
    main["EVENT_LIST"] = JEVENT_LIST["EVENT_INFO"]["SEL"];
    obj["MAIN"] = main;
    break;
  case CPU2_TEMP:
    get_temp_cpu1(JCPU1);
    main["CPU2_TEMP"] = JCPU1;
    obj["MAIN"] = main;
    break;

  case CPU1_TEMP:
    get_temp_cpu1(JCPU1);
    main["CPU2_TEMP"] = JCPU1;
    get_temp_cpu0(JCPU0);
    main["CPU1_TEMP"] = JCPU0;
    get_temp_board(JBOARD_TEMP);
    main["BOARD_TEMP"] = JBOARD_TEMP;
    obj["MAIN"] = main;
    break;
  case BOARD_TEMP:
    get_temp_cpu1(JCPU1);
    main["CPU2_TEMP"] = JCPU1;
    get_temp_cpu0(JCPU0);
    main["CPU1_TEMP"] = JCPU0;
    get_temp_board(JBOARD_TEMP);
    main["BOARD_TEMP"] = JBOARD_TEMP;
    obj["MAIN"] = main;
    break;
  case POWER:
    get_voltage_fan_power(JPOWER);
    main["POWER"] = JPOWER;
    obj["MAIN"] = main;
    break;
  case FANS:
    get_fans(JFANS);
    main["FANS"] = JFANS;
    obj["MAIN"] = main;
    break;
  case SYS:
    get_sys_info(JSYS_INFO);
    main["SYS_INFO"] = JSYS_INFO;
    obj["MAIN"] = main;
    break;
  case SUMMARY:
    get_health_summary(JHEALTH_SUMMARY);
    main["HEALTH_SUMMARY"] = JHEALTH_SUMMARY;
    obj["MAIN"] = main;
    break;
  default:
    cout << "rest_show_main:: not option number" << menu << endl;
    break;
  }

  strcpy(ret, obj.serialize().c_str());

  cout << "Leave rest show_main\n\n\n\n\n\n" << endl;
  cout << "obj.size =" << obj.serialize().length() << endl;
  return obj.serialize().length();
}

/**
 * @brief sNum에 해당하는 sensor_name 찾아줌
 * @return 1 성공
 * @return 0 실패
 * @param sType 센서종류
 * @param sNum 센서 인덱스
 * @param msg string msg 레퍼런스 센서이름을 반환함
 * @todo sdr sensor만 정상작동 SENSOR_TYPE_TEMPERATURE만 수행되는중 나머지부분
 */
int find_sensor_name(uint8_t sType, uint8_t sNum, string &msg) {
  int sensor_index = -1, namelen = -1;
  sensor_thresh_t temp;
  string sensorname;
  switch (sType) {
  case SENSOR_TYPE_TEMPERATURE: // temperature
    sensor_index = plat_find_sdr_index(sNum);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (sensor_index < 0) {
      cout << "get_pef_alert_msg: can't find sensor." << endl;
      return 0;
    }
    sensorname = std::string(temp.str);
    break;
  case SENSOR_TYPE_VOLTAGE: // temperature
    sensor_index = plat_find_sdr_index(sNum);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (sensor_index < 0) {
      cout << "get_pef_alert_msg: can't find sensor." << endl;
      return 0;
    }
    sensorname = std::string(temp.str);
    break;
  case SENSOR_TYPE_CURRENT: // temperature
    sensor_index = plat_find_sdr_index(sNum);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (sensor_index < 0) {
      cout << "get_pef_alert_msg: can't find sensor." << endl;
      return 0;
    }
    sensorname = std::string(temp.str);
    break;
  case SENSOR_TYPE_FAN: // temperature
    sensor_index = plat_find_sdr_index(sNum);
    temp =
        sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    if (sensor_index < 0) {
      cout << "get_pef_alert_msg: can't find sensor." << endl;
      return 0;
    }
    sensorname = std::string(temp.str);
    break;
  default:
    // Sensor Type 9= P12PSU
    // cout << "warning: find_sensor_name sensor s_Type"
    // <<static_cast<int>(sType)<< endl; sensor_index =
    // plat_find_sdr_index(sNum); temp =
    // sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
    // if (sensor_index < 0)
    // {
    // 	cout << "sensor s_Type: can't find sensor. sensor name =" <<temp.str<<
    // endl; 	return 0;
    // }
    // cout << "sensor name = " <<temp.str<< endl;
    break;
  }
  msg = sensorname;
  return 1;
}

void get_build_time(unsigned char *build_time) {

  char buildtime[50] = {
      0,
  };
  char unamev[100] = {
      0,
  };
  char cmds[20] = {
      0,
  };
  FILE *p = NULL;

  p = fopen("/proc/sys/kernel/version", "r");
  if (p)
    while (fgets(unamev, sizeof(unamev), p) != NULL)
      ;
  fclose(p);

  char *p_1 = NULL;
  p_1 = index(unamev, ' ');
  char *p_2 = NULL;
  p_2 = index(p_1 + 1, ' ');
  char *p_3 = NULL;
  p_3 = rindex(p_2, '\n');
  strncpy(g_firmware_build_time, p_2, strlen(p_2) - strlen(p_3));
}
/**
 * @brief redfish 및 rest를 통한 유저 추가
 */
int new_ipmi_user(string user_name, string password, bool enabled , int priv, int index) {
  int count = index-1;

  if (count > 10)
    return 0;

  Ipmiuser *user = &ipmiUser[count];
  cout << "new ipmi user " << count <<endl;
  user->setUserEnable((uint8_t)enabled);
  user->setUserPasswd(password);
  user->setUserName(user_name);
  user->priv = priv;
  cout << "user add" << endl;
  plat_user_save();
  return 1;
}

/**
 * @brief get chassis power status, return json format string & size
 * @date 21.05.21
 * @author doyoung
 */
int rest_get_power_status(unsigned char *res) {
  json::value obj = json::value::object();
  json::value power = json::value::object();
  unsigned char status = 0;

  status = ipmiChassis.get_power_status();

  if (status == 1)
    power["STATUS"] = json::value::string(U("1"));
  else if (status == 0)
    power["STATUS"] = json::value::string(U("0"));
  else
    power["STATUS"] = json::value::string(U("Unknown")); // error

  obj["POWER"] = power;
  strncpy(res, obj.serialize().c_str(), obj.serialize().length());

  return obj.serialize().length();
}

int rest_get_sysinfo_config(char *res) {
  cout << "REST Get System Information" << endl;
  // printf("ipmi_system_information: %d\n", IPMI_SYSTEM_INFORMATION);
  // #if IPMI_SYSTEM_INFORMATION
  unsigned char net_priority = 0;

  net_priority = get_eth_priority();

  json::value obj = json::value::object();

  json::value power_info = json::value::object();
  power_info["STATUS"] =
      json::value::string(U(ipmiChassis.get_power_status() ? "on" : "off"));

  json::value generic_info = json::value::object();
  json::value generic = json::value::object();
  generic["IPMIFW_TAG"] = json::value::string(U(ipmiApplication.g_host_name));

  char ip_str[150], mac_str[150];
  memset(ip_str, 0, sizeof(ip_str));
  memset(mac_str, 0, sizeof(mac_str));

  if (net_priority == 1) {
    sprintf(ip_str, "%u.%u.%u.%u", ipmiNetwork[0].ip_addr[0],
            ipmiNetwork[0].ip_addr[1], ipmiNetwork[0].ip_addr[2],
            ipmiNetwork[0].ip_addr[3]);
    sprintf(mac_str, "%u.%u.%u.%u.%u.%u", ipmiNetwork[0].ip_addr[0],
            ipmiNetwork[0].ip_addr[1], ipmiNetwork[0].ip_addr[2],
            ipmiNetwork[0].ip_addr[3], ipmiNetwork[0].ip_addr[4],
            ipmiNetwork[0].ip_addr[5]);

    cout << "ip 0: " << ip_str << endl;
    cout << "mac 0: " << mac_str << endl;

    generic["BMC_IP"] = json::value::string(U(ip_str));
    generic["BMC_MAC"] = json::value::string(U(mac_str));
  } else if (net_priority == 8) {
    sprintf(ip_str, "%u.%u.%u.%u", ipmiNetwork[1].ip_addr[0],
            ipmiNetwork[1].ip_addr[1], ipmiNetwork[1].ip_addr[2],
            ipmiNetwork[1].ip_addr[3]);
    sprintf(mac_str, "%u.%u.%u.%u.%u.%u", ipmiNetwork[1].ip_addr[0],
            ipmiNetwork[1].ip_addr[1], ipmiNetwork[1].ip_addr[2],
            ipmiNetwork[1].ip_addr[3], ipmiNetwork[1].ip_addr[4],
            ipmiNetwork[1].ip_addr[5]);

    cout << "ip 1: " << ip_str << endl;
    cout << "mac 1: " << mac_str << endl;

    generic["BMC_IP"] = json::value::string(U(ip_str));
    generic["BMC_MAC"] = json::value::string(U(mac_str));
  }
  generic["FRU_VERSION"] = json::value::string(U("1.31"));
  generic["IPMIFW_VERSION"] = json::value::string(U("V2.0"));
  generic["BIOS_VERSION"] = json::value::string(U("0.01"));
  generic["SDR_VERSION"] = json::value::string(U("0.51"));
  generic["WEB_VERSION"] = json::value::string(U("1.01"));
  generic["IPMIFW_BLDTIME"] = json::value::string(U(g_firmware_build_time));

  json::value kernel = json::value::object();
  kernel["VERSION"] = json::value::string(U(ipmiApplication.g_kernel_version));

  generic_info["GENERIC"] = generic;
  generic_info["KERNEL"] = kernel;
  obj["GENERIC_INFO"] = generic_info;

  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  // #else
  // 	strcpy(res, "{\"POWER_INFO\": {\n\t\"STATUS\": \"Not
  // supported\"\n\t},\n\t\"GENERIC_INFO\":{\n\t\t\"GENERIC\":
  // {\n\t\t\t\"IPMIFW_TAG\":\"Not Supported\",\n\t\t\t\"BMC_IP\":\"Not
  // Supported\",\n\t\t\t\"BMC_MAC\":\"Not
  // Supported\",\n\t\t\t\"FRU_VERSION\":\"Not
  // Supported\",\n\t\t\t\"IPMIFW_VERSION\":\"Not
  // Supported\",\n\t\t\t\"BIOS_VERSION\":\"Not
  // Supported\",\n\t\t\t\"SDR_VERSION\":\"Not
  // Supported\",\n\t\t\t\"WEB_VERSION\":\"Not
  // Supported\",\n\t\t\t\"IPMIFW_BLDTIME\":\"Not
  // Supported\"\n\t\t},\n\t\"KERNEL\": {\n\t\t\"VERSION\":\"Not
  // Supported\"\n\t\t}\n\t}\n}"); #endif
  return strlen(res);
}

int rest_get_ddns_info(unsigned char *res) {
  char buf[128];
  unsigned char domain_name[50] = {0};
  unsigned char host_name[50] = {0};
  unsigned char nameserver_pre[30] = {0};
  unsigned char nameserver_alt[30] = {0};

  get_ddns_host_name(host_name);
  get_ddns_domain_name(domain_name);
  get_ddns_nameserver(1, nameserver_pre);
  get_ddns_nameserver(2, nameserver_alt);

  host_name[strlen(host_name) - 1] = '\0';

  json::value obj = json::value::object();
  json::value DNS_INFO = json::value::object();
  json::value GENERIC = json::value::object();
  GENERIC["REGISTER_BMC_METHOD"] = json::value::string(U("DIRECT"));
  GENERIC["HOST_NAME"] = json::value::string(U((char *)host_name));
  GENERIC["DOMAIN_NAME"] = json::value::string(U((char *)domain_name));
  GENERIC["REGISTER_BMC"] = json::value::string(U("1"));

  json::value IPV6 = json::value::object();
  string s_ipaddr_v6(ipmiNetwork[0].ip_addr_v6.begin(),
                     ipmiNetwork[0].ip_addr_v6.end());
  IPV6["IPV6_PREFERRED"] = json::value::string(s_ipaddr_v6);
  IPV6["IPV6_ALTERNATE"] = json::value::string(U("localhost"));

  json::value IPV4 = json::value::object();
  IPV4["IPV4_PREFERRED"] = json::value::string(U((char *)nameserver_pre));
  IPV4["IPV4_ALTERNATE"] = json::value::string(U((char *)nameserver_alt));

  DNS_INFO["GENERIC"] = GENERIC;
  DNS_INFO["IPV6"] = IPV6;
  DNS_INFO["IPV4"] = IPV4;
  obj["DNS_INFO"] = DNS_INFO;

  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  return strlen(res);
}

void get_ddns_host_name(unsigned char *host_name) {
  unsigned char ddns_buf[100];

  FILE *fp;

  fp = popen("hostname", "r");
  if (fp == NULL) {
    perror("popen() failed");
    return;
  }

  while (fgets(ddns_buf, 100, fp))

    strncpy(host_name, ddns_buf, strlen(ddns_buf));
  pclose(fp);
}

void get_ddns_domain_name(unsigned char *domain_name) {
  FILE *ddns_fp;
  unsigned char ddns_buf[512];

  if (access(DOMAINNAME_FILE, F_OK) == 0) {
    ddns_fp = popen("hostname -f", "r");

    if (ddns_fp == NULL) {
      perror("popen() failed!\n");
    }
    while (fgets(ddns_buf, 100, ddns_fp))
      ddns_buf[strlen(ddns_buf) - 1] = '\0';
    strncpy(domain_name, ddns_buf, strlen(ddns_buf));
    pclose(ddns_fp);
  } else
    strcpy(domain_name, "-");
}

void get_ddns_nameserver(int flag, unsigned char *nameserver) {
  FILE *ddns_fp;
  unsigned char buffer[1024] = {0};
  unsigned char ddns_buf[100];
  int line_count = 0;
  int f_count = 0;

  if (access(DNSSERVER_FILE, F_OK) == 0) {
    ddns_fp = fopen(DNSSERVER_FILE, "r");
    while (feof(ddns_fp) == 0) {
      fread(buffer, sizeof(buffer), 1, ddns_fp);
    }
    if (strstr((char *)buffer, "nameserver") != NULL) {
      int i, dns_count = 0;

      while (i < strlen(buffer)) {
        if (buffer[i] == '\n')
          dns_count = dns_count + 1;
        i++;
      }

      char *n_server[dns_count];
      char *ptr = strtok(buffer, "\n");
      i = 0;

      while (ptr != NULL && i < dns_count) {
        n_server[i] = ptr;
        i++;
        ptr = strtok(NULL, "\n");
      }
      if (dns_count > 1) {
        char *ptrs = strtok(n_server[flag - 1], " ");
        ptrs = strtok(NULL, " ");
        strcpy(nameserver, ptrs);
      }

      else if (dns_count == 1) {
        if (flag == 1) {
          char *ptrs = strtok(n_server[flag - 1], " ");
          ptrs = strtok(NULL, " ");
          strcpy(nameserver, ptrs);
        } else {
          strcpy(nameserver, "-");
        }
      } else {
        strcpy(nameserver, "-");
      }

      fclose(ddns_fp);
    } else {
      strcpy(nameserver, "-");
    }
  }
}

int set_ddns_host_name(unsigned char *host_name) {
  unsigned char ddns_cmd[100] = {0};
  int rets = 0;
  set_ddns_domain_name(1, host_name);
  sprintf(ddns_cmd, "hostname %s\n", host_name);
  rets = system(ddns_cmd);

  if (rets == 0)
    return 0;
  else
    return 0;
}

int set_ddns_domain_name(int flag, unsigned char *domain_name) {
  unsigned char buffer[4096], t_buffer[100], t_cmd[100] = {0};
  FILE *ddns_fp;
  if (access(INTERFACE_FILE, F_OK) == 0) {
    ddns_fp = fopen(DOMAINNAME_FILE, "r");
    if (ddns_fp == NULL) {
      perror("fopen error!\n");
      return 0;
    }

    while (feof(ddns_fp) == 0) {
      fread(buffer, sizeof(buffer), 1, ddns_fp);
    }
    fclose(ddns_fp);

    int dns_count = 0;
    int i = 0;
    while (i < strlen(buffer)) {
      if (buffer[i] == '\n')
        dns_count += 1;
      i++;
    }
    if (dns_count != 0) {
      char hosts_line[dns_count][200];
      char *ptr = strtok(buffer, "\n");
      i = 0;

      while (ptr != NULL && i < dns_count) {
        strcpy(hosts_line[i], ptr);
        i++;
        ptr = strtok(NULL, "\n");
      }
      unsigned char host_name[50], dns_addr[50] = {0};

      if (flag == 1) {
        get_ddns_nameserver(1, dns_addr);
        FILE *dn_fp = popen("hostname -f", "r");
        if (dn_fp == NULL)
          perror("popen failed\n");

        while (fgets(host_name, 50, dn_fp))
          host_name[strlen(host_name) - 1] = '\0';
        pclose(dn_fp);
        sprintf(t_buffer, "%s\t%s\t%s\n", dns_addr, host_name, domain_name);
      }
      if (flag == 2) {
        FILE *p_fp = popen("hostname", "r");
        if (p_fp == NULL) {
          perror("popen failed");
          return 0;
        }
        while (fgets(host_name, 50, p_fp))
          host_name[strlen(host_name) - 1] = '\0';

        get_ddns_nameserver(1, dns_addr);
        sprintf(t_buffer, "%s\t%s\t%s\n", dns_addr, domain_name, host_name);
        pclose(p_fp);
      }
      memset(hosts_line[1], 0, sizeof(hosts_line[1]));
      strcpy(hosts_line[1], t_buffer);
      ddns_fp = fopen(DOMAINNAME_FILE, "w");
      for (i = 0; i < dns_count - 1; i++) {
        strcat(hosts_line[i], "\n");
        fwrite(hosts_line[i], strlen(hosts_line[i]), 1, ddns_fp);
      }
      fclose(ddns_fp);
      return 0;
    } else
      return 0;
  }
}

int set_ddns_nameserver(int flag, unsigned char *nameserver) {

  unsigned char dns_count = 0;
  unsigned char buffer[1024];
  FILE *ddns_fp;

  if (access(DNSSERVER_FILE, F_OK) == 0) {

    ddns_fp = fopen(DNSSERVER_FILE, "r");

    if (ddns_fp == NULL) {
      perror("popen() failed!\n");
      return 0;
    }

    while (feof(ddns_fp) == 0) {
      fread(buffer, sizeof(buffer), 1, ddns_fp);
    }
    fclose(ddns_fp);
    if (strstr((char *)buffer, "nameserver") != NULL) {
      int i = 0;
      while (i < strlen(buffer)) {
        if (buffer[i] == '\n')
          dns_count = dns_count + 1;
        i++;
      }

      if (dns_count != 0 && (strcmp(nameserver, "-") != 0)) {
        ddns_fp = fopen(DNSSERVER_FILE, "w");

        char *n_server[dns_count];
        char *ptr = strtok(buffer, "\n");
        i = 0;

        while ((ptr != NULL) && (i < dns_count)) {
          n_server[i] = ptr;
          i++;
          ptr = strtok(NULL, "\n");
        }
        switch (dns_count) {
        case 1:
          if (flag == 1)
            fprintf(ddns_fp, "nameserver %s\n", nameserver);
          fclose(ddns_fp);
          return 0;
          break;
        case 2:
          if (flag == 1) {
            fprintf(ddns_fp, "nameserver %s\n", nameserver);
            fprintf(ddns_fp, "%s\n", n_server[1]);
            fclose(ddns_fp);
            return 0;
          } else if (flag == 2) {
            fprintf(ddns_fp, "%s\n", n_server[0]);
            fprintf(ddns_fp, "nameserver %s\n", nameserver);
            fclose(ddns_fp);
            return 0;
          }
          break;
        case 3:
          if (flag == 1) {
            fprintf(ddns_fp, "nameserver %s\n", nameserver);
            fprintf(ddns_fp, "%s\n", n_server[1]);
            fprintf(ddns_fp, "%s\n", n_server[2]);
            fclose(ddns_fp);
            return 0;
          } else if (flag == 2) {
            fprintf(ddns_fp, "%s\n", n_server[0]);
            fprintf(ddns_fp, "nameserver %s\n", nameserver);
            fprintf(ddns_fp, "%s\n", n_server[2]);
            fclose(ddns_fp);
            return 0;
          }
          break;
        }
      } else
        return 0;
    }
  }
}

#define MAX_NIC 2
int rest_get_lan_config(char *res) {
  int ipv4_srcs = 0;
  int vlan_enables = 0;
  int c = 0;
  int channel = 0;
  uint8_t r_ip_addr6[SIZE_IP_ADDR_V6] = {
      0,
  };
  uint8_t r_netmask6[SIZE_NET_MASK_V6] = {
      0,
  };
  uint8_t r_gateway6[SIZE_IP_ADDR_V6] = {
      0,
  };

  for (int i = 0; i < MAX_NIC; i++) {
    if (i == 0)
      channel = 1;
    else if (i == 1)
      channel = 8;
    else
      channel = 0; // undefined

    if (get_ipv6_info(channel, (unsigned char *)r_ip_addr6,
                      (unsigned char *)r_netmask6,
                      (unsigned char *)r_gateway6) == -1) {
      fprintf(stderr, "get ipv6 info failed\n");
      return 0;
    }

    // printf("ip : %s\nnetmask : %s\ngateway : %s\n", r_ip_addr6, r_netmask6,
    // r_gateway6); printf("\t\t\t dy : get lan checkpoint 1 in channel
    // %d===========\n", channel);

    ipmiNetwork[i].ip_addr_v6.assign(r_ip_addr6, r_ip_addr6 + SIZE_IP_ADDR_V6);
    ipmiNetwork[i].net_mask_v6.assign(r_netmask6,
                                      r_netmask6 + SIZE_NET_MASK_V6);
    ipmiNetwork[i].df_gw_ip_addr_v6.assign(r_gateway6,
                                           r_gateway6 + SIZE_IP_ADDR_V6);

    // printf("\t\t\t dy : get lan checkpoint 2 ===========\n");
    // string ip6(ipmiNetwork[i].ip_addr_v6.begin(),
    // ipmiNetwork[i].ip_addr_v6.end()); string
    // nm6(ipmiNetwork[i].net_mask_v6.begin(),
    // ipmiNetwork[i].net_mask_v6.end()); string
    // gw6(ipmiNetwork[i].df_gw_ip_addr_v6.begin(),
    // ipmiNetwork[i].df_gw_ip_addr_v6.end()); printf("ip_v6 : %s\nnet_mask_v6 :
    // %s\ngw_ip : %s\n", ip6.c_str(), nm6.c_str(), gw6.c_str());
  }

  switch (ipmiNetwork[0].ip_src) {
  case 1:
    ipv4_srcs = 0;
    break;
  case 2:
    ipv4_srcs = 1;
    break;
  }

  char prior[8];
  if (get_eth_priority() == 1) {
    strcpy(prior, "eth0");
  }

  else if (get_eth_priority() == 8) {
    strcpy(prior, "eth1");
  } else {
    strcpy(prior, "Unknown");
  }

  json::value obj = json::value::object();
  obj["NETWORK_PRIORITY"] = json::value::string(U(prior));
  vector<json::value> net_info_vec;

  char device[16];
  for (int i = 0; i < MAX_NIC; i++) {
    // i == 0 ==> DEV_NAME_SHARED, i == 1 ==> DEV_NAME_DEDI
    json::value NETWORK_INFO = json::value::object();

    sprintf(device, "eth%d", i);
    if ((ipmiNetwork[i].vlan_enable && 0x80) == 1)
      vlan_enables = 1;
    else
      vlan_enables = 0;

    switch (ipmiNetwork[i].ip_src) {
    case 1:
      ipv4_srcs = 0;
      break;
    case 2:
      ipv4_srcs = 1;
      break;
    }

    NETWORK_INFO["LAN_INTERFACE"] = json::value::string(U(device));
    json::value GENERIC = json::value::object();
    json::value IPV4 = json::value::object();
    json::value IPV6 = json::value::object();
    json::value VLAN = json::value::object();
    char buf[64] = {
        0,
    };

    GENERIC["LAN_SETTING_ENABLE"] =
        json::value::string(U(to_string(ipmiNetwork[i].set_enable)));
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", ipmiNetwork[i].mac_addr[0],
            ipmiNetwork[i].mac_addr[1], ipmiNetwork[i].mac_addr[2],
            ipmiNetwork[i].mac_addr[3], ipmiNetwork[i].mac_addr[4],
            ipmiNetwork[i].mac_addr[5]);
    GENERIC["MAC_ADDRESS"] = json::value::string(U(buf));

    memset(buf, 0, sizeof(buf));

    IPV4["IPV4_PREFERRED"] = json::value::string(U(""));
    sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].df_gw_ip_addr[0],
            ipmiNetwork[i].df_gw_ip_addr[1], ipmiNetwork[i].df_gw_ip_addr[2],
            ipmiNetwork[i].df_gw_ip_addr[3]);
    IPV4["IPV4_GATEWAY"] = json::value::string(U(buf));
    sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].net_mask[0],
            ipmiNetwork[i].net_mask[1], ipmiNetwork[i].net_mask[2],
            ipmiNetwork[i].net_mask[3]);
    IPV4["IPV4_NETMASK"] = json::value::string(U(buf));
    sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].ip_addr[0],
            ipmiNetwork[i].ip_addr[1], ipmiNetwork[i].ip_addr[2],
            ipmiNetwork[i].ip_addr[3]);
    IPV4["IPV4_ADDRESS"] = json::value::string(U(buf));
    IPV4["IPV4_DHCP_ENABLE"] = json::value::string(U(to_string(ipv4_srcs)));

    string s_subnet_mask_v6(ipmiNetwork[i].net_mask_v6.begin(),
                            ipmiNetwork[i].net_mask_v6.end());
    IPV6["IPV6_SUBNET_PREFIX_LENGTH"] = json::value::string(s_subnet_mask_v6);
    string s_ip_addr_v6(ipmiNetwork[i].ip_addr_v6.begin(),
                        ipmiNetwork[i].ip_addr_v6.end());
    IPV6["IPV6_ADDRESS"] = json::value::string(s_ip_addr_v6);
    IPV6["IPV6_ENABLE"] =
        json::value::string(U(to_string(ipmiNetwork[i].set_enable_v6)));
    IPV6["IPV6_DHCP_ENABLE"] =
        json::value::string(U(to_string(ipmiNetwork[i].ip_src_v6)));
    string s_gateway(ipmiNetwork[i].df_gw_ip_addr_v6.begin(),
                     ipmiNetwork[i].df_gw_ip_addr_v6.end());
    IPV6["IPV6_GATEWAY"] = json::value::string(s_gateway);

    VLAN["VLAN_SETTINGS_ENABLE"] =
        json::value::string(U(to_string(vlan_enables)));
    VLAN["VLAN_ID"] = json::value::string(U(to_string(ipmiNetwork[i].vlan_id)));
    VLAN["VLAN_PRIORITY"] =
        json::value::string(U(to_string(ipmiNetwork[i].vlan_priority)));

    NETWORK_INFO["GENERIC"] = GENERIC;
    NETWORK_INFO["IPV4"] = IPV4;
    NETWORK_INFO["IPV6"] = IPV6;
    NETWORK_INFO["VLAN"] = VLAN;
    net_info_vec.push_back(NETWORK_INFO);
  }
  obj["NETWORK_INFO"] = json::value::array(net_info_vec);
  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  return strlen(res);
}

typedef enum {
  Jan = 1,
  Feb,
  Mar,
  Apr,
  May,
  Jun,
  Jul,
  Aug,
  Sep,
  Oct,
  Nov,
  Dec
} month2Int;
const char *etable[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                        "Aug", "Sep", "Oct", "Nov", "Dec", NULL};

int parse_ntp_conf_file(char *res) {
  json::value obj = json::value::object();
  json::value NTP_INFO = json::value::object();
  json::value NTP = json::value::object();

  if (access(NTPFILE, F_OK) == 0) {
    char buf[64];
    char server[32];
    char cmds[100] = {
        0,
    };

    NTP["AUTO_SYNC"] = json::value::string(U("1"));

    // ntp server 백업 주소 가져오기.
    if (access("/etc/ntp.conf.bak", F_OK) == 0) {
      sprintf(cmds, "mv /etc/ntp.conf.bak /etc/ntp.conf");
      system(cmds);
    }
    sprintf(buf, "awk '$1 == \"server\" {print $2}' %s", NTPFILE);

    FILE *fp = popen(buf, "r");

    fgets(server, 32, fp);
    server[strlen(server) - 1] = '\0';

    pclose(fp);

    NTP["NTP_SERVER"] = json::value::string(U(server));
    NTP["TIME_ZONE"] = json::value::string(U(""));
    NTP["YEAR"] = json::value::string(U(""));
    NTP["MONTH"] = json::value::string(U(""));
    NTP["DAY"] = json::value::string(U(""));
    NTP["HOUR"] = json::value::string(U(""));
    NTP["MIN"] = json::value::string(U(""));
    NTP["SEC"] = json::value::string(U(""));
  } else {
    char date[32];
    char buf[8];

    FILE *fp = popen("date -R", "r");
    fgets(date, 32, fp);
    pclose(fp);

    char *token;
    token = strtok(date, " ");
    token = strtok(NULL, " ");

    NTP["AUTO_SYNC"] = json::value::string(U("0"));
    NTP["NTP_SERVER"] = json::value::string(U(""));
    NTP["DAY"] = json::value::string(U(token));

    token = strtok(NULL, " ");
    for (int i = 0; etable[i] != NULL; i++)
      if (!strcmp(token, etable[i]))
        NTP["MONTH"] = json::value::string(to_string(i + 1));

    token = strtok(NULL, " ");
    NTP["YEAR"] = json::value::string(U(token));

    token = strtok(NULL, " :");
    NTP["HOUR"] = json::value::string(U(token));

    token = strtok(NULL, " :");
    NTP["MIN"] = json::value::string(U(token));

    token = strtok(NULL, " :");
    NTP["SEC"] = json::value::string(U(token));

    token = strtok(NULL, " ");

    if (token[0] == '-')
      sprintf(buf, "GMT-%d", atoi(token + 1));
    else
      sprintf(buf, "GMT+%d", atoi(token + 1) / 100);

    NTP["TIME_ZONE"] = json::value::string(U(buf));
  }

  NTP_INFO["NTP"] = NTP;
  obj["NTP_INFO"] = NTP_INFO;
  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  return strlen(res);
}

int set_ntp_conf_auto(char *server) {
  FILE *fp = fopen("/etc/ntp.conf", "w");
  fprintf(fp, "restrict 127.0.0.1\n");
  fprintf(fp, "server %s\n", server);
  fclose(fp);
  system("killall -9 ntpd");
  if (system("/usr/sbin/ntpd") == 0)
    return 0;
  else
    return 0;
}

int rest_get_smtp_json(char *res, int flag) {
  char machine_name[50] = "\0", sender_address[50] = "\0",
       primary_server_address[50] = "\0", primary_user_name[50] = "\0";
  char primary_user_password[50] = "\0", secondary_server_address[50] = "\0",
       secondary_user_name[50] = "\0", secondary_user_password[50] = "\0";
  smtp_config_t smtp_config;

  if (access(SMTP_BIN, F_OK) == -1) {
    fprintf(stderr, "\t\tWarning : No SMTP configured.\n");
  }

  FILE *fp = fopen(SMTP_BIN, "r");

  if (fp != NULL) {
    if (fread(&smtp_config, sizeof(smtp_config_t), 1, fp) < 1) {
      fprintf(stderr, "\t\tError : fread smtp_config for parsing failed\n");
      fclose(fp);
      return 0;
    }

    strcpy(machine_name, smtp_config.machine);
    strcpy(sender_address, smtp_config.sender);
    strcpy(primary_server_address, smtp_config.server1);
    strcpy(primary_user_name, smtp_config.id1);
    strcpy(primary_user_password, smtp_config.pwd1);
    strcpy(secondary_server_address, smtp_config.server2);
    strcpy(secondary_user_name, smtp_config.id2);
    strcpy(secondary_user_password, smtp_config.pwd2);
    fclose(fp);
  }
  primary_user_password[strlen(primary_user_password)] = '\0';

  char buf[200];
  json::value obj = json::value::object();
  json::value SMTP_INFO = json::value::object();

  if (flag == 0) {
    json::value DEVICE = json::value::object();
    DEVICE["MACHINE_NAME"] = json::value::string(U(machine_name));
    DEVICE["SENDER_ADDRESS"] = json::value::string(U(sender_address));

    json::value PRIMARY = json::value::object();
    PRIMARY["PRIMARY_SERVER_ADDRESS"] =
        json::value::string(U(primary_server_address));
    PRIMARY["PRIMARY_USER_NAME"] = json::value::string(U(primary_user_name));
    PRIMARY["PRIMARY_USER_PASSWORD"] =
        json::value::string(U(primary_user_password));

    json ::value SECONDARY = json::value::object();
    SECONDARY["SECONDARY_SERVER_ADDRESS"] =
        json::value::string(U(secondary_server_address));
    SECONDARY["SECONDARY_USER_NAME"] =
        json::value::string(U(secondary_user_name));
    SECONDARY["SECONDARY_USER_PASSWORD"] =
        json::value::string(U(secondary_user_password));

    SMTP_INFO["DEVICE"] = DEVICE;
    SMTP_INFO["PRIMARY"] = PRIMARY;
    SMTP_INFO["SECONDARY"] = SECONDARY;
    obj["SMTP_INFO"] = SMTP_INFO;
    strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  } else if (flag == 1) { // get primary receiver
    if (strcmp(primary_server_address, "\0") == 0) {
      fprintf(stderr, "No primary receiver\n");
      return 0;
    }
    sscanf(primary_server_address, "smtp.%s", buf);
    sprintf(res, "%s@%s", primary_user_name, buf);
  } else if (flag == 2) { // get secondary receiver
    if (strcmp(secondary_server_address, "\0") == 0) {
      fprintf(stderr, "No secondary receiver\n");
      return 0;
    }
    sscanf(secondary_server_address, "smtp.%s", buf);
    sprintf(res, "%s@%s", secondary_user_name, buf);
  }
  return strlen(res);
}

/**
 * @brief Set the smtp sender machine object
 *
 * @param sender 송신자 이메일
 * @param machine 디바이스 이름
 * @return int
 */
int set_smtp_sender_machine(char *sender, char *machine) {
  try {

    FILE *fp = fopen(SMTP_BIN, "w");
    EventService *es = ((EventService *)g_record[ODATA_EVENT_SERVICE_ID]);
    if (es == nullptr) {
      log(info) << "set_smtp_sender_machine: EventService NullPoint error";
      return 0;
    }
    es->smtp.smtp_sender_address = sender;
    smtp_config_t smtp_config;
    strcpy(smtp_config.sender, sender);
    strcpy(smtp_config.machine, machine);
    es->save_json();

    if (fwrite(&smtp_config, sizeof(smtp_config_t), 1, fp) < 1) {
      fprintf(stderr, "\t\tError : fwrite sender and machine failed\n");
      fclose(fp);
      return 0;
    }
    fclose(fp);
    return 0;
  } catch (const std::exception &e) {
    log(info) << "set_smtp_sender_machine error:";
  }
}
/**
 * @brief Set the smtp primary receiver object
 * @details Redifsh의 EventService 부분은 1개의 SMTP서버만을 가지고
  있기때문에 Primary 정보만을 작성
 * @param server SMTP 서버
 * @param id SMTP 아이디
 * @param pwd SMTP 비밀번호
 * @return int
 */
int set_smtp_primary_receiver(char *server, char *id, char *pwd) {
  FILE *fp_read = fopen(SMTP_BIN, "r");
  EventService *es = ((EventService *)g_record[ODATA_EVENT_SERVICE_ID]);
  if (fp_read == NULL) {
    fprintf(stderr, "To read SMTP_BIN for setting primary receiver failed\n");
    return 0;
  }
  smtp_config_t smtp_config;

  if (fread(&smtp_config, sizeof(smtp_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread smtp_conig for primary failed\n");
    fclose(fp_read);
    return 0;
  }
  fclose(fp_read);
  es->smtp.smtp_server = server;
  es->smtp.smtp_username = id;
  es->smtp.smtp_password = pwd;
  es->save_json();

  strcpy(smtp_config.server1, server);
  strcpy(smtp_config.id1, id);
  strcpy(smtp_config.pwd1, pwd);

  FILE *fp_write = fopen(SMTP_BIN, "w");
  if (fwrite(&smtp_config, sizeof(smtp_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite smtp_config for primary failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_smtp_secondary_receiver(char *server, char *id, char *pwd) {
  FILE *fp_read = fopen(SMTP_BIN, "r");
  if (fp_read == NULL) {
    fprintf(stderr, "To read SMTP_BIN for setting secondary receiver failed\n");
    return 0;
  }
  smtp_config_t smtp_config;
  if (fread(&smtp_config, sizeof(smtp_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread smtp_conig for secondary failed\n");
    fclose(fp_read);
    return 0;
  }
  fclose(fp_read);

  strcpy(smtp_config.server2, server);
  strcpy(smtp_config.id2, id);
  strcpy(smtp_config.pwd2, pwd);

  FILE *fp_write = fopen(SMTP_BIN, "w");
  if (fwrite(&smtp_config, sizeof(smtp_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite smtp_config for secondary failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}
/**
 * @brief SMTP 설정을 redfish에 로드 및 /conf/.msmtprc에 저장
 *
 * @return  None
 * @bug 기존 bin 파일에서 Redfish로 수정중 ..
 */
int write_smtp_config_to_file() {

  FILE *fp_read = fopen(SMTP_BIN, "r");
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tNo SMTP_BIN\n");
    return 0;
  }
  smtp_config_t smtp_config;
  if (fread(&smtp_config, sizeof(smtp_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread smtp_config for writing failed\n");
    fclose(fp_read);
    return 0;
  }
  fclose(fp_read);

  FILE *fp_write = fopen(SMTPCONF, "w");
  fprintf(fp_write, "#machine %s\n", smtp_config.machine);
  fprintf(fp_write, "defaults\nfrom %s\n", smtp_config.sender);
  fprintf(fp_write,
          "auth on\nport 587\ntls on\ntls_starttls on\ntls_trust_file "
          "/etc/ssl/certs/ca-certificates.crt\n\n");
  fprintf(fp_write, "account PRIMARY\nhost %s\nuser %s\npassword %s\n\n",
          smtp_config.server1, smtp_config.id1, smtp_config.pwd1);
  fprintf(fp_write, "account SECONDARY\nhost %s\nuser %s\npassword %s\n",
          smtp_config.server2, smtp_config.id2, smtp_config.pwd2);

  fclose(fp_write);
  return 0;
}
/**
 * @brief ssl_conf_redfish로 변경 예정
 * @details Cerificate Action 수행
 * @bug ssl redfish 수정중
 * @param res
 * @return int
 */
int parse_ssl_conf_file(unsigned char *res) {
  char country[3] = "\0", state_province[32] = "\0", city_locality[32] = "\0",
       organ[32] = "\0";
  char organ_unit[32] = "\0", common[32] = "\0", email[32] = "\0",
       keylen[5] = "\0";
  char valid_from[16] = "\0", valid_to[16] = "\0";
  int valid_for = 0;
  FILE *fp_read = fopen(SSL_BIN, "r");
  CertificateService *certificate =
      ((CertificateService *)g_record[ODATA_CERTIFICATE_SERVICE_ID]);

  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : No ssl_bin.\n");
  } else {
    ssl_config_t ssl_config;
    if (fread(&ssl_config, sizeof(ssl_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "\t\tError : fread ssl_config for parsing failed\n");
      fclose(fp_read);
      return 0;
    }

    fclose(fp_read);

    strcpy(country, ssl_config.country);
    strcpy(state_province, ssl_config.state_province);
    strcpy(city_locality, ssl_config.city_locality);
    strcpy(organ, ssl_config.organ);
    strcpy(organ_unit, ssl_config.organ_unit);
    strcpy(common, ssl_config.common);
    strcpy(email, ssl_config.email);
    strcpy(keylen, ssl_config.keylen);
    strcpy(valid_from, ssl_config.valid_from);
    strcpy(valid_to, ssl_config.valid_to);
    valid_for = ssl_config.valid_for;
  }

  json::value obj = json::value::object();
  json::value SSL_INFO = json::value::object();

  json::value BASIC = json::value::object();
  BASIC["VERSION"] = json::value::string(U("1.0.2e"));
  BASIC["SERIAL_NUMBER"] = json::value::string(U("9FF7A"));
  BASIC["SIGNATURE_ALGORITHM"] = json::value::string(U("RSA"));

  json::value ISSUED_FROM = json::value::object();
  ISSUED_FROM["COMMON_NAME"] = json::value::string(U(common));
  ISSUED_FROM["ORGANIZATION"] = json::value::string(U(organ));
  ISSUED_FROM["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));
  ISSUED_FROM["CITY_OR_LOCALITY"] = json::value::string(U(city_locality));
  ISSUED_FROM["STATE_OR_PROVINCE"] = json::value::string(U(state_province));
  ISSUED_FROM["COUNTRY"] = json::value::string(U(country));
  ISSUED_FROM["EMAIL_ADDRESS"] = json::value::string(U(email));
  ISSUED_FROM["VALID_FOR"] = json::value::string(to_string(valid_for));
  ISSUED_FROM["KEY_LENGTH"] = json::value::string(U("1024"));

  json::value VALIDITY_INFORMATION = json::value::object();
  VALIDITY_INFORMATION["VALID_FROM"] = json::value::string(U(valid_from));
  VALIDITY_INFORMATION["VALID_FOR"] = json::value::string(U(valid_to));

  json::value ISSUED_TO = json::value::object();
  ISSUED_TO["COMMON_NAME"] = json::value::string(U(common));
  ISSUED_TO["ORGANIZATION"] = json::value::string(U(organ));
  ISSUED_TO["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));

  SSL_INFO["BASIC"] = BASIC;
  SSL_INFO["ISSUED_FROM"] = ISSUED_FROM;
  SSL_INFO["ISSUED_TO"] = ISSUED_TO;
  SSL_INFO["VALIDITY_INFORMATION"] = VALIDITY_INFORMATION;
  obj["SSL_INFO"] = SSL_INFO;

  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  return strlen(res);
}

int set_ssl_1(char *keylen, char *country, char *state_province,
              char *city_locality, char *organ, int valid_for) {
  FILE *fp_write = fopen(SSL_BIN, "w");

  if (fp_write == NULL) {
    fprintf(stderr, "fopen SSL_BIN for domain failed. No SSL_BIN\n");
    return 0;
  }

  ssl_config_t ssl_config;
  strcpy(ssl_config.country, country);
  strcpy(ssl_config.keylen, keylen);
  strcpy(ssl_config.state_province, state_province);
  strcpy(ssl_config.city_locality, city_locality);
  strcpy(ssl_config.organ, organ);
  ssl_config.valid_for = valid_for;
  get_expire_day(valid_for, ssl_config.valid_from, ssl_config.valid_to);
  if (fwrite(&ssl_config, sizeof(ssl_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ssl_config 1 failed\n");
    fclose(fp_write);
    return 0;
  }

  fclose(fp_write);
  return 0;
}

int set_ssl_2(char *organ_unit, char *common, char *email) {
  FILE *fp_read = fopen(SSL_BIN, "r");
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tError : fread ssl_config failed\n");
    return 0;
  }
  ssl_config_t ssl_config;
  if (fread(&ssl_config, sizeof(ssl_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread ssl_config failed\n");
    fclose(fp_read);
    return 0;
  }
  fclose(fp_read);

  strcpy(ssl_config.organ_unit, organ_unit);
  strcpy(ssl_config.common, common);
  strcpy(ssl_config.email, email);

  FILE *fp_write = fopen(SSL_BIN, "w");
  if (fp_write == NULL) {
    fprintf(stderr, "fopen AD_BIN failed\n");
    return 0;
  }

  if (fwrite(&ssl_config, sizeof(ssl_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ssl_config 1 failed\n");
    fclose(fp_write);
    return 0;
  }

  fclose(fp_write);
  return 0;
}

void get_expire_day(int days, char *today, char *expire_day) {
  time_t now;
  struct tm *now_t;
  now = time(NULL);
  now_t = localtime(&now);
  sprintf(today, "%d-%d-%d", now_t->tm_year + 1900, now_t->tm_mon + 1,
          now_t->tm_mday);

  time_t timer;
  struct tm *t;
  timer = time(NULL) + (24 * days * 60 * 60);
  t = localtime(&timer);

  sprintf(expire_day, "%d-%d-%d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

  return;
}

int parse_ad_conf_file(char *res) {
  int enable = 0;
  char ip[16] = {
      0,
  };
  char domain[64] = {
      0,
  };
  char s_username[64] = {
      0,
  };
  char s_password[32] = {
      0,
  };

  FILE *fp_read = fopen(AD_BIN, "rb");
  if (fp_read != NULL) {
    ad_config_t ad_config;
    if (fread(&ad_config, sizeof(ad_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "Error : fread ad_config for GET failed\n");
    }
    enable = ad_config.enable;
    if (enable) {
      strcpy(ip, ad_config.dc_ip);
      strcpy(domain, ad_config.domain);
      strcpy(s_username, ad_config.secret_name);
      strcpy(s_password, ad_config.secret_pwd);
    }
    fclose(fp_read);
  }

  json::value obj = json::value::object();
  json::value ACTIVE_DIRECTORY = json::value::object();
  ACTIVE_DIRECTORY["ENABLE"] = json::value::string(U(std::to_string(enable)));
  ACTIVE_DIRECTORY["IP"] = json::value::string(U(ip));
  ACTIVE_DIRECTORY["DOMAIN"] = json::value::string(U(domain));
  ACTIVE_DIRECTORY["SECRET_NAME"] = json::value::string(U(s_username));
  ACTIVE_DIRECTORY["SECRET_PWD"] = json::value::string(U(s_password));
  obj["ACTIVE_DIRECTORY"] = ACTIVE_DIRECTORY;
  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  return strlen(res);
}

int set_ad_enable(int enable) {
  FILE *fp_write = fopen(AD_BIN, "wb");
  ad_config_t ad_config;
  memset(&ad_config, 0, sizeof(ad_config_t));
  ad_config.enable = enable;// - 48;
  if (fwrite(&ad_config, sizeof(ad_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ad_config for enable failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ad_conf_file_ip_pwd(char *ip, char *pwd) {
  FILE *fp_read = fopen(AD_BIN, "r");
  ad_config_t ad_config;
  if (fread(&ad_config, sizeof(ad_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread ad_config for ip, pwd failed\n");
    return 0;
  }
  fclose(fp_read);

  FILE *fp_write = fopen(AD_BIN, "w");
  strcpy(ad_config.dc_ip, ip);
  strcpy(ad_config.secret_pwd, pwd);
  if (fwrite(&ad_config, sizeof(ad_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ad_config (ip, pwd) failed\n");
    return 0;
  }

  fclose(fp_write);
}

int set_ad_conf_file_domain(char *domain) {
  FILE *fp_read = fopen(AD_BIN, "r");
  ad_config_t ad_config;

  if (fp_read == NULL) {
    fprintf(stderr, "fopen AD_BIN for domain failed. No AD_BIN\n");
    return 0;
  }
  if (fread(&ad_config, sizeof(ad_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread ad_config for domain failed\n");
    fclose(fp_read);
    return 0;
  }

  fclose(fp_read);

  strcpy(ad_config.domain, domain);

  FILE *fp_write = fopen(AD_BIN, "w");
  if (fp_write == NULL) {
    fprintf(stderr, "fopen AD_BIN failed\n");
    return 0;
  }

  if (fwrite(&ad_config, sizeof(ad_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ad_config for domain failed\n");
    fclose(fp_write);
    return 0;
  }

  fclose(fp_write);
  return 0;
}

int set_ad_conf_file_username(char *s_username) {
  FILE *fp_read = fopen(AD_BIN, "r");
  ad_config_t ad_config;
  if (fp_read == NULL) {
    fprintf(stderr, "fp_read for s_username failed\n");
    return 0;
  }

  if (fread(&ad_config, sizeof(ad_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread ad_config for s_username failed\n");
    return 0;
  }

  fclose(fp_read);
  strcpy(ad_config.secret_name, s_username);

  FILE *fp_write = fopen(AD_BIN, "w");
  if (fwrite(&ad_config, sizeof(ad_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ad_config for s_username failed\n");
    return 0;
  }

  fclose(fp_write);
  return 0;
}

int write_ad_to_file() {
  int enable = 0;
  char ip[16] = "\0";
  char domain[64] = "\0";
  char s_username[64] = "\0";
  char s_password[32] = "\0";

  FILE *fp_read = fopen(AD_BIN, "r");
  ad_config_t ad_config;
  if (fp_read == NULL) {
    fprintf(stderr, "fp_read to write down failed\n");
    return 0;
  }

  if (fread(&ad_config, sizeof(ad_config_t), 1, fp_read) < 1) {
    fprintf(stderr, "\t\tError : fread ad_config for write down failed\n");
    fclose(fp_read);
    return 0;
  }

  fclose(fp_read);

  if (access(NSLCD_FILE, F_OK) == 0)
    system("rm /etc/nslcd.conf");

  int fd_write = open(NSLCD_FILE, O_RDWR | O_CREAT, 0700);

  char buf[128];
  char full_buf[4092];
  sprintf(buf, "uri ldap://%s\ntls_reqcert allow\n", ad_config.dc_ip);
  strcpy(full_buf, buf);
  sprintf(buf, "base %s\npagesize 1000\nreferrals off\nnss_nested_groups yes\n",
          ad_config.domain);
  strcat(full_buf, buf);
  sprintf(buf, "binddn %s\n", ad_config.secret_name);
  strcat(full_buf, buf);
  sprintf(buf, "bindpw %s\n", ad_config.secret_pwd);
  strcat(full_buf, buf);
  sprintf(buf, "filter     passwd (objectClass=*)\nmap     passwd  uid     "
               "sAMAccountName\n");
  strcat(full_buf, buf);
  sprintf(buf, "filter     shadow (objectClass=*)\nmap     shadow  uid     "
               "sAMAccountName\n");
  strcat(full_buf, buf);

  write(fd_write, full_buf, strlen(full_buf));
  close(fd_write);
  return 0;
}

int parse_ldap_conf_file(char *res) {
  char ip[16] =
      {
          0,
      },
       port[6] =
           {
               0,
           },
       searchbase[32] =
           {
               0,
           },
       binddn[32] =
           {
               0,
           },
       pwd[32] = {
           0,
       };
  int timelimit = 0;
  int enable = 0;
  int ssl = 0;
  ldap_config_t ldap_config;

  memset(&ldap_config, 0, sizeof(ldap_config_t));
  FILE *fp_read = fopen(LDAP_BIN, "rb");
  if (fp_read != NULL) {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "Error : fread ldap_config\n");
    }
    enable = ldap_config.enable;
    printf("\t\t\t\tcheckpoint2 : ldap en : %d\n", enable);

    if (enable) {
      strcpy(ip, ldap_config.ip);
      strcpy(port, ldap_config.port);
      strcpy(searchbase, ldap_config.basedn);
      strcpy(binddn, ldap_config.binddn);
      strcpy(pwd, ldap_config.bindpw);
      ssl = ldap_config.ssl;
      timelimit = ldap_config.timelimit;
    }
    fclose(fp_read);
  }

  json::value obj = json::value::object();
  json::value LDAP_INFO = json::value::object();
  json::value LDAP = json::value::object();

  LDAP["LDAP_EN"] = json::value::string(U(to_string(enable)));
  LDAP["BIND_PW"] = json::value::string(U(pwd));
  LDAP["LDAP_IP"] = json::value::string(U(ip));
  LDAP["LDAP_PORT"] = json::value::string(U(port));
  LDAP["TIMEOUT"] = json::value::string(U(to_string(timelimit)));
  LDAP["BASE_DN"] = json::value::string(U(searchbase));
  LDAP["LDAP_SSL"] = json::value::string(U(to_string(ssl)));
  LDAP["BIND_DN"] = json::value::string(U(binddn));
  LDAP_INFO["LDAP"] = LDAP;
  obj["LDAP_INFO"] = LDAP_INFO;

  strncpy(res, obj.serialize().c_str(), obj.serialize().length());

  return strlen(res);
}

int set_ldap_enable(int enable) {
  ldap_config_t ldap_config;
  memset(&ldap_config, 0, sizeof(ldap_config_t));

  FILE *fp_write = fopen(LDAP_BIN, "wb");
  ldap_config.enable = enable;// - 48;
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_conig for enable failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ldap_ip(char *ip) {
  FILE *fp_read = fopen(LDAP_BIN, "r");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread ldap_config for ip failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "\t\tError : fread ldap_config for setting ip failed\n");
    }
    fclose(fp_read);
  }

  FILE *fp_write = fopen(LDAP_BIN, "w");
  strcpy(ldap_config.ip, ip);
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_config for setting failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ldap_port(char *port) {
  FILE *fp_read = fopen(LDAP_BIN, "r");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread ldap_config for port failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr,
              "\t\tError : fread ldap_config for setting port failed\n");
    }
    fclose(fp_read);
  }

  FILE *fp_write = fopen(LDAP_BIN, "w");
  strcpy(ldap_config.port, port);
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_config for setting failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ldap_searchbase(char *basedn) {
  FILE *fp_read = fopen(LDAP_BIN, "r");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread ldap_config for searchbase failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr,
              "\t\tError : fread ldap_config for setting searchbase failed\n");
    }
    fclose(fp_read);
  }

  FILE *fp_write = fopen(LDAP_BIN, "w");
  strcpy(ldap_config.basedn, basedn);
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_config for setting failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ldap_binddn(char *binddn) {
  FILE *fp_read = fopen(LDAP_BIN, "r");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread ldap_config for binddn failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr,
              "\t\tError : fread ldap_config for setting binddn failed\n");
    }
    fclose(fp_read);
  }

  FILE *fp_write = fopen(LDAP_BIN, "w");
  strcpy(ldap_config.binddn, binddn);
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_config for setting failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ldap_bindpw(char *bindpw) {
  FILE *fp_read = fopen(LDAP_BIN, "r");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread ldap_config for bindpw failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr,
              "\t\tError : fread ldap_config for setting bindpw failed\n");
    }
    fclose(fp_read);
  }

  FILE *fp_write = fopen(LDAP_BIN, "w");
  strcpy(ldap_config.bindpw, bindpw);
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_config for setting failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ldap_ssl(int ssl) {
  FILE *fp_read = fopen(LDAP_BIN, "rb");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread ldap_config for ssl failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "\t\tError : fread ldap_config for setting ssl failed\n");
    }
    fclose(fp_read);
  }

  FILE *fp_write = fopen(LDAP_BIN, "wb");
  ldap_config.ssl = ssl - 48;
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_config for setting failed\n");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int set_ldap_timelimit(int time) {
  FILE *fp_read = fopen(LDAP_BIN, "r");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread ldap_config for time failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr,
              "\t\tError : fread ldap_config for setting time failed\n");
    }
    fclose(fp_read);
  }

  FILE *fp_write = fopen(LDAP_BIN, "w");
  ldap_config.timelimit = time;
  if (fwrite(&ldap_config, sizeof(ldap_config_t), 1, fp_write) < 1) {
    fprintf(stderr, "\t\tError : fwrite ldap_config for time failed");
    fclose(fp_write);
    return 0;
  }
  fclose(fp_write);
  return 0;
}

int write_ldap_to_nslcd() {
  FILE *fp_read = fopen(LDAP_BIN, "r");
  ldap_config_t ldap_config;
  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : fread LDAP_BIN for nslcd failed\n");
  } else {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "\t\tError : fread ldap_config for nslcd failed\n");
    }
    fclose(fp_read);
  }

  char buf[128];
  char full_buf[4092];
  if (access(NSLCD_FILE, F_OK) == 0)
    system("rm /etc/nslcd.conf");

  int fd_write = open(NSLCD_FILE, O_RDWR | O_CREAT, 0700);
  sprintf(buf, "uri ldap://%s:%s\n", ldap_config.ip, ldap_config.port);
  strcpy(full_buf, buf);
  sprintf(buf, "base %s\n", ldap_config.basedn);
  strcat(full_buf, buf);
  sprintf(buf, "binddn %s\n", ldap_config.binddn);
  strcat(full_buf, buf);
  sprintf(buf, "bindpw %s\n", ldap_config.bindpw);
  strcat(full_buf, buf);
  sprintf(buf, "timelimit %d\n", ldap_config.timelimit);
  strcat(full_buf, buf);

  write(fd_write, full_buf, strlen(full_buf));
  close(fd_write);

  return 0;
}

int parse_radius_conf_file(char *res) {
  char ip[16] = "\0", port[6] = "\0", secret[32] = "\0";
  FILE *fp = fopen(RAD_BIN, "r");
  rad_config_t rad_config;
  int enable;
  if (fp != NULL) {
    if (fread(&rad_config, sizeof(rad_config_t), 1, fp) < 1) {
      fprintf(stderr, "\t\tError : fread error radc_config");
      fclose(fp);
      return 0;
    }
    enable = rad_config.enable;
    strcpy(ip, rad_config.ip);
    strcpy(port, rad_config.port);
    strcpy(secret, rad_config.secret);
    fclose(fp);
  }

  json::value obj = json::value::object();
  json::value RADIUS_INFO = json::value::object();
  json::value RADIUS = json::value::object();

  RADIUS["RADIUS_ENABLE"] = json::value::string(to_string(enable));
  RADIUS["IP"] = json::value::string(U(ip));
  RADIUS["PORT"] = json::value::string(U(port));
  RADIUS["SECRET"] = json::value::string(U(secret));
  RADIUS_INFO["RADIUS"] = RADIUS;
  obj["RADIUS_INFO"] = RADIUS_INFO;

  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  return strlen(res);
}

int set_radius_config(char *ip, char *port, char *secret) {
  FILE *fp_bin = fopen(RAD_BIN, "w");
  rad_config_t rad_config;
  rad_config.enable = 1;
  strcpy(rad_config.ip, ip);
  strcpy(rad_config.port, port);
  strcpy(rad_config.secret, secret);

  if (fwrite(&rad_config, sizeof(rad_config_t), 1, fp_bin) < 1) {
    fprintf(stderr, "\t\tError : fwrite error: rad config\n");
    fclose(fp_bin);
    return 0;
  }

  fclose(fp_bin);

  if (access(RADFILE, F_OK) != 0) {
    system("mkdir /etc/raddb");
  }

  FILE *fp = fopen(RADFILE, "w");
  fprintf(fp, "%s:%s\t%s\t5\n", ip, port, secret);
  fclose(fp);
}

int set_radius_disable() {
  rad_config_t rad_config;
  rad_config.enable = 0;
  memset(&rad_config, 0, sizeof(rad_config_t));
  FILE *fp_bin = fopen(RAD_BIN, "w");
  if (fwrite(&rad_config, sizeof(rad_config_t), 1, fp_bin) < 1) {
    fprintf(stderr, "\t\tError : fwrite in set_radius_disable");
    fclose(fp_bin);
    return 0;
  }

  fclose(fp_bin);
  if (access(RADFILE, F_OK) == 0)
    system("rm /etc/raddb/server");
  return 0;
}
/*
 * Editor : KICHEOL PARK
 * Description : get watchdog timer functions
 */

//
static void app_get_watchdog_timer_params(unsigned char *request,
                                          unsigned char *response,
                                          unsigned char *res_len) {
  cout << "Get watchdog timer parameter" << endl;
  ;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];

  res->cc = CC_SUCCESS;

  *data++ = g_watchdog_config.timer_use;
  *data++ = g_watchdog_config.timer_actions;
  *data++ = g_watchdog_config.pre_timeout;
  *data++ = g_watchdog_config.timer_use_exp;
  *data++ = g_watchdog_config.initial_countdown_lsb;
  *data++ = g_watchdog_config.initial_countdown_msb;
  *data++ = g_watchdog_config.present_countdown_lsb;
  *data++ = g_watchdog_config.present_countdown_msb;

  *data++ = 0x00; // Extra error info in case of failure
  if (res->cc == CC_SUCCESS) {
    res_len = data - &res->data[0];
  }

  return;
}
/*
 * Editor : KICHEOL PARK
 * Description : set watchdog timer functions
 */
static void app_set_watchdog_timer_params(unsigned char *request,
                                          unsigned char *response,
                                          unsigned char *res_len) {
  cout << ("Get watchdog timer parameter") << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  cout << ("Get watchdog timer 11") << endl;

  bool Ischanged = false;
  for (int i = 0; i < 6; i++) {
    cout << "bitset data[" << i << "] =" << uint32_t(req->data[i]) << endl;
    bitset<16> bit = bitset<16>(req->data[i]);
    cout << bit << endl;
    if (bitset<16>(req->data[i]).any()) {

      switch (i) {
      case 0:

        if (bitset<16>(req->data[i]).test(7)) {
          g_watchdog_config.Islogging = false;
          Ischanged = true;
        }
        break;
      case 1:

        if (bit.test(4))
          g_watchdog_config.pretimeoutInterrupt = 0x01; // SMI
        if (bit.test(5))
          g_watchdog_config.pretimeoutInterrupt = 0x02; // NVI
        break;
        if (bit.test(6))
          g_watchdog_config.pretimeoutInterrupt = 0x03; // MSI
        if ((bit.test(6) || bit.test(5) || bit.test(4)) == 0)
          g_watchdog_config.pretimeoutInterrupt = 0x00;

        g_watchdog_config.timer_actions = 0x00;
        if (bit.test(0))
          g_watchdog_config.timer_actions = 0x01; // HardReset
        if (bit.test(1))
          g_watchdog_config.timer_actions = 0x02; // PowerDown
        if (bit.test(2))
          g_watchdog_config.timer_actions = 0x03; // PowerDown
        if ((bit.test(0) || bit.test(1) || bit.test(2)) == 0)
          g_watchdog_config.pretimeoutInterrupt = 0x00;

        break;
      case 2:
        g_watchdog_config.pre_timeout = uint32_t(req->data[i]);
        Ischanged = true;
        break;
      case 3:
        break;
      case 4:
        g_watchdog_config.initial_countdown_lsb = uint32_t(req->data[i]);
        Ischanged = true;
        break;
      case 5:
        g_watchdog_config.initial_countdown_msb = uint32_t(req->data[i]);
        Ischanged = true;
        break;
      }
    }
  }

  res->cc = (Ischanged) ? CC_SUCCESS : CC_UNSPECIFIED_ERROR;
  *res_len = 0;

  return;
}
/* used by ReadConfigurationFile, check the line if it's valuable*/
/* This file refer to the watchdog version 5.5*/
static int spool(char *line, int *i, int offset) {
  for ((*i) += offset; line[*i] == ' ' || line[*i] == '\t'; (*i)++)
    ;
  if (line[*i] == '=')
    (*i)++;
  for (; line[*i] == ' ' || line[*i] == '\t'; (*i)++)
    ;
  if (line[*i] == '\0')
    return (1);
  else
    return (0);
}

/*Function used to read the configuration from the conf file in firectory
 * defined by 'filename' */
/* This file refer to the watchdog version 5.5*/
static int ReadConfigurationFile(char *file) {
  FILE *ReadConfigurationFile;

  /* Open the configuration file with readonly parameter*/
  printf("Trying the configuration file %s \n", ConfigurationFileDir);
  if ((ReadConfigurationFile = fopen(ConfigurationFileDir, "r")) == NULL) {
    printf("There is no configuration file, use default values for IPMI "
           "watchdog \n");
    return (1);
  }

  /* Check to see the configuration has data or not*/
  while (!feof(ReadConfigurationFile)) {
    char Configurationline[CONFIG_LINE_LEN];

    /* Read the line from configuration file */
    if (fgets(Configurationline, CONFIG_LINE_LEN, ReadConfigurationFile) ==
        NULL) {
      if (!ferror(ReadConfigurationFile)) {
        break;
      } else {
        return (1);
      }
    } else {
      int i, j;

      /* scan the actual line for an option , first remove the leading blanks*/
      for (i = 0; Configurationline[i] == ' ' || Configurationline[i] == '\t';
           i++)
        ;

      /* if the next sign is a '#' we have a comment , so we ignore the
       * configuration line */
      if (Configurationline[i] == '#') {
        continue;
      }

      /* also remove the trailing blanks and the \n */
      for (j = strlen(Configurationline) - 1;
           Configurationline[j] == ' ' || Configurationline[j] == '\t' ||
           Configurationline[j] == '\n';
           j--)
        ;

      Configurationline[j + 1] = '\0';

      /* if the line is empty now, we don't have to parse it */
      if (strlen(Configurationline + i) == 0) {
        continue;
      }

      /* now check for an option , interval first */

      /*Interval */
      if (strncmp(Configurationline + i, IPMI_INTERVAL,
                  strlen(IPMI_INTERVAL)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_INTERVAL))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Interval = atol(Configurationline + i);

          { printf(" IPMI_Interval = %d \n", IPMI_Interval); }
        }
      }

      /*Timeout */
      else if (strncmp(Configurationline + i, IPMI_TIMEOUT,
                       strlen(IPMI_TIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_TIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Timeout = atol(Configurationline + i);
          g_watchdog_config.initial_countdown_lsb = IPMI_Timeout & 0xFF;
          g_watchdog_config.initial_countdown_msb = IPMI_Timeout >> 8;
          printf(" IPMI_Timeout = %d \n", IPMI_Timeout);
          printf(" initial_countdown_lsb = %d \n",
                 g_watchdog_config.initial_countdown_msb);
          printf(" initial_countdown_msb = %d \n",
                 g_watchdog_config.initial_countdown_msb);
        }
      }

      /*Pretimeout */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUT,
                       strlen(IPMI_PRETIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Pretimeout = atol(Configurationline + i);

          g_watchdog_config.pre_timeout = IPMI_Pretimeout;
          printf(" IPMI_Pretimeout = %d \n", IPMI_Pretimeout);
        }
      }

      /*Daemon */
      else if (strncmp(Configurationline + i, IPMI_DAEMON,
                       strlen(IPMI_DAEMON)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_DAEMON))) {
          IPMI_Daemon = NULL;
        } else {
          IPMI_Daemon = strdup(Configurationline + i);

          printf(" IPMI_Daemon = %s \n", IPMI_Daemon);
        }
      }

      /*PretimeoutInterrupt */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUTINTERRUPT,
                       strlen(IPMI_PRETIMEOUTINTERRUPT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUTINTERRUPT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          g_watchdog_config.pretimeoutInterrupt = atol(Configurationline + i);
          printf(" IPMI_PretimeoutInterrupt = %d \n",
                 g_watchdog_config.pretimeoutInterrupt);
        }
      }

      /*Action */
      else if (strncmp(Configurationline + i, IPMI_ACTION,
                       strlen(IPMI_ACTION)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_ACTION))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Action = atol(Configurationline + i);
          g_watchdog_config.timer_actions = IPMI_Action;
          printf(" IPMI_Action = %d \n", IPMI_Action);
        }
      }

      /*Pidfile */
      else if (strncmp(Configurationline + i, IPMI_PIDFILE,
                       strlen(IPMI_PIDFILE)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PIDFILE))) {
          IPMI_Pidfile = NULL;
        } else {
          IPMI_Pidfile = strdup(Configurationline + i);

          printf(" IPMI_Pidfile = %s \n", IPMI_Pidfile);
        }
      }

      else {
        fprintf(stderr, "Ignoring config Configurationline: %s\n",
                Configurationline);
      }
    }
  }

  /* Close the configuration file */
  if (fclose(ReadConfigurationFile) != 0) {
    return (1);
  }
}

static int WriteConfigurationFile(char *file) {

  vector<string> lines;
  ifstream readFromFile(ConfigurationFileDir);
  if (readFromFile.is_open()) {
    lines.clear();
    while (!readFromFile.eof()) {
      string tmp;
      getline(readFromFile, tmp);
      cout << tmp << endl;
      lines.push_back(tmp);
    }
    readFromFile.close();
  } else {
    printf("%s not exist", ConfigurationFileDir);
    return 1;
  }

  std::ofstream writeFile(ConfigurationFileDir);
  if (writeFile.is_open()) {
    for (int i = 0; i < lines.size(); i++) {
      string tmp = lines[i];
      cout << tmp << endl;
      if (tmp.find(IPMI_INTERVAL) != string::npos) {
        tmp = "Interval = " + to_string(*g_watchdog_config.interval);
      } else if (tmp.find(IPMI_TIMEOUT) != string::npos) {
        tmp = "Timeout = " + to_string(IPMI_Timeout);
      } else if (tmp.find(IPMI_PRETIMEOUT) != string::npos) {
        tmp = "Pretimeout = " + to_string((int)g_watchdog_config.pre_timeout);
      } else if (tmp.find(IPMI_DAEMON) != string::npos) {
        if (IPMI_Daemon != NULL) {
          tmp = "Daemon = " + string(IPMI_Daemon);
        }
      } else if (tmp.find(IPMI_PRETIMEOUTINTERRUPT) != string::npos) {
        tmp = "Daemon5 = " +
              to_string((int)g_watchdog_config.pretimeoutInterrupt);
      } else if (tmp.find(IPMI_ACTION) != string::npos) {
        tmp = "Action = " + to_string((int)g_watchdog_config.timer_actions);
      } else if (tmp.find(IPMI_PIDFILE) != string::npos) {
        if (IPMI_Pidfile != NULL) {
          tmp = "Pidfile = " + string(IPMI_Pidfile);
        }
      }

      if (i != lines.size() - 1) {
        tmp += "\n";
      }
      cout << "change =" << tmp << endl;
      writeFile.write(tmp.c_str(), tmp.size());
    }

  } else {
    printf("file not exist");
  }
  writeFile.close();

  return 1;
}
/**
 * @brief LAN Alert Configuration Descriptor
 *
 */
typedef struct {
  unsigned char no_of_dest;
  unsigned char dest_type[SIZE_NUM_OF_DEST][SIZE_DEST_TYPE];
  unsigned char dest_addr[SIZE_NUM_OF_DEST][SIZE_DEST_ADDR];
} lan_alert_config_t;

void plat_lan_alert_init() {

  cout << ("Initializing LAN Alert information...") << endl;
  FILE *l_a;

  int i = 0;
  static lan_alert_config_t g_lan_alert[4];

  if (access(IPMI_LAN_ALERT_PATH, F_OK) != 0) {
    l_a = fopen(IPMI_LAN_ALERT_PATH, "w");
  } else {
    l_a = fopen(IPMI_LAN_ALERT_PATH, "r");
  }

  fread(g_lan_alert, sizeof(g_lan_alert), 4, l_a);

  for (i = 0; i < ETH_COUNT; i++) {
    memcpy(ipmiNetwork[i].dest_type, g_lan_alert[i].dest_type,
           sizeof(g_lan_alert[i].dest_type));
    memcpy(ipmiNetwork[i].dest_addr, g_lan_alert[i].dest_addr,
           sizeof(g_lan_alert[i].dest_addr));
  }
  fclose(l_a);
}
/////////////////////////////////////////////////////////////PSU관련///////////

float upper = 0;
int min_total_peak, hour_total_peak = 0;
int rm_count_1, rm_count_2, rh_count_1, rh_count_2, rd_count_1, rd_count_2,
    upper_int = 0;
int fh_count_1, fh_count_2, fd_count_1, fd_count_2 = 0;
int high_index_1[4], low_index_1[4], high_index_2[4], low_index_2[4] = {0};
power_usage_t g_min_power_1[MAX_LAST_MIN_COUNT] = {0};
power_usage_t g_min_power_2[MAX_LAST_MIN_COUNT] = {0};
power_usage_t g_hour_power_1[MAX_LAST_HOUR_COUNT] = {0};
power_usage_t g_hour_power_2[MAX_LAST_HOUR_COUNT] = {0};
power_usage_t g_day_power_1[MAX_LAST_DAY_COUNT] = {0};
power_usage_t g_day_power_2[MAX_LAST_DAY_COUNT] = {0};

/**
 * @brief Get the min power usage object Webpage에 출력
 *
 * @param menu 값에 따라 전체, 구역별 개별 출력이 가능
 */
void get_min_power_usage(int menu) {

  char buf[94] = {0};
  char query[500] = {0};
  char *zErrMsg = 0;
  sqlite3 *db;
  int rows, loop, h_watt = 0;
  int rc = 0;
  try {
    rc = sqlite3_open(POWER_USAGE_DB, &db);
  } catch (const std::exception &) {
    log(info) << "get_min_power_usage not open error";
    return;
  }
  sqlite3_stmt *statement = NULL;

  power_usage_t *hour_power_1[LAST_HOUR_COUNT_DB];
  power_usage_t *hour_power_2[LAST_HOUR_COUNT_DB];
  power_usage_t *day_power_1[LAST_DAY_COUNT_DB];
  power_usage_t *day_power_2[LAST_DAY_COUNT_DB];
  log(info) << "get_min_power_usage: menu=" << menu << endl;
  // if(rc==0)
  // {
  // 	log(info)<<"rc 0 error"<<menu<<endl;
  // 	return;
  // }
  if (!rc) {

    hour_total_peak = 0;
    min_total_peak = 0;
    // log(info)<<"get_min_power_usage: rc="<<rc<<endl;
    for (loop = 0; loop < LAST_HOUR_COUNT_DB; loop++) {
      hour_power_1[loop] = malloc(sizeof(power_usage_t));
      hour_power_2[loop] = malloc(sizeof(power_usage_t));
    }
    for (loop = 0; loop < LAST_DAY_COUNT_DB; loop++) {
      day_power_1[loop] = malloc(sizeof(power_usage_t));
      day_power_2[loop] = malloc(sizeof(power_usage_t));
    }

    for (loop = 0; loop < MAX_LAST_MIN_COUNT; loop++)
      memset(&g_min_power_1[loop], 0, sizeof(power_usage_t));
    for (loop = 0; loop < MAX_LAST_HOUR_COUNT; loop++)
      memset(&g_hour_power_1[loop], 0, sizeof(power_usage_t));
    for (loop = 0; loop < MAX_LAST_DAY_COUNT; loop++)
      memset(&g_day_power_1[loop], 0, sizeof(power_usage_t));

    for (loop = 0; loop < MAX_LAST_MIN_COUNT; loop++)
      memset(&g_min_power_2[loop], 0, sizeof(power_usage_t));
    for (loop = 0; loop < MAX_LAST_HOUR_COUNT; loop++)
      memset(&g_hour_power_2[loop], 0, sizeof(power_usage_t));
    for (loop = 0; loop < MAX_LAST_DAY_COUNT; loop++)
      memset(&g_day_power_2[loop], 0, sizeof(power_usage_t));
    // log(info)<<"init power_usage_t structrue="<<rc<<endl;

    switch (menu) {
    case WATT_ALL:
      // log(info)<<"WATT_ALL 1"<<endl;
      sprintf(query, "SELECT count(*) FROM last_min_1;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      // log(info)<<"WATT_ALL 2"<<endl;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rm_count_1 = sqlite3_column_int(statement, 0);
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);
      // log(info)<<"WATT_ALL3"<<endl;
      sprintf(query, "SELECT count(*) FROM last_min_2;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      // log(info)<<"WATT_ALL 4"<<endl;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rm_count_2 = sqlite3_column_int(statement, 0);
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      // log(info)<<"WATT_ALL 5"<<endl;
      sqlite3_finalize(statement);

      sprintf(query, "SELECT count(*) FROM last_hour_1;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      // log(info)<<"WATT_ALL 6"<<endl;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rh_count_1 = sqlite3_column_int(statement, 0);
          if (rh_count_1 > MAX_LAST_HOUR_COUNT)
            fh_count_1 = MAX_LAST_HOUR_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      log(info) << "WATT_ALL 7" << endl;
      sqlite3_finalize(statement);

      sprintf(query, "SELECT count(*) FROM last_hour_2;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      log(info) << "WATT_ALL 8" << endl;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rh_count_2 = sqlite3_column_int(statement, 0);
          if (rh_count_2 > MAX_LAST_HOUR_COUNT)
            fh_count_2 = MAX_LAST_HOUR_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      // log(info)<<"WATT_ALL 10"<<endl;
      sqlite3_finalize(statement);

      sprintf(query, "SELECT count(*) FROM last_day_1;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rd_count_1 = sqlite3_column_int(statement, 0);
          if (rd_count_1 > MAX_LAST_DAY_COUNT)
            fd_count_1 = MAX_LAST_DAY_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);
      // log(info)<<"WATT_ALL 11"<<endl;
      sprintf(query, "SELECT count(*) FROM last_day_2;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rd_count_2 = sqlite3_column_int(statement, 0);
          if (rd_count_2 > MAX_LAST_HOUR_COUNT)
            fd_count_2 = MAX_LAST_HOUR_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);
      // log(info)<<"WATT_ALL 12"<<endl;

      sprintf(query, "SELECT * FROM last_day_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          day_power_1[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            day_power_1[loop]->watt = h_watt;
          else
            day_power_1[loop]->watt = 0;
          strcpy(day_power_1[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);
      // log(info)<<"WATT_ALL 13"<<endl;
      sprintf(query, "SELECT * FROM last_day_2 ORDER BY dt;");

      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          day_power_2[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            day_power_2[loop]->watt = h_watt;
          else
            day_power_2[loop]->watt = 0;
          strcpy(day_power_2[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);
      // log(info)<<"WATT_ALL 14"<<endl;
      sprintf(query, "SELECT * FROM last_hour_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          hour_power_1[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            hour_power_1[loop]->watt = h_watt;
          else
            hour_power_1[loop]->watt = 0;
          strcpy(hour_power_1[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          hour_total_peak += hour_power_1[loop]->watt;
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_hour_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          hour_power_2[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            hour_power_2[loop]->watt = h_watt;
          else
            hour_power_2[loop]->watt = 0;
          strcpy(hour_power_2[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          hour_total_peak += hour_power_2[loop]->watt;
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_min_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          g_min_power_1[loop].id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            g_min_power_1[loop].watt = h_watt;
          else
            g_min_power_1[loop].watt = 0;
          strcpy(g_min_power_1[loop].dt,
                 (char *)sqlite3_column_text(statement, 2));
          min_total_peak += g_min_power_1[loop].watt;
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_min_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          g_min_power_2[loop].id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            g_min_power_2[loop].watt = h_watt;
          else
            g_min_power_2[loop].watt = 0;
          strcpy(g_min_power_2[loop].dt,
                 (char *)sqlite3_column_text(statement, 2));
          min_total_peak += g_min_power_2[loop].watt;
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);
      log(info) << "WATT_ALL 2-1" << endl;
      convert_power_to_g_power(hour_power_1, 1);

      convert_power_to_g_power(hour_power_2, 2);
      convert_power_to_g_power(day_power_1, 3);
      convert_power_to_g_power(day_power_2, 4);
      // log(info)<<"WATT_ALL 2-2"<<endl;

      break;
    case WATT_TOP:
    case WATT_MIDDLE:
      sprintf(query, "SELECT * FROM last_min_1 ORDER BY dt;");
      sqlite3_prepare(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          g_min_power_1[loop].id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            g_min_power_1[loop].watt = h_watt;
          else
            g_min_power_1[loop].watt = 0;
          strcpy(g_min_power_1[loop].dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_min_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          g_min_power_2[loop].id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            g_min_power_2[loop].watt = h_watt;
          else
            g_min_power_2[loop].watt = 0;
          strcpy(g_min_power_2[loop].dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_hour_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          hour_power_1[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            hour_power_1[loop]->watt = h_watt;
          else
            hour_power_1[loop]->watt = 0;
          strcpy(hour_power_1[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_hour_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          hour_power_2[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            hour_power_2[loop]->watt = h_watt;
          else
            hour_power_2[loop]->watt = 0;
          strcpy(hour_power_2[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_day_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          day_power_1[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            day_power_1[loop]->watt = h_watt;
          else
            day_power_1[loop]->watt = 0;
          strcpy(day_power_1[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_day_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          day_power_2[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            day_power_2[loop]->watt = h_watt;
          else
            day_power_2[loop]->watt = 0;
          strcpy(day_power_2[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      convert_power_to_g_power(hour_power_1, 1);
      convert_power_to_g_power(hour_power_2, 2);
      convert_power_to_g_power(day_power_1, 3);
      convert_power_to_g_power(day_power_2, 4);

      break;
    case WATT_BOTTOM_MIN:
      sprintf(query, "SELECT count(*) FROM last_min_1;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rm_count_1 = sqlite3_column_int(statement, 0);
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT count(*) FROM last_min_2;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rm_count_2 = sqlite3_column_int(statement, 0);
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_min_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          g_min_power_1[loop].id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            g_min_power_1[loop].watt = h_watt;
          else
            g_min_power_1[loop].watt = 0;
          strcpy(g_min_power_1[loop].dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_min_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          g_min_power_2[loop].id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            g_min_power_2[loop].watt = h_watt;
          else
            g_min_power_2[loop].watt = 0;
          strcpy(g_min_power_2[loop].dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      break;
    case WATT_BOTTOM_HOUR:
      sprintf(query, "SELECT count(*) FROM last_hour_1;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rh_count_1 = sqlite3_column_int(statement, 0);
          if (rh_count_1 > MAX_LAST_HOUR_COUNT)
            fh_count_1 = MAX_LAST_HOUR_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT count(*) FROM last_hour_2;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rh_count_2 = sqlite3_column_int(statement, 0);
          if (rh_count_2 > MAX_LAST_HOUR_COUNT)
            fd_count_2 = MAX_LAST_HOUR_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_hour_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          hour_power_1[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            hour_power_1[loop]->watt = h_watt;
          else
            hour_power_1[loop]->watt = 0;
          strcpy(hour_power_1[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else {
          break;
        }
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_hour_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          hour_power_2[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            hour_power_2[loop]->watt = h_watt;
          else
            hour_power_2[loop]->watt = 0;
          strcpy(hour_power_2[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      convert_power_to_g_power(hour_power_1, 1);
      convert_power_to_g_power(hour_power_2, 2);

      break;
    case WATT_BOTTOM_DAY:

      sprintf(query, "SELECT count(*) FROM last_day_1;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rd_count_1 = sqlite3_column_int(statement, 0);
          if (rd_count_1 > MAX_LAST_DAY_COUNT)
            fd_count_1 = MAX_LAST_DAY_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT count(*) FROM last_day_2;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);

      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          rd_count_2 = sqlite3_column_int(statement, 0);
          if (rd_count_2 > MAX_LAST_DAY_COUNT)
            fd_count_2 = MAX_LAST_DAY_COUNT;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_day_1 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          day_power_1[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            day_power_1[loop]->watt = h_watt;
          else
            day_power_1[loop]->watt = 0;
          strcpy(day_power_1[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      sprintf(query, "SELECT * FROM last_day_2 ORDER BY dt;");
      sqlite3_prepare_v2(db, query, -1, &statement, NULL);
      loop = 0;
      while (1) {
        rc = sqlite3_step(statement);
        if (SQLITE_ROW == rc) {
          day_power_2[loop]->id = sqlite3_column_int(statement, 0);
          h_watt = sqlite3_column_int(statement, 1);
          if (h_watt >= 0 && h_watt <= 800)
            day_power_2[loop]->watt = h_watt;
          else
            day_power_2[loop]->watt = 0;
          strcpy(day_power_2[loop]->dt,
                 (char *)sqlite3_column_text(statement, 2));
          loop++;
        } else if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc)
          continue;
        else
          break;
      }
      sqlite3_finalize(statement);

      convert_power_to_g_power(day_power_1, 3);
      convert_power_to_g_power(day_power_2, 4);

      break;
    }
    for (loop = 0; loop < LAST_HOUR_COUNT_DB; loop++) {
      free(hour_power_1[loop]);
      free(hour_power_2[loop]);
    }
    for (loop = 0; loop < LAST_DAY_COUNT_DB; loop++) {
      free(day_power_1[loop]);
      free(day_power_2[loop]);
    }
    log(info) << "WATT_ALL final 1" << endl;
    // try
    // {
    // 	//sqlite3_finalize(statement);
    // }
    // catch (const std::exception&)
    // {
    // 	log(info)<<"sqlite3_finalize error: WATT ALL"<<endl;
    // }

    log(info) << "WATT_ALL final 2" << endl;
    // sqlite3_finalize(statement);
    sqlite3_close(db);
    log(info) << "WATT_ALL final 3" << endl;
  }
}

void convert_power_to_g_power(power_usage_t **power_in, int flag) {
  int l, m = 0;

  power_usage_t **in_power = (power_usage_t **)power_in;
  switch (flag) {
  case 1:
    if (rh_count_1 > MAX_LAST_HOUR_COUNT)
      m = MAX_LAST_HOUR_COUNT - 1;
    else
      m = rh_count_1 - 1;
    for (l = (rh_count_1 - 1); l >= 0; l -= 10) {
      g_hour_power_1[m].id = in_power[l]->id;
      g_hour_power_1[m].watt = in_power[l]->watt;
      strcpy(g_hour_power_1[m].dt, in_power[l]->dt);
      if (m != 0)
        m--;
    }
    break;

  case 2:
    if (rh_count_2 > MAX_LAST_HOUR_COUNT)
      m = MAX_LAST_HOUR_COUNT - 1;
    else
      m = rh_count_2 - 1;

    for (l = (rh_count_2 - 1); l >= 0; l -= 10) {
      g_hour_power_2[m].id = in_power[l]->id;
      g_hour_power_2[m].watt = in_power[l]->watt;
      strcpy(g_hour_power_2[m].dt, in_power[l]->dt);
      if (m != 0)
        m--;
    }
    break;

  case 3:
    if (rd_count_1 > MAX_LAST_DAY_COUNT)
      m = MAX_LAST_DAY_COUNT - 1;
    else
      m = rd_count_1 - 1;
    for (l = (rd_count_1 - 1); l >= 0; l -= 60) {
      g_day_power_1[m].id = in_power[l]->id;
      g_day_power_1[m].watt = in_power[l]->watt;
      strcpy(g_day_power_1[m].dt, in_power[l]->dt);
      if (m != 0)
        m--;
    }
    break;
  case 4:

    if (rd_count_2 > MAX_LAST_DAY_COUNT)
      m = MAX_LAST_DAY_COUNT - 1;
    else
      m = rd_count_2 - 1;

    for (l = (rd_count_2 - 1); l >= 0; l -= 60) {
      g_day_power_2[m].id = in_power[l]->id;
      g_day_power_2[m].watt = in_power[l]->watt;
      strcpy(g_day_power_2[m].dt, in_power[l]->dt);
      if (m != 0)
        m--;
    }
    break;
  }
}
int get_highest_power_1(int flag) {
  int i, min_highest, hour_highest, day_highest, return_value = 0;
  min_highest = hour_highest = day_highest = return_value = 0;
  for (i = 0; i < MAX_LAST_MIN_COUNT; i++) {
    if (min_highest < g_min_power_1[i].watt) {
      min_highest = g_min_power_1[i].watt;
      high_index_1[1] = i;
    }
  }
  for (i = 0; i < MAX_LAST_HOUR_COUNT; i++) {
    if (hour_highest < g_hour_power_1[i].watt) {
      hour_highest = g_hour_power_1[i].watt;
      high_index_1[2] = i;
    }
  }
  for (i = 0; i < MAX_LAST_DAY_COUNT; i++) {
    if (day_highest < g_day_power_1[i].watt) {
      day_highest = g_day_power_1[i].watt;
      high_index_1[3] = i;
    }
  }
  switch (flag) {
  case 1:
    if ((day_highest > hour_highest) && (day_highest > min_highest)) {
      return_value = day_highest;
      high_index_1[0] = high_index_1[3];
    } else if ((hour_highest > min_highest) && (hour_highest > day_highest)) {
      return_value = hour_highest;
      high_index_1[0] = high_index_1[2];
    } else if ((min_highest > day_highest) && (min_highest > hour_highest)) {
      return_value = min_highest;
      high_index_1[0] = high_index_1[1];
    }
    break;
  case 2:
    return_value = min_highest;
    high_index_1[0] = high_index_1[1];
    break;
  case 3:
    return_value = hour_highest;
    high_index_1[0] = high_index_1[2];
    break;
  case 4:
    return_value = day_highest;
    high_index_1[0] = high_index_1[3];
  }

  return return_value;
}

int get_highest_power_2(int flag) {
  int i, min_highest, hour_highest, day_highest, return_value = 0;
  min_highest = hour_highest = day_highest = return_value = 0;
  for (i = 0; i < MAX_LAST_MIN_COUNT; i++) {
    if (min_highest < g_min_power_2[i].watt) {
      min_highest = g_min_power_2[i].watt;
      high_index_2[1] = i;
    }
  }
  for (i = 0; i < MAX_LAST_HOUR_COUNT; i++) {
    if (hour_highest < g_hour_power_2[i].watt) {
      hour_highest = g_hour_power_2[i].watt;
      high_index_2[2] = i;
    }
  }
  for (i = 0; i < MAX_LAST_DAY_COUNT; i++) {
    if (day_highest < g_day_power_2[i].watt) {
      day_highest = g_day_power_2[i].watt;
      high_index_2[3] = i;
    }
  }
  switch (flag) {
  case 1:
    if ((day_highest > hour_highest) && (day_highest > min_highest)) {
      return_value = day_highest;
      high_index_2[0] = high_index_2[3];
    } else if ((hour_highest > min_highest) && (hour_highest > day_highest)) {
      return_value = hour_highest;
      high_index_2[0] = high_index_2[2];
    } else if ((min_highest > day_highest) && (min_highest > hour_highest)) {
      return_value = min_highest;
      high_index_2[0] = high_index_2[1];
    }
    break;
  case 2:
    return_value = min_highest;
    high_index_2[0] = high_index_2[1];
    break;
  case 3:
    return_value = hour_highest;
    high_index_2[0] = high_index_2[2];
    break;
  case 4:
    return_value = day_highest;
    high_index_2[0] = high_index_2[3];
  }

  return return_value;
}

int get_lowest_power_1(int flag) {
  int i, min_lowest, hour_lowest, day_lowest, return_value = 0;
  for (i = 0; i < MAX_LAST_MIN_COUNT; i++) {
    if (i == 0)
      min_lowest = g_min_power_1[i].watt;
    else {
      if ((min_lowest > g_min_power_1[i].watt) &&
          (g_min_power_1[i].watt != 0)) {
        min_lowest = g_min_power_1[i].watt;
        low_index_1[1] = i;
      }
    }
  }
  for (i = 0; i < MAX_LAST_HOUR_COUNT; i++) {
    if (i == 0)
      hour_lowest = g_hour_power_1[i].watt;
    else {
      if ((hour_lowest > g_hour_power_1[i].watt) &&
          (g_hour_power_1[i].watt != 0)) {
        hour_lowest = g_hour_power_1[i].watt;
        low_index_1[2] = i;
      }
    }
  }
  for (i = 0; i < MAX_LAST_DAY_COUNT; i++) {
    if (i == 0)
      day_lowest = g_day_power_1[i].watt;
    else {
      if ((day_lowest > g_day_power_1[i].watt) &&
          (g_day_power_1[i].watt != 0)) {
        day_lowest = g_day_power_1[i].watt;
        low_index_1[3] = i;
      }
    }
  }

  switch (flag) {
  case 1:
    if ((day_lowest < hour_lowest) && (day_lowest < min_lowest)) {
      return_value = day_lowest;
      low_index_1[0] = low_index_1[3];
    } else if ((hour_lowest < min_lowest) && (hour_lowest < day_lowest)) {
      return_value = hour_lowest;
      low_index_1[0] = low_index_1[2];
    } else if ((min_lowest < day_lowest) && (min_lowest < hour_lowest))
      return_value = min_lowest;
    low_index_1[0] = low_index_1[1];
    break;

  case 2:
    return_value = min_lowest;
    low_index_1[0] = low_index_1[1];
    break;

  case 3:
    return_value = hour_lowest;
    low_index_1[0] = low_index_1[2];
    break;

  case 4:
    return_value = day_lowest;
    low_index_1[0] = low_index_1[3];
    break;
  }
  return return_value;
}

int get_lowest_power_2(int flag) {
  int i, min_lowest, hour_lowest, day_lowest, return_value = 0;
  for (i = 0; i < MAX_LAST_MIN_COUNT; i++) {
    if (i == 0)
      min_lowest = g_min_power_2[i].watt;
    else {
      if ((min_lowest > g_min_power_2[i].watt) &&
          (g_min_power_2[i].watt != 0)) {
        min_lowest = g_min_power_2[i].watt;
        low_index_2[1] = i;
      }
    }
  }
  for (i = 0; i < MAX_LAST_HOUR_COUNT; i++) {
    if (i == 0)
      hour_lowest = g_hour_power_2[i].watt;
    else {
      if ((hour_lowest > g_hour_power_2[i].watt) &&
          (g_hour_power_2[i].watt != 0)) {
        hour_lowest = g_hour_power_2[i].watt;
        low_index_2[2] = i;
      }
    }
  }
  for (i = 0; i < MAX_LAST_DAY_COUNT; i++) {
    if (i == 0)
      day_lowest = g_day_power_2[i].watt;
    else {
      if ((day_lowest > g_day_power_2[i].watt) &&
          (g_day_power_2[i].watt != 0)) {
        day_lowest = g_day_power_2[i].watt;
        low_index_2[3] = i;
      }
    }
  }
  switch (flag) {
  case 1:
    if ((day_lowest < hour_lowest) && (day_lowest < min_lowest)) {
      return_value = day_lowest;
      low_index_2[0] = low_index_2[3];
    } else if ((hour_lowest < min_lowest) && (hour_lowest < day_lowest)) {
      return_value = hour_lowest;
      low_index_2[0] = low_index_2[2];
    } else if ((min_lowest < day_lowest) && (min_lowest < hour_lowest))
      return_value = min_lowest;
    low_index_2[0] = low_index_2[1];
    break;

  case 2:
    return_value = min_lowest;
    low_index_2[0] = low_index_2[1];
    break;

  case 3:
    return_value = hour_lowest;
    low_index_2[0] = low_index_2[2];
    break;

  case 4:
    return_value = day_lowest;
    low_index_2[0] = low_index_2[3];
    break;
  }
  return return_value;
}
// [수정] json::value 이용으로 변환
void get_power_peak(json::value &response_json){
    int l = 0;
    int highest_peak[2], lowest_peak[2] = {0};
    highest_peak[0] = get_highest_power_1(1);
    lowest_peak[0] = get_lowest_power_1(1);
    highest_peak[1] = get_highest_power_2(1);
    lowest_peak[1] = get_lowest_power_2(1);
    

    json::value highest, lowest;
    
    string h_value = to_string(highest_peak[0] + highest_peak[1]);
    highest["VALUE"] = json::value::string(h_value);

    string h_date;
    if(high_index_1[0] == high_index_1[1])
      h_date = (const char*)g_min_power_1[high_index_1[0]].dt;
    else if(high_index_1[0] == high_index_1[2])
      h_date = (const char*)g_hour_power_1[high_index_1[0]].dt;
    else if(high_index_1[0] == high_index_1[3])
      h_date = (const char*)g_day_power_1[high_index_1[0]].dt;

    highest["DATETIME"] = json::value::string(h_date);

    response_json["HIGHEST_PEAK"] = highest;

    string l_value = to_string(lowest_peak[0] + lowest_peak[1]);
    lowest["VALUE"] = json::value::string(l_value);

    string l_date;
    if(low_index_1[0] == low_index_1[1])
      l_date = (const char*)g_min_power_1[low_index_1[0]].dt;
    else if(low_index_1[0] == low_index_1[2])
      l_date = (const char*)g_hour_power_1[low_index_1[0]].dt;
    else if(low_index_1[0] == low_index_1[3])
      l_date = (const char*)g_day_power_1[low_index_1[0]].dt;

    lowest["DATETIME"] = json::value::string(l_date);

    response_json["LOWEST_PEAK"] = lowest;

    response_json["MIN_TOTAL_PEAK"] = json::value::string(to_string(min_total_peak));
    response_json["HOUR_TOTAL_PEAK"] = json::value::string(to_string(hour_total_peak));

    return ;
}


void get_power_peak(char *ret) {
  char buf[100]; 
  int l = 0;
  int highest_peak[2], lowest_peak[2] = {0};
  highest_peak[0] = get_highest_power_1(1);
  lowest_peak[0] = get_lowest_power_1(1);
  highest_peak[1] = get_highest_power_2(1);
  lowest_peak[1] = get_lowest_power_2(1);
  
  strcpy(ret, "\t\"HIGHEST_PEAK\" : {\n");
  sprintf(buf, "\t\t \"VALUE\" : \"%d\",\n", highest_peak[0] + highest_peak[1]);

  strcat(ret, buf);
  if (high_index_1[0] == high_index_1[1])
    sprintf(buf, "\t\t \"DATETIME\" : \"%s\"\n",
            g_min_power_1[high_index_1[0]].dt);
  else if (high_index_1[0] == high_index_1[2])
    sprintf(buf, "\t\t \"DATETIME\" : \"%s\"\n",
            g_hour_power_1[high_index_1[0]].dt);
  else if (high_index_1[0] == high_index_1[3])
    sprintf(buf, "\t\t \"DATETIME\" : \"%s\"\n",
            g_day_power_1[high_index_1[0]].dt);

  strcat(ret, buf);
  sprintf(buf, "\t},\n");
  strcat(ret, buf);
  sprintf(buf, "\t \"LOWEST_PEAK\" :{\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t \"VALUE\" : \"%d\",\n", lowest_peak[0] + lowest_peak[1]);
  strcat(ret, buf);

  if (low_index_1[0] == low_index_1[1])
    sprintf(buf, "\t\t \"DATETIME\" : \"%s\"\n",
            g_min_power_1[low_index_1[0]].dt);
  else if (low_index_1[0] == low_index_1[2])
    sprintf(buf, "\t\t \"DATETIME\" : \"%s\"\n",
            g_hour_power_1[low_index_1[0]].dt);
  else if (low_index_1[0] == low_index_1[3])
    sprintf(buf, "\t\t \"DATETIME\" : \"%s\"\n",
            g_day_power_1[low_index_1[0]].dt);

  strcat(ret, buf);
  sprintf(buf, "\t},\n");
  strcat(ret, buf);
  // sprintf(buf, "\t \"MIN_TOTAL_PEAK\" : \"%d\",\n", 50);

  sprintf(buf, "\t \"MIN_TOTAL_PEAK\" : \"%d\",\n", min_total_peak);


  strcat(ret, buf);
  // sprintf(buf, "\t \"HOUR_TOTAL_PEAK\" : \"%d\"\n", 100);


  sprintf(buf, "\t \"HOUR_TOTAL_PEAK\" : \"%d\"\n", hour_total_peak);


  strcat(ret, buf);
}


// [수정] json::value 이용으로 변환
void get_power_consumption(json::value &response_json)
{
  int highest_peak[2], lowest_peak[2] = {0};

  highest_peak[0] = get_highest_power_1(2);
  highest_peak[1] = get_highest_power_2(2);
  lowest_peak[0] = get_lowest_power_1(2);
  lowest_peak[1] = get_lowest_power_2(2);

  json::value past_min, past_hour, past_day;

  json::value high_min, low_min;
  string min_val, min_date;
  min_val = to_string(highest_peak[0] + highest_peak[1]);
  min_date = (const char*)g_min_power_1[high_index_1[0]].dt;
  high_min["VALUE"] = json::value::string(min_val);
  high_min["DATETIME"] = json::value::string(min_date);
  past_min["HIGHEST"] = high_min;

  min_val = to_string(lowest_peak[0] + lowest_peak[1]);
  min_date = (const char*)g_min_power_1[low_index_1[0]].dt;
  low_min["VALUE"] = json::value::string(min_val);
  low_min["DATETIME"] = json::value::string(min_date);
  past_min["LOWEST"] = low_min;

  response_json["PAST_MIN"] = past_min;

  highest_peak[0] = get_highest_power_1(3);
  highest_peak[1] = get_highest_power_2(3);
  lowest_peak[1] = get_lowest_power_1(3);
  lowest_peak[1] = get_lowest_power_2(3);

  json::value high_hour, low_hour;
  string hour_val, hour_date;
  hour_val = to_string(highest_peak[0] + highest_peak[1]);
  hour_date = (const char*)g_hour_power_1[high_index_1[0]].dt;
  high_hour["VALUE"] = json::value::string(hour_val);
  high_hour["DATETIME"] = json::value::string(hour_date);
  past_hour["HIGHEST"] = high_hour;

  hour_val = to_string(lowest_peak[0] + lowest_peak[1]);
  hour_date = (const char*)g_hour_power_1[low_index_1[0]].dt;
  low_hour["VALUE"] = json::value::string(hour_val);
  low_hour["DATETIME"] = json::value::string(hour_date);
  past_hour["LOWEST"] = low_hour;

  response_json["PAST_HOUR"] = past_hour;

  highest_peak[0] = get_highest_power_1(4);
  highest_peak[1] = get_highest_power_2(4);
  lowest_peak[0] = get_lowest_power_1(4);
  lowest_peak[1] = get_lowest_power_2(4);

  json::value high_day, low_day;
  string day_val, day_date;
  day_val = to_string(highest_peak[0] + highest_peak[1]);
  day_date = (const char*)g_day_power_1[high_index_1[0]].dt;
  high_day["VALUE"] = json::value::string(day_val);
  high_day["DATETIME"] = json::value::string(day_date);
  past_day["HIGHEST"] = high_day;

  day_val = to_string(lowest_peak[0] + lowest_peak[1]);
  day_date = (const char*)g_day_power_1[low_index_1[0]].dt;
  low_day["VALUE"] = json::value::string(day_val);
  low_day["DATETIME"] = json::value::string(day_date);
  past_day["LOWEST"] = low_day;

  response_json["PAST_DAY"] = past_day;
  
  return ;

}

void get_power_consumption(char *ret) {

  char buf[100];
  int highest_peak[2], lowest_peak[2] = {0};

  highest_peak[0] = get_highest_power_1(2);
  highest_peak[1] = get_highest_power_2(2);
  lowest_peak[0] = get_lowest_power_1(2);
  lowest_peak[1] = get_lowest_power_2(2);

  strcpy(ret, "\t \"PAST_MIN\" : {\n");
  sprintf(buf, "\t\t \"HIGHEST\" : {\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t\t \"VALUE\" : \"%d\",\n",
          highest_peak[0] + highest_peak[1]);
  strcat(ret, buf);
  sprintf(buf, "\t\t\t \"DATETIME\" : \"%s\"\n",
          g_min_power_1[high_index_1[0]].dt);
  strcat(ret, buf);

  sprintf(buf, "\t\t},\n");
  strcat(ret, buf);
  sprintf(buf, "\t\t \"LOWEST\" : {\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t\t \"VALUE\" : \"%d\",\n", lowest_peak[0] + lowest_peak[1]);
  strcat(ret, buf);
  sprintf(buf, "\t\t\t \"DATETIME\" : \"%s\"\n",
          g_min_power_1[low_index_1[0]].dt);
  strcat(ret, buf);

  sprintf(buf, "\t\t}\n");
  strcat(ret, buf);
  sprintf(buf, "\t},\n");
  strcat(ret, buf);

  highest_peak[0] = get_highest_power_1(3);
  log(info) << "highest_peak[0]=" << highest_peak[0];
  highest_peak[1] = get_highest_power_2(3);
  log(info) << "highest_peak[1]=" << highest_peak[1];
  lowest_peak[1] = get_lowest_power_1(3);
  lowest_peak[1] = get_lowest_power_2(3);

  sprintf(buf, "\t \"PAST_HOUR\" : {\n");
  strcat(ret, buf);
  sprintf(buf, "\t\t \"HIGHEST\" : {\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t\t \"VALUE\" : \"%d\",\n",
          highest_peak[0] + highest_peak[1]);
  strcat(ret, buf);
  sprintf(buf, "\t\t\t \"DATETIME\" : \"%s\"\n",
          g_hour_power_1[high_index_1[0]].dt);
  strcat(ret, buf);
  sprintf(buf, "\t\t},\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t \"LOWEST\" : {\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t\t \"VALUE\" : \"%d\",\n", lowest_peak[0] + lowest_peak[1]);
  strcat(ret, buf);
  sprintf(buf, "\t\t\t \"DATETIME\" : \"%s\"\n",
          g_hour_power_1[low_index_1[0]].dt);
  strcat(ret, buf);

  sprintf(buf, "\t\t}\n");
  strcat(ret, buf);
  sprintf(buf, "\t},\n");
  strcat(ret, buf);

  highest_peak[0] = get_highest_power_1(4);
  highest_peak[1] = get_highest_power_2(4);
  lowest_peak[0] = get_lowest_power_1(4);
  lowest_peak[1] = get_lowest_power_2(4);

  sprintf(buf, "\t \"PAST_DAY\" : {\n");
  strcat(ret, buf);
  sprintf(buf, "\t\t \"HIGHEST\" : {\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t\t \"VALUE\" : \"%d\",\n",
          highest_peak[0] + highest_peak[1]);
  strcat(ret, buf);
  sprintf(buf, "\t\t\t \"DATETIME\" : \"%s\"\n",
          g_day_power_1[high_index_1[0]].dt);
  strcat(ret, buf);

  sprintf(buf, "\t\t},\n");
  strcat(ret, buf);
  sprintf(buf, "\t\t \"LOWEST\" : {\n");
  strcat(ret, buf);

  sprintf(buf, "\t\t\t \"VALUE\" : \"%d\",\n", lowest_peak[0] + lowest_peak[1]);
  strcat(ret, buf);
  sprintf(buf, "\t\t\t \"DATETIME\" : \"%s\"\n",
          g_day_power_1[low_index_1[0]].dt);
  strcat(ret, buf);

  sprintf(buf, "\t\t}\n");
  strcat(ret, buf);
  sprintf(buf, "\t}\n");
  strcat(ret, buf);
}

// [수정] json::value 이용으로 변환
void get_min_power_graph(json::value &response_json, int index)
{
    int l = 0;
    json::value inner;

    if(index == 0)
    {
        if(rm_count_1 == 0)
          inner = json::value::array();
        else
        {
            vector<json::value> v;
            for (l = 0; l < rm_count_1; l++)
            {
                json::value element;
                string ele_id, ele_watt, ele_date;
                ele_id = to_string(g_min_power_1[l].id);
                ele_watt = to_string(g_min_power_1[l].watt);
                if(g_min_power_1[l].id != 0)
                  ele_date = (const char*)g_min_power_1[l].dt;
                else
                  ele_date = "-";
                element["ID"] = json::value::string(ele_id);
                element["WATT"] = json::value::string(ele_watt);
                element["DATETIME"] = json::value::string(ele_date);

                v.push_back(element);
            }

            inner = json::value::array(v);
        }
    }
    else if(index == 1)
    {
        if(rm_count_2 == 0)
          inner = json::value::array();
        else
        {
            vector<json::value> v;
            for (l = 0; l < rm_count_2; l++)
            {
                json::value element;
                string ele_id, ele_watt, ele_date;
                ele_id = to_string(g_min_power_2[l].id);
                ele_watt = to_string(g_min_power_2[l].watt);
                if(g_min_power_2[l].id != 0)
                  ele_date = (const char*)g_min_power_2[l].dt;
                else
                  ele_date = "-";
                element["ID"] = json::value::string(ele_id);
                element["WATT"] = json::value::string(ele_watt);
                element["DATETIME"] = json::value::string(ele_date);

                v.push_back(element);
            }

            inner = json::value::array(v);
        }
    }

    string str = "LAST_MIN_GRAPH_";
    response_json[str.append(to_string(index+1))] = inner;
}


void get_min_power_graph(char *ret, int index) {

  char buf[100];
  int l = 0;

  sprintf(buf, "\t\t \"LAST_MIN_GRAPH_%d\" : [\n", index + 1);
  strcat(ret, buf);

  if (index == 0) {
    if (rm_count_1 == 0) {
      sprintf(buf, "\t\t]\n");
      strcat(ret, buf);
    } else {
      for (l = 0; l < rm_count_1; l++) {
        sprintf(buf, "\t\t\t {\n");
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"ID\" : \"%d\",\n", g_min_power_1[l].id);
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"WATT\" : \"%d\",\n", g_min_power_1[l].watt);
        strcat(ret, buf);
        if (g_min_power_1[l].id != 0)
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"%s\"\n", g_min_power_1[l].dt);
        else
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"-\"\n");
        strcat(ret, buf);
        if (l < rm_count_1 - 1)
          sprintf(buf, "\t\t\t },\n");
        else
          sprintf(buf, "\t\t\t }\n\t\t]\n");
        strcat(ret, buf);
      }
    }
  } else if (index == 1) {
    if (rm_count_2 == 0) {
      sprintf(buf, "\t\t]\n");
      strcat(ret, buf);
    } else {
      for (l = 0; l < rm_count_2; l++) {
        sprintf(buf, "\t\t\t {\n");
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"ID\" : \"%d\",\n", g_min_power_2[l].id);
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"WATT\" : \"%d\",\n", g_min_power_2[l].watt);
        strcat(ret, buf);
        if (g_min_power_2[l].id != 0)
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"%s\"\n", g_min_power_2[l].dt);
        else
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"-\"\n");
        strcat(ret, buf);
        if (l < rm_count_2 - 1)
          sprintf(buf, "\t\t\t },\n");
        else
          sprintf(buf, "\t\t\t }\n\t\t]\n");
        strcat(ret, buf);
      }
    }
  }
}


// [수정] json::value 이용으로 변환
void get_hour_power_graph(json::value &response_json, int index)
{
  int l = 0;
    json::value inner;

    if(index == 0)
    {
        if(rh_count_1 == 0)
          inner = json::value::array();
        else
        {
            vector<json::value> v;
            for (l = 0; l < fh_count_1; l++)
            {
                json::value element;
                string ele_id, ele_watt, ele_date;
                ele_id = to_string(g_hour_power_1[l].id);
                ele_watt = to_string(g_hour_power_1[l].watt);
                if(g_hour_power_1[l].id != 0)
                  ele_date = (const char*)g_hour_power_1[l].dt;
                else
                  ele_date = "-";
                element["ID"] = json::value::string(ele_id);
                element["WATT"] = json::value::string(ele_watt);
                element["DATETIME"] = json::value::string(ele_date);

                v.push_back(element);
            }

            inner = json::value::array(v);
        }
    }
    else if(index == 1)
    {
        if(rh_count_2 == 0)
          inner = json::value::array();
        else
        {
            vector<json::value> v;
            for (l = 0; l < fh_count_2; l++)
            {
                json::value element;
                string ele_id, ele_watt, ele_date;
                ele_id = to_string(g_hour_power_2[l].id);
                ele_watt = to_string(g_hour_power_2[l].watt);
                if(g_hour_power_2[l].id != 0)
                  ele_date = (const char*)g_hour_power_2[l].dt;
                else
                  ele_date = "-";
                element["ID"] = json::value::string(ele_id);
                element["WATT"] = json::value::string(ele_watt);
                element["DATETIME"] = json::value::string(ele_date);

                v.push_back(element);
            }

            inner = json::value::array(v);
        }
    }

    string str = "LAST_HOUR_GRAPH_";
    response_json[str.append(to_string(index+1))] = inner;

}


void get_hour_power_graph(char *ret, int index) {

  char buf[100];
  int l = 0;

  sprintf(buf, "\t\t \"LAST_HOUR_GRAPH_%d\" : [\n", index + 1);
  strcat(ret, buf);

  if (index == 0) {
    if (rh_count_1 == 0) {
      sprintf(buf, "\t\t]\n");
      strcat(ret, buf);
    } else {
      for (l = 0; l < fh_count_1; l++) {
        sprintf(buf, "\t\t\t {\n");
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"ID\" : \"%d\",\n", g_hour_power_1[l].id);
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"WATT\" : \"%d\",\n", g_hour_power_1[l].watt);
        strcat(ret, buf);
        if (g_hour_power_1[l].id != 0)
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"%s\"\n", g_hour_power_1[l].dt);
        else
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"-\"\n");
        strcat(ret, buf);
        if (l < fh_count_1 - 1)
          sprintf(buf, "\t\t\t },\n");
        else
          sprintf(buf, "\t\t\t }\n\t\t]\n");
        strcat(ret, buf);
      }
    }
  } else if (index == 1) {
    if (rh_count_2 == 0) {
      sprintf(buf, "\t\t]\n");
      strcat(ret, buf);
    } else {
      for (l = 0; l < fh_count_2; l++) {
        sprintf(buf, "\t\t\t {\n");
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"ID\" : \"%d\",\n", g_hour_power_2[l].id);
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"WATT\" : \"%d\",\n", g_hour_power_2[l].watt);
        strcat(ret, buf);
        if (g_hour_power_2[l].id != 0)
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"%s\"\n", g_hour_power_2[l].dt);
        else
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"-\"\n");
        strcat(ret, buf);
        if (l < fh_count_2 - 1)
          sprintf(buf, "\t\t\t },\n");
        else
          sprintf(buf, "\t\t\t }\n\t\t]\n");
        strcat(ret, buf);
      }
    }
  }
}

// [수정] json::value 이용으로 변환
void get_day_power_graph(json::value &response_json, int index)
{
    int l = 0;
    json::value inner;

    if(index == 0)
    {
        if(rd_count_1 == 0)
          inner = json::value::array();
        else
        {
            vector<json::value> v;
            for (l = 0; l < fd_count_1; l++)
            {
                json::value element;
                string ele_id, ele_watt, ele_date;
                ele_id = to_string(g_day_power_1[l].id);
                ele_watt = to_string(g_day_power_1[l].watt);
                if(g_day_power_1[l].id != 0)
                  ele_date = (const char*)g_day_power_1[l].dt;
                else
                  ele_date = "-";
                element["ID"] = json::value::string(ele_id);
                element["WATT"] = json::value::string(ele_watt);
                element["DATETIME"] = json::value::string(ele_date);

                v.push_back(element);
            }

            inner = json::value::array(v);
        }
    }
    else if(index == 1)
    {
        if(rd_count_2 == 0)
          inner = json::value::array();
        else
        {
            vector<json::value> v;
            for (l = 0; l < fd_count_2; l++)
            {
                json::value element;
                string ele_id, ele_watt, ele_date;
                ele_id = to_string(g_day_power_2[l].id);
                ele_watt = to_string(g_day_power_2[l].watt);
                if(g_day_power_2[l].id != 0)
                  ele_date = (const char*)g_day_power_2[l].dt;
                else
                  ele_date = "-";
                element["ID"] = json::value::string(ele_id);
                element["WATT"] = json::value::string(ele_watt);
                element["DATETIME"] = json::value::string(ele_date);

                v.push_back(element);
            }

            inner = json::value::array(v);
        }
    }

    string str = "LAST_DAY_GRAPH_";
    response_json[str.append(to_string(index+1))] = inner;

}


void get_day_power_graph(char *ret, int index) {

  char buf[100];
  int l = 0;
  sprintf(buf, "\t\t \"LAST_DAY_GRAPH_%d\" : [\n", index + 1);
  strcat(ret, buf);
  if (index == 0) {
    if (rd_count_1 == 0) {
      sprintf(buf, "\t\t]\n");
      strcat(ret, buf);
    } else {
      for (l = 0; l < fd_count_1; l++) {
        sprintf(buf, "\t\t\t {\n");
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"ID\" : \"%d\",\n", g_day_power_1[l].id);
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"WATT\" : \"%d\",\n", g_day_power_1[l].watt);
        strcat(ret, buf);
        if (g_day_power_1[l].id != 0)
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"%s\"\n", g_day_power_1[l].dt);
        else
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"-\"\n");
        strcat(ret, buf);
        if (l < fd_count_1 - 1)
          sprintf(buf, "\t\t\t },\n");
        else
          sprintf(buf, "\t\t\t }\n\t\t]\n");
        strcat(ret, buf);
      }
    }
  } else if (index == 1) {
    if (rd_count_2 == 0) {
      // sprintf(buf, "\t\t]\n");
      // strcat(ret, buf);
    } else {
      for (l = 0; l < fd_count_2; l++) {
        sprintf(buf, "\t\t\t {\n");
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"ID\" : \"%d\",\n", g_day_power_2[l].id);
        strcat(ret, buf);
        sprintf(buf, "\t\t\t\t\"WATT\" : \"%d\",\n", g_day_power_2[l].watt);
        strcat(ret, buf);
        if (g_day_power_2[l].id != 0)
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"%s\"\n", g_day_power_2[l].dt);
        else
          sprintf(buf, "\t\t\t\t\"DATETIME\" : \"-\"\n");
        strcat(ret, buf);
        if (l < fd_count_2 - 1)
          sprintf(buf, "\t\t\t },\n");
        else
          sprintf(buf, "\t\t\t }\n");
        // sprintf(buf, "\t\t\t }\n\t\t]\n");
        strcat(ret, buf);
      }
    }
    sprintf(buf, "\t\t]\n");
    strcat(ret, buf);
  }
}



// [수정] json::value 이용으로 변환
void get_power_response(int menu, json::value &response_json)
{
    cout << "get_power_response Menu = " << menu << endl;
    switch(menu)
    {
        case WATT_ALL: {
            json::value tmp;

            get_power_peak(tmp);
            get_power_consumption(tmp);
            get_min_power_graph(tmp, 0);
            get_min_power_graph(tmp, 1);
            get_hour_power_graph(tmp, 0);
            get_hour_power_graph(tmp, 1);
            get_day_power_graph(tmp, 0);
            get_day_power_graph(tmp, 1);

            response_json["POWER_USAGE"] = tmp;
            break;
        }
        case WATT_TOP: {
            get_power_peak(response_json);
            break;
        }
        case WATT_MIDDLE: {
            get_power_consumption(response_json);
            break;
        }
        case WATT_BOTTOM_MIN: {
            get_min_power_graph(response_json, 0);
            get_min_power_graph(response_json, 1);
            break;
        }
        case WATT_BOTTOM_HOUR: {
            get_hour_power_graph(response_json, 0);
            get_hour_power_graph(response_json, 1);
            break;
        }
        case WATT_BOTTOM_DAY: {
            get_day_power_graph(response_json, 0);
            get_day_power_graph(response_json, 1);
            break;
        }
    }
}


/**
 * @brief Get the power response object
 * @bug menu 값이 48이들어와서 수행하지않는문제 발생 따라서 강제 0으로 변경
 * @param menu
 * @param ret
 */
void get_power_response(int menu, char *ret) {
  unsigned char buf[30000];
  cout << "get_power_response Menu = " << menu << endl;
  switch (menu) {
  case WATT_ALL:
    strcpy(ret, "{\n\t\"POWER_USAGE\" : {\n");
    get_power_peak(buf);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");

    get_power_consumption(buf);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");

    get_min_power_graph(buf, 0);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");

    get_min_power_graph(buf, 1);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");

    get_hour_power_graph(buf, 0);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");

    get_hour_power_graph(buf, 1);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");

    get_day_power_graph(buf, 0);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");

    get_day_power_graph(buf, 1);
    strcat(ret, buf);
    strcat(ret, "\t\n");
    strcat(ret, "\t}\n}\n");

    break;
  case WATT_TOP:
    strcpy(ret, "{\n");
    get_power_peak(buf);
    strcat(ret, buf);
    strcat(ret, "}\n");
    break;
  case WATT_MIDDLE:
    strcpy(ret, "{\n");
    get_power_consumption(buf);
    strcat(ret, buf);
    strcat(ret, "}\n");
    break;

  case WATT_BOTTOM_MIN:
    strcpy(ret, "{\n");
    get_min_power_graph(buf, 0);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");
    get_min_power_graph(buf, 1);
    strcat(ret, buf);
    strcat(ret, "}\n");
    break;

  case WATT_BOTTOM_HOUR:
    strcpy(ret, "{\n");
    get_hour_power_graph(buf, 0);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");
    get_hour_power_graph(buf, 1);
    strcat(ret, buf);
    strcat(ret, "}\n");
    break;

  case WATT_BOTTOM_DAY:
    strcpy(ret, "{\n");
    get_day_power_graph(buf, 0);
    strcat(ret, buf);
    strcat(ret, "\t,\n");
    strcpy(buf, "\0");
    get_day_power_graph(buf, 1);
    strcat(ret, buf);
    strcat(ret, "}\n");
    break;
  }
}