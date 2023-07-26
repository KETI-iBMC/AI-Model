#include "ipmi/network.hpp"
#include "ipmi/pef_policy.hpp"
#include <redfish/resource.hpp>
#define ETH_PRIORITY_CONFIG "/conf/ipmi/eth_priority"

static std::condition_variable cv;

/**
 * @brief network 정보들을 redfish
 */
void Ipminetwork::plat_network_save(void) {

  // ranmonitering 에게 알림
  this->lockFlag = true;
  cv.notify_all();
  char buf[64] = {
      0,
  };

  cout << "plat_network_save " << endl;

  // if(ch =??? 같은채널이면 NIC 0 ){}
  string _uri;
  if (this->chan_index == 0)
    _uri = ODATA_ETHERNET_INTERFACE_ID + string("/NIC");
  else
    _uri = ODATA_ETHERNET_INTERFACE_ID + string("/NIC") +
           to_string(this->chan_index);

  if (record_is_exist(_uri)) {
    cout << "plat_network_save" << _uri << endl;
    EthernetInterfaces *ether = (EthernetInterfaces *)g_record[_uri];
    if (ip_addr.size() > 2) {
      sprintf(buf, "%u.%u.%u.%u", ip_addr[0], ip_addr[1], ip_addr[2],
              ip_addr[3]);
      if (ether->v_ipv4.size() > 1)
        ether->v_ipv4[0].address = buf;
    }
    memset(buf, 0, sizeof(buf));
    if (df_gw_ip_addr.size() > 2) {
      sprintf(buf, "%d.%d.%d.%d", df_gw_ip_addr[0], df_gw_ip_addr[1],
              df_gw_ip_addr[2], df_gw_ip_addr[3]);
      if (ether->v_ipv4.size() > 1)
        ether->v_ipv4[0].gateway = buf;
    }
    memset(buf, 0, sizeof(buf));
    if (net_mask.size() > 2) {
      sprintf(buf, "%d.%d.%d.%d", net_mask[0], net_mask[1], net_mask[2],
              net_mask[3]);
      if (ether->v_ipv4.size() > 1)
        ether->v_ipv4[0].subnet_mask = buf;
    }
    cout << "plat_network_save" << _uri << endl;
    string s_ip_addr_v6(ip_addr_v6.begin(), ip_addr_v6.end());
    cout << "plat_network_save" << _uri << endl;
    if (ether->v_ipv6.size() > 1)
      ether->v_ipv6[0].address = s_ip_addr_v6;
    string s_gateway(df_gw_ip_addr_v6.begin(), df_gw_ip_addr_v6.end());
    ether->ipv6_default_gateway = s_gateway;
    cout << "save resource uri" << endl;
    resource_save_json(ether);
  }
}

bool getRmcpMacAddr(struct sockaddr_in cliInfo) {

  int s = 0;
  char ip_str[17];
  struct in_addr s_ipAddr;
  struct arpreq areq;
  struct sockaddr_in *sin;

  strcpy(ip_str, inet_ntoa(cliInfo.sin_addr));

  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    return false;
  }

  return true;
}

int create_socket() {
  int sockfd = 0;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    return -1;
  }
  return sockfd;
}

void get_gateway(uint8_t chan, uint8_t *gateway) {
  char cmd[1000] = {0x0};
  uint8_t dev[5] = {0};
  int i;
  sprintf(reinterpret_cast<char *>(dev), "eth%d", chan);
  sprintf(cmd, "route -n | grep %s  | grep \'UG[ \t]\' | awk \'{print $2}\'",
          dev);
  FILE *fp = popen(cmd, "r");
  char line[256] = {0x0};

  if (fgets(line, sizeof(line), fp) != NULL) {
    for (i = 0; i < 1000; i++) {
      if ((line[i] != '.') && (line[i] < '0' || line[i] > '9'))
        break;
    }
    line[i] = 0;
    inet_pton(AF_INET, line, gateway);
  }
  pclose(fp);
}

int set_route(int sockfd, char *gateway_addr, struct sockaddr_in *addr,
              char *dev_name) {

  struct rtentry route;
  struct ifreq ifr;
  int err = 0;
  memset(&route, 0, sizeof(route));
  strncpy(ifr.ifr_name, dev_name, IFNAMSIZ - 1);
  route.rt_dev = ifr.ifr_name;
  addr = (struct sockaddr_in *)&route.rt_gateway;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(gateway_addr);
  addr = (struct sockaddr_in *)&route.rt_dst;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr("0.0.0.0");
  addr = (struct sockaddr_in *)&route.rt_genmask;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr("0.0.0.0");
  route.rt_flags = RTF_UP | RTF_GATEWAY;
  route.rt_metric = 100;

  if ((err = ioctl(sockfd, SIOCADDRT, &route)) < 0) {
    printf("route error\n");
    return -1;
  }
  return 1;
}

