#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__

#include "ipmi/ipmi.hpp"
#include "ipmi/common.hpp"
#include <fstream>
#include <net/if_arp.h>
#include <functional>

#define ETH0_PATH "/sys/devices/platform/ahb/1e660000.ftgmac/net/eth0"
#define ETH1_PATH "/sys/devices/platform/ahb/1e680000.ftgmac/net/eth1"
#define ETH2_PATH "/sys/devices/platform/ahb/1e670000.ftgmac/net/eth2"
#define ETH3_PATH "/sys/devices/platform/ahb/1e690000.ftgmac/net/eth3"

#define SIZE_AUTH_ENABLES 5
#define SIZE_IP_ADDR 4
#define SIZE_IP_ADDR_V6 39
#define SIZE_NET_MASK_V6 6
#define SIZE_MAC_ADDR 6
#define SIZE_NET_MASK 4
#define SIZE_IP_HDR 3
#define SIZE_RMCP_PORT 2
#define SIZE_COMMUNITY_STR 18
#define SIZE_NUM_OF_DEST 5
#define SIZE_DEST_TYPE 4
#define SIZE_DEST_ADDR 18
#define SIZE_VLAN_ID 8
#define SIZE_VLAN_ENABLE 1
#define SIZE_VLAN_PRIORITY 3
#define SIZE_TIME_STAMP 4

/**
 * @brief LAN Configuration parameters (IPMI/Table 23-4)
 * 
 */
enum
{
  LAN_PARAM_SET_IN_PROG,
  LAN_PARAM_AUTH_SUPPORT,
  LAN_PARAM_AUTH_ENABLES,
  LAN_PARAM_IP_ADDR,
  LAN_PARAM_IP_SRC,
  LAN_PARAM_MAC_ADDR,
  LAN_PARAM_NET_MASK,
  LAN_PARAM_IP_HDR,
  LAN_PARAM_PRI_RMCP_PORT,
  LAN_PARAM_SEC_RMCP_PORT,
  LAN_PARAM_ARP_CTRL,
  LAN_PARAM_GARP_INTERVAL,
  LAN_PARAM_DF_GW_IP_ADDR,
  LAN_PARAM_DF_GW_MAC_ADDR,
  LAN_PARAM_BACK_GW_IP_ADDR,
  LAN_PARAM_BACK_GW_MAC_ADDR,
  LAN_PARAM_COMMUNITY_STR,
  LAN_PARAM_NO_OF_DEST,
  LAN_PARAM_DEST_TYPE,
  LAN_PARAM_DEST_ADDR,
  LAN_PARAM_VLAN_ID,
  LAN_PARAM_VLAN_PRIORITY,
  LAN_PARAM_RMCP_CIPHER_SUPPORT,
  LAN_PARAM_RMCP_CIPHERS,
  LAN_PARAM_RMCP_PRIV_LEVELS,
  LAN_PARAM_IPV6_ADDR = 197, /* OEM parameter for IPv6 197 */
  LAN_PARAM_IPV6_NET_MASK,
  LAN_PARAM_IPV6_GATEWAY,
  LAN_PARAM_IPV6_SRC,
  LAN_PARAM_IPV6_ENABLE,
};

class Ipminetwork
{
public:
  std::map<uint8_t, uint8_t> lanConfigChanged;
  bool lockFlag;
  uint8_t chan;
  uint8_t chan_index;
  uint8_t set_enable;
  uint8_t set_enable_v6;
  uint8_t set_in_prog;
  uint8_t auth_support;
  std::vector<uint8_t> auth_enables;
  std::vector<uint8_t> ip_hdr;        // 3
  std::vector<uint8_t> pri_rmcp_port; // 2
  std::vector<uint8_t> sec_rmcp_port; // 2
  uint8_t arp_ctrl;
  uint8_t garp_interval;
  std::vector<uint8_t> df_gw_ip_addr;
  std::vector<uint8_t> df_gw_mac_addr;
  std::vector<uint8_t> back_gw_ip_addr;
  std::vector<uint8_t> back_gw_mac_addr;
  std::string community_str;                        //alert
  uint8_t no_of_dest;                               //alert
  std::vector<uint8_t> dest_type[SIZE_NUM_OF_DEST]; // 4
  std::vector<uint8_t> dest_addr[SIZE_NUM_OF_DEST]; // 4
  uint8_t vlan_enable;
  uint8_t vlan_id;
  uint8_t vlan_priority;

