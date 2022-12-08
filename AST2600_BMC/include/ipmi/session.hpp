#ifndef __SESSION_HPP__
#define __SESSION_HPP__
#include <cstdint>
#include <sys/types.h>
#pragma once
#include "ipmi/common.hpp"
#include "ipmi/ipmi.hpp"
class Ipmisession {
private:
  uint8_t session_handle;
  uint8_t sol_activator;
#if WORDS_BIGENDIAN
  uint8_t __reserved1 : 2;
  uint8_t session_slot_count : 6; /* 1-based */
#else
  uint8_t session_slot_count : 6; /* 1-based */
  uint8_t __reserved1 : 2;
#endif

#if WORDS_BIGENDIAN
  uint8_t __reserved2 : 2;
  char active_session_count : 6; /* 1-based */
#else
  char active_session_count : 6; /* 1-based */
  uint8_t __reserved2 : 2;
#endif

#if WORDS_BIGENDIAN
  uint8_t __reserved3 : 2;
  uint8_t user_id : 6;
#else
  uint8_t user_id : 6;
  uint8_t __reserved3 : 2;
#endif

#if WORDS_BIGENDIAN
  uint8_t __reserved4 : 4;
  uint8_t privilege_level : 4;
#else
  uint8_t privilege_level : 4;
  uint8_t __reserved4 : 4;
#endif

#if WORDS_BIGENDIAN
  uint8_t auxiliary_data : 4;
  uint8_t channel_number : 4;
#else
  uint8_t channel_number : 4;
  uint8_t auxiliary_data : 4;
#endif

  struct {
    std::vector<uint8_t> console_ip;  // Max Size 4
    std::vector<uint8_t> console_mac; // Max Size 6
    uint16_t console_port;
  } lan_data;

  struct {
    std::vector<uint8_t> bmc_id;        // Size 4
    std::vector<uint8_t> mgntSessionId; // Size 4
    std::vector<uint8_t> clntSessionId; // Size 4
    std::vector<uint8_t> rmcpUsername;  // Size 17
    std::vector<uint8_t> hmac_rm;       // Size 16
    std::vector<uint8_t> hmac_rc;       // Size 16
    std::vector<uint8_t> kuid;          // Size 20
    std::vector<uint8_t> hmac_k1;       // Size 32
    std::vector<uint8_t> hmac_aeskey;   // Size 16
    uint8_t rolem;
    uint8_t userPriv;
    uint8_t userNameLength;
  } rmcpData;

public:
  Ipmisession() {
    session_handle = 0x0;
    active_session_count = 0x0;
    session_slot_count = MAX_SESSION;
  }
  ~Ipmisession() {
    rmcpData.bmc_id.clear();
    rmcpData.mgntSessionId.clear();
    rmcpData.clntSessionId.clear();
    rmcpData.rmcpUsername.clear();
    rmcpData.hmac_rm.clear();
    rmcpData.hmac_rc.clear();
    rmcpData.kuid.clear();
    rmcpData.hmac_k1.clear();
    rmcpData.hmac_aeskey.clear();
    rmcpData.rolem = 0;
    rmcpData.userPriv = 0;
    rmcpData.userNameLength = 0;
    lan_data.console_ip.clear();
    lan_data.console_mac.clear();
    lan_data.console_port = 0;
    active_session_count = 0x0;
  }
  uint8_t setActiveSessionCount(uint8_t _count);
  uint16_t getSessionPort();
  uint8_t getSessionChanData();
  uint8_t getActiveSessCount();
  uint8_t getSessionSlotCount();
  uint8_t getSessionUserId();
  void setSessionUserId(uint8_t _id);
  bool closeSession();
  int setSessionPriv(uint8_t _priv);
  std::vector<uint8_t> getSessionUsername();
  uint8_t getSessionPriv();
  std::vector<uint8_t> getSessionAesKey();
  std::vector<uint8_t> getSessionK1();
  bool setSessionAesKey(uint8_t *_k1, uint8_t *_aes);
  std::vector<uint8_t> buildHmacRAKP4();
  std::vector<uint8_t> buildHmacRAKP3();
  std::vector<uint8_t> buildHmacSikData();
  bool setSessionActive(uint8_t _id);
  std::vector<uint8_t> getSessionKUID();
  std::vector<uint8_t> buildHmacData();
  std::vector<uint8_t> getSessionMacAddr();
  uint8_t getSessionHandle();
  void SetSessionHandle(uint8_t set_handler) {
    this->session_handle = set_handler;
  }
  bool getSessionId(uint32_t _cliIp, std::vector<uint8_t> mgntSId);
  std::vector<uint8_t> getSessionIpAddr();
  std::vector<uint8_t> getSessionMgntId();
  bool setRmcpData(std::vector<uint8_t> _mgntId, std::vector<uint8_t> _clntId,
                   std::vector<uint8_t> _rc, std::vector<uint8_t> _rm,
                   uint8_t _rolem, uint8_t _uPriv,
                   std::vector<uint8_t> _userName, uint8_t _nameLength,
                   std::string _passwd);
  bool setRmcpLanData(std::vector<uint8_t> _ip, std::vector<uint8_t> _mac,
                      uint16_t _port);
  u_int8_t Get_sol_activator() { return this->sol_activator; }
  void Set_sol_activator(int set_sol_active) {
    this->sol_activator = static_cast<u_int8_t>(set_sol_active);
  }
};

#endif