int set_ip(char *iface_name, char *ip_addr, char *net_mask,
           char *gateway_addr) {
  struct ifreq ifr;
  struct sockaddr_in sin;
  int sockfd = create_socket();

  sin.sin_family = AF_INET;
  // Convert IP from numbers and dots to binary notation

  /* get interface name */
  strncpy(ifr.ifr_name, iface_name, IFNAMSIZ);

  /* Read interface flags */
  if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
  }

  // If interface is down, bring it up
  if (ifr.ifr_flags | ~(IFF_UP)) {
    ifr.ifr_flags |= IFF_UP;
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
    }
  }
  // Set route
  inet_pton(AF_INET, ip_addr, &sin.sin_addr);
  memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
  // Set interface address
  if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
    {
    }
    perror(ifr.ifr_name);
    return -1;
  }

  inet_pton(AF_INET, net_mask, &sin.sin_addr);
  memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

  if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
    {
    }
    perror(ifr.ifr_name);
    return -1;
  }

#undef IRFFLAGS

  set_route(sockfd, gateway_addr, &sin, iface_name);

  return 0;
}

#if 0
int set_ip(char *iface_name, char *ip_addr, char *net_mask, char *gateway_addr)
{
	if(!iface_name){
		return -1;
	}
	struct ifreq ifr;
	struct sockaddr_in *sin;
	int sockfd = create_socket();
	
	/* get interface name */
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface_name, IFNAMSIZ-1);
    printf("ifr name : %s\n", ifr.ifr_name);
    if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
        printf("SIOCGIFFLAGS ioctl error!\n");

    if(!(ifr.ifr_flags & IFF_UP))
    {
        printf("Interface is down\n");
        ifr.ifr_flags |= IFF_UP;
        if(ioctl(sockfd, SIOCSIFFLAGS, &ifr)<0)
            printf("SIOCSIFFLAGS ioctl error!\n");        
    }
	// Set route
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    inet_pton(AF_INET, ip_addr, &sin->sin_addr);

	// Set interface address
	if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
		printf("Cannot set IP address.\n");
		perror(ifr.ifr_name);
		return -1;
	}  
    printf("IP Address updated sucessfully.\n");

    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    inet_pton(AF_INET, net_mask, &sin->sin_addr);

	if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
		printf("Cannot set Netmask address.\n");
		perror(ifr.ifr_name);
		return -1;
	}
    printf("Netmask Address updated sucessfully.\n");

#undef IRFFLAGS 
	set_route(sockfd, gateway_addr, sin, iface_name);

    close(sockfd);
	return 0;
}
#endif

unsigned char init_eth_priority() {
  cout << "Initializing ethernet priority" << endl;
  FILE *fp = fopen(ETH_PRIORITY_CONFIG, "w");
  unsigned char dev_num = 8;
  fwrite(&dev_num, sizeof(unsigned char), 1, fp);
  fclose(fp);
  return 8;
}

unsigned char get_eth_priority() {
  cout << "Get ethernet priority" << endl;
  unsigned char ret = 0;
  if (access(ETH_PRIORITY_CONFIG, F_OK) != 0) {
    ret = init_eth_priority();
  } else {
    FILE *fp = fopen(ETH_PRIORITY_CONFIG, "r");

    if (fp == NULL) {
      ret = init_eth_priority();
    }
    fread(&ret, sizeof(unsigned char), 1, fp);
    fclose(fp);
  }
  return ret;
}

int set_eth_priority(unsigned char dev_num) {
  FILE *fp = fopen(ETH_PRIORITY_CONFIG, "w");
  fwrite(&dev_num, sizeof(unsigned char), 1, fp);
  fclose(fp);

  if (dev_num == 1)
    system("sed -i \"16s/.*/	\\/sbin\\/ifup eth0/g\" "
           "/etc/init.d/S40network");
  else if (dev_num == 8)
    system("sed -i \"16s/.*/	\\/sbin\\/ifup eth1/g\" "
           "/etc/init.d/S40network");
  return 0;
}

int get_ip_info(uint8_t index, uint8_t *ip_addr, uint8_t *netmask,
                uint8_t *gateway, uint8_t *mac_addr) {
  struct ifreq ifr;
  uint8_t mac_address[6];
  struct sockaddr_in *sin;
  int sockfd = create_socket();
  int i = 0;
  uint8_t dev[5] = {
      0,
  };

  sprintf(reinterpret_cast<char *>(dev), "eth%d", index);
  strncpy(ifr.ifr_name, reinterpret_cast<char *>(dev), IFNAMSIZ);

  if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
  }
  if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
  }

  for (i = 0; i < 6; i++) {
    mac_addr[i] = ifr.ifr_addr.sa_data[i];
  }

  if (ioctl(sockfd, SIOCGIFADDR, &ifr) >= 0) {
    sin = (struct sockaddr_in *)&ifr.ifr_addr;

    for (i = 0; i < 4; i++)
      ip_addr[i] = ifr.ifr_addr.sa_data[i + 2];
  } else {
    for (i = 0; i < 4; i++)
      ip_addr[i] = 0;
  }

  if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) >= 0) {
    sin = (struct sockaddr_in *)&ifr.ifr_addr;

    for (i = 0; i < 4; i++)
      netmask[i] = ifr.ifr_addr.sa_data[i + 2];
  } else {
    for (i = 0; i < 4; i++)
      netmask[i] = 0;
  }

  get_gateway(index, gateway);
  close(sockfd);
  return 0;
}