  std::vector<uint8_t> temp_df_gw_ip;
  std::vector<uint8_t> cipher_suite_entry; // 17
  std::vector<uint8_t> cipher_suite_level; // 9
  std::vector<uint8_t> dns_pre_ip;
  std::vector<uint8_t> dns_alt_ip;

  std::vector<uint8_t> ip_addr_v6;
  std::vector<uint8_t> net_mask_v6;
  std::vector<uint8_t> df_gw_ip_addr_v6;
  std::vector<uint8_t> ip_addr;
  uint8_t ip_src;
  uint8_t ip_src_v6;
  std::vector<uint8_t> mac_addr; // 6
  std::vector<uint8_t> net_mask; // 4
  Ipminetwork(uint8_t _chan);
  void plat_network_save(void);

  /**
 * @brief 설정된 SNMP Alert 서버로 snmptrap 메시지를 전송하는 함수\n
 * 사전에 설정된 PEF Policy에 따라 이벤트가 발생한 경우 SNMP 메시지를 전송함.\n
 * @param struct pef_policy_entry entry - PEF Policy Entry
 * @param string alert_string - 전송할 Alert 문자열
 * @return 성공 시 0 또는 1 / 실패 시 -1을 반환
 */
  int send_alert_snmp(struct pef_policy_entry entry, const string alert_string);
  void lanMonitorHandler();
  void get_lan_config(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
  void set_lan_config(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
  void plat_lan_init(void);
  int prepare_alert_snmp(unsigned char policy_set, char *msg);
  

  /**
  * @brief 네트워크 정보가 변경된 이후 ipmitool 에서 정보를 적용 가능하도록 변경하는 함수\n
  * IPMI / REST 요청에 의해 네트워크 정보가 변경된 이후 본 함수를 실행시켜 정보를 적용시킨다. 
  */
  void plat_lan_changed(int eth_num);
};

/**
 * @brief 클라이언트 소켓 생성 함수
 * 본 함수는 BMC 네트워크 정보를 가져오거나 설정하기 위하여 생성하는 클라이언트 소켓 생성 함수입니다.\n
 * BMC 네트워크 정보를 가져오거나 변경 할 때 소켓을 생성하여 소켓 파일 디스크립터를 반환합니다.\n
 * 반환된 파일 디스크립터를 사용하여 MAC 주소, IP 주소, Netmask 주소 등 전반적인 네트워크 정보를 가져오거나 설정할 때 사용합니다.\n
 * 
 * @return int sockfd - 소켓 파일디스크립터
 */
int create_socket();
int get_ip_info(uint8_t index, uint8_t *ip_addr, uint8_t *netmask, uint8_t *gateway, uint8_t *mac_addr);
/**
 * @brief 선택한 채널의 Gateway 주소를 반환하는 함수\n
 * Shared / Dedicated 네트워크의 Gateway 주소를 반환한다.\n
 * @param unsigned char chan - 네트워크 인터페이스 채널 번호
 * @param unsigned char *gateway - Gateway 주소
 */
void get_gateway(unsigned char chan, unsigned char *gateway);
uint8_t getEthernetStatus(uint8_t chan);

/**
 * @brief 네트워크 인터페이스 우선순위를 초기화하는 함수. 기본적으로 Dedicated를 우선순위로 정한다.
 */
unsigned char init_eth_priority();
/**
 * @brief 네트워크 인터페이스 우선순위를 정보를 반환하는 함수.\n
 * 우선순위 파일인 ETH_PRIORITY_CONFIG 파일이 없을 경우 Dedicated를 우선으로 초기화 하며\n
 * 파일이 존재할 경우 파일의 내용을 가져와 반환한다.
 * @return 네트워크 우선순위
 */
unsigned char get_eth_priority();
/**
 * @brief 네트워크 인터페이스 우선순위를 설정하는 함수.\n
 * 입력된 네트워크 인터페이스 번호에 따라 우선순위를 설정한다.\n
 * @param unsigned char dev_num - 설정할 채널 번호
 * @return 0
 */
int set_eth_priority(unsigned char dev_num);
/**
 * @brief 선택한 채널의 IPv6 IP, Prefix, Gateway 주소를 반환하는 함수\n
 * @param unsigned char chan - 네트워크 인터페이스 채널 번호
 * @param unsigned char *ipv6_str_out - IPv6 주소
 * @param unsigned char *prefix_out - IPv6 Prefix 주소
 * @param unsigned char *ipv6_gw_out - IPv6 Gateway 주소
 * @return 실패시 -1을 반환
 */
int get_ipv6_info(unsigned char chan, unsigned char *ipv6_str_out, unsigned char *prefix_out, unsigned char *ipv6_gw_out);

#endif