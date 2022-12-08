#include "ipmi/session.hpp"

extern char uuid_hex[16];

uint8_t Ipmisession::setActiveSessionCount(uint8_t _count) {
  return (this->active_session_count = _count);
}

uint8_t Ipmisession::getSessionChanData() {
  return ((this->auxiliary_data << 4) | this->channel_number);
}

uint8_t Ipmisession::getActiveSessCount() { return this->active_session_count; }
uint8_t Ipmisession::getSessionSlotCount() { return this->session_slot_count; }

uint8_t Ipmisession::getSessionUserId() { return this->user_id; }

void Ipmisession::setSessionUserId(uint8_t _id) { this->user_id = _id; }

int Ipmisession::setSessionPriv(uint8_t _priv) {
  if (_priv <= 4 && _priv >= 0) {
    if (this->rmcpData.userPriv < _priv) {
      uint8_t tmp_role = this->rmcpData.rolem & 0x10;
      this->rmcpData.rolem = tmp_role | (_priv & 0x07);
      return 0;
    } else
      return -1;
  } else
    return -2;
}

std::vector<uint8_t> Ipmisession::getSessionUsername() {
  return this->rmcpData.rmcpUsername;
}

uint8_t Ipmisession::getSessionPriv() { return this->rmcpData.rolem & 0x07; }

std::vector<uint8_t> Ipmisession::getSessionAesKey() {
  return this->rmcpData.hmac_aeskey;
}

std::vector<uint8_t> Ipmisession::getSessionK1() {
  return this->rmcpData.hmac_k1;
}

bool Ipmisession::setSessionAesKey(uint8_t *_k1, uint8_t *_aes) {
  std::vector<uint8_t> t_k1(_k1, _k1 + 32);
  std::vector<uint8_t> t_aes(_aes, _aes + 16);
  if (t_k1.size() != 0 && t_aes.size() != 0) {
    this->rmcpData.hmac_k1 = t_k1;
    this->rmcpData.hmac_aeskey = t_aes;
  } else
    return false;

  return true;
}

std::vector<uint8_t> Ipmisession::buildHmacRAKP4() {
  std::vector<uint8_t> ret;
  std::vector<uint8_t>::iterator it;

  ret.insert(ret.begin(), this->rmcpData.hmac_rm.begin(),
             this->rmcpData.hmac_rm.end());
  it = ret.end();
  ret.insert(it, this->rmcpData.mgntSessionId.begin(),
             this->rmcpData.mgntSessionId.end());
  it = ret.end();
  ret.insert(it, uuid_hex, uuid_hex + 16);

  return ret;
}

std::vector<uint8_t> Ipmisession::buildHmacRAKP3() {
  std::vector<uint8_t> ret;
  std::vector<uint8_t>::iterator it;

  ret.insert(ret.begin(), this->rmcpData.hmac_rc.begin(),
             this->rmcpData.hmac_rc.end());
  it = ret.end();
  ret.insert(it, this->rmcpData.clntSessionId.begin(),
             this->rmcpData.clntSessionId.end());
  ret.push_back(this->rmcpData.rolem);
  ret.push_back(this->rmcpData.userNameLength);
  it = ret.end();
  ret.insert(it, this->rmcpData.rmcpUsername.begin(),
             this->rmcpData.rmcpUsername.end());

  return ret;
}

std::vector<uint8_t> Ipmisession::buildHmacSikData() {
  std::vector<uint8_t> ret;
  std::vector<uint8_t>::iterator it;
  ret.insert(ret.begin(), this->rmcpData.hmac_rm.begin(),
             this->rmcpData.hmac_rm.end());
  it = ret.end();
  ret.insert(it, this->rmcpData.hmac_rc.begin(), this->rmcpData.hmac_rc.end());
  ret.push_back(this->rmcpData.rolem);
  ret.push_back(this->rmcpData.userNameLength);
  it = ret.end();
  ret.insert(it, this->rmcpData.rmcpUsername.begin(),
             this->rmcpData.rmcpUsername.end());

  return ret;
}

bool Ipmisession::closeSession() {
  this->session_handle = 0;
  return true;
}

bool Ipmisession::setSessionActive(uint8_t _id) {
  this->session_handle = 32 + _id;

  return true;
}

std::vector<uint8_t> Ipmisession::getSessionKUID() {
  return this->rmcpData.kuid;
}

std::vector<uint8_t> Ipmisession::buildHmacData() {
  std::vector<uint8_t> hmac;
  std::vector<uint8_t>::iterator it = hmac.begin();
  std::vector<uint8_t> v_uuid(uuid_hex, uuid_hex + 16);
#if 0
    printf("[build rmcp authcode] CLIENT ID : ");
    for(int i = 0 ; i < rmcpData.clntSessionId.size() ; i++)
        printf("0x%02X ", rmcpData.clntSessionId[i]);
    printf("\n");
    printf("[build rmcp authcode] SESSION ID : ");
    for(int i = 0 ; i < rmcpData.mgntSessionId.size() ; i++)
        printf("0x%02X ", rmcpData.mgntSessionId[i]);
    printf("\n");
    printf("[build rmcp authcode] RM ID : ");
    for(int i = 0 ; i < rmcpData.hmac_rm.size() ; i++)
        printf("0x%02X ", rmcpData.hmac_rm[i]);
    printf("\n");
    printf("[build rmcp authcode] CLIENT ID : ");
    for(int i = 0 ; i < rmcpData.hmac_rc.size() ; i++)
        printf("0x%02X ", rmcpData.hmac_rc[i]);
    printf("\n");
    printf("[build rmcp authcode] UUID : ");
    for(int i = 0 ; i < v_uuid.size() ; i++)
        printf("0x%02X ", v_uuid[i]);
    printf("\n");
    printf("[build rmcp authcode] ROLEM : 0x%02X\n", rmcpData.rolem);
    printf("[build rmcp authcode] UNAME LENGTH : 0x%02X\n", rmcpData.userNameLength);
    printf("[build rmcp authcode] USERNAME : ");
    for(int i = 0 ; i < rmcpData.rmcpUsername.size() ; i++)
        printf("0x%02X ", rmcpData.rmcpUsername[i]);
    printf("\n");
#endif
  hmac.insert(it, this->rmcpData.clntSessionId.begin(),
              this->rmcpData.clntSessionId.end());
  it = hmac.end();
  hmac.insert(it, this->rmcpData.mgntSessionId.begin(),
              this->rmcpData.mgntSessionId.end());
  it = hmac.end();
  hmac.insert(it, this->rmcpData.hmac_rm.begin(), this->rmcpData.hmac_rm.end());
  it = hmac.end();
  hmac.insert(it, this->rmcpData.hmac_rc.begin(), this->rmcpData.hmac_rc.end());
  it = hmac.end();
  hmac.insert(it, v_uuid.begin(), v_uuid.end());
  hmac.push_back(this->rmcpData.rolem);
  hmac.push_back(this->rmcpData.userNameLength);
  it = hmac.end();
  hmac.insert(it, this->rmcpData.rmcpUsername.begin(),
              this->rmcpData.rmcpUsername.end());

  return hmac;
}

uint16_t Ipmisession::getSessionPort() { return this->lan_data.console_port; }

std::vector<uint8_t> Ipmisession::getSessionIpAddr() {
  return this->lan_data.console_ip;
}

std::vector<uint8_t> Ipmisession::getSessionMacAddr() {
  return this->lan_data.console_mac;
}

uint8_t Ipmisession::getSessionHandle() { return this->session_handle; }

bool Ipmisession::getSessionId(uint32_t _cliIp, std::vector<uint8_t> _mgntSId) {
  std::vector<uint8_t> temp_ipaddr;

  for (int i = 0; i < 4; i++) {
    temp_ipaddr.push_back(_cliIp & 0xff);
    _cliIp = _cliIp >> 8;
  }

  if (this->lan_data.console_ip == temp_ipaddr) {
    if (this->rmcpData.mgntSessionId == _mgntSId) {
      return true;
    } else
      return false;
  } else
    return false;
}

std::vector<uint8_t> Ipmisession::getSessionMgntId() {
  return this->rmcpData.mgntSessionId;
}

bool Ipmisession::setRmcpLanData(std::vector<uint8_t> _ip,
                                 std::vector<uint8_t> _mac, uint16_t _port) {
  this->lan_data.console_ip.assign(_ip.begin(), _ip.end());
  this->lan_data.console_mac.assign(_mac.begin(), _mac.end());
  this->lan_data.console_port = _port;

  return true;
}
bool Ipmisession::setRmcpData(std::vector<uint8_t> _mgntId,
                              std::vector<uint8_t> _clntId,
                              std::vector<uint8_t> _rc,
                              std::vector<uint8_t> _rm, uint8_t _rolem,
                              uint8_t _uPriv, std::vector<uint8_t> _userName,
                              uint8_t _nameLength, std::string _passwd) {
  this->rmcpData.bmc_id.assign(_mgntId.begin(), _mgntId.end());
  this->rmcpData.mgntSessionId.assign(_mgntId.begin(), _mgntId.end());
  this->rmcpData.clntSessionId.assign(_clntId.begin(), _clntId.end());
  this->rmcpData.hmac_rm.assign(_rm.begin(), _rm.end());
  this->rmcpData.hmac_rc.assign(_rc.begin(), _rc.end());
  this->rmcpData.rolem = _rolem;
  this->rmcpData.rmcpUsername.assign(_userName.begin(), _userName.end());
  this->rmcpData.userNameLength = _nameLength;
  this->rmcpData.kuid.assign(_passwd.begin(), _passwd.end());
  this->rmcpData.userPriv = _uPriv;
  this->auxiliary_data = 1;
  return true;
}