uint8_t getEthernetStatus(uint8_t chan) {
  std::string filename, state;

  switch (chan) {
  case 1:
    filename = ETH0_PATH;
    break;
  case 6:
    filename = ETH1_PATH;
    break;
  case 7:
    filename = ETH2_PATH;
    break;
  case 8:
    filename = ETH3_PATH;
    break;
  }

  std::ifstream ethfile;

  ethfile.open(filename);
  ethfile >> state;
  if (ethfile.fail() || state == "down")
    return 0;
  else
    return 1;
}

void Ipminetwork::get_lan_config(ipmi_req_t *request, ipmi_res_t *response,
                                 uint8_t *res_len) {
  uint8_t param = request->data[1];
  uint8_t select = request->data[2];
  uint8_t *data = &response->data[0];

  response->cc = CC_SUCCESS;
  *data++ = 0x01;

  switch (param) {
  case LAN_PARAM_SET_IN_PROG:
    *data++ = this->set_in_prog;
    break;
  case LAN_PARAM_AUTH_SUPPORT:
    *data++ = this->auth_support;
    break;
  case LAN_PARAM_AUTH_ENABLES:
    std::copy(this->auth_enables.begin(), this->auth_enables.end(), data);
    data += SIZE_AUTH_ENABLES;
    break;
  case LAN_PARAM_IP_ADDR:
    std::copy(this->ip_addr.begin(), this->ip_addr.end(), data);
    data += SIZE_IP_ADDR;
    break;
  case LAN_PARAM_IP_SRC:
    *data++ = this->ip_src;
    break;
  case LAN_PARAM_MAC_ADDR:
    std::copy(this->mac_addr.begin(), this->mac_addr.end(), data);
    data += SIZE_MAC_ADDR;
    break;
  case LAN_PARAM_NET_MASK:
    std::copy(this->net_mask.begin(), this->net_mask.end(), data);
    data += SIZE_NET_MASK;
    break;
  case LAN_PARAM_IP_HDR:
    std::copy(this->ip_hdr.begin(), this->ip_hdr.end(), data);
    data += SIZE_IP_HDR;
    break;
  case LAN_PARAM_PRI_RMCP_PORT:
    std::copy(this->pri_rmcp_port.begin(), this->pri_rmcp_port.end(), data);
    data += SIZE_RMCP_PORT;
    break;
  case LAN_PARAM_SEC_RMCP_PORT:
    std::copy(this->sec_rmcp_port.begin(), this->sec_rmcp_port.end(), data);
    data += SIZE_RMCP_PORT;
    break;
  case LAN_PARAM_ARP_CTRL:
    *data++ = this->arp_ctrl;
    break;
  case LAN_PARAM_GARP_INTERVAL:
    *data++ = this->garp_interval;
    break;
  case LAN_PARAM_DF_GW_IP_ADDR:
    std::copy(this->df_gw_ip_addr.begin(), this->df_gw_ip_addr.end(), data);
    data += SIZE_IP_ADDR;
    break;
  case LAN_PARAM_DF_GW_MAC_ADDR:
    std::copy(this->df_gw_mac_addr.begin(), this->df_gw_mac_addr.end(), data);
    data += SIZE_MAC_ADDR;
    break;
  case LAN_PARAM_BACK_GW_IP_ADDR:
    std::copy(this->back_gw_ip_addr.begin(), this->back_gw_ip_addr.end(), data);
    data += SIZE_IP_ADDR;
    break;
  case LAN_PARAM_BACK_GW_MAC_ADDR:
    std::copy(this->back_gw_mac_addr.begin(), this->back_gw_mac_addr.end(),
              data);
    data += SIZE_MAC_ADDR;
    break;
  case LAN_PARAM_COMMUNITY_STR:
    std::copy(this->community_str.begin(), this->community_str.end(), data);
    data += this->community_str.length() + 1;
    break;
  case LAN_PARAM_NO_OF_DEST:
    *data++ = this->no_of_dest;
    break;
  case LAN_PARAM_DEST_TYPE:
    std::copy(this->dest_type[select].begin(), this->dest_type[select].end(),
              data);
    // plat_lan_alert_save();
    data += SIZE_DEST_TYPE;
    break;
  case LAN_PARAM_DEST_ADDR:
    std::copy(this->dest_addr[select].begin(), this->dest_addr[select].end(),
              data);
    // plat_lan_alert_save();
    data += SIZE_DEST_ADDR;
    break;
  case LAN_PARAM_VLAN_ID:
    *data++ = this->vlan_id;
    *data++ = this->vlan_enable;
    break;
  case LAN_PARAM_VLAN_PRIORITY:
    *data++ = this->vlan_priority;
    break;
  case LAN_PARAM_RMCP_CIPHER_SUPPORT:
    *data++ = 4;
    break;
  case LAN_PARAM_RMCP_CIPHERS:
    *data++ = 0xff;
    *data++ = 1;
    *data++ = 3;
    *data++ = 5;
    *data++ = 7;
    break;
  case LAN_PARAM_RMCP_PRIV_LEVELS:
    *data++ = 0xff;
    *data++ = 0x44;
    *data++ = 0x44;
    *data++ = 0x44;
    *data++ = 0x44;
    *data++ = 0x44;
    *data++ = 0x44;
    *data++ = 0x44;
    *data++ = 0x44;
    break;
  default:
    response->cc = CC_UNKNOWN;
    break;
  }
  *res_len = data - &response->data[0];
}

