#include <ctime>
#include <iostream>
#include <ipmi/apps.hpp>
#include <ipmi/channel.hpp>
#include <ipmi/ipmi.hpp>
#include <ipmi/session.hpp>
#include <ipmi/sol.hpp>
#include <ipmi/user.hpp>
#include <openssl/aes.h>
#include <openssl/hmac.h>
#include <redfish/msg_session.hpp>
#include <redfish/rmcp.hpp>
extern Ipmichannel ipmiChannel[9];
extern Ipmiuser ipmiUser[10];
extern char uuid_hex[16];
Ipmisession ipmiSession[5];

const EVP_MD *evp_md = NULL;
std::vector<uint8_t> Rmcppacket::rmcp_process_packet() {
  printf("\npacket : <%x>\n", this->packet[5]);
  std::vector<uint8_t> packet_out;

  if (this->packet[RMCP_HEADER_OFFSET_VERSION] ==
      RMCP_HEADER_VERSION) //&& (acc_mode != 0))
  {
    if (this->packet[RMCP_HEADER_OFFSET_RESERVED] == RMCP_HEADER_RESERVED) {
      if (this->packet[RMCP_HEADER_OFFSET_SEQ_NR] == RMCP_HEADER_SEQ_NOACK) {
        if ((this->packet[RMCP_HEADER_OFFSET_CLASS] & RMCP_HEADER_CLASS_MASK) ==
            RMCP_HEADER_CLASS_ASF) {
        } else if ((this->packet[RMCP_HEADER_OFFSET_CLASS] &
                    RMCP_HEADER_CLASS_MASK) == RMCP_HEADER_CLASS_IPMI) {
          if (this->packet[RMCP_HEADER_OFFSET_AUTH] == AUTH_TYPE_RMCP_PLUS) {
            if (this->packet[RMCP_PACKET_OFFSET_TYPE] ==
                RMCP_OPENSESSION_REQUEST) {
              uint8_t rmcpplus_length = this->packet[14];
              uint8_t rmcpresp_length = rmcpplus_length + 4;

              std::vector<uint8_t> rmcp_plus_data(rmcpplus_length);
              std::vector<uint8_t> auth_payload;
              std::vector<uint8_t>::iterator it = this->packet.begin();
              std::vector<uint8_t> mgnt_session_id, clnt_session_id;

              rmcp_plus_data.assign(
                  this->packet.begin() + RMCP_DATA_OFFSET_REQUEST,
                  this->packet.begin() + RMCP_DATA_OFFSET_REQUEST +
                      this->packet[14]);
              auth_payload.assign(rmcp_plus_data.begin() + 8,
                                  rmcp_plus_data.end());

              this->setRmcpAuthentication(auth_payload[4], auth_payload[12],
                                          auth_payload[20]);
              // Create Managed session id
              for (int i = 0; i < 4; i++) {
                uint8_t r_num = rand() % 255;
                mgnt_session_id.push_back(r_num);
              }
              // Get Client Session id
              clnt_session_id.assign(rmcp_plus_data.begin() + 4,
                                     rmcp_plus_data.begin() + 8);

              bool ret =
                  this->setRmcpSessionId(clnt_session_id, mgnt_session_id);

              this->getMgmtSessionId();
              this->getClientSessionId();

              packet_out.assign(it, it + 4);             // Packet In Header
              packet_out.push_back(AUTH_TYPE_RMCP_PLUS); // Auth Type
              packet_out.push_back(
                  RMCP_OPENSESSION_RESPONSE); // Open Session Response Type

              for (int i = 0; i < 8; i++)
                packet_out.push_back(0);               // Zero padding
              packet_out.push_back(rmcpresp_length);   // response length
              packet_out.push_back(0);                 // Zero padding
              packet_out.push_back(rmcp_plus_data[0]); // Client Tag
              packet_out.push_back(0); // Zero padding (Status code)
              packet_out.push_back(
                  4); // Max Privilege (ipmiChannel Priv Limit으로 변경 요망)
              packet_out.push_back(0); // Zero padding (Reserved)
              it = packet_out.end();
              packet_out.insert(it, this->sessionIdClient.begin(),
                                this->sessionIdClient.end());
              it = packet_out.end();
              packet_out.insert(it, this->sessionIdServer.begin(),
                                this->sessionIdServer.end());
              it = packet_out.end();
              packet_out.insert(it, auth_payload.begin(), auth_payload.end());
            } else if (this->packet[RMCP_PACKET_OFFSET_TYPE] ==
                       RAKP1_RMCP_PLUS) {
              std::vector<uint8_t> cliMac;
              std::vector<uint8_t> userName;
              std::vector<uint8_t> passwd;
              std::vector<uint8_t> rm;
              std::vector<uint8_t> rc;
              std::vector<uint8_t> rmcp_plus_data;
              Ipmiuser rmcpUser;
              std::string c_password;
              uint8_t unameLength;
              uint8_t rmcp_rolem;
              uint8_t max_priv;
              uint8_t user_idx = 0;
              uint8_t response_length = 0;
              uint8_t authcode_size, auth_type, cliTag = 0;
              uint32_t cliIpAddress = this->getClientIpAddress();
              uint16_t cliPort = this->getClientPort();
              bool flag_session = false;
              int index_session = -1;
              rmcp_plus_data.assign(this->packet.begin() +
                                        RMCP_DATA_OFFSET_REQUEST,
                                    this->packet.end());
              cliTag = rmcp_plus_data[0];
              rmcp_rolem = rmcp_plus_data[24];
              unameLength = rmcp_plus_data[27];
              max_priv = rmcp_rolem & 0x07;
              userName.assign(rmcp_plus_data.begin() + 28,
                              rmcp_plus_data.begin() + 28 + unameLength);
              std::string c_username(userName.begin(), userName.end());
              rm.assign(rmcp_plus_data.begin() + 8,
                        rmcp_plus_data.begin() + 24);
              cout << "c_username =" << c_username << endl;
              int ret = app_getUserindex(c_username);
              cout << "app_getUserindex =" << ret << endl;
              if (ret == -1)
                return packet_out;
              user_idx = ret;
              c_password = app_getUserpassword(ret);
              cout << "c_password =" << c_password << endl;
              cout << "rmcp_rolem =" << rmcp_rolem << endl;
              if (rmcp_rolem == 0) {
                ret = app_getUserPriv(c_username, ret);
                cout << "app_getUserPriv =" << ret << endl;
                if (ret == -1)
                  return packet_out;

                if (ret < max_priv)
                  return packet_out;
              }
              srand((unsigned int)time(0));

              for (int i = 0; i < 16; i++) {
                uint8_t r_num = rand() % 256;
                rc.push_back(r_num);
              }

              for (int i = 0; i < 5; i++) {
                flag_session = ipmiSession[i].getSessionId(
                    this->getClientIpAddress(), this->sessionIdServer);
                if (flag_session == true) {
                  index_session = i;
                  break;
                }
              }
              std::vector<uint8_t> temp_Cipaddr;
              for (int i = 0; i < 4; i++) {
                temp_Cipaddr.push_back(cliIpAddress & 0xff);
                cliIpAddress = cliIpAddress >> 8;
              }

              if (flag_session == true) {
                ipmiSession[index_session].setRmcpData(
                    this->sessionIdServer, this->sessionIdClient, rc, rm,
                    rmcp_rolem, ret, userName, unameLength, c_password);
                ipmiSession[index_session].setRmcpLanData(temp_Cipaddr, cliMac,
                                                          cliPort);
                ipmiSession[index_session].setSessionUserId(user_idx);
              } else {
                for (int i = 0; i < 5; i++) {
                  cliMac = this->getClientMacAddress();
                  if (cliMac.size() != 0) {
                    std::vector<uint8_t> sesMac =
                        ipmiSession[i].getSessionMacAddr();
                    if ((ipmiSession[i].getSessionHandle() == 0x0) &&
                        (cliMac != sesMac)) {
                      ipmiSession[i].setRmcpData(
                          this->sessionIdServer, this->sessionIdClient, rc, rm,
                          rmcp_rolem, ret, userName, unameLength, c_password);
                      ipmiSession[i].setRmcpLanData(temp_Cipaddr, cliMac,
                                                    cliPort);
                      ipmiSession[index_session].setSessionUserId(user_idx);
                      index_session = i;
                      break;
                    } else if ((ipmiSession[i].getSessionHandle() == 0x0) &&
                               cliMac == sesMac) {
                      ipmiSession[i].setRmcpData(
                          this->sessionIdServer, this->sessionIdClient, rc, rm,
                          rmcp_rolem, ret, userName, unameLength, c_password);
                      ipmiSession[i].setRmcpLanData(temp_Cipaddr, cliMac,
                                                    cliPort);
                      ipmiSession[index_session].setSessionUserId(user_idx);
                      index_session = i;
                      break;
                    }
                  }
                }
              }
              auth_type = this->getRmcpAuthentication(1);
              if (auth_type == 0x0)
                response_length = 56;
              if (auth_type == 0x1) {
                evp_md = EVP_sha1();
                authcode_size = 20;
                response_length = 60;
              } else if (auth_type == 0x2) {
                evp_md = EVP_md5();
                authcode_size = 16;
                response_length = 56;
              } else if (auth_type == 0x3) {
                evp_md = EVP_sha256();
                authcode_size = 32;
                response_length = 72;
              }

              uint8_t authcode[authcode_size];
              uint32_t authLength = 0;
              if (auth_type != 0x0) {
                std::vector<uint8_t> hmac_data =
                    ipmiSession[index_session].buildHmacData();
                std::vector<uint8_t> kuid =
                    ipmiSession[index_session].getSessionKUID();

                HMAC(evp_md, kuid.data(), kuid.size(), hmac_data.data(),
                     hmac_data.size(), authcode, &authLength);
              }

              std::vector<uint8_t>::iterator it = packet_out.begin();
              packet_out.insert(it, this->packet.begin(),
                                this->packet.begin() + RMCP_HEADER_LENGTH);
              it = packet_out.end();
              packet_out.push_back(AUTH_TYPE_RMCP_PLUS); // Packet Auth Type
              packet_out.push_back(RAKP2_RMCP_PLUS);     // Payload Auth Type
              for (int i = 0; i < 8; i++)
                packet_out.push_back(
                    0); // Zero padding (Session id & Sequence number)
              packet_out.push_back(response_length);
              packet_out.push_back(0);      // Status Code
              packet_out.push_back(cliTag); // Client Tag
              packet_out.push_back(0);      // Status Code
              packet_out.push_back(0);      // Reserve
              packet_out.push_back(0);      // Reserve
              it = packet_out.end();
              packet_out.insert(it, this->sessionIdClient.begin(),
                                this->sessionIdClient.end());
              it = packet_out.end();
              packet_out.insert(it, rc.begin(), rc.end());
              std::vector<uint8_t> v_uuid(uuid_hex, uuid_hex + 16);
              it = packet_out.end();
              packet_out.insert(it, v_uuid.begin(), v_uuid.begin() + 16);

              if (auth_type != 0x0) {
                it = packet_out.end();
                packet_out.insert(it, authcode, authcode + authcode_size);
              }
              return packet_out;
            } else if (this->packet[RMCP_PACKET_OFFSET_TYPE] ==
                       RAKP3_RMCP_PLUS) {
              int index_session = 0;
              bool ret = false;
              bool flag_session = false;

              std::vector<uint8_t>::iterator it;
              std::vector<uint8_t> rmcp_plus_data, kuid;
              std::vector<uint8_t> hmac_sik_data, hmac_rakp3_data,
                  rmcp_authcode, hmac_rakp3_v;
              std::vector<uint8_t> hmac_k1_data(20, 0x1), hmac_k2_data(20, 0x2),
                  hmac_k1_v, hmac_aeskey_v;
              uint32_t hmac_sik_length = 0, hmac_rakp3_length = 0,
                       hmac_k1_length, hmac_aes_length, hmac_rakp4_length;
              uint8_t rmcp_authcode_length = 0, cliTag, hmac_rakp4[20];
              uint8_t auth_type = this->getRmcpAuthentication(1);

              rmcp_plus_data.assign(this->packet.begin() +
                                        RMCP_DATA_OFFSET_REQUEST,
                                    this->packet.end());
              cliTag = rmcp_plus_data[0];
              rmcp_authcode.assign(rmcp_plus_data.begin() + 8,
                                   rmcp_plus_data.end());

              if (auth_type == 0x1) {
                evp_md = EVP_sha1();
                rmcp_authcode_length = 20;
              } else if (auth_type == 0x2) {
                evp_md = EVP_md5();
                rmcp_authcode_length = 16;
              } else if (auth_type == 0x3) {
                evp_md = EVP_sha256();
                rmcp_authcode_length = 32;
              }

              for (int i = 0; i < 5; i++) {
                flag_session = ipmiSession[i].getSessionId(
                    this->getClientIpAddress(), this->sessionIdServer);
                if (flag_session == true) {
                  index_session = i;
                  break;
                }
              }

              if (flag_session == true) {
                ret =
                    ipmiSession[index_session].setSessionActive(index_session);
                if (ret == false)
                  return packet_out;
              } else
                return packet_out;

              if (auth_type != 0x0) {

                hmac_sik_data = ipmiSession[index_session].buildHmacSikData();
                hmac_rakp3_data = ipmiSession[index_session].buildHmacRAKP3();
                kuid = ipmiSession[index_session].getSessionKUID();

                hmac_sik_length = hmac_sik_data.size();
                hmac_rakp3_length = hmac_rakp3_data.size();

                uint8_t hmac_sik[hmac_sik_length];
                uint8_t hmac_rakp3[hmac_rakp3_length];
                uint8_t hmac_k1[32];
                uint8_t hmac_aeskey[16];

                HMAC(evp_md, kuid.data(), kuid.size(), hmac_sik_data.data(),
                     hmac_sik_data.size(), hmac_sik, &hmac_sik_length);
                HMAC(evp_md, kuid.data(), kuid.size(), hmac_rakp3_data.data(),
                     hmac_rakp3_data.size(), hmac_rakp3, &hmac_rakp3_length);
                HMAC(evp_md, hmac_sik, hmac_sik_length, hmac_k1_data.data(),
                     hmac_k1_data.size(), hmac_k1, &hmac_k1_length);
                HMAC(evp_md, hmac_sik, hmac_sik_length, hmac_k2_data.data(),
                     hmac_k2_data.size(), hmac_aeskey, &hmac_aes_length);

                ipmiSession[index_session].setSessionAesKey(hmac_k1,
                                                            hmac_aeskey);
                hmac_rakp3_v.assign(hmac_rakp3, hmac_rakp3 + hmac_rakp3_length);

                if (rmcp_authcode != hmac_rakp3_v) {
                  std::cout << "Failed to compare auth code" << std::endl;
                  return packet_out;
                }

                if (rmcp_plus_data[0] != 0) {
                  std::cout << "Some error in rmcp plus data" << std::endl;
                  return packet_out;
                }

                std::vector<uint8_t> hmac_rakp4_data =
                    ipmiSession[index_session].buildHmacRAKP4();
                HMAC(evp_md, hmac_sik, hmac_sik_length, hmac_rakp4_data.data(),
                     hmac_rakp4_data.size(), hmac_rakp4, &hmac_rakp4_length);
              }

              it = packet_out.begin();
              packet_out.insert(it, this->packet.begin(),
                                this->packet.begin() + RMCP_HEADER_LENGTH);
              packet_out.push_back(AUTH_TYPE_RMCP_PLUS); // Packet Auth Type
              packet_out.push_back(RAKP4_RMCP_PLUS);

              for (int i = 0; i < 8; i++)
                packet_out.push_back(0);

              packet_out.push_back(cliTag);
              packet_out.push_back(0); // status code;
              packet_out.push_back(0); // reserve;
              packet_out.push_back(0); // reserve;
              packet_out.push_back(0); // reserve;
              packet_out.push_back(0); // reserve;
              it = packet_out.end();
              packet_out.insert(it, this->sessionIdClient.begin(),
                                this->sessionIdClient.end());
              it = packet_out.end();

              if (auth_type != 0x0)
                packet_out.insert(it, hmac_rakp4,
                                  hmac_rakp4 + hmac_rakp4_length);

              return packet_out;
            }

            else if ((this->packet[5] == IPMI_RMCP_PLUS) ||
                     (this->packet[5] == IPMI_RMCP_PLUS_SOL) ||
                     (this->packet[5] == 0x40)) {

              uint8_t auth_type = this->auth_type, index_session;
              uint8_t integrity_type = this->integrity_type;
              uint8_t enc_bit = 0, encryption_length = this->packet[14] +
                                                       (this->packet[15] << 8);
              uint8_t encrypt_data[encryption_length - 16];
              uint8_t decrypt_out[encryption_length - 16];
              uint8_t exAuthcode_out[20];
              uint32_t exAuthcode_length = 0;
              uint32_t checksum_flag = 0;
              uint32_t decrypt_length = 0;
              bool ret = false;
              std::vector<uint8_t> rmtSessId(this->packet.begin() + 6,
                                             this->packet.begin() + 10);
              std::vector<uint8_t> inAuthcode, exAuthcode, hmac_k1,
                  tmpSeqNumber(this->packet.begin() + 10,
                               this->packet.begin() + 14);
              std::vector<uint8_t> seqDummy(4, 0xff);
              if (auth_type == 0x1)
                evp_md = EVP_sha1();
              else if (auth_type == 0x2)
                evp_md = EVP_md5();
              else if (auth_type == 0x3)
                evp_md = EVP_sha256();

              if ((this->packet[5] == 0x80) ||
                  (this->packet[5] == IPMI_RMCP_PLUS) ||
                  (this->packet[5] == IPMI_RMCP_PLUS_SOL))
                enc_bit = 1;

              for (int i = 0; i < 5; i++) {
                ret = ipmiSession[i].getSessionId(this->getClientIpAddress(),
                                                  rmtSessId);
                if (ret == true) {
                  index_session = i;
                  break;
                }
              }

              if (ret == false) {
                return packet_out;
              }

              if (enc_bit == 1) {
                //
                hmac_k1 = ipmiSession[index_session].getSessionK1();
                hmac_k1.assign(hmac_k1.begin(), hmac_k1.begin() + 20);

                uint8_t inPacket_length = this->packet.size();
                std::vector<uint8_t> Authcode_v;
                if (auth_type == 0x1) {
                  inAuthcode.insert(inAuthcode.begin(),
                                    this->packet.begin() + inPacket_length - 12,
                                    this->packet.end());
                  exAuthcode.insert(exAuthcode.begin(),
                                    this->packet.begin() + RMCP_HEADER_LENGTH,
                                    this->packet.begin() + inPacket_length -
                                        12);
                  HMAC(evp_md, hmac_k1.data(), 20, exAuthcode.data(),
                       exAuthcode.size(), exAuthcode_out, &exAuthcode_length);
                  Authcode_v.assign(exAuthcode_out, exAuthcode_out + 12);
                } else if (auth_type == 0x2 ||
                           (auth_type == 0x3 && integrity_type == 0x4)) {

                  inAuthcode.insert(inAuthcode.begin(),
                                    this->packet.begin() + inPacket_length - 16,
                                    this->packet.end());
                  exAuthcode.insert(exAuthcode.begin(),
                                    this->packet.begin() + RMCP_HEADER_LENGTH,
                                    this->packet.begin() + inPacket_length -
                                        16);
                  HMAC(evp_md, hmac_k1.data(), 20, exAuthcode.data(),
                       exAuthcode.size(), exAuthcode_out, &exAuthcode_length);
                  Authcode_v.assign(exAuthcode_out, exAuthcode_out + 16);
                }

                if (Authcode_v != inAuthcode) {

                  std::cout << "Authentication Data is not same!" << std::endl;
                  return packet_out;
                }
              }

              if (tmpSeqNumber == this->sequenceNumber &&
                  tmpSeqNumber == seqDummy) {
                std::cout << "Sequence number is different!" << std::endl;
                return packet_out;
              }

              this->sequenceNumber = tmpSeqNumber;
              if (this->sequenceNumber[0] < 0xff)
                this->sequenceNumber[0]++;

              if (enc_bit == 1) {
                std::vector<uint8_t> iv(this->packet.begin() + 16,
                                        this->packet.begin() + 32);
                std::vector<uint8_t> aes_key =
                    ipmiSession[index_session].getSessionAesKey();
                std::vector<uint8_t> enc_data(this->packet.begin() + 32,
                                              this->packet.begin() + 32 +
                                                  encryption_length);

                AES_KEY dec_key;

                AES_set_decrypt_key(aes_key.data(), aes_key.size() * 8,
                                    &dec_key);
                AES_cbc_encrypt(enc_data.data(), decrypt_out,
                                sizeof(uint8_t) * encryption_length, &dec_key,
                                iv.data(), AES_DECRYPT);

                checksum_flag = encryption_length - 17 -
                                decrypt_out[encryption_length - 17];
                decrypt_length = checksum_flag;
              }
              if (this->packet[5] == IPMI_RMCP_PLUS) {
                std::vector<uint8_t> ipmi_session_packet_in;
                std::vector<uint8_t> ipmi_session_packet_out;
                std::vector<uint8_t>::iterator it;
                ipmi_session_packet_in.push_back(0);
                it = ipmi_session_packet_in.end();
                ipmi_session_packet_in.insert(it, this->packet.begin() + 10,
                                              this->packet.begin() + 14);
                for (int i = 0; i < 4; i++)
                  ipmi_session_packet_in.push_back(0);
                ipmi_session_packet_in.push_back(decrypt_length);
                it = ipmi_session_packet_in.end();
                ipmi_session_packet_in.insert(it, decrypt_out,
                                              decrypt_out + decrypt_length);

                ipmi_session_packet_out =
                    ipmi_session_process_packet(ipmi_session_packet_in);
                if (ipmi_session_packet_out.size() > 0) {
                  uint8_t psize = ipmi_session_packet_out.size() - 10;
                  uint8_t padval = 1;
                  uint8_t pad = (psize + 1) % 16;
                  if (pad)
                    pad = (16 - pad);
                  uint8_t new_psize = psize + pad + 17;
                  uint32_t current_length = psize + 1;
                  uint32_t hmac_auth_len = 0;
                  uint32_t needed_pad = current_length % 16;
                  uint8_t aes_payload_len = new_psize - 16;
                  uint8_t aes_out[aes_payload_len], hmac_authcode_out[16];
                  std::vector<uint8_t> aes_payload(
                      ipmi_session_packet_out.begin() + 10,
                      ipmi_session_packet_out.begin() + 10 + psize);
                  std::vector<uint8_t> mgnt_iv, send_iv;
                  std::vector<uint8_t> sessionAesKey =
                      ipmiSession[index_session].getSessionAesKey();
                  std::vector<uint8_t> packet_payload, hmac_auth_input,
                      payloadDummy{0xff, 0xff, 0x02, 0x07};
                  std::vector<uint8_t>::iterator it;
                  for (int i = 0; i < aes_payload.size(); i++) {
                    printf(" %x ", aes_payload[i]);
                  }
                  if (auth_type == 0x1)
                    evp_md = EVP_sha1();
                  else if (auth_type == 0x2)
                    evp_md = EVP_md5();
                  else if (auth_type == 0x3)
                    evp_md = EVP_sha256();

                  if (needed_pad)
                    needed_pad = 16 - needed_pad;

                  while (padval <= needed_pad) {
                    aes_payload.push_back(padval);
                    padval++;
                  }
                  aes_payload.push_back(needed_pad);

                  AES_KEY enc_key;

                  for (int i = 0; i < 16; i++) {
                    uint8_t r_num = rand() % 255;
                    mgnt_iv.push_back(r_num);
                  }
                  send_iv.assign(mgnt_iv.begin(), mgnt_iv.end());

                  AES_set_encrypt_key(sessionAesKey.data(),
                                      sessionAesKey.size() * 8, &enc_key);
                  AES_cbc_encrypt(aes_payload.data(), aes_out,
                                  aes_payload.size(), &enc_key, mgnt_iv.data(),
                                  AES_ENCRYPT);

                  it = packet_payload.begin();
                  packet_payload.insert(it, mgnt_iv.begin(), mgnt_iv.end());
                  it = packet_payload.end();
                  packet_payload.insert(it, aes_out, aes_out + aes_payload_len);

                  it = packet_out.begin();
                  packet_out.insert(it, this->packet.begin(),
                                    this->packet.begin() + RMCP_HEADER_LENGTH +
                                        PAYLOAD_TYPE_LENGTH);
                  it = packet_out.end();
                  packet_out.insert(it, this->sessionIdClient.begin(),
                                    this->sessionIdClient.end());
                  it = packet_out.end();
                  packet_out.insert(it, this->sequenceNumber.begin(),
                                    this->sequenceNumber.end());
                  packet_out.push_back(new_psize);
                  packet_out.push_back(0);
                  it = packet_out.end();
                  packet_out.insert(it, send_iv.begin(), send_iv.end());
                  it = packet_out.end();
                  packet_out.insert(it, aes_out, aes_out + aes_payload_len);
                  it = packet_out.end();
                  packet_out.insert(it, payloadDummy.begin(),
                                    payloadDummy.end());
                  it = packet_out.end();

                  // packet_out[14] = packet_out.size() - 20;

                  hmac_auth_input.assign(packet_out.begin() + 4,
                                         packet_out.end());
                  HMAC(evp_md, hmac_k1.data(), hmac_k1.size(),
                       hmac_auth_input.data(), hmac_auth_input.size(),
                       hmac_authcode_out, &hmac_auth_len);
                  if (auth_type == 0x1)
                    packet_out.insert(it, hmac_authcode_out,
                                      hmac_authcode_out + 12);
                  else if (auth_type == 0x02 || (auth_type == 0x03))
                    packet_out.insert(it, hmac_authcode_out,
                                      hmac_authcode_out + 16);

                  if (ipmiSession[index_session].getSessionHandle() == 0x0) {
                    // ipmiSession[index_session].~Ipmisession();
                  }

                  return packet_out;
                }

              } else if (this->packet[5] == IPMI_RMCP_PLUS_SOL) {
                uint8_t psize = 4;
                uint8_t padval = 1;
                uint8_t pad = (psize + 1) % 16;
                if (pad)
                  pad = (16 - pad);
                uint8_t new_psize = psize + pad + 17;
                uint32_t current_length = psize + 1;
                uint32_t hmac_auth_len = 0;
                uint32_t needed_pad = current_length % 16;
                uint8_t aes_payload_len = new_psize - 16;
                uint8_t aes_out[aes_payload_len], hmac_authcode_out[12];
                uint8_t tmpsolseq_num = decrypt_out[0];
                std::vector<uint8_t> aes_payload;
                std::vector<uint8_t> mgnt_iv, send_iv;
                std::vector<uint8_t> sessionAesKey =
                    ipmiSession[index_session].getSessionAesKey();
                std::vector<uint8_t> packet_payload, hmac_auth_input,
                    payloadDummy{0xff, 0xff, 0x02, 0x07};
                std::vector<uint8_t>::iterator it;

                memcpy(SerialOverLan::Instance().sol_write_buf, decrypt_out + 4,
                       checksum_flag - 4);

                if (auth_type == 0x1)
                  evp_md = EVP_sha1();
                else if (auth_type == 0x2)
                  evp_md = EVP_md5();
                else if (auth_type == 0x3)
                  evp_md = EVP_sha256();
                tmpsolseq_num++;
                if (SerialOverLan::Instance().sol_rdcnt > 0) {
                  aes_payload.push_back(0);
                  aes_payload.push_back(tmpsolseq_num);
                  aes_payload.push_back(0);
                  aes_payload.push_back(0);
                } else {
                  tmpsolseq_num--;
                  aes_payload.push_back(0);
                  aes_payload.push_back(tmpsolseq_num);
                  aes_payload.push_back(1);
                  aes_payload.push_back(0);
                }

                if (needed_pad)
                  needed_pad = 16 - needed_pad;
                while (padval <= needed_pad) {
                  aes_payload.push_back(padval);
                  padval++;
                }
                aes_payload.push_back(needed_pad);

                AES_KEY enc_key;

                for (int i = 0; i < 16; i++) {
                  uint8_t r_num = rand() % 255;
                  mgnt_iv.push_back(r_num);
                }

                send_iv.assign(mgnt_iv.begin(), mgnt_iv.end());
                AES_set_encrypt_key(sessionAesKey.data(),
                                    sessionAesKey.size() * 8, &enc_key);
                AES_cbc_encrypt(aes_payload.data(), aes_out, aes_payload.size(),
                                &enc_key, mgnt_iv.data(), AES_ENCRYPT);

                it = packet_payload.begin();
                packet_payload.insert(it, mgnt_iv.begin(), mgnt_iv.end());
                it = packet_payload.end();
                packet_payload.insert(it, aes_out, aes_out + aes_payload_len);
                it = packet_out.begin();
                packet_out.insert(it, this->packet.begin(),
                                  this->packet.begin() + RMCP_HEADER_LENGTH +
                                      PAYLOAD_TYPE_LENGTH);
                it = packet_out.end();
                packet_out.insert(it, this->sessionIdClient.begin(),
                                  this->sessionIdClient.end());
                it = packet_out.end();
                packet_out.insert(it, this->sequenceNumber.begin(),
                                  this->sequenceNumber.end());
                packet_out.push_back(new_psize);
                packet_out.push_back(0);
                it = packet_out.end();
                packet_out.insert(it, send_iv.begin(), send_iv.end());
                it = packet_out.end();
                packet_out.insert(it, aes_out, aes_out + aes_payload_len);
                it = packet_out.end();
                packet_out.insert(it, payloadDummy.begin(), payloadDummy.end());
                it = packet_out.end();

                hmac_auth_input.assign(packet_out.begin() + 4,
                                       packet_out.end());
                HMAC(evp_md, hmac_k1.data(), hmac_k1.size(),
                     hmac_auth_input.data(), hmac_auth_input.size(),
                     hmac_authcode_out, &hmac_auth_len);
                if (auth_type == 0x1)
                  packet_out.insert(it, hmac_authcode_out,
                                    hmac_authcode_out + 12);
                else if (auth_type == 0x02 || (auth_type == 0x03))
                  packet_out.insert(it, hmac_authcode_out,
                                    hmac_authcode_out + 16);
                if (ipmiSession[index_session].getSessionHandle() == 0x0) {
                  // ipmiSession[index_session].~Ipmisession();
                }
                SerialOverLan::Instance().sol_rdcnt = 0;
                return packet_out;

              } else {
                printf("else exitst");
              }
            } else {
            }
          } else {
            std::vector<uint8_t> ipmi_session_packet_in;
            std::vector<uint8_t> ipmi_session_packet_out;

            //                         ipmi_session_packet_in.resize(this->packet.size()
            //                         - RMCP_HEADER_LENGTH);
            ipmi_session_packet_in.assign(
                this->packet.begin() + RMCP_HEADER_LENGTH, this->packet.end());
            ipmi_session_packet_out =
                ipmi_session_process_packet(ipmi_session_packet_in);

            if (ipmi_session_packet_out.size() > 0) {
              packet_out.assign(ipmi_session_packet_out.begin(),
                                ipmi_session_packet_out.end());
              packet_out.insert(packet_out.begin(), this->packet.begin(),
                                this->packet.begin() + RMCP_HEADER_LENGTH);
            }
            return packet_out;
          }
        }
      }
    }
  }
  return packet_out;
}

