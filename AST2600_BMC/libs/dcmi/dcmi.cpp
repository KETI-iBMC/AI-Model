#include "ipmi/dcmi.hpp"
#include <ipmi/sel.hpp>
extern std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;
extern uint8_t uuid_hex[16];
extern uint8_t uuid_str[37];
extern uint16_t dcmi_power_sample;
static pthread_mutex_t m_dcmi;
Dcmiconfiguration dcmiConfiguration;

Dcmiconfiguration::Dcmiconfiguration() {
  this->dcmi_guid.insert(dcmi_guid.begin(), std::begin(uuid_hex),
                         std::end(uuid_hex));
  this->dcmi_mngctrl.insert(dcmi_mngctrl.begin(), std::begin(uuid_str),
                            std::end(uuid_str));
  this->power_reading_state = 0x40;
}

void plat_dcmi_init() {
  dcmiConfiguration.plat_dcmi_capa_init();
  dcmiConfiguration.plat_dcmi_power_lmt_init();
}
/**
* @brief DCMI 기본 Capabilities 정보 초기화 함수.\n
DCMI_CAPA_PATH 파일이 존재할 경우 dcmi_capabilities_t 구조체에 파일 내용을
복사한다.\n
* @param void
* @return void
*/

void Dcmiconfiguration::plat_dcmi_power_lmt_init() {
  cout << "Initializing DCMI Power Limitation" << endl;
  this->dcmi_power_lmt.push_back(0xdc); // grp_id
  this->dcmi_power_lmt.push_back(0x00); // reserved_1
  this->dcmi_power_lmt.push_back(0x00); // reserved_1
  this->dcmi_power_lmt.push_back(0x00); // action

  this->dcmi_power_lmt.push_back(0x00); // limit[0]
  this->dcmi_power_lmt.push_back(0x00); // limit[1]
  this->dcmi_power_lmt.push_back(0x00); // correction[0]
  this->dcmi_power_lmt.push_back(0x00); // correction[1]
  this->dcmi_power_lmt.push_back(0x00); // correction[2]
  this->dcmi_power_lmt.push_back(0x00); // correction[3]
  this->dcmi_power_lmt.push_back(0x00); // reserved_2[0]
  this->dcmi_power_lmt.push_back(0x00); // reserved_2[1]
}

void Dcmiconfiguration::plat_dcmi_capa_init() {
  this->dcmi_capa.push_back(0xdc); // Group id
  this->dcmi_capa.push_back(0x1);  // Conformance
  this->dcmi_capa.push_back(0x5);  // Conformance
  this->dcmi_capa.push_back(0x2);  // Revision
  this->dcmi_capa.push_back(0x0);  // Data 1
  this->dcmi_capa.push_back(0x1);  // Data 2
  this->dcmi_capa.push_back(0x1);  // Data 3

  this->dcmi_mandatory_capa.push_back(0xdc); // Group id
  this->dcmi_mandatory_capa.push_back(0x1);  // Conformance
  this->dcmi_mandatory_capa.push_back(0x5);  // Conformance
  this->dcmi_mandatory_capa.push_back(0x2);  // Revision
  this->dcmi_mandatory_capa.push_back(0x0);  // Data 1
  this->dcmi_mandatory_capa.push_back(0xa4); // Data 2
  this->dcmi_mandatory_capa.push_back(0x0);  // Data 3
  this->dcmi_mandatory_capa.push_back(0x0);  // Data 4
  this->dcmi_mandatory_capa.push_back(0x1);  // Data 5

  this->dcmi_optional_capa.push_back(0xdc);
  this->dcmi_optional_capa.push_back(0x1);
  this->dcmi_optional_capa.push_back(0x5);
  this->dcmi_optional_capa.push_back(0x2);
  this->dcmi_optional_capa.push_back(0x20);
  this->dcmi_optional_capa.push_back(0xff);

  this->dcmi_mgnt_access_capa.push_back(0xdc);
  this->dcmi_mgnt_access_capa.push_back(0x1);
  this->dcmi_mgnt_access_capa.push_back(0x5);
  this->dcmi_mgnt_access_capa.push_back(0x2);
  this->dcmi_mgnt_access_capa.push_back(0x1);
  this->dcmi_mgnt_access_capa.push_back(0xff);
  this->dcmi_mgnt_access_capa.push_back(0x2);
}
/**
* @brief DCMI Discover 및 Compact 명령어 전송 시 DCMI 전체 설정 정보를 반환하는
함수.\n IPMI Request 데이터에 의해 전송된 Selector에 따라 Capabilites 정보를
반환한다.\n 호출 CMD : CMD_DCMI_DISCOVER (0x0), CMD_DCMI_COMPACT (0x1)\n
* @param unsigned char *request - IPMI Request message pointer\n
* @param unsigned char *response - IPMI Response message pointer\n
* @param unsigned char *res_len - IPMI Response message length\n
* @return void
*/
void Dcmiconfiguration::dcmi_discover(uint8_t *request, uint8_t *response,
                                      uint8_t *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];
  uint8_t grp_extension = req->data[0];
  uint8_t selector = req->data[1];
  cout << "grp_extension=" << std::hex << (int)grp_extension << endl;

  if (grp_extension != 0xdc && grp_extension != 0x0) {
    res->cc = CC_INVALID_CMD;
    cout << "1.dcmi_discover CC_INVALID_CMD" << endl;
  } else if (grp_extension == 0xdc || grp_extension == 0x0) {
    cout << "2.dcmi_discover CC_SUCCESS" << endl;
    res->cc = CC_SUCCESS;
    switch (selector) {
    case 0x1:
      if (grp_extension == 0x0)
        *data++ = grp_extension;
      else
        *data++ = this->dcmi_mandatory_capa[0];

      for (int i = 1; i < this->dcmi_capa.size(); i++)
        *data++ = this->dcmi_capa[i];
      break;

    case 0x2:
      if (grp_extension == 0x0)
        *data++ = grp_extension;
      else
        *data++ = this->dcmi_mandatory_capa[0];

      for (int i = 1; i < this->dcmi_mandatory_capa.size(); i++)
        *data++ = this->dcmi_mandatory_capa[i];
      break;

    case 0x3:
      if (grp_extension == 0x0)
        *data++ = grp_extension;
      else
        *data++ = this->dcmi_mandatory_capa[0];

      for (int i = 1; i < this->dcmi_optional_capa.size(); i++)
        *data++ = this->dcmi_optional_capa[i];
      break;

    case 0x4:
      if (grp_extension == 0x0)
        *data++ = grp_extension;
      else
        *data++ = this->dcmi_mandatory_capa[0];

      for (int i = 1; i < this->dcmi_mgnt_access_capa.size(); i++)
        *data++ = this->dcmi_mgnt_access_capa[i];
      break;

    default:
      break;
    }
  }

  if (res->cc == CC_SUCCESS)
    *res_len = data - &res->data[0];
}