void Ipminetwork::set_lan_config(ipmi_req_t *request, ipmi_res_t *response,
                                 uint8_t *res_len) {
  uint8_t param = request->data[1];
  uint8_t data = request->data[2];

  response->cc = CC_SUCCESS;

  switch (param) {
  case LAN_PARAM_SET_IN_PROG:
    switch (data & 0x0f) {
    case 0:
      this->set_in_prog = data;
      break;
    case 1:
      if (this->set_in_prog) {
        response->cc = 0x81;
      } else {
        this->set_in_prog = data;
      }
      break;
    case 2:
      response->cc = 0x81;
      break;
    case 3:
      response->cc = CC_INVALID_PARAM;
      break;
    default:
      response->cc = CC_SUCCESS;
      break;
    }
    break;

  case LAN_PARAM_AUTH_SUPPORT:
    this->auth_support = data & 0x3F;
    break;

  case LAN_PARAM_AUTH_ENABLES:
    this->auth_enables.assign(request->data + 2, request->data + 7);
    break;

  case LAN_PARAM_IP_ADDR:
    if ((request->data[2] == 0) && (request->data[3] == 0) &&
        (request->data[4] == 0) && (request->data[5] == 0))
      response->cc = CC_PARAM_OUT_OF_RANGE;
    else if ((request->data[2] == 0xff) && (request->data[3] == 0xff) &&
             (request->data[4] == 0xff) && (request->data[5] == 0xff))
      response->cc = CC_PARAM_OUT_OF_RANGE;
    else {
      lanConfigChanged[param] = 1;
      this->lockFlag = true;
      cv.notify_all();
      this->ip_addr.assign(request->data + 2, request->data + 6);
    }

    break;

  case LAN_PARAM_IP_SRC:
    lanConfigChanged[param] = 1;
    this->lockFlag = true;
    this->ip_src = data & 0xF;
    cv.notify_all();
    break;

  case LAN_PARAM_MAC_ADDR:
    lanConfigChanged[param] = 1;
    this->lockFlag = true;
    this->mac_addr.assign(request->data + 2, request->data + 8);
    cv.notify_all();
    break;

  case LAN_PARAM_NET_MASK:
    if ((request->data[2] == 0xff) && (request->data[3] == 0xff) &&
        (request->data[4] == 0xff) && (request->data[5] == 0xff)) {
      response->cc = CC_INVALID_PARAM;
      break;
    }
    lanConfigChanged[param] = 1;
    this->lockFlag = true;
    this->net_mask.assign(request->data + 2, request->data + 6);
    cv.notify_all();
    break;

  case LAN_PARAM_IP_HDR:
    this->ip_hdr.assign(request->data + 2, request->data + 5);
    break;

  case LAN_PARAM_ARP_CTRL:
    this->arp_ctrl = data;
    break;

  case LAN_PARAM_GARP_INTERVAL:
    this->garp_interval = data;
    break;

  case LAN_PARAM_DF_GW_IP_ADDR:
    if (((request->data[2] == 0) && (request->data[3] == 0) &&
         (request->data[4] == 0) && (request->data[5] == 0)) ||
        ((request->data[2] == 0xff) && (request->data[3] == 0xff) &&
         (request->data[4] == 0xff) && (request->data[5] == 0xff))) {
      response->cc = CC_INVALID_PARAM;
      break;
    }
    lanConfigChanged[param] = 1;
    this->lockFlag = true;
    this->df_gw_ip_addr.assign(request->data + 2, request->data + 6);
    cv.notify_all();
    break;

  case LAN_PARAM_DF_GW_MAC_ADDR:
  case LAN_PARAM_BACK_GW_IP_ADDR:
  case LAN_PARAM_BACK_GW_MAC_ADDR:
    response->cc = CC_INVALID_PARAM;
    break;

  case LAN_PARAM_COMMUNITY_STR:
    this->community_str.assign(request->data + 2, request->data + 20);
    break;

  case LAN_PARAM_NO_OF_DEST:
    this->no_of_dest = data;
    break;

  case LAN_PARAM_DEST_TYPE:
    this->dest_type[data].assign(request->data + 2, request->data + 6);
    break;

  case LAN_PARAM_DEST_ADDR:
    this->dest_addr[data].assign(request->data + 4, request->data + 8);
    break;

  // case LAN
  default:
    response->cc = CC_INVALID_PARAM;
    break;
  }
  plat_network_save();
}

void Ipminetwork::lanMonitorHandler() {

  printf("Lan monitor Thread is running : %d\n", this->chan);
  std::mutex m;

  while (1) {
    std::unique_lock<std::mutex> lk(m);
    bool *lock = &this->lockFlag;
    cv.wait(lk, [&] { return this->lockFlag; });
    printf("Unlock thread : %d\n", this->chan);
    std::this_thread::sleep_for(std::chrono::seconds(5)); // wait 5 seconds;

    char dev[5] = {
        0,
    };
    char command[128] = {
        0,
    };
    char str_ip[20] =
        {
            0,
        },
         str_net[20] =
             {
                 0,
             },
         str_gw[20] = {
             0,
         };
    uint8_t *b_ip = new uint8_t[4], *b_netmask = new uint8_t[4],
            *b_gw = new uint8_t[4], *b_mac = new uint8_t[6];
    uint8_t *t_ip = new uint8_t[4], *t_netmask = new uint8_t[4],
            *t_gw = new uint8_t[4], *t_mac = new uint8_t[6];

    printf("lan monitor thread() / IPSRC : %d / IPADDR : %d / NETMASK : %d\n",
           this->lanConfigChanged[LAN_PARAM_IP_SRC],
           this->lanConfigChanged[LAN_PARAM_IP_ADDR],
           this->lanConfigChanged[LAN_PARAM_NET_MASK]);

    get_ip_info(this->chan_index, b_ip, b_netmask, b_gw,
                b_mac); // get current network configuration

    if (this->lanConfigChanged[LAN_PARAM_IP_SRC] == 1 && this->ip_src == 1) {
      if (this->lanConfigChanged[LAN_PARAM_IP_ADDR] == 1)
        std::copy(this->ip_addr.begin(), this->ip_addr.end(), t_ip);
      else
        t_ip = b_ip;
      if (this->lanConfigChanged[LAN_PARAM_NET_MASK] == 1)
        std::copy(this->net_mask.begin(), this->net_mask.end(), t_netmask);
      else
        t_netmask = b_netmask;
      if (this->lanConfigChanged[LAN_PARAM_DF_GW_IP_ADDR] == 1)
        std::copy(this->df_gw_ip_addr.begin(), this->df_gw_ip_addr.end(), t_gw);
      else
        t_gw = b_gw;

      for (int i = 0; i < 4; i++) {
        if (i != this->chan_index) {
          sprintf(command, "ifconfig eth%d down", i);
          system(command);
        }
      }

      sprintf(dev, "eth%d", this->chan_index);
      sprintf(str_ip, "%d.%d.%d.%d", t_ip[0], t_ip[1], t_ip[2], t_ip[3]);
      sprintf(str_net, "%d.%d.%d.%d", t_netmask[0], t_netmask[1], t_netmask[2],
              t_netmask[3]);
      sprintf(str_gw, "%d.%d.%d.%d", t_gw[0], t_gw[1], t_gw[2], t_gw[3]);
      set_ip(dev, str_ip, str_net, str_gw);
      // sprintf(command, "ifconfig %s %s netmask %s up", dev, str_ip, str_net);
      // printf("command : %s\n", command);
      // system(command);
      // sprintf(command, "route add default gw %s %s", str_gw, dev);
      // printf("command : %s\n", command);
      // system(command);

      if (this->lanConfigChanged[LAN_PARAM_MAC_ADDR] == 1) {
        std::copy(this->mac_addr.begin(), this->mac_addr.end(), t_mac);

        sprintf(command, "ifconfig eth%d down", this->chan_index);
        system(command);

        sprintf(command,
                "ifconfig eth%d hw ether %02X:%02X:%02X:%02X:%02X:%02X",
                this->chan_index, this->mac_addr[0], this->mac_addr[1],
                this->mac_addr[2], this->mac_addr[3], this->mac_addr[4],
                this->mac_addr[5]);
        system(command);

        sprintf(command, "ifconfig eth%d up", this->chan_index);
        system(command);

        sprintf(command, "route add default gw %d.%d.%d.%d eth%d",
                this->df_gw_ip_addr[0], this->df_gw_ip_addr[1],
                this->df_gw_ip_addr[2], this->df_gw_ip_addr[3],
                this->chan_index);
        system(command);
      }
    } else if (this->lanConfigChanged[LAN_PARAM_IP_SRC] == 1 &&
               this->ip_src == 2) {
      for (int i = 0; i < 4; i++) {
        if (i != this->chan_index) {
          sprintf(command, "ifconfig eth%d down", i);
          system(command);
        }
      }

      if (this->lanConfigChanged[LAN_PARAM_MAC_ADDR] == 1) {
        std::copy(this->mac_addr.begin(), this->mac_addr.end(), t_mac);

        sprintf(command, "ifconfig eth%d down", this->chan_index);
        system(command);

        sprintf(command,
                "ifconfig eth%d hw ether %02X:%02X:%02X:%02X:%02X:%02X",
                this->chan_index, this->mac_addr[0], this->mac_addr[1],
                this->mac_addr[2], this->mac_addr[3], this->mac_addr[4],
                this->mac_addr[5]);
        system(command);

        sprintf(command, "ifconfig eth%d up", this->chan_index);
        system(command);

        sprintf(command, "route add default gw %d.%d.%d.%d eth%d",
                this->df_gw_ip_addr[0], this->df_gw_ip_addr[1],
                this->df_gw_ip_addr[2], this->df_gw_ip_addr[3],
                this->chan_index);
        system(command);
      }

      sprintf(command, "udhcpc -i eth%d -t 3 -T 3 -q -n", this->chan_index);
      system(command);
    }

    this->lanConfigChanged[LAN_PARAM_IP_SRC] = 0;
    this->lanConfigChanged[LAN_PARAM_IP_ADDR] = 0;
    this->lanConfigChanged[LAN_PARAM_NET_MASK] = 0;
    this->lanConfigChanged[LAN_PARAM_DF_GW_IP_ADDR] = 0;
    this->lanConfigChanged[LAN_PARAM_MAC_ADDR] = 0;

    this->lockFlag = false;
  }
}
/**
 * @author 기철
 * @todo channel 기본 값 기존 없음에서 0으로 수정
 */
Ipminetwork::Ipminetwork(uint8_t _chan) {
  uint8_t r_chan = 0;
  uint8_t ipAddr[4] =
      {
          0,
      },
          netMask[4] =
              {
                  0,
              },
          macAddr[6] =
              {
                  0,
              },
          gwAddr[4] = {
              0,
          };
  uint8_t alert_addr[4] = {10, 0, 0, 124};

  this->ip_addr_v6.reserve(SIZE_IP_ADDR_V6);
  this->net_mask_v6.reserve(SIZE_NET_MASK_V6);
  this->df_gw_ip_addr_v6.reserve(SIZE_IP_ADDR_V6);

  this->set_in_prog = 0;
  this->set_enable = getEthernetStatus(_chan);
  this->set_enable_v6 = 0;
  this->auth_support = 0b011101; // Password, MD5, None Support Default
  this->chan = _chan;

  for (int i = 0; i < 4; i++)
    this->auth_enables.push_back(auth_support);

  this->ip_src = 0; // 0 unspecified, 1 static, 2 dhcp
  this->ip_src_v6 = 0;

  switch (_chan) {
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
  default:
    r_chan = 0;
  }
  this->chan_index = r_chan;
  get_ip_info(r_chan, ipAddr, netMask, gwAddr, macAddr);

  this->ip_addr.assign(ipAddr, ipAddr + 4);
  this->net_mask.assign(netMask, netMask + 4);
  this->df_gw_ip_addr.assign(gwAddr, gwAddr + 4);
  this->mac_addr.assign(macAddr, macAddr + 6);

  this->ip_hdr.push_back(0x40);
  this->ip_hdr.push_back(0x40);
  this->ip_hdr.push_back(0x10);

  this->pri_rmcp_port.push_back(663 & 0x00ff);
  this->pri_rmcp_port.push_back(666 / 255);
  this->sec_rmcp_port.push_back(0x98);
  this->sec_rmcp_port.push_back(0x02);

  this->arp_ctrl = 0x02;

  this->no_of_dest = 5;

  for (int j = 0; j < no_of_dest; j++)
    dest_type[j].push_back(j);
  dest_addr[0].push_back(0);
  dest_addr[0].insert(dest_addr[0].end(), std::begin(alert_addr),
                      std::end(alert_addr));
  this->community_str = "KETI";

  this->lanConfigChanged.insert(
      std::make_pair<uint8_t, uint8_t>(LAN_PARAM_IP_SRC, 0));
  this->lanConfigChanged.insert(
      std::make_pair<uint8_t, uint8_t>(LAN_PARAM_IP_ADDR, 0));
  this->lanConfigChanged.insert(
      std::make_pair<uint8_t, uint8_t>(LAN_PARAM_MAC_ADDR, 0));
  this->lanConfigChanged.insert(
      std::make_pair<uint8_t, uint8_t>(LAN_PARAM_NET_MASK, 0));
  this->lanConfigChanged.insert(
      std::make_pair<uint8_t, uint8_t>(LAN_PARAM_IP_SRC, 0));

  this->lockFlag = false;
}

int get_ipv6_info(unsigned char chan, unsigned char *ipv6_str_out,
                  unsigned char *prefix_out, unsigned char *ipv6_gw_out) {
  char cmds[200];
  char ipv6_gw[20];
  char buff[BUFF_SIZE];
  char ipv6_str[40];
  unsigned char addr6_str[40];
  unsigned char ipv6_hex[16];
  struct in6_addr st_addr6;
  int i = 0;
  unsigned char pref = 0;
  unsigned char tmps = '/';

  if (chan == 1) {
    sprintf(cmds, "ifconfig %s | grep inet6", DEV_NAME_SHARED);
    FILE *op = popen(cmds, "r");
    if (op != NULL) {
      char text[200] = {0};
      fgets(text, sizeof(text), op);
      pclose(op);
      if (strlen(text) != 0) {
        sprintf(cmds,
                "ifconfig %s | grep inet6 | awk \'{print $3}\' | awk -F \'%c\' "
                "\'{print $1}\'",
                DEV_NAME_SHARED, tmps);
        FILE *pp = popen(cmds, "r");

        sprintf(cmds, "route -A inet6 | grep %s | awk \'{print $1}\'",
                DEV_NAME_SHARED);
        FILE *gp = popen(cmds, "r");

        sprintf(cmds,
                "ifconfig %s | grep inet6 | awk \'{print $3}\' | awk -F \'%c\' "
                "\'{print $2}\' | awk -F \'/\' \'{print $2}\'",
                DEV_NAME_SHARED, tmps);
        FILE *cp = popen(cmds, "r");

        if (pp == NULL) {
          fprintf(stderr, "pp popen() failed\n");
          return -1;
        }
        if (gp == NULL) {
          fprintf(stderr, "gp popen() failed\n");
          return -1;
        }
        if (cp == NULL) {
          fprintf(stderr, "cp popen() failed\n");
          return -1;
        }
        fgets(ipv6_gw, 20, gp);
        ipv6_gw[strlen(ipv6_gw)] = '\0';
        fgets(buff, BUFF_SIZE, pp);
        buff[strlen(buff)] = '\0';
        // printf("in network.cpp ==\n ip : %s, size : %d\n gw : %s, size :
        // %d\n", buff, strlen(buff), ipv6_gw, strlen(ipv6_gw));
        memcpy((char *)ipv6_str_out, buff, sizeof(ipv6_str));
        memcpy((char *)ipv6_gw_out, ipv6_gw, sizeof(unsigned char) * 20);

        memset(buff, 0, sizeof(buff));
        fgets(buff, BUFF_SIZE, cp);

        // printf("netmask : %s, size : %d\n", buff, strlen(buff));
        buff[strlen(buff)] = '\0';
        memcpy((char *)prefix_out, buff,
               sizeof(unsigned char) * SIZE_NET_MASK_V6);

        pclose(pp);
        pclose(gp);
        pclose(cp);
      } else {
        strcpy((char *)ipv6_str_out, "");
        strcpy((char *)ipv6_gw_out, "");
        strcpy((char *)prefix_out, "");
      }
    }
  } else if (chan == 8) {
    sprintf(cmds, "ifconfig %s | grep inet6", DEV_NAME_DEDI);

    FILE *op = popen(cmds, "r");
    if (op != NULL) {
      char text[200] = {0};
      fgets(text, sizeof(text), op);
      pclose(op);
      if (strlen(text) != 0) {

        sprintf(cmds,
                "ifconfig %s | grep inet6 | awk \'{print $3}\' | awk -F \'%c\' "
                "\'{print $1}\'",
                DEV_NAME_DEDI, tmps);
        FILE *pp = popen(cmds, "r");

        sprintf(cmds, "route -A inet6 | grep %s | awk \'{print $1}\'",
                DEV_NAME_DEDI);
        FILE *gp = popen(cmds, "r");

        sprintf(cmds,
                "ifconfig %s | grep inet6 | awk \'{print $3}\' | awk -F \'%c\' "
                "\'{print $2}\' | awk -F \'/\' \'{print $2}\'",
                DEV_NAME_DEDI, tmps);
        FILE *cp = popen(cmds, "r");

        if (pp == NULL) {
          fprintf(stderr, "popen() failed");
          return -1;
        }
        if (gp == NULL) {
          fprintf(stderr, "popen() failed");
        }
        if (cp == NULL) {
          fprintf(stderr, "popen() failed");
        }
        fgets(ipv6_gw, 20, gp);
        ipv6_gw[strlen(ipv6_gw)] = '\0';
        fgets(buff, BUFF_SIZE, pp);
        buff[strlen(buff)] = '\0';
        pclose(pp);
        pclose(gp);
        pclose(cp);

        memcpy((char *)ipv6_str_out, buff, sizeof(ipv6_str));
        memcpy((char *)ipv6_gw_out, ipv6_gw, sizeof(unsigned char) * 20);
        memset(buff, 0, sizeof(buff));
        fgets(buff, BUFF_SIZE, cp);
        buff[strlen(buff)] = '\0';
        memcpy((char *)prefix_out, buff,
               sizeof(unsigned char) * SIZE_NET_MASK_V6);
      }
    } else {
      strcpy((char *)ipv6_str_out, "");
      strcpy((char *)ipv6_gw_out, "");
      strcpy((char *)prefix_out, "");
    }
  }

  return 1;
}

void Ipminetwork::plat_lan_changed(int eth_num) {
  uint8_t ip_addr[6];
  uint8_t net_mask[6];
  uint8_t gateway[6];
  uint8_t mac_addr[6];

  get_ip_info(eth_num, ip_addr, net_mask, gateway, mac_addr);

  if (ip_addr[0] == 0x00) {
    std::fill(this->ip_addr.begin(), this->ip_addr.end(), 0);
    std::fill(this->net_mask.begin(), this->net_mask.end(), 0);
    std::fill(this->df_gw_ip_addr.begin(), this->df_gw_ip_addr.end(), 0);
  } else {
    memcpy(&(this->ip_addr[0]), ip_addr, 4);
    memcpy(&(this->net_mask[0]), net_mask, 4);
    memcpy(&(this->df_gw_ip_addr[0]), gateway, 4);
    memcpy(&(this->mac_addr[0]), mac_addr, 6);
  }

  return;
}
uint8_t parse_alert_num(uint8_t chan_dest) {
  uint8_t alert_num;
  alert_num = chan_dest % 0x10;
  // sprintf(alert_num, "%x", chan_dest % 0x10);
  return alert_num;
}
// uint8_t parse_policy_set(uint8_t policy) {
// 	uint8_t policy_set ;
// 	if (policy == 0x00) {

// 		return 0;
// 	}
// 	policy_set = policy / 0x10;

// 	return policy_set;
// }
int Ipminetwork::send_alert_snmp(struct pef_policy_entry entry,
                                 const string alert_string) {
  uint8_t policy_set = parse_policy_set(entry.policy);
  uint8_t alert_num = parse_alert_num(entry.chan_dest);
  if (policy_set == 0) {
    return 0;
  }
  this->dest_addr[alert_num];
}

// return  0: success & terminate
// return  1: success & continue
// return -1:   fail  & continue
//
// int send_alert_snmp(struct pef_policy_entry entry, char* alert_string) {

// 	uint8_t snmp_priority = get_eth_priority();
// 	uint8_t chan = plat_lan_channel_selector(snmp_priority);
// 	uint8_t alert_num = parse_alert_num(entry.chan_dest);
// 	uint8_t policy_set = parse_policy_set(entry.policy);
// 	if (policy_set == 0) {
// 		return 0;
// 	}

// 	uint8_t* dest_addr = g_lan_config[chan].dest_addr[alert_num];
// 	uint8_t str_key = entry.alert_string_key;
// 	uint8_t rule = entry.policy % 0x10;
// 	if (rule == 0) {
// 		return -1;
// 	}

// 	uint8_t cmd[128];
// 	uint8_t ip[13];
// 	memcpy(ip, g_lan_config[chan].dest_addr[alert_num]+1,
// sizeof(uint8_t)*4);

// 	sprintf(cmd, "snmptrap -v 2c -Ci -c %s %d.%d.%d.%d \"\" ucdStart
// sysContact.0 s \"%s\"", g_lan_config[chan].community_str,
// ip[0], ip[1], ip[2], ip[3], alert_string); 	int ret = system(cmd); if (ret
// != 0) { 		printf("%s\n", "To send snmptrap message failed. Move to
// the next policy"); 		return -1;
// 	}
// 	else {
// 		if (rule > 8) {
// 			printf("%s\n", "snmptrap success. Move to the next
// policy"); 			return 1;
// 		}
// 		else {
// 			printf("%s\n", "snmptrap success. Terminate the
// routine."); 			return 0;
// 		}
// 	}
// }
// int prepare_alert_snmp(unsigned char policy_set, char* msg) {
// 	struct pef_policy_entry matching_entries[8];
// 	memset(matching_entries, 0, sizeof(struct pef_policy_entry)*8);
// 	int count = get_entry_by_policy_set(policy_set, matching_entries);
// 	int ret = 0;
// 	int i = 0;
// 	if (count == 0) {

// 		return -1;
// 	}
// 	else {
// 		for (i=0; i<count; i++) {
// 			ret = send_alert_snmp(matching_entries[i], msg);
// 			if (ret == 0)
// 				break;
// 		}
// 	}
// 	if (ret == -1) {
// 		printf("%s\n", "[ERROR] To send alert failed");
// 		return -1;
// 	}
// }