#pragma once
#define SOL_CONFIG_PATH "/conf/ipmi/sol_config.bin"
#include <ipmi/ipmi.hpp>
#include <unordered_map>
enum {
  SOL_PARAMETER_SET_IN_PROGRESS = 0x00,
  SOL_PARAMETER_SOL_ENABLE = 0x01,
  SOL_PARAMETER_SOL_AUTHENTICATION = 0x02,
  SOL_PARAMETER_CHARACTER_INTERVAL = 0x03,
  SOL_PARAMETER_SOL_RETRY = 0x04,
  SOL_PARAMETER_SOL_NON_VOLATILE_BIT_RATE = 0x05,
  SOL_PARAMETER_SOL_VOLATILE_BIT_RATE = 0x06,
  SOL_PARAMETER_SOL_PAYLOAD_CHANNEL = 0x07,
  SOL_PARAMETER_SOL_PAYLOAD_PORT = 0x08,
};
struct sol_struct_t {
  int sockfd;
  struct sockaddr_in sol_cliaddr;
};

/**
 * @brief 피닉스 싱글톤구조의 SOL
 *
 */

class SerialOverLan {
private:
  struct sol_config_parameter {
    uint8_t set_in_progress;
    uint8_t enabled;
    uint8_t force_auth_encrypt_priv;
    uint8_t character_accumulate_level;
    uint8_t character_send_threshold;
    uint8_t retry_count;
    uint8_t retry_interval;
    uint8_t non_volatile_bit_rate;
    uint8_t volatile_bit_rate;
    uint8_t payload_channel;
    uint16_t payload_port[2];
  };
  sol_struct_t sol_struct;
  unordered_map<string, int8_t> ipmi_sol_param_str;
  sol_config_parameter g_sol_config;
  SerialOverLan() {
    log(info) << ("========================Serial Over Lan configuration "
                  "initializing");
    if (access(SOL_CONFIG_PATH, F_OK) != 0) {
      g_sol_config.set_in_progress = 0;
      g_sol_config.enabled = 1; //시작시 기본적으로 enable 하게설정해놈
      g_sol_config.force_auth_encrypt_priv = 0x04;
      g_sol_config.payload_port[0] = 0x6F;
      g_sol_config.payload_port[1] = 0x02;
      g_sol_config.payload_channel = 1;
    } else {
      FILE *s_config = fopen(SOL_CONFIG_PATH, "r");
      fread(&g_sol_config, sizeof(sol_config_parameter), 1, s_config);
      fclose(s_config);
    }
    ipmi_sol_param_str["SOL_PARAMETER_SET_IN_PROGRESS"] = 0x00;
    ipmi_sol_param_str["SOL_PARAMETER_SOL_ENABLE"] = 0x01;
    ipmi_sol_param_str["SOL_PARAMETER_SOL_AUTHENTICATION"] = 0x02;
    ipmi_sol_param_str["SOL_PARAMETER_CHARACTER_INTERVAL"] = 0x03;
    ipmi_sol_param_str["SOL_PARAMETER_SOL_RETRY"] = 0x04;
    ipmi_sol_param_str["SOL_PARAMETER_SOL_NON_VOLATILE_BIT_RATE"] = 0x05;
    ipmi_sol_param_str["SOL_PARAMETER_SOL_VOLATILE_BIT_RATE"] = 0x06;
    ipmi_sol_param_str["SOL_PARAMETER_SOL_PAYLOAD_CHANNEL"] = 0x07;
    ipmi_sol_param_str["SOL_PARAMETER_SOL_PAYLOAD_PORT"] = 0x08;
    DEVNAME = "/dev/ttyS4";
  };
  SerialOverLan(SerialOverLan &other);
  ~SerialOverLan() { destroyed = true; };
  static SerialOverLan *phoenixInstance;

  static void Create() {
    static SerialOverLan m_instance;
    phoenixInstance = &m_instance;
  }
  static void KillPhoenix() { phoenixInstance->~SerialOverLan(); };
  static bool destroyed;

public:
  char sol_buf[255];
  string DEVNAME;
  /**
   * @brief Serial Over Lan 활성화 플래그
   *
   */
  bool sol_activate;
  /**
   * @brief Serial Over Lan 설정 플래그
   *
   */
  unsigned char sol_flag;
  int sol_rdcnt;
  /**
   * @brief Serial Over Lan 문자열
   *
   */
  unsigned char sol_write_buf[255];
  static SerialOverLan &Instance() {
    if (destroyed) {
      new (phoenixInstance) SerialOverLan;
      // Init_ipmi_sol_param_str();
      atexit(KillPhoenix);
      destroyed = false;
    } else if (phoenixInstance == nullptr) {
      Create();
    }
    return *phoenixInstance;
  }
  void test() { cout << "test" << endl; }
  void transport_get_sol_config_params(ipmi_req_t *request,
                                       ipmi_res_t *response, uint8_t *res_len);
  void transport_set_sol_config_params(ipmi_req_t *request,
                                       ipmi_res_t *response, uint8_t *res_len);
  void plat_sol_config_save(void);
  sol_struct_t *get_sol_struct();
  void set_sol_struct(sol_struct_t *temp);
  void app_active_payload(ipmi_req_t *request, ipmi_res_t *response,
                          uint8_t *res_len);
  void app_deactive_payload(ipmi_req_t *request, ipmi_res_t *response,
                            uint8_t *res_len);
};