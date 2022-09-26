#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__

#include "ipmi/common.hpp"
#include "ipmi/ipmi.hpp"

/**
 * @brief Channel Protocol Type List
 *
 */
enum {
  PROTOCOL_IPMB_LAN = 0x01,
  PROTOCOL_ICMB = 0x02,
  PROTOCOL_SMBUS = 0x04,
  PROTOCOL_KCS = 0x05,
  PROTOCOL_SMIC = 0x06,
  PROTOCOL_BT_10 = 0x07,
  PROTOCOL_BT_15 = 0x08,
  PROTOCOL_TMMOD = 0x09,
};
/**
 * @brief Channel Type List
 *
 */
enum {
  MEDIUM_IPMB = 0x01,
  MEDIUM_ICMB_V10 = 0x02,
  MEDIUM_ICMB_V09 = 0x03,
  MEDIUM_802_3_LAN = 0x04,
  MEDIUM_RS232 = 0x05,
  MEDIUM_OTHER_LAN = 0x06,
  MEDIUM_PCI_SMBUS = 0x07,
  MEDIUM_SMBUS_V10_V11 = 0x08,
  MEDIUM_SMBUS_V20 = 0x09,
  MEDIUM_USB_1X = 0x0A,
  MEDIUM_USB_2X = 0x0B,
  MEDIUM_SI = 0x0C,
};
/**
 * @brief Channel Command List
 *
 */
enum {
  CHANNEL_SESSIONLESS = 0x00,
  CHANNEL_SINGLE_SESSION = 0x40,
  CHANNEL_MULTI_SESSION = 0x80,
  CHANNEL_SESSION_BASE = 0xC0,
};
/**
 * @brief DCMI Command List
 *
 */

class Ipmichannel {
private:
  uint8_t medium_type;
  const uint8_t protocol_type;
  uint8_t session_info;
  uint8_t access_mode;
  uint8_t alerting;
  uint8_t per_message_auth;
  const uint8_t channel_number;
  uint8_t priv_limit;
  uint8_t user_lvl_auth;

public:
  Ipmichannel(uint8_t _c_num)
      : medium_type(0), protocol_type(0), channel_number(_c_num) {}
  Ipmichannel(uint8_t _m_type, uint8_t _p_type, uint8_t _c_num, uint8_t _s_info)
      : medium_type(_m_type), protocol_type(_p_type), channel_number(_c_num),
        session_info(_s_info) {
    access_mode = 0;
    alerting = 0;
    priv_limit = 0;
    user_lvl_auth = 0;
    per_message_auth = 0;
  }
  void getChannelInfo(ipmi_res_t *response, uint8_t *res_len, uint8_t _ch_num);
  void getChannelAccess(ipmi_res_t *response, uint8_t *res_len);
  void setChannelAccess(ipmi_req_t *request);
  void getChannelParams(uint8_t ch_num, int type_idx, uint8_t *data);
  bool setChannelParams(uint8_t flag, uint8_t data);
  uint8_t getChannelAccessMode(uint8_t chan);
  uint8_t getChannelMaxPriv(uint8_t chan);
};
#endif