void Dcmiconfiguration::dcmi_set_power_limit(uint8_t *request,
                                             uint8_t *response,
                                             uint8_t *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  uint16_t limit_val = req->data[6] | (req->data[7] << 8);
  uint32_t correction_val = (req->data[11] << 24) | (req->data[10] << 16) |
                            (req->data[9] << 8) | req->data[8];
  uint16_t sample_time = (req->data[15] << 8) | req->data[14];
  char buf[500] = {
      0,
  };
  sprintf(buf, "DCMI Set Power Limit : %d ", limit_val);
  // ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf, "Power",
  // "System"));
  if (limit_val > 0xffff)
    res->cc = CC_POWER_OOL;
  else if (correction_val > 0xffffffff)
    res->cc = CC_POWER_CORRECTION;
  else if (sample_time > 0xffff)
    res->cc = CC_POWER_SAMPLE_TIME;
  else {
    res->cc = CC_SUCCESS;
    res->data[0] = 0xdc;

    dcmi_power_lmt.assign(req->data, req->data + 15);
    *res_len = 1;
  }
}
/**
* @brief DCMI 전체 정보 저장 함수. DCMI 관련 정보를 모두 파일에 분리하여
저장한다.\n (1)	DCMI_CAPA_PATH - dcmi_capable_vals 정보 저장\n (2)
DCMI_MANDATORY_CAPA_PATH - dcmi_mandatory_plat_capa 정보 저장\n (3)
DCMI_OPTION_CAPA_PATH - dcmi_optional_plat_capa 정보 저장\n (4)
DCMI_MGNT_CAPA_PATH - dcmi_management_access_capa정보 저장\n (5)
DCMI_POWER_LIMIT_PATH - dcmi_power_lmt 정보 저장\n (6)
DCMI_ASSET_MNGCTRL_PATH - dcmi_asset_mngctrl 정보 저장\n
* @param void
* @return void
*/
void Dcmiconfiguration::plat_dcmi_data_save() { cout << "미구현" << endl; }
void Dcmiconfiguration::dcmi_get_power_reading(uint8_t *request,
                                               uint8_t *response,
                                               uint8_t *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t *data = &res->data[0];
  uint8_t grp_extension = req->data[0];
  uint8_t selector = req->data[1];

  sel_msg_t entry;
  int ret;
  //#if IPMI_EVENT_LOG_VIEW
  ret = plat_sel_time_get(&entry);
  if (ret) {
    res->cc = CC_UNSPECIFIED_ERROR;
    cout << "dcmi_get_power_reading error" << endl;
    return;
  }
  //#endif

  if (grp_extension != 0xdc && grp_extension != 0x0)
    res->cc = CC_INVALID_CMD;
  else if (grp_extension == 0xdc || grp_extension == 0x0) {
    res->cc = CC_SUCCESS;
  }
  int power_1_watt = 0;
  int power_2_watt = 0;
  int max_power = 0;
  // uint8_t s_num = (int)th_data[6];
  uint8_t sdr_idx = 0;
  // sdr_idx = plat_find_sdr_index(NVA_SENSOR_PSU1_WATT);
  // power_1_watt = (sdr_rec[sdr_idx].find(sdr_idx)->second.sdr_sensor_read()) *
  // 10;
  power_1_watt = 155;
  // sdr_idx = plat_find_sdr_index(NVA_SENSOR_PSU2_WATT);
  // power_2_watt = (sdr_rec[sdr_idx].find(sdr_idx)->second.sdr_sensor_read()) *
  // 10;
  power_2_watt = 155;

  max_power = power_1_watt + power_2_watt;

  *data++ = this->dcmi_capa[0];
  *data++ = max_power & 0xff;
  *data++ = (max_power >> 8) & 0xff;
  *data++ = power_1_watt & 0xff;
  *data++ = (power_1_watt >> 8) & 0xff;
  *data++ = power_1_watt;
  *data++ = (power_1_watt >> 8) & 0xff;
  *data++ = power_1_watt;
  *data++ = (power_1_watt >> 8) & 0xff; // Minimum Power reading MSB
  memcpy(data, entry.msg, 4);
  *data += 4;
  struct sysinfo info;
  uint32_t count = info.uptime;
  *data++ = (count >> 24) && 0xff;
  *data++ = (count >> 16) & 0xff;
  *data++ = (count >> 8) & 0xff;
  *data++ = count & 0xff;
  *data++ = power_reading_state; //임시 power_reading_state
  res->cc = CC_SUCCESS;
  *res_len = data - &res->data[0];
}
void Dcmiconfiguration::dcmi_power_get_limit(uint8_t *request,
                                             uint8_t *response,
                                             uint8_t *res_len) {

  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  unsigned char *data = &res->data[0];

  if ((power_reading_state >> 6) == 1) {

    std::copy(dcmi_power_lmt.begin(), dcmi_power_lmt.end(), data);
    data += (dcmi_power_lmt).size();
    res->cc = CC_SUCCESS;
  } else {
    res->cc = CC_INVALID_PARAM;
  }
  *res_len = data - &res->data[0];
}
void Dcmiconfiguration::dcmi_get_mngctrl(uint8_t *request, uint8_t *response,
                                         uint8_t *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  res->cc = CC_SUCCESS;
  res->data[0] = 0xDC; // DCMI Extension

  if (req->data[0] == 0 && req->data[2] == 1) {
    res->data[1] = dcmi_mngctrl.size();
    *res_len += 2;
  } else {
    res->data[1] = req->data[2];
    std::copy(dcmi_mngctrl.begin() + req->data[1],
              dcmi_mngctrl.begin() + req->data[1] + req->data[2], res->data);
    *res_len = 2 + req->data[2];
  }
}
void Dcmiconfiguration::dcmi_get_disvry(unsigned char *request,
                                        unsigned char *response,
                                        unsigned char *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];

  // Fill response with default values
  res->cc = CC_SUCCESS;
  *data++ = 0xDC; // 0x51;					// IPMI_DCMI
  if (res->cc == CC_SUCCESS) {
    *res_len = data - &res->data[0];
  }
}
void Dcmiconfiguration::dcmi_power_activate(unsigned char *request,
                                            unsigned char *response,
                                            unsigned char *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  unsigned char *data = &res->data[0];

  power_reading_state = req->data[1] << 6;

  res->cc = CC_SUCCESS;

  *data++ = 0xdc;

  *res_len = data - &res->data[0];
}

void ipmi_handle_dcmi(uint8_t *request, uint8_t *response, uint8_t *res_len) {
  cout << "ipmi_handle_dcmi" << endl;
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  uint8_t cmd = req->cmd;
  cout << "dcmi cmd =" << (int)req->cmd << endl;

  switch (cmd) {
  case CMD_DCMI_DISCOVER:
    dcmiConfiguration.dcmi_discover(request, response, res_len);
    break;
  case CMD_DCMI_COMPACT:
    dcmiConfiguration.dcmi_discover(request, response, res_len);
    break;
  case CMD_DCMI_POWER_READING:
    // dcmiConfiguration.dcmi_power_reading(request, response, res_len);
    dcmiConfiguration.dcmi_get_power_reading(request, response, res_len);
    break;
  case CMD_DCMI_POWER_GET_LIMIT:
    dcmiConfiguration.dcmi_power_get_limit(request, response, res_len);
    break;
  case CMD_DCMI_POWER_SET_LIMIT:
    dcmiConfiguration.dcmi_set_power_limit(request, response, res_len);
    dcmiConfiguration.plat_dcmi_data_save();
    break;
  case CMD_DCMI_POWER_ACT:
    dcmiConfiguration.dcmi_power_activate(request, response, res_len);
    break;
  case CMD_DCMI_GETASSET:
    /* ipmi_dcmi_getassettag */
    break;
  case CMD_DCMI_GETSNSR: // IPMI_DCMI_GETSNSR:
    dcmiConfiguration.dcmi_get_disvry(request, response, res_len);
    break;
  case CMD_DCMI_SETASSET:

    break;
  case CMD_DCMI_GETTEMPRED:
    cout << "INFO" << endl;
    cout << "INFO" << endl;
    break;
  default:
    res->cc = CC_INVALID_CMD;
    break;
  }
}