std::vector<uint8_t> Rmcppacket::getMgmtSessionId() {
  return this->sessionIdServer;
}

std::vector<uint8_t> Rmcppacket::getClientSessionId() {
  return this->sessionIdClient;
}

std::vector<uint8_t> Rmcppacket::getClientMacAddress() {
  uint8_t mac[6];
  std::vector<uint8_t> macAddr;

  int s;
  struct arpreq areq;
  struct sockaddr_in *sin;
  struct in_addr s_ipaddr;

  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
  }

  memset(&areq, 0, sizeof(areq));
  sin = (struct sockaddr_in *)&areq.arp_pa;
  sin->sin_family = AF_INET;

  sin->sin_addr = this->clientAddr.sin_addr;
  sin = (struct sockaddr_in *)&areq.arp_ha;
  sin->sin_family = ARPHRD_ETHER;

  strncpy(areq.arp_dev, INTF_NAME_ETH0, 15);
  if (ioctl(s, SIOCGARP, (caddr_t)&areq) == -1) {
    strncpy(areq.arp_dev, INTF_NAME_ETH1, 15);
    if (ioctl(s, SIOCGARP, (caddr_t)&areq) == -1) {
      strncpy(areq.arp_dev, INTF_NAME_ETH2, 15);
      if (ioctl(s, SIOCGARP, (caddr_t)&areq) == -1) {
        strncpy(areq.arp_dev, INTF_NAME_ETH3, 15);
        if (ioctl(s, SIOCGARP, (caddr_t)&areq) == -1) {
        }
      }
    }
  }

  uint8_t *ptr = reinterpret_cast<uint8_t *>(areq.arp_ha.sa_data);

  for (int i = 0; i < 6; i++) {
    macAddr.push_back(ptr[i] & 0xff);
  }
  return macAddr;
}

uint32_t Rmcppacket::getClientIpAddress() {
  return this->clientAddr.sin_addr.s_addr;
}

uint16_t Rmcppacket::getClientPort() { return this->clientAddr.sin_port; }

bool Rmcppacket::setRmcpSessionId(std::vector<uint8_t> _clientId,
                                  std::vector<uint8_t> _managedId) {
  this->sessionIdClient.assign(_clientId.begin(), _clientId.end());
  this->sessionIdServer.assign(_managedId.begin(), _managedId.end());

  return true;
}

uint8_t Rmcppacket::getRmcpAuthentication(uint8_t flag) {
  switch (flag) {
  case 1:
    return this->auth_type;
    break;
  case 2:
    return this->integrity_type;
    break;
  case 3:
    return this->confidential_type;
    break;
  }
}

bool Rmcppacket::setRmcpAuthentication(uint8_t _auth, uint8_t _integrity,
                                       uint8_t _confidential) {
  this->auth_type = _auth;
  this->integrity_type = _integrity;
  this->confidential_type = _confidential;

  return true;
}

void Rmcppacket::getRmcpClientAddr() {
  printf("Client Address : %s:%d\n", inet_ntoa(clientAddr.sin_addr),
         clientAddr.sin_port);
}

bool Rmcppacket::setRmcpInformation(std::vector<uint8_t> _InPacket,
                                    struct sockaddr_in _cliAddress) {
  this->packet = _InPacket;
  this->clientAddr = _cliAddress;

  return true;
}

Rmcppacket::Rmcppacket(struct sockaddr_in _cliAddress,
                       std::vector<uint8_t> _packetIn) {
  this->packet = _packetIn;
  this->clientAddr = _cliAddress;
}
