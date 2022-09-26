#include "ipmi/session.hpp"
#include <ipmi/sol.hpp>
#include <new>

bool SerialOverLan::destroyed = false;
SerialOverLan *SerialOverLan::phoenixInstance = NULL;

void SerialOverLan::plat_sol_config_save(void) {
  log(info) << ("Saving Serial Over Lan configuration parameter");
  FILE *s_config = fopen(SOL_CONFIG_PATH, "w");

  fwrite(&g_sol_config, sizeof(sol_config_parameter), 1, s_config);

  fclose(s_config);
}
extern Ipmisession ipmiSession[5];

void SerialOverLan::app_deactive_payload(ipmi_req_t *request,
                                         ipmi_res_t *response,
                                         uint8_t *res_len) {
  ipmi_res_t *res = (ipmi_res_t *)response;
  ipmi_req_t *req = (ipmi_req_t *)request;
  int loop = 0;
  if (sol_activate == 0) {
    log(info) << ("SOL Already Deactivated");
    res->cc = 0x80;
  } else {
    log(info) << ("Set SOL Deactivate");
    sol_activate = 0;
    res->cc = 0x00;
    for (loop = 0; loop < 5; loop++) {
      if (ipmiSession[loop].getSessionHandle() == 1) {
        log(info) << ("SOL Deactivate Session ID : %d", loop);
        ipmiSession[loop].Set_sol_activator(0);
        ipmiSession[loop].SetSessionHandle(0);
        //초기화까지는 잘모르겠음. 확인중 ..
        memset(&ipmiSession[loop], 0, sizeof(ipmiSession[loop]));
      }

      ipmiSession[loop].setActiveSessionCount(
          ipmiSession[loop].getActiveSessCount() - 1);
    }
  }
}

void SerialOverLan::app_active_payload(ipmi_req_t *request,
                                       ipmi_res_t *response, uint8_t *res_len) {

  log(info) << "Activate Serial over Lan";
  ipmi_res_t *res = (ipmi_res_t *)response;
  ipmi_req_t *req = (ipmi_req_t *)request;
  unsigned char *data = &res->data[0];
  unsigned char ll = 0;

  if (sol_activate == 0) {
    if (g_sol_config.enabled == 1) {
      res->cc = CC_SUCCESS;
      for (ll = 0; ll < 5; ll++) {
        if (ipmiSession[ll].getSessionHandle() != 0) {
          ipmiSession[ll].Set_sol_activator(1);
        }
      }
      *data++ = 0x00;
      *data++ = 0x00;
      *data++ = 0x00;
      *data++ = 0x00;
      *data++ = 0x2E;
      *data++ = 0x00;
      *data++ = 0xFF;
      *data++ = 0x00;
      *data++ = g_sol_config.payload_port[0];
      *data++ = g_sol_config.payload_port[1];
      *data++ = 0xFF; // VLAN NUMBER [1] FF (DON'T USE)
      *data++ = 0xFF; // VLAN NUMBER [2] FF (DON'T USE)
      sol_activate = 1;
      sol_flag = 1;
      *res_len = data - &res->data[0];

    } else {
      res->cc = 0x81;
    }

  } else {
    if (sol_activate == 1)
      res->cc = 0x80;
  }

  return;
}
void SerialOverLan::transport_get_sol_config_params(ipmi_req_t *request,
                                                    ipmi_res_t *response,
                                                    uint8_t *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];
  unsigned char param = req->data[1];

  res->cc = CC_SUCCESS;
  *data++ = 0x01;

  switch (param) {
  case SOL_PARAMETER_SET_IN_PROGRESS:
    *data++ = g_sol_config.set_in_progress;
    break;
  case SOL_PARAMETER_SOL_ENABLE:
    *data++ = g_sol_config.enabled;
    break;
  case SOL_PARAMETER_SOL_AUTHENTICATION:
    *data++ = g_sol_config.force_auth_encrypt_priv;
    break;
  case SOL_PARAMETER_CHARACTER_INTERVAL:
    *data++ = g_sol_config.character_accumulate_level;
    *data++ = g_sol_config.character_send_threshold;
    break;
  case SOL_PARAMETER_SOL_RETRY:
    *data++ = g_sol_config.retry_count;
    *data++ = g_sol_config.retry_interval;
    break;
  case SOL_PARAMETER_SOL_NON_VOLATILE_BIT_RATE:
    *data++ = g_sol_config.non_volatile_bit_rate;
    break;
  case SOL_PARAMETER_SOL_VOLATILE_BIT_RATE:
    *data++ = g_sol_config.volatile_bit_rate;
    break;
  case SOL_PARAMETER_SOL_PAYLOAD_CHANNEL:
    *data++ = g_sol_config.payload_channel;
    break;
  case SOL_PARAMETER_SOL_PAYLOAD_PORT:
    *data++ = g_sol_config.payload_port[0];
    *data++ = g_sol_config.payload_port[1];
    break;
  }
  if (res->cc == CC_SUCCESS) {
    *res_len = data - &res->data[0];
  }
}

void SerialOverLan::transport_set_sol_config_params(ipmi_req_t *request,
                                                    ipmi_res_t *response,
                                                    uint8_t *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];
  unsigned char param = req->data[1];

  res->cc = CC_SUCCESS;

  switch (param) {
  case SOL_PARAMETER_SET_IN_PROGRESS:
    g_sol_config.set_in_progress = req->data[2];
    break;
  case SOL_PARAMETER_SOL_ENABLE:
    g_sol_config.enabled = req->data[2];
    break;
  case SOL_PARAMETER_SOL_AUTHENTICATION:
    g_sol_config.force_auth_encrypt_priv = req->data[2];
    break;
  case SOL_PARAMETER_CHARACTER_INTERVAL:
    g_sol_config.character_accumulate_level = req->data[2];
    g_sol_config.character_send_threshold = req->data[3];
    break;
  case SOL_PARAMETER_SOL_RETRY:
    g_sol_config.retry_count = req->data[2];
    g_sol_config.retry_interval = req->data[3];
    break;
  case SOL_PARAMETER_SOL_NON_VOLATILE_BIT_RATE:
    g_sol_config.non_volatile_bit_rate = req->data[2];
    break;
  case SOL_PARAMETER_SOL_VOLATILE_BIT_RATE:
    g_sol_config.volatile_bit_rate = req->data[2];
    break;
  }
  plat_sol_config_save();
}

sol_struct_t *SerialOverLan::get_sol_struct() { return &this->sol_struct; }
void SerialOverLan::set_sol_struct(sol_struct_t *temp) {
  sol_struct.sockfd = temp->sockfd;
  sol_struct.sol_cliaddr = temp->sol_cliaddr;
}