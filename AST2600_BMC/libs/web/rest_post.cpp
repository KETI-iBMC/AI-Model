#include "ipmi/ipmi.hpp"
#include "ipmi/network.hpp"
#include "ipmi/rest_get.hpp"
#include "redfish/resource.hpp"
#include <cstdint>
#include <cstdlib>
#include <ipmi/rest_post.hpp>
#include <sys/types.h>
extern Ipmiuser ipmiUser[10];
extern unordered_map<string, Resource *> g_record;


extern TempWebValues temporary;

int Ipmiweb_POST::Try_Login(string username , string pwd){

  int response = 0;

	log(info) << "[try login] username : " << username;
  log(info) << "[try login] password : " << pwd;

  if (response == NULL) {
    response = authenticate_ipmi(username, pwd);
  }
  if (response == NULL) {
    response = authenticate_ldap(username, pwd);
  }
  if (response == NULL) {
    response = authenticate_ad(username, pwd);
  }

	if (ipmiUser[0].getUsername() == username){
      if (ipmiUser[0].getUserpassword() == pwd)
        {
			sprintf(response, "%d", 4);
			cout << response << endl;
		}
	}

	return response ;
};



void Ipmiweb_POST::Set_Ddns(json::value request_json){
  json::value myobj, orgobj, orggen, orgipv4, orgipv6, orgval;

      uint8_t cmds[300], response[QSIZE], res_msg[QSIZE];
      uint8_t header[1000];
      uint8_t data[1000] = {0};
      int res_len = 0;

      // [테스트] dns 임시 값 처리
      json::value jv_dns_info, jv_generic, jv_ipv4, jv_ipv6;
      string domain_name, host_name, register_bmc, register_bmc_method;
      string v4_prefer, v4_alter, v6_prefer, v6_alter;

      // cout << " [INPUT JSON] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << endl;
      // cout << request_json << endl;

      get_value_from_json_key(request_json, "DNS_INFO", jv_dns_info);

      get_value_from_json_key(jv_dns_info, "GENERIC", jv_generic);
      get_value_from_json_key(jv_dns_info, "IPV4", jv_ipv4);
      get_value_from_json_key(jv_dns_info, "IPV6", jv_ipv6);

      get_value_from_json_key(jv_generic, "DOMAIN_NAME", domain_name);
      get_value_from_json_key(jv_generic, "HOST_NAME", host_name);
      get_value_from_json_key(jv_generic, "REGISTER_BMC", register_bmc);
      get_value_from_json_key(jv_generic, "REGISTER_BMC_METHOD", register_bmc_method);

      get_value_from_json_key(jv_ipv4, "IPV4_PREFERRED", v4_prefer);
      get_value_from_json_key(jv_ipv4, "IPV4_ALTERNATE", v4_alter);

      get_value_from_json_key(jv_ipv6, "IPV6_PREFERRED", v6_prefer);
      get_value_from_json_key(jv_ipv6, "IPV6_ALTERNATE", v6_alter);

      string eth_str = "/redfish/v1/Managers/EthernetInterfaces/NIC";
      EthernetInterfaces *eth = (EthernetInterfaces *)g_record[eth_str];

      eth->hostname = host_name;
      eth->fqdn = domain_name;
      temporary.dns_registerBMC = register_bmc;
      temporary.dns_registerBMCMethod = register_bmc_method;

      try
      {
        /* code */
        eth->name_servers[0] = v4_prefer;
        eth->name_servers[1] = v4_alter;
        eth->name_servers[2] = v6_prefer;
        eth->name_servers[3] = v6_alter;
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      
      

      





    // [테스트] 원본(함수끝까지)
    // // get_value_from_json_key(request_json, "ID", id);
    //   string org_host_name, org_domain_name, org_ipv4_preferred, org_ipv4_alter,
    //       org_ipv6_preferred;
    //   // string srsp((char *)response);
    //   // cout << "sdata in dns_call : " << srsp << endl;
    //   // orgobj = json::value::parse(srsp);
    //   cout << "respons json start" <<endl;
    //   json::value response_json;
    //   Ipmiweb_GET::Get_DDNS_Info(response_json);
    //   cout << response_json <<endl;
    //   cout << "respons json dns info "<<endl;
    //   orgval = response_json.at("DNS_INFO");
    //   orggen = orgval.at("GENERIC");
    //   cout << "orgval " <<orgval <<endl;
    //   org_host_name = orggen.at("HOST_NAME").as_string();
    //   org_domain_name = orggen.at("DOMAIN_NAME").as_string();

    //   orgipv4 = orgval.at("IPV4");
    //   org_ipv4_preferred = orgipv4.at("IPV4_PREFERRED").as_string();
    //   org_ipv4_alter = orgipv4.at("IPV4_ALTERNATE").as_string();

    //   orgipv6 = orgval.at("IPV6");
    //   org_ipv6_preferred = orgipv6.at("IPV6_PREFERRED").as_string();

    //   string new_host_name, new_domain_name, new_ipv4_preferred,
    //       new_ipv4_alter, new_ipv6_preferred;
      
    //   json::value new_dns_obj,new_gen_obj,new_ipv4,new_ipv6;

    //   cout << "json re start1"<< request_json<<endl;
    //   new_dns_obj = request_json.at("DNS_INFO");
    //      cout << "json re start2"<< request_json<<endl;
    //   new_gen_obj = new_dns_obj.at("GENERIC");
    //      cout << "json re start3"<< request_json<<endl;
    //   new_ipv4 = new_dns_obj.at("IPV4");
    //      cout << "json re start4"<< request_json<<endl;
    //   new_ipv6 = new_dns_obj.at("IPV6");
    //      cout << "json re start5"<< request_json<<endl;
    //   cout << "host name " <<endl;
    //   new_host_name = new_gen_obj.at("HOST_NAME").as_string();
    //   cout << "domain request json" <<endl;
    //   new_domain_name = new_gen_obj.at("DOMAIN_NAME").as_string();
    //   cout << "ipv4 pre request json" <<endl;
    //   new_ipv4_preferred = new_ipv4.at("IPV4_PREFERRED").as_string();
    //   new_ipv4_alter = new_ipv4.at("IPV4_ALTERNATE").as_string();
    //   new_ipv6_preferred = new_ipv6.at("IPV6_PREFERRED").as_string();
    //   cout << "set json " <<endl;
    //   int rets = 0;
    //   if ((new_host_name != "-")  &&
    //       (org_host_name != new_host_name)) {
    //     rets = set_ddns_host_name(new_host_name.c_str());
    //   }

    //   if ((new_domain_name != "-")  &&
    //       (org_domain_name != new_domain_name) ) {
    //     rets = set_ddns_domain_name(2,new_domain_name.c_str());
    //   }

    //   if ((new_ipv4_preferred != "-") &&
    //       (org_ipv4_preferred != new_ipv4_preferred)) {
    //     rets = set_ddns_nameserver(1,new_ipv4_preferred.c_str());
    //   }

    //   if ((new_ipv4_alter != "-") &&
    //       (org_ipv4_alter != new_ipv4_alter)) {
    //     rets = set_ddns_nameserver(1,new_ipv4_alter.c_str());
    //   }

    //   if ((new_ipv6_preferred != "-")  &&
    //       (org_ipv6_preferred != new_ipv6_preferred)) {
    //     rets = set_ddns_nameserver(1,new_ipv6_preferred.c_str());
    //   }
}
int post_cmdline_macaddr(char *arg, uint8_t *buf) {
    uint32_t m1 = 0;
    uint32_t m2 = 0;
    uint32_t m3 = 0;
    uint32_t m4 = 0;
    uint32_t m5 = 0;
    uint32_t m6 = 0;
    if (sscanf(arg, "%02x:%02x:%02x:%02x:%02x:%02x", &m1, &m2, &m3, &m4, &m5,
            &m6) != 6) {
        return -1;
    }
    if (m1 > UINT8_MAX || m2 > UINT8_MAX || m3 > UINT8_MAX || m4 > UINT8_MAX
            || m5 > UINT8_MAX || m6 > UINT8_MAX) {
        return -1;
    }
    buf[0] = (uint8_t) m1;
    buf[1] = (uint8_t) m2;
    buf[2] = (uint8_t) m3;
    buf[3] = (uint8_t) m4;
    buf[4] = (uint8_t) m5;
    buf[5] = (uint8_t) m6;
    return 0;
}
void Ipmiweb_POST::Set_Network(json::value request_json){
  //  int changed = 0;

  //     json::value myobj, ninfo, laninf, generic, ipv4, ipv6, vlan;

  //     // uint8_t response[QSIZE], res_msg[QSIZE];
  //     uint8_t header[1000], mac_buf[32];
  //     int res_len = 0;
  //     json::value response_json;
      
  //     string org_network_priority, org_lan_setting_enable, org_lan_interface,
  //         org_mac_address, org_ipv4_preferred, org_ipv4_dhcp_enable,
  //         org_ipv4_address, org_ipv4_netmask, org_ipv4_gateway,
  //         org_ipv6_subnet_prefix_length, org_ipv6_address, org_ipv6_enable,
  //         org_ipv6_dhcp_enable, org_ipv6_gateway, org_vlan_settings_enable,
  //         org_vlan_id, org_vlan_priority;

  //     string new_network_priority, new_lan_setting_enable,
  //         new_lan_interface, new_mac_address, new_ipv4_preferred,
  //         new_ipv4_dhcp_enable, new_ipv4_address,
  //         new_ipv4_netmask, new_ipv4_gateway,
  //         new_ipv6_subnet_prefix_length, new_ipv6_address,
  //         new_ipv6_enable, new_ipv6_dhcp_enable, new_ipv6_gateway,
  //         new_vlan_settings_enable, new_vlan_id,
  //         new_vlan_priority;

            
  //     Ipmiweb_GET::Get_Lan_Info(response_json);
  //     org_network_priority = response_json.at("NETWORK_PRIORITY").as_string();
  //     ninfo = response_json.at("NETWORK_INFO");

  //     json::value tmpobj1, tmpobj2;
  //     string tmpprior1, tmpprior2;

  //     tmpobj1 = ninfo.at(0);
  //     tmpobj2 = ninfo.at(1);
  //     tmpprior1 = tmpobj1.at("LAN_INTERFACE").as_string();
  //     tmpprior2 = tmpobj2.at("LAN_INTERFACE").as_string();

  //     if (tmpprior1 == org_network_priority) {
  //       ninfo = tmpobj1;
  //     } else if (tmpprior2 == org_network_priority) {
  //       ninfo = tmpobj2;
  //     } else {
  //       printf("network set error\n");
  //     }

  //     org_lan_interface = ninfo.at("LAN_INTERFACE").as_string();

  //     generic = ninfo.at("GENERIC");
  //     org_lan_setting_enable = generic.at("LAN_SETTING_ENABLE").as_string();
  //     org_mac_address = generic.at("MAC_ADDRESS").as_string();

  //     ipv4 = ninfo.at("IPV4");
  //     org_ipv4_preferred = ipv4.at("IPV4_PREFERRED").as_string();
  //     org_ipv4_gateway = ipv4.at("IPV4_GATEWAY").as_string();
  //     org_ipv4_netmask = ipv4.at("IPV4_NETMASK").as_string();
  //     org_ipv4_address = ipv4.at("IPV4_ADDRESS").as_string();
  //     org_ipv4_dhcp_enable = ipv4.at("IPV4_DHCP_ENABLE").as_string();

  //     ipv6 = ninfo.at("IPV6");
  //     org_ipv6_subnet_prefix_length =
  //         ipv6.at("IPV6_SUBNET_PREFIX_LENGTH").as_string();
  //     org_ipv6_address = ipv6.at("IPV6_ADDRESS").as_string();
  //     org_ipv6_enable = ipv6.at("IPV6_ENABLE").as_string();
  //     org_ipv6_dhcp_enable = ipv6.at("IPV6_DHCP_ENABLE").as_string();
  //     org_ipv6_gateway = ipv6.at("IPV6_GATEWAY").as_string();

  //     vlan = ninfo.at("VLAN");
  //     org_vlan_settings_enable = vlan.at("VLAN_SETTINGS_ENABLE").as_string();
  //     org_vlan_id = vlan.at("VLAN_ID").as_string();
  //     org_vlan_priority = vlan.at("VLAN_PRIORITY").as_string();

  //     json::value new_info,new_gen,new_ipv4,new_ipv6,new_vlan;
  //     new_info = request_json.at("NETWORK_INFO");
  //     new_gen = new_info.at("GENERIC");
  //     new_ipv4 = new_info.at("IPV4");
  //     new_ipv6 = new_info.at("IPV6");
  //     new_vlan = new_info.at("VLAN");
  //     new_network_priority = request_json.at("NETWORK_PRIORITY").as_string();
  //     if(new_info.has_field("LAN_INTERFACE"))
  //     new_lan_interface = new_info.at("LAN_INTERFACE").as_string();
  //      if(new_gen.has_field("LAN_SETTING_ENABLE"))
  //     new_lan_setting_enable = new_gen.at("LAN_SETTING_ENABLE").as_string();
  //      if(new_gen.has_field("MAC_ADDRESS"))
  //     new_mac_address = new_gen.at("MAC_ADDRESS").as_string();

  //      if(new_ipv4.has_field("IPV4_GATEWAY"))
  //     new_ipv4_gateway = new_ipv4.at("IPV4_GATEWAY").as_string();
  //      if(new_ipv4.has_field("IPV4_NETMASK"))
  //     new_ipv4_netmask = new_ipv4.at("IPV4_NETMASK").as_string();
  //      if(new_ipv4.has_field("IPV4_ADDRESS"))
  //     new_ipv4_address = new_ipv4.at("IPV4_ADDRESS").as_string();
  //      if(new_ipv4.has_field("IPV4_DHCP_ENABLE"))
  //     new_ipv4_dhcp_enable = new_ipv4.at("IPV4_DHCP_ENABLE").as_string();

  //      if(new_ipv6.has_field("IPV6_SUBNET_PREFIX_LENGTH"))
  //     new_ipv6_subnet_prefix_length = new_ipv6.at("IPV6_SUBNET_PREFIX_LENGTH").as_string();
  //      if(new_ipv6.has_field("IPV6_ADDRESS"))
  //     new_ipv6_address = new_ipv6.at("IPV6_ADDRESS").as_string();
  //      if(new_ipv6.has_field("IPV6_ENABLE"))
  //     new_ipv6_enable = new_ipv6.at("IPV6_ENABLE").as_string();
  //      if(new_ipv6.has_field("IPV6_DHCP_ENABLE"))
  //     new_ipv6_dhcp_enable = new_ipv6.at("IPV6_DHCP_ENABLE").as_string();
  //      if(new_ipv6.has_field("IPV6_GATEWAY"))
  //     new_ipv6_gateway = new_ipv6.at("IPV6_GATEWAY").as_string();

  //      if(new_vlan.has_field("VLAN_SETTINGS_ENABLE"))
  //     new_vlan_settings_enable = new_vlan.at("VLAN_SETTINGS_ENABLE").as_string();
  //      if(new_vlan.has_field("VLAN_ID"))
  //     new_vlan_id = new_vlan.at("VLAN_ID").as_string();
  //      if(new_vlan.has_field("VLAN_PRIORITY"))
  //     new_vlan_priority = new_vlan.at("VLAN_PRIORITY").as_string();

  //     printf("\t\t\tdy : checkpoint1 : parse query\n");

  //      unsigned char new_intf = 0;
  //     if (strcmp(new_lan_interface.c_str(), "eth0") == 0)
  //       new_intf = 0;
  //     else if (strcmp(new_lan_interface.c_str(), "eth1") == 0)
  //       new_intf = 1;
  //     if (strcmp(org_mac_address.c_str(), new_mac_address.c_str())) {
  //       post_cmdline_macaddr(new_mac_address.c_str(),mac_buf);
  //       memcpy(&(ipmiNetwork[new_intf].mac_addr[0]), mac_buf, 6);
  //       ipmiNetwork[new_intf].lanConfigChanged[LAN_PARAM_MAC_ADDR] = 1;
  //       changed = 1;

  //     }

  //     if (strcmp(org_ipv4_dhcp_enable.c_str(), new_ipv4_dhcp_enable.c_str()))
  //               // set_network_ipv4_dhcp(new_intf, new_ipv4_dhcp_enable);
  //       ipmiNetwork[new_intf].ip_src = new_ipv4_dhcp_enable.c_str();
  //       ipmiNetwork[new_intf].lanConfigChanged[LAN_PARAM_IP_SRC] = 1;

  //     if (!strcmp(new_ipv4_dhcp_enable.c_str(), "0")) {
  //       if (strcmp(org_ipv4_address.c_str(), new_ipv4_address.c_str()) ||
  //           strcmp(org_ipv4_netmask.c_str(), new_ipv4_netmask.c_str()) ||
  //           strcmp(org_ipv4_gateway.c_str(), new_ipv4_gateway.c_str())) {
  //         ipmiNetwork[new_intf].ip_addr[0] = new_ipv4_address.c_str();
  //         ipmiNetwork[new_intf].net_mask[0] = new_ipv4_netmask.c_str();
  //         ipmiNetwork[new_intf].lanConfigChanged[LAN_PARAM_IP_ADDR] = 1;
  //         ipmiNetwork[new_intf].lanConfigChanged[LAN_PARAM_NET_MASK] = 1;
  //         // rets += set_network_gateway(new_intf, new_ipv4_gateway);
  //         ipmiNetwork[new_intf].df_gw_ip_addr[0] = new_ipv4_gateway.c_str();
  //         ipmiNetwork[new_intf].lanConfigChanged[LAN_PARAM_DF_GW_IP_ADDR] = 1;
  //         changed = 1;
  //       }
  //     }
  //     int ip_src_v6=0;
  //     printf("\t\t\tdy : checkpoint2 : ipv4 set\n");

  //     if (strcmp(org_ipv6_enable.c_str(), new_ipv6_enable.c_str())) {
  //       // ipv6 enable 변경 되었을 때만
        
  //       ipmiNetwork[new_intf].set_enable_v6 = new_ipv6_enable.c_str();

  //       if (!strcmp(new_ipv6_enable.c_str(), "1")) {
  //         // dhcp 변경 되었을 때만
  //         if (strcmp(org_ipv6_dhcp_enable.c_str(), new_ipv6_dhcp_enable.c_str()))
            
  //           if(new_ipv6_dhcp_enable=="1") ip_src_v6=2;
  //           ipmiNetwork[new_intf].ip_src_v6 = ip_src_v6;
  //           ipmiNetwork[new_intf].lanConfigChanged[LAN_PARAM_IPV6_SRC] = 1;

  //         if (!strcmp(new_ipv6_dhcp_enable.c_str(), "0")) {
  //         //   set_network_ipv6_ip(new_intf, new_ipv6_address);
  //           uint8_t u_data[SIZE_IP_ADDR_V6] = {0,};
  //           memcpy(u_data , new_ipv6_address.c_str(), 39);
  //           ipmiNetwork[new_intf].ip_addr_v6.assign(u_data, u_data + SIZE_IP_ADDR_V6);
  //           ipmiNetwork[new_intf].net_mask_v6[0] =new_ipv6_subnet_prefix_length.c_str();
  //           memcpy(u_data , new_ipv6_gateway.c_str(), 39);
  //           ipmiNetwork[new_intf].df_gw_ip_addr_v6.assign(u_data, u_data + SIZE_IP_ADDR_V6);
  //         }
  //       }
  //     }

  //   printf("\t\t\tdy : checkpoint3 : ipv6 set\n");
  //       unsigned char enable_vlan = 0x80;
  //     if (org_vlan_settings_enable != new_vlan_settings_enable)
        
      

  //       if (new_vlan_settings_enable == "1") {
  //         ipmiNetwork[new_intf].vlan_enable = enable_vlan;
  //       } else {
  //         char cmds[100] = { 0,};

  //         sprintf(cmds, "ifconfig eth0.%d down", ipmiNetwork[new_intf].vlan_id);
  //         system(cmds);
  //         memset(cmds, 0, sizeof(cmds));

  //         sprintf(cmds, "vconfig rem eth0.%d", ipmiNetwork[new_intf].vlan_id);
  //         system(cmds);

  //         ipmiNetwork[new_intf].vlan_enable = 0;
  //         ipmiNetwork[new_intf].vlan_id = 0;
  //       }

  //     if (new_vlan_settings_enable ==  "1") {
  //       if (org_vlan_id != new_vlan_id) {
          
  //         char cmds[100] = {0,};
          
  //         ipmiNetwork[new_intf].vlan_enable = enable_vlan;
  //         ipmiNetwork[new_intf].vlan_id =new_vlan_id.c_str();

  //         sprintf(cmds, "vconfig add eth0 %d", ipmiNetwork[new_intf].vlan_id);
  //         system(cmds);
  //         memset(cmds, 0, sizeof(cmds));

  //         sprintf(cmds, "ifconfig eth0.%d %d.%d.%d.%d netmask %d.%d.%d.%d up",
  //                 ipmiNetwork[new_intf].vlan_id,
  //                 ipmiNetwork[new_intf].ip_addr[0],
  //                 ipmiNetwork[new_intf].ip_addr[1],
  //                 ipmiNetwork[new_intf].ip_addr[2],
  //                 ipmiNetwork[new_intf].ip_addr[3],
  //                 ipmiNetwork[new_intf].net_mask[0],
  //                 ipmiNetwork[new_intf].net_mask[1],
  //                 ipmiNetwork[new_intf].net_mask[2],
  //                 ipmiNetwork[new_intf].net_mask[3]);
  //         system(cmds);
  //         ipmiNetwork[new_intf].vlan_priority = new_vlan_priority.c_str();
  //       } else if (org_vlan_priority != new_vlan_priority)
  //         ipmiNetwork[new_intf].vlan_priority = new_vlan_priority.c_str();
  //     }

  //     printf("\t\t\tdy : checkpoint4 : vlan set\n");

  //     char priority;
  //     if (new_network_priority == "eth0") {
  //       priority = '1';
  //     } else if (new_network_priority == "eth1") {
  //       priority = '8';
  //     } else {
  //       priority = '8';
  //     }

  //     if (new_network_priority != org_network_priority)
  //       set_eth_priority(priority - '0');

  // [테스트] network 임시처리
  
    get_value_from_json_key(request_json, "NETWORK_PRIORITY", temporary.net_priority);
    Collection *eth_col = (Collection *)g_record[ODATA_ETHERNET_INTERFACE_ID];

    json::value jv_network;
    get_value_from_json_key(request_json, "NETWORK_INFO", jv_network);
    
    
    json::value jv_generic, jv_ipv4, jv_ipv6, jv_vlan;
    string lan_interface;
    EthernetInterfaces *cur_eth = nullptr;

    get_value_from_json_key(jv_network, "LAN_INTERFACE", lan_interface);
    string lan_id = lan_interface.substr(3);
    // cout << "LAN ID! : " << lan_id << endl;

    for(int j=0; j<eth_col->members.size(); j++)
    {
        if(((EthernetInterfaces *)(eth_col->members[j]))->id == lan_id)
        {
            cur_eth = (EthernetInterfaces *)(eth_col->members[j]);
            break;
        }
    }

    if(cur_eth == nullptr){
      log(error) << " No EthernetInterface? why?";
      return ;
    }

    get_value_from_json_key(jv_network, "GENERIC", jv_generic);
    get_value_from_json_key(jv_network, "IPV4", jv_ipv4);
    get_value_from_json_key(jv_network, "IPV6", jv_ipv6);
    get_value_from_json_key(jv_network, "VLAN", jv_vlan);

    string lan_enabled;
    get_value_from_json_key(jv_generic, "MAC_ADDRESS", cur_eth->mac_address);
    get_value_from_json_key(jv_generic, "LAN_SETTING_ENABLE", lan_enabled);
    
    if(lan_enabled == "1")
      cur_eth->interfaceEnabled = true;
    else if(lan_enabled == "0")
      cur_eth->interfaceEnabled = false;

    string v4_dhcp;
    get_value_from_json_key(jv_ipv4, "IPV4_DHCP_ENABLE", v4_dhcp);
    if(v4_dhcp == "1")
      cur_eth->dhcp_v4.dhcp_enabled = true;
    else if(v4_dhcp == "0")
    {
      cur_eth->dhcp_v4.dhcp_enabled = false;
      get_value_from_json_key(jv_ipv4, "IPV4_ADDRESS", cur_eth->v_ipv4[0].address);
      get_value_from_json_key(jv_ipv4, "IPV4_NETMASK", cur_eth->v_ipv4[0].subnet_mask);
      get_value_from_json_key(jv_ipv4, "IPV4_GATEWAY", cur_eth->v_ipv4[0].gateway);

      string a = "/etc/netchange.sh -i 1 -c " + cur_eth->v_ipv4[0].address +
                 " -g " + cur_eth->v_ipv4[0].gateway + " -n " +
                 cur_eth->v_ipv4[0].subnet_mask;
      cout << "cmd =" << a << endl;
      system(a.c_str());
      system("reboot");
    }

    get_value_from_json_key(jv_ipv6, "IPV6_ENABLE", temporary.net_v6_enabled);
    get_value_from_json_key(jv_ipv6, "IPV6_DHCP_ENABLE", temporary.net_v6_dhcp);
    if(temporary.net_v6_dhcp == "0")
    {
      get_value_from_json_key(jv_ipv6, "IPV6_ADDRESS", cur_eth->v_ipv6[0].address);
      string tmp_prefix;
      get_value_from_json_key(jv_ipv6, "IPV6_SUBNET_PREFIX_LENGTH", tmp_prefix);
      cur_eth->v_ipv6[0].prefix_length = improved_stoi(tmp_prefix);
      get_value_from_json_key(jv_ipv6, "IPV6_GATEWAY", cur_eth->ipv6_default_gateway);
    }
    
    string vlan_str;
    get_value_from_json_key(jv_vlan, "VLAN_SETTINGS_ENABLE", vlan_str);
    if(vlan_str == "1")
      cur_eth->vlan.vlan_enable = true;
    else if(vlan_str == "0")
      cur_eth->vlan.vlan_enable = false;

    string tmp_vid;
    get_value_from_json_key(jv_vlan, "VLAN_ID", tmp_vid);
    cur_eth->vlan.vlan_id = improved_stoi(tmp_vid);
    get_value_from_json_key(jv_vlan, "VLAN_PRIORITY", temporary.net_vlan_prior);
  
}

 void Ipmiweb_POST::Set_Ntp(json::value request_json){

    //  uint8_t res_msg[20], response[QSIZE];
    //   int res_len = 0;
    //   uint8_t header[1000], cmds[300], cmds2[300];
    //   string auto_sync, ntp_server, year, month, day,
    //       hour, min, sec;
      
    //   uint8_t u_year, u_month, u_day,
    //       u_hour, u_min, u_sec;
    //   // string sdata((char *)data);
    //   // cout << "sdata : " << sdata << endl;
    //   json::value ntp_info  = json::value::object(),ntp = json::value::object();
    //   ntp_info = request_json.at("NTP_INFO");
    //   ntp = ntp_info.at("NTP");
    //   auto_sync = ntp.at("AUTO_SYNC").as_string();


    //   if (auto_sync == "1") {
    //     ntp_server = ntp.at("NTP_SERVER").as_string();
    //     set_ntp_conf_auto(ntp_server.c_str());
    //     cout << "set ntp auto " <<endl;
    //   } else {
    //     year = ntp.at("YEAR").as_string();
    //     month = ntp.at("MONTH").as_string();
    //     day = ntp.at("DAY").as_string();
    //     hour = ntp.at("HOUR").as_string();
    //     min = ntp.at("MIN").as_string();
    //     sec = ntp.at("SEC").as_string();

    //     string cmd3 = {0,};
    //     sprintf(cmds, "killall -9 ntpd");
    //     system(cmds);
    //     // sprintf(cmds, "mv /etc/ntp.conf /etc/ntp.conf.bak");
    //     cmd3 +="mv /etc/ntp.conf /etc/ntp.conf.bak";
    //     system(cmd3.c_str());
    //     cmd3 ={0,};
    //     cmd3 = "date --set=";
    //     cmd3 += year;
    //     cmd3 += "-";
    //     cmd3 += month;
    //     cmd3 += "-";
    //     cmd3 += day;
    //     cmd3 += " ";
    //     cmd3 += hour;
    //     cmd3 += ":";
    //     cmd3 += min;
    //     cmd3 += ":";
    //     cmd3 += sec;
        
    //     cout<<"ntp not auto "<<cmd3<<endl;
    //     // sprintf(cmds2, "date --set=\"%s-%s-%s %s:%s:%s\"", year.c_str(), month.c_str(), day.c_str(),
    //     //         hour.c_str(), min.c_str(), sec.c_str());
    //     int rets = 0;
    //     rets = system(cmd3.c_str());
    //   }


      // [테스트] ntp 임시처리 (redfish이용)
      json::value jv_ntp_info, jv_ntp;
      get_value_from_json_key(request_json, "NTP_INFO", jv_ntp_info);
      get_value_from_json_key(jv_ntp_info, "NTP", jv_ntp);

      string network_odata = ODATA_MANAGER_ID;
      network_odata.append("/NetworkProtocol");
      NetworkProtocol *net = (NetworkProtocol *)g_record[network_odata];

      string year, month, day, date_str;
      string hour, min, sec, time_str;
      string auto_sync;
      string time_server;
      string time_zone;

      get_value_from_json_key(jv_ntp, "AUTO_SYNC", auto_sync);

      if(auto_sync == "1")
      {
          // time server
          get_value_from_json_key(jv_ntp, "NTP_SERVER", time_server);
          set_time_by_ntp_server(time_server);
          net->ntp.primary_ntp_server = time_server;
          net->ntp.protocol_enabled = true;

      }
      else
      {
          net->ntp.protocol_enabled = false;
          get_value_from_json_key(jv_ntp, "TIME_ZONE", time_zone);
          cout << " TIMEZONE : " << time_zone << endl;

          if(time_zone != "")
          {
              string front = time_zone.substr(0,3);
              string end = time_zone.substr(3);
              if(front == "%2B")
              {
                  time_zone = "+";
                  time_zone.append(end);
              }
                  
              set_time_by_userTimezone(time_zone);
              net->ntp.timezone = time_zone;
          }
          else
          {
              get_value_from_json_key(jv_ntp, "YEAR", year);
              get_value_from_json_key(jv_ntp, "MONTH", month);
              get_value_from_json_key(jv_ntp, "DAY", day);
              get_value_from_json_key(jv_ntp, "HOUR", hour);
              get_value_from_json_key(jv_ntp, "MIN", min);
              get_value_from_json_key(jv_ntp, "SEC", sec);

              date_str = year + "-" + month + "-" + day;
              time_str = hour + ":" + min + ":" + sec;

              set_time_by_userDate(date_str, time_str);
          }
      }

 }

int post_file_exist(char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

void Ipmiweb_POST::Set_Ssl(json::value request_json){
  //  uint8_t result[QSIZE], response[QSIZE] , cmds[300];
  //     uint8_t header[1000];
  //     string key_length, country, state_or_province, city_or_locality, organization, organization_unit, common_name, email_address, valid_for;
  //     cout << "dsf"<<request_json <<endl;

  //   json::value ssl;
  //  // ssl = request_json.at("SSL_INFO");
  //    cout <<"gdfg"<<request_json.has_field("COUNTRY")<<endl;
  //   //  cout << "asdasd"<<ssl.at("COUNTRY")<<endl;
  //    if(request_json.has_field("KEY_LENGTH"))
  //     get_value_from_json_key(request_json, "KEY_LENGTH", key_length);
  //     if(request_json.has_field("COUNTRY"))
  //     get_value_from_json_key(request_json, "COUNTRY", country);
  //     if(request_json.has_field("STATE_OR_PROVINCE"))
  //     get_value_from_json_key(request_json, "STATE_OR_PROVINCE", state_or_province);
  //     if(request_json.has_field("CITY_OR_LOCALITY"))
  //     get_value_from_json_key(request_json, "CITY_OR_LOCALITY", city_or_locality);
  //     if(request_json.has_field("ORGANIZATION"))
  //     get_value_from_json_key(request_json, "ORGANIZATION", organization);
  //     if(request_json.has_field("ORGANIZATION_UNIT"))
  //     get_value_from_json_key(request_json, "ORGANIZATION_UNIT", organization_unit);
  //     if(request_json.has_field("COMMON_NAME"))
  //     get_value_from_json_key(request_json, "COMMON_NAME", common_name);
  //     if(request_json.has_field("EMAIL_ADDRESS"))
  //     get_value_from_json_key(request_json, "EMAIL_ADDRESS", email_address);
  //     if(request_json.has_field("VALID_FOR"))
  //     get_value_from_json_key(request_json, "VALID_FOR", valid_for);



  //     // key_length = request_json.at("KEY_LENGTH").as_string();
  //     // cout <<"asdads"<< key_length << endl;
  //     // country = request_json.at("COUNTRY").as_string();
  //     // state_or_province = request_json.at("STATE_OR_PROVINCE").as_string();
  //     // city_or_locality = request_json.at("CITY_OR_LOCALITY").as_string();
  //     // organization = request_json.at("ORGANIZATION").as_string();
  //     // organization_unit = request_json.at("ORGANIZATION_UNIT").as_string();
  //     // common_name = request_json.at("COMMON_NAME").as_string();
  //     // email_address = request_json.at("EMAIL_ADDRESS").as_string();
  //     // valid_for = request_json.at("VALID_FOR").as_string();

  //     char *cert_config = "/conf/ssl/cert.conf";
  //     char *finalcsr_file_path = "/conf/ssl/cert.csr";
  //     char *server_key = "/conf/ssl/server.key";

  //     printf("[...] Setting SSL\n");
      
  //     if (key_length == "Default") {
  //       key_length = "1024";
  //     }
  //     cout << "key_len"<<key_length<<endl;
  //     printf("[...] Erasing cert.conf\n");
  //     if (post_file_exist(cert_config)) {
  //       sprintf(cmds, "rm -f %s", cert_config);
  //       system(cmds);
  //       memset(cmds, 0, sizeof(cmds));
  //     } else {
  //       printf("[...] No cert.conf file. Keep moving\n");
  //     }

  //     printf("[...] Erasing exist SSL Key\n");
  //     if (post_file_exist(server_key)) {
  //       sprintf(cmds, "rm -f %s", server_key);
  //       system(cmds);
  //       memset(cmds, 0, sizeof(cmds));
  //     } else {
  //       printf("[...] No server.key file. Keep moving\n");
  //     }
  //     printf("[...] Create SSL Key\n");
  //     sprintf(cmds, "openssl genrsa -out /conf/ssl/server.key %s", key_length);
  //     system(cmds);

  //     FILE *cert_file = fopen(cert_config, "w");

  //     char cert_text[14][200];

  //     sprintf(cert_text[0], "[ req ]\n");
  //     sprintf(cert_text[1], "default_bits\t= %d\n", key_length);
  //     sprintf(cert_text[2], "default_md\t= sha256\n");
  //     sprintf(cert_text[3], "default_keyfile\t=%s\n", server_key);
  //     sprintf(cert_text[4], "prompt\t = no\n");
  //     sprintf(cert_text[5], "encrypt_key\t= no\n\n", state_or_province);
  //     sprintf(cert_text[6],
  //             "# base request\ndistinguished_name = req_distinguished_name\n");
  //     sprintf(cert_text[7],
  //             "\n# distinguished_name\n[ req_distinguished_name ]\n");
  //     sprintf(cert_text[8], "countryName\t= \"%s\"\n", country);
  //     sprintf(cert_text[9], "localityName\t=\"%s\"\n", city_or_locality);
  //     sprintf(cert_text[10], "organizationName\t=\"%s\"\n", organization);
  //     sprintf(cert_text[11], "organizationalUnitName\t=\"%s\"\n",
  //             organization_unit);
  //     sprintf(cert_text[12], "commonName\t=\"%s\"\n", common_name);
  //     sprintf(cert_text[13], "emailAddress\t=\"%s\"\n", email_address);

  //     int i;
  //     for (i = 0; i < 14; i++)
  //       fprintf(cert_file, "%s", cert_text[i]);

  //     fclose(cert_file);

  //     sprintf(cmds, "openssl req -config %s -new -key %s -out %s -verbose",
  //             cert_config, server_key, finalcsr_file_path);
  //     if (!system(cmds)) {
  //       printf("[...] Checking SSl Key & CSR File\n");
  //       if (post_file_exist(finalcsr_file_path)) {
  //         printf("[###] CSR File checked\n");
  //       } else {
  //         printf("[ERROR] CSR File is not exist\n");
 
  //       }

  //       if (post_file_exist(server_key)) {
  //         printf("[###] SSL Key File checked\n");
  //       } else {
  //         printf("[ERROR} SSL Key File is not exist\n");
 
  //       }
  //       memset(cmds, 0, sizeof(cmds));

  //       sprintf(cmds, "cp %s /backup_conf/ssl/", cert_config);
  //       system(cmds);
  //       memset(cmds, 0, sizeof(cmds));

  //       sprintf(cmds, "cp %s /backup_conf/ssl/", server_key);
  //       system(cmds);
  //       memset(cmds, 0, sizeof(cmds));

  //       sprintf(cmds, "cp %s /backup_conf/ssl/", finalcsr_file_path);
  //       system(cmds);
  //       memset(cmds, 0, sizeof(cmds));

  
  //     } else {
  //       printf("[ERROR] Failed to Create CSR and SSL Key\n");
  
  //     }
  //     set_ssl_1(key_length.c_str(), country.c_str(), state_or_province.c_str(), city_or_locality.c_str(),
  //                    organization.c_str(), valid_for.c_str());
  //     set_ssl_2(organization_unit.c_str(), common_name.c_str(), email_address.c_str());

    // [테스트] ssl 변경 및 임시적용
    fs::path keyFile("/conf/ssl/edge_server.key");
    fs::path configFile("/conf/ssl/edge_server.conf");
    fs::path csrFile("/conf/ssl/edge_server.csr");
    fs::path certFile("/conf/ssl/edge_server.crt");

    string country, state, city, organization, organizationUnit, keyLength, commonName, email, validDays;

    get_value_from_json_key(request_json, "COUNTRY", country);
    get_value_from_json_key(request_json, "STATE_OR_PROVINCE", state);
    get_value_from_json_key(request_json, "CITY_OR_LOCALITY", city);
    get_value_from_json_key(request_json, "ORGANIZATION", organization);
    get_value_from_json_key(request_json, "ORGANIZATION_UNIT", organizationUnit);
    get_value_from_json_key(request_json, "KEY_LENGTH", keyLength);
    get_value_from_json_key(request_json, "COMMON_NAME", commonName);
    get_value_from_json_key(request_json, "EMAIL_ADDRESS", email);
    get_value_from_json_key(request_json, "VALID_FOR", validDays);

    if(keyLength == "Default")
        keyLength = "1024";

    // [step 1] generate private key
    string cmd_key = "openssl genrsa -out " + keyFile.string() + " " + keyLength;
    try
    {
        system(cmd_key.c_str());
        log(info) << "[###]Generate Private Key SUCCESS";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        log(error) << "[###]Generate Private Key FAIL";
    }

    // [step 2] generate ssl config file
    char cert_text[15][200];

    sprintf(cert_text[0], "[ req ]\n");	
    sprintf(cert_text[1], "default_bits\t= %s\n", keyLength.c_str());
    sprintf(cert_text[2], "default_md\t= sha256\n");	
    sprintf(cert_text[3], "default_keyfile\t=%s\n", keyFile.c_str());
    sprintf(cert_text[4], "prompt\t = no\n");
    sprintf(cert_text[5], "encrypt_key\t= no\n\n");
    sprintf(cert_text[6], "# base request\ndistinguished_name = req_distinguished_name\n");
    sprintf(cert_text[7], "\n# distinguished_name\n[ req_distinguished_name ]\n");	
    sprintf(cert_text[8], "countryName\t= \"%s\"\n", country.c_str());
    sprintf(cert_text[9], "stateOrProvinceName\t= \"%s\"\n", state.c_str());
    sprintf(cert_text[10], "localityName\t=\"%s\"\n", city.c_str());
    sprintf(cert_text[11], "organizationName\t=\"%s\"\n", organization.c_str());	
    sprintf(cert_text[12], "organizationalUnitName\t=\"%s\"\n", organizationUnit.c_str());	
    sprintf(cert_text[13], "commonName\t=\"%s\"\n", commonName.c_str());
    sprintf(cert_text[14], "emailAddress\t=\"%s\"\n", email.c_str());

    ofstream cert_conf(configFile);
    for (int i = 0; i < 15; i++)
        cert_conf << cert_text[i];
    cert_conf.close();

    
    // [step 3] generate csr
    string cmd_csr = "openssl req -config " + configFile.string() + " -new -key " + keyFile.string()
    + " -out " + csrFile.string() + " -verbose";
    
    try
    {
        system(cmd_csr.c_str());
        log(info) << "[###]Generate CSR SUCCESS";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        log(error) << "[###]Generate CSR FAIL";
    }


    // [step 4] generate certificate
    string cmd_crt = "openssl req -x509 -in " + csrFile.string() + " -key " + keyFile.string()
    + " -out " + certFile.string() + " -days " + validDays;

    try
    {
        system(cmd_crt.c_str());
        log(info) << "[###]Generate Certificate SUCCESS";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        log(error) << "[###]Generate Certificate FAIL";
    }

    
}

void Ipmiweb_POST::Set_AD(json::value request_json){
  // uint8_t result[QSIZE], response[QSIZE];
  //     uint8_t header[1000];
  //     string  ip, domain, secret_name, secret_pwd;
  //     // int enable;
  //     string enable;
  //     int ret = 0;

  //     enable = request_json.at("ENABLE").as_string();

  //     if (enable == "1") {
  //       ip = request_json.at("IP").as_string();
  //       domain = request_json.at("DOMAIN").as_string();
  //       secret_name = request_json.at("SECRET_NAME").as_string();
  //       secret_pwd = request_json.at("SECRET_PWD").as_string();
  //       cout<<" ad json setting" <<endl;
  //       try {
  //         cout << "1" << endl;
  //         ret = set_ad_enable(1);
  //         cout << "2" << endl;
  //         ret = set_ad_conf_file_ip_pwd(ip.c_str(), secret_pwd.c_str());
  //         cout << "3" << endl;
  //         ret = set_ad_conf_file_domain(domain.c_str());
  //         cout << "4" << endl;
  //         // ret = set_ad_conf_file_domain(secret_name.c_str());
  //         cout << "5" << endl;
  //         set_ad_conf_file_username(secret_name.c_str());
  //         cout << "6" <<endl;
  //         write_ad_to_file();
  //         cout << "7" << endl;
  //         ret = set_ldap_enable(0);
  //         cout << "8" << endl;
  //       } catch (const std::exception &) {
  //         cout << "4"
  //              << "set_active_directory error";
  //       }

  //     } else {
  //       ret = set_ad_enable(0);       
  //     }
  //     cout << "end ad " <<endl;

      // [테스트] ad 임시 값 처리...
      string enabled, username, password, domain, address;

      get_value_from_json_key(request_json, "ENABLE", enabled);
      get_value_from_json_key(request_json, "SECRET_NAME", username);
      get_value_from_json_key(request_json, "SECRET_PWD", password);
      get_value_from_json_key(request_json, "DOMAIN", domain);
      get_value_from_json_key(request_json, "IP", address);

      temporary.ad_enabled = enabled;
      temporary.ad_username = username;
      temporary.ad_password = password;
      temporary.ad_domain = domain;
      temporary.ad_domainControllerAddress = address;
}





void Ipmiweb_POST::Set_Radius(json::value request_json){

  // uint8_t result[QSIZE], response[QSIZE];
  //     uint8_t header[1000];
  //     string radius_enable, ip, port, secret;
  //     uint8_t cmds[100] = {0};

  //     json::value radius_info  = json::value::object(),radius = json::value::object();
  //     radius_info = request_json.at("RADIUS_INFO");
  //     radius = radius_info.at("RADIUS");
  //     radius_enable = radius.at("RADIUS_ENABLE").as_string();
  //     ip = radius.at("IP").as_string();
  //     port = radius.at("PORT").as_string();
  //     secret = radius.at("SECRET").as_string();


  //     if (radius_enable == "1") {
  //       int rets = 0;
  //       rets = set_radius_disable();
  //       rets += set_radius_config(ip.c_str(), port.c_str(), secret.c_str());

  //     } else {
  //       int rets = 0;
  //       rets = set_radius_disable();
  //       if (rets == 0) {
  //         memset(cmds, 0, sizeof(cmds));
  //         if (access("/etc/raddb/server", F_OK) == 0) {
  //           sprintf(cmds, "mv /etc/raddb/server /etc/raddb/server.bak");
  //           rets = system(cmds);
  //         }
  //       }
  //     }
  //     cout << "radius" <<endl;
  // string _uri = "/redfish/v1/Managers/radius";
  // if (record_is_exist(_uri)) {
  //   Radius *re = (Radius *)g_record[_uri];
  //   re->id = "Radius";
  //   re->radius_port = stoi(port);
  //   re->radius_secret = secret;
  //   re->radius_server = ip;
  //   if(radius_enable=="0")
  //   re->radius_enabled="fasle";
  //   else re->radius_enabled = "enable";
  //   resource_save_json(re);
  // }

      // [테스트] radius 임시 값 처리...
      string enabled, address, port, secret;
      json::value jv_radius;

      get_value_from_json_key(request_json, "RADIUS", jv_radius);

      get_value_from_json_key(jv_radius, "RADIUS_ENABLE", enabled);
      get_value_from_json_key(jv_radius, "IP", address);
      get_value_from_json_key(jv_radius, "PORT", port);
      get_value_from_json_key(jv_radius, "SECRET", secret);

      temporary.radius_enabled = enabled;
      temporary.radius_serverAddress = address;
      temporary.radius_port = port;
      temporary.radius_secret = secret;


}


void Ipmiweb_POST::Set_Power(json::value request_json){
      uint8_t response[50];
      uint8_t header[1000];
      json::value response_json;
      int c_status, res_len = 0;
      // string sdata((char *)data);
      // cout << "sdata in power_call : " << sdata << endl;
      string s_status;
      // myobj = json::value::parse(sdata);

      if (request_json.at("STATUS").is_integer())
        c_status = request_json.at("STATUS").as_integer();
      else if (request_json.at("STATUS").is_string()) {
        s_status = request_json.at("STATUS").as_string();
        c_status = stoi(s_status);
      }
      ipmi_req_t *req;
      
      if (c_status == 1)
        c_status = 0x3;
      else if (c_status == 2)
        c_status = 0x0;
      else if (c_status == 3)
        c_status = 0x6;
      else if (c_status == 4)
        c_status = 0x1;
      else if (c_status == 5)
        c_status = 0x2;

      req->netfn_lun = 0;
      req->cmd = CMD_SET_POWER_STATUS;
      req->data[0] = c_status;
      // req.msg.data_len = 4;

      // ipmi_handle_rest(req, response, res_len);
      ipmiChassis.chassis_control((ipmi_req_t *)req, (ipmi_res_t *)response,
                                (uint8_t *)res_len);
     

}


bool Ipmiweb_POST::Set_Usb(json::value request_json){
      // int rets = umount_cmd();

      // json::value myobj;
      // string u_ip, u_path, u_user, u_pwd;
      // uint8_t ip_addr[50], user[20], path[50], pwd[20], path_dir[50] = {0}, cmds[300]={0,};

      // memset(path, 0, sizeof(uint8_t) * 50);


      // u_ip = request_json.at("IP_ADDRESS").as_string();
      // u_path = request_json.at("PATH").as_string();
      // u_user = request_json.at("USER").as_string();
      // u_pwd = request_json.at("PASSWORD").as_string();

      // strcpy(ip_addr, u_ip.c_str());
      // strcpy(path, u_path.c_str());
      // strcpy(user, u_user.c_str());
      // strcpy(pwd, u_pwd.c_str());

      // int i, count = 0;

      // memset(cmds, 0, sizeof(uint8_t) * 300);
      
      // uint8_t l_mod_res[500], l_mod_ret[500] = {0};

      // memset(l_mod_res, 0, sizeof(uint8_t) * 500);
      // memset(cmds, 0, sizeof(uint8_t) * 300);
      // sprintf(cmds, "lsmod | grep storage");
      // FILE *p = popen(cmds, "r");
      // if (p != NULL) {
      //   while (fgets(l_mod_ret, sizeof(l_mod_ret), p) != NULL)
      //     strncat(l_mod_res, l_mod_ret, strlen(l_mod_ret));

      //   pclose(p);
      // }

      // if (strlen(l_mod_res) > 0) {
      //   rets = system("rmmod g_mass_storage");
      //   // if (rets != 0)
      //   //   send_result_message(nc, 4);
      // }

      // memset(l_mod_res, 0, sizeof(uint8_t) * 500);
      // memset(l_mod_ret, 0, sizeof(uint8_t) * 500);
      // memset(cmds, 0, sizeof(uint8_t) * 300);
      // sprintf(cmds, "df | grep /nfs | grep -v nfs_check");
      // p = popen(cmds, "r");
      // if (p != NULL) {
      //   while (fgets(l_mod_ret, sizeof(l_mod_ret), p) != NULL)
      //     strncat(l_mod_res, l_mod_ret, strlen(l_mod_ret));
      //   pclose(p);
      // }

      // if (strlen(l_mod_res) > 0) {
      //   system("umount -l /nfs > /dev/null 2>&1");
      // }
      // for (i = 0; i < strlen(path); i++) {
      //   if (path[i] == '/')
      //     count++;
      // }

      // if (count == 1) {
      //   // send_result_message(nc, 3);
      //   cout << "post usb error " << endl;
      // } else {
      //   if (strstr((char *)path, "iso") != NULL) {
      //     uint8_t *r_index;
      //     r_index = rindex((char *)path, '/');

      //     strncpy(path_dir, path, strlen(path) - strlen(r_index));
      //     FILE *fp = fopen("/conf/nfs_save.sh", "w");
      //     fputs("#!/bin/sh\n", fp);

      //     memset(cmds, 0, sizeof(cmds));
      //     sprintf(cmds, "mount -t nfs %s:%s /nfs\n", ip_addr, path_dir);
      //     fputs(cmds, fp);
      //     rets = system(cmds);

      //     memset(cmds, 0, sizeof(cmds));
      //     sprintf(cmds,
      //             "modprobe g_mass_storage file=%s iSerialNumber=123456abcdef "
      //             "ro=y cdrom=y stall=0\n",
      //             path);
      //     fputs(cmds, fp);
      //     rets = system(cmds);

      //     sprintf(cmds, "chmod 777 /conf/nfs_save.sh");
      //     rets = system(cmds);

      //     fclose(fp);
       
      //   } 
      // }

      //

      // [테스트] VM 리소스 Mount 식으로..
      // post할때 일단 마운트 되는 위치 /tmp/nfs라고 하면 이거에 다른게 마운트되어있는지 체크하는게 필요할거같다
      string vm_id;
      get_value_from_json_key(request_json, "ID", vm_id);

      cout << "vm_id : " << vm_id << endl;

      string odata_id = ODATA_MANAGER_ID;
      odata_id.append("/VirtualMedia/").append(vm_id);

      VirtualMedia *vm = (VirtualMedia *)g_record[odata_id];
      string image = vm->image;

      if(improved_stoi(get_popen_string("mount | grep /tmp/nfs | wc -l")) > 0)
        return false;

      if(!fs::exists("/tmp/nfs"))
          mkdir("/tmp/nfs", 777);

      string cmd = "mount -vt nfs " + image + " /tmp/nfs -o vers=3";
      int ret = system(cmd.c_str());
      
      if(ret == -1)
        return false;

      vm->size = get_popen_string("df -hP | grep \"" + image + "\" | awk {\'print $2\'}");
      vm->inserted = true;
      // vm->image_name = get_current_object_name(image, "/");

      return true;
}

void Ipmiweb_POST::Set_BMC_Reset(){

  uint8_t cmds[100] = {0};
  log(info) << "Reboot";
  sprintf(cmds, "rm -r /conf/ipmi/* && rm -r /conf/dcmi/* && rm -r "
                "/backup_conf/ipmi/* && rm -r /backup_conf/dcmi/*");
  system(cmds);
  system("reboot");
}

void Ipmiweb_POST::Set_Warm_Reset(){
  // uint8_t cmds[100] = {0};
  // log(info) << "Warm Reset";
  // sprintf(cmds, "/etc/init.d/rcK && /etc/init.d/rcS");
  // system(cmds);
}
void Ipmiweb_POST::Set_Upload(http_request request){
  // uint8_t methods[10], str[100] = {0};
  // uint8_t data_in[1000] = {0};
  // uint8_t m_len, b_len = 0;
  // uint8_t file_name[100];
  // size_t file_length = 0;
  // int i, j = 0;
  // int offset = 0x70000;
  // int maxsize = 0x90;
  // int mtd_size[10], mtd_offset[10] = {0};
  // uint8_t cmds[500] = {0};
  // uint8_t *result;

    // [테스트] firmware update 파일 저장 구현
    if(!fs::exists(UPDATE_FILE_STORED_DIRECTORY_PATH))
        mkdir(UPDATE_FILE_STORED_DIRECTORY_PATH, 0755);

    string date, time;
    get_current_date_info(date);
    get_current_time_info(time);

    string updateFileName = UPDATE_FILE_STORED_DIRECTORY_PATH;
    updateFileName.append("/updatefile_").append(date).append("_").append(time);
    
    fs::path updateFile(updateFileName);

    auto body_stream = request.body();
    auto file_stream = concurrency::streams::fstream::open_ostream(
      utility::conversions::to_string_t(updateFile.string()), std::ios::out | std::ios::binary).get();
    file_stream.flush();

    body_stream.read_to_end(file_stream.streambuf()).wait();
    file_stream.close().wait();

    string cmd_exe = "chmod +x ";
    cmd_exe.append(updateFile.string());

    system(cmd_exe.c_str());

    // 여기까진 파일 저장이고
    // 이 아래로는 파일이 KETI-Edge 파일이라고 생각하고 해당파일로 교체후 reboot하는 과정
    
    string shellFile = "/tmp/update_edge.sh";
    // string targetName = "/firmware/KETI-Edge";
    string cmd_update = "";
    cmd_update.append(shellFile).append(" -n").append(updateFile.string());

    system(cmd_update.c_str());

}

void Ipmiweb_POST::Set_Smtp(json::value request_json){
  //  string sender_address,
  //             machine_name,
  //             primary_server_address,
  //             primary_user_name,
  //             primary_user_password,
  //             secondary_server_address,
  //             secondary_user_name,
  //             secondary_user_password;

  //     json::value smtp;
  //     smtp = request_json.at("SMTP");
  //     sender_address = smtp.at("SENDER_ADDRESS").as_string();
  //     machine_name = smtp.at("MACHINE_NAME").as_string();
  //     primary_server_address = smtp.at("PRIMARY_SERVER_ADDRESS").as_string();
  //     primary_user_name = smtp.at("PRIMARY_USER_NAME").as_string();
  //     primary_user_password = smtp.at("PRIMARY_USER_PASSWORD").as_string();
  //     secondary_server_address = smtp.at("SECONDARY_SERVER_ADDRESS").as_string();
  //     secondary_user_name = smtp.at("SECONDARY_USER_NAME").as_string();
  //     secondary_user_password = smtp.at("SECONDARY_USER_PASSWORD").as_string();

  //     set_smtp_sender_machine(sender_address.c_str(), machine_name.c_str());
  //     set_smtp_primary_receiver(primary_server_address.c_str(), primary_user_name.c_str(),
  //                      primary_user_password.c_str());
  //     set_smtp_secondary_receiver(secondary_server_address.c_str(), secondary_user_name.c_str(),
  //                        secondary_user_password.c_str());
      // write_smtp_config_to_file();

      json::value jv_smtp;
      EventService *es = (EventService *)g_record[ODATA_EVENT_SERVICE_ID];

      string machineName, sender, primaryServer, primaryUsername, primaryPassword, secondaryServer, secondaryUsername, secondaryPassword;

      get_value_from_json_key(request_json, "SMTP", jv_smtp);

      get_value_from_json_key(jv_smtp, "MACHINE_NAME", machineName);
      get_value_from_json_key(jv_smtp, "SENDER_ADDRESS", sender);
      get_value_from_json_key(jv_smtp, "PRIMARY_SERVER_ADDRESS", primaryServer);
      get_value_from_json_key(jv_smtp, "PRIMARY_USER_NAME", primaryUsername);
      get_value_from_json_key(jv_smtp, "PRIMARY_USER_PASSWORD", primaryPassword);
      get_value_from_json_key(jv_smtp, "SECONDARY_SERVER_ADDRESS", secondaryServer);
      get_value_from_json_key(jv_smtp, "SECONDARY_USER_NAME", secondaryUsername);
      get_value_from_json_key(jv_smtp, "SECONDARY_USER_PASSWORD", secondaryPassword);

      if(machineName != "")
          temporary.smtp_machineName = machineName;
      if(sender != "")
          es->smtp.smtp_sender_address = sender;
      if(primaryServer != "")
          es->smtp.smtp_server = primaryServer;
      if(primaryUsername != "")
          es->smtp.smtp_username = primaryUsername;
      if(primaryPassword != "")
          es->smtp.smtp_password = primaryPassword;
      if(secondaryServer != "")
          temporary.smtp_second_server = secondaryServer;
      if(secondaryUsername != "")
          temporary.smtp_second_username = secondaryUsername;
      if(secondaryPassword != "")
          temporary.smtp_second_password = secondaryPassword;

  
}

bool Ipmiweb_POST::Set_User(json::value request_json){
  // uint8_t response[QSIZE];
  //     uint8_t header[1000];

  //     // uint8_t username [16], password[16];
  //     char result[20];
  //     string name , index, password, enable_status, callback, linkauth, ipmimsg,priv;

  //     name = request_json.at("NAME").as_string();
  //     index = request_json.at("INDEX").as_string();
  //     password = request_json.at("PASSWORD").as_string();
  //     enable_status = request_json.at("ENABLE_STATUS").as_string();
  //     callback = request_json.at("CALLBACK").as_string();
  //     linkauth = request_json.at("LINKAUTH").as_string();
  //     ipmimsg = request_json.at("IPMIMSG").as_string();
  //     priv = request_json.at("PRIVILEGE").as_string();
  //     cout << "set user json end"<<endl;
  //     bool en_disable ;
  //     if (enable_status=="1") en_disable = true;
  //     else en_disable = false; 
  //     int is_user = new_ipmi_user(name,password,en_disable,atoi(priv.c_str()),atoi(index.c_str()));

    // [테스트] Account 레드피시로 적용
    string user_name, password, privilege, call_in, ipmi_msg, link_auth, enabled;

    get_value_from_json_key(request_json, "NAME", user_name);
    get_value_from_json_key(request_json, "PASSWORD", password);
    get_value_from_json_key(request_json, "PRIVILEGE", privilege);
    get_value_from_json_key(request_json, "ENABLE_STATUS", enabled);
    // get_value_from_json_key(request_json, "CALLIN", call_in);
    get_value_from_json_key(request_json, "CALLBACK", call_in);
    get_value_from_json_key(request_json, "IPMIMSG", ipmi_msg);
    get_value_from_json_key(request_json, "LINKAUTH", link_auth);

    // AccountService enabled 검사는 일단 안할게
    //  근데 패스워드 길이검사나 유저네임 중복검사는 해야됨
    unsigned int min_pass_length = ((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])->min_password_length;
    unsigned int max_pass_length = ((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])->max_password_length;
    if (password.size() < min_pass_length)
    {
        log(warning) << "[Web User Post] : Short Password Length";
        return false;
    }
    if (password.size() > max_pass_length)
    {
        log(warning) << "[Web User Post] : Long Password Length";
        return false;
    }

    Collection *account_col = (Collection *)g_record[ODATA_ACCOUNT_ID];
    std::vector<Resource *>::iterator iter;
    for(iter = account_col->members.begin(); iter != account_col->members.end(); iter++)
    {
        if(((Account *)(*iter))->user_name == user_name)
        {
            log(warning) << "[Web User Post] : Duplicated UserName";
            return false;
        }
    }

    string odata_id = ODATA_ACCOUNT_ID;
    string acc_id = to_string(allocate_numset_num(ALLOCATE_ACCOUNT_NUM));
    odata_id.append("/").append(acc_id);
    string role_id;

    if(privilege == "4")
        role_id = "Administrator";
    else
        role_id = "ReadOnly";

    Account *account = new Account(odata_id, acc_id, role_id);
    account->user_name = user_name;
    account->password = password;
    account->locked = false;

    if(enabled == "1")
        account->enabled = true;
    else if(enabled == "0")
        account->enabled = false;

    account->callin = improved_stoi(call_in);
    account->ipmi = improved_stoi(ipmi_msg);
    account->priv = improved_stoi(privilege);
    account->link_auth = improved_stoi(link_auth);

    account_col->add_member(account);

    return true;

}
int post_str2ulong(const char * str, uint64_t * ulng_ptr)
{
	char * end_ptr = 0;
	if (!str || !ulng_ptr)
		return (-1);

	*ulng_ptr = 0;
	errno = 0;
	*ulng_ptr = strtoul(str, &end_ptr, 0); 

	if (*end_ptr != '\0')
		return (-2);

	if (errno != 0)
		return (-3);

	return 0;
}
int post_str2uchar(const char * str, uint8_t * uchr_ptr)
{
	int rc = (-3);
	uint64_t arg_ulong = 0;
	if (!str || !uchr_ptr)
		return (-1);

	if ( (rc = post_str2ulong(str, &arg_ulong)) != 0 ) {
		*uchr_ptr = 0;
		return rc;
	}

	if (arg_ulong > UINT8_MAX)
		return (-3);

	*uchr_ptr = (uint8_t)arg_ulong;
	return 0;
} /* str2uchar(...) */

void Ipmiweb_POST::Ddns_request_json(json::value &request_json,vector<string> split_tokens){
        vector<string> split_value;
        cout << "split token " << split_tokens[0] << "\n" << split_tokens[1]<<endl;
        json::value dns_info, info, gen, ipv4_info, ipv6_info;
        cout << "split size" <<split_tokens.size()<<endl;

        for (int i = 0; i<split_tokens.size();i++){
          cout << "split value" << endl;
          split_value = string_split(split_tokens[i],'=');
          if(split_value[0]== "DOMAIN_NAME") 
          if(split_value.size() !=2) 
        gen[split_value[0]] = json::value::string(" ");
      else gen[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "HOST_NAME") 
          if(split_value.size() !=2) 
        gen[split_value[0]] = json::value::string(" ");
      else gen[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "REGISTER_BMC")
          if(split_value.size() !=2) 
        gen[split_value[0]] = json::value::string(" ");
        else  gen[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "REGISTER_BMC_METHOD")
          if(split_value.size() !=2) 
        gen[split_value[0]] = json::value::string(" ");
       else   gen[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "IPV4_ALTERNATE") 
          if(split_value.size() !=2) 
        ipv4_info[split_value[0]] = json::value::string(" ");
        else  ipv4_info[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "IPV4_PREFERRED")
          if(split_value.size() !=2) 
        ipv4_info[split_value[0]] = json::value::string(" ");
       else   ipv4_info[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "IPV6_ALTERNATE")
          if(split_value.size() !=2) 
        ipv6_info[split_value[0]] = json::value::string(" ");
        else  ipv6_info[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "IPV6_PREFERRED"){
            if(split_value.size() !=2) 
          ipv6_info[split_value[0]] = json::value::string("-");
          else ipv6_info[split_value[0]] = json::value::string(split_value[1]);
        }    }
        dns_info["GENERIC"] = gen;
        dns_info["IPV4"] = ipv4_info;
        dns_info["IPV6"] = ipv6_info;
        request_json["DNS_INFO"] = dns_info;
        cout << "jv " << request_json<<endl;
}



void Ipmiweb_POST::Radius_request_json(json::value &request_json, vector<string> split_tokens){
  json::value radius_info= json::value::object(), radius = json::value::object();
  cout << "split size" <<split_tokens.size()<<endl;
  vector<string> split_value;
  for (int i = 0; i<split_tokens.size();i++){
    cout << "split value" << endl;
    split_value = string_split(split_tokens[i],'=');
    if(split_value[0]== "IP")
      if(split_value.size() !=2) 
        radius[split_value[0]] = json::value::string(" ");
      else radius[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "PORT")
      if(split_value.size() !=2) 
        radius[split_value[0]] = json::value::string(" ");
      else radius[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "RADIUS_ENABLE")
      if(split_value.size() !=2)  
        radius[split_value[0]] = json::value::string(" ");
      else radius[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "SECRET")
      if(split_value.size() !=2) 
        radius[split_value[0]] = json::value::string(" ");
      else radius[split_value[0]] = json::value::string(split_value[1]);
  }
  // radius_info["RADIUS"] = radius;
  // request_json["RADIUS_INFO"] = radius_info;
  request_json["RADIUS"] = radius;
  cout << "jv " << request_json<<endl;
}


void Ipmiweb_POST::Ssl_request_json(json::value &request_json, vector<string> split_tokens){
  json::value ssl_info= json::value::object(), ssl = json::value::object();
  cout << "split size" <<split_tokens.size()<<endl;
  vector<string> split_value;

  for (int i = 0; i<split_tokens.size();i++){
    cout << "split value" << endl;
    split_value = string_split(split_tokens[i],'=');
    if(split_value[0]== "CITY_OR_LOCALITY")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "COMMON_NAME")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "COUNTRY")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "EMAIL_ADDRESS")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "KEY_LENGTH")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "ORGANIZATION")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "ORGANIZATION_UNIT")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "STATE_OR_PROVINCE")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
    if(split_value[0]== "VALID_FOR")
      if(split_value.size() !=2) 
        request_json[split_value[0]] = json::value::string(" ");
      else request_json[split_value[0]] = json::value::string(split_value[1]);
  }
  // ssl["COUNTRY"]=json::value::string("123");
  // request_json["SSL"] = ssl;
  // request_json["SSL_INFO"] = ssl_info;
  cout << "jv " << request_json<<endl;
  cout <<"ytuy"<<request_json.has_field("COUNTRY")<<endl;
}

void Ipmiweb_POST::Ntp_request_json(json::value &request_jon,vector<string> split_tokens){
    json::value ntp_info= json::value::object() ,ntp=json::value::object();
        cout << "split size" <<split_tokens.size()<<endl;
        vector<string> split_value;
        for (int i = 0; i<split_tokens.size();i++){
          cout << "split value" << endl;
          split_value = string_split(split_tokens[i],'=');
          if(split_value[0]== "AUTO_SYNC")
            ntp[split_value[0]] = json::value::string(split_value[1]);
            
          if(split_value[0]== "YEAR"){
            if(split_value.size() !=2) 
              ntp[split_value[0]] = json::value::string("-");
            else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
          if(split_value[0]== "MONTH"){
            if(split_value.size() !=2)
              ntp[split_value[0]] = json::value::string("-");
              else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
          if(split_value[0]== "DAY"){
            if(split_value.size() !=2) 
              ntp[split_value[0]] = json::value::string("-");
            else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
          if(split_value[0]== "HOUR"){
            if(split_value.size() != 2) 
              ntp[split_value[0]] = json::value::string("-");
            else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
          if(split_value[0]== "MIN"){
            if(split_value.size() !=2) 
              ntp[split_value[0]] = json::value::string("-");
            else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
          if(split_value[0]== "SEC"){
            if(split_value.size() !=2) 
              ntp[split_value[0]] = json::value::string("-");
          else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
       
          if(split_value[0]== "NTP_SERVER"){
            if(split_value.size() !=2) //split_value[1] = "-";
              ntp[split_value[0]] = json::value::string("-");
          else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
          if(split_value[0]== "TIME_ZONE"){
            if(split_value.size() !=2) //split_value[1] = "-";
              ntp[split_value[0]] = json::value::string("");
          else ntp[split_value[0]] = json::value::string(split_value[1]);
          } 
        }
        ntp_info["NTP"] = ntp;
        request_jon["NTP_INFO"] = ntp_info;
        cout << request_jon<<endl;
}


void Ipmiweb_POST::Network_request_json(json::value &request_json, vector<string> split_tokens){
  json::value network_info= json::value::object() ,gen= json::value::object(), ipv4_info= json::value::object(),ipv6_info= json::value::object(),vlan= json::value::object();
        cout << "split size" <<split_tokens.size()<<endl;
        vector<string> split_value;
        for (int i = 0; i<split_tokens.size();i++){
          cout << "split value" <<split_value << endl;
          split_value = string_split(split_tokens[i],'=');
          if(split_value[0]== "LAN_SETTING_ENABLE")
          gen[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "MAC_ADDRESS")
            if(split_value.size() ==2)
              gen[split_value[0]] = json::value::string(split_value[1]);
            else
              gen[split_value[0]] = json::value::string("");

          if(split_value[0]== "IPV4_ADDRESS")
            if(split_value.size() ==2)
              ipv4_info[split_value[0]] = json::value::string(split_value[1]);
            else
              ipv4_info[split_value[0]] = json::value::string("");

          if(split_value[0]== "IPV4_DHCP_ENABLE")
            ipv4_info[split_value[0]] = json::value::string(split_value[1]);

          if(split_value[0]== "IPV4_GATEWAY")
            if(split_value.size() ==2)
              ipv4_info[split_value[0]] = json::value::string(split_value[1]);
            else
              ipv4_info[split_value[0]] = json::value::string("");

          if(split_value[0]== "IPV4_NETMASK")
            if(split_value.size() ==2)
              ipv4_info[split_value[0]] = json::value::string(split_value[1]);
            else
              ipv4_info[split_value[0]] = json::value::string("");

          if(split_value[0]== "IPV4_PREFERRED")
            if(split_value.size() ==2)   
              ipv4_info[split_value[0]] = json::value::string(split_value[1]);

          if(split_value[0]== "IPV6_ENABLE")
          ipv6_info[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "IPV6_ADDRESS")
            if(split_value.size() ==2)
              ipv6_info[split_value[0]] = json::value::string(split_value[1]);
            else ipv6_info[split_value[0]] = json::value::string("");
          if(split_value[0]== "IPV6_DHCP_ENABLE")
          ipv6_info[split_value[0]] = json::value::string(split_value[1]);
          if(split_value[0]== "IPV6_GATEWAY")
            if(split_value.size() ==2)
              ipv6_info[split_value[0]] = json::value::string(split_value[1]);
            else ipv6_info[split_value[0]] = json::value::string("");
          if(split_value[0]== "IPV6_SUBNET_PREFIX_LENGTH")
            if(split_value.size() ==2)
              ipv6_info[split_value[0]] = json::value::string(split_value[1]);
            else ipv6_info[split_value[0]] = json::value::string("");

          if(split_value[0]== "LAN_INTERFACE")
            if(split_value.size() ==2)
              network_info[split_value[0]] = json::value::string(split_value[1]);
            else ipv6_info[split_value[0]] = json::value::string("");
          if(split_value[0]== "NETWORK_PRIORITY")
            if(split_value.size() ==2)
              request_json[split_value[0]] = json::value::string(split_value[1]);
            else ipv6_info[split_value[0]] = json::value::string("");
          
          if(split_value[0]== "VLAN_ID")
            if(split_value.size() ==2)
              vlan[split_value[0]] = json::value::string(split_value[1]);
            else ipv6_info[split_value[0]] = json::value::string("");
          if(split_value[0]== "VLAN_PRIORITY")
            if(split_value.size() ==2)
              vlan[split_value[0]] = json::value::string(split_value[1]);
            else ipv6_info[split_value[0]] = json::value::string("");
          if(split_value[0]== "VLAN_SETTINGS_ENABLE")
          vlan[split_value[0]] = json::value::string(split_value[1]);
       }
        network_info["GENERIC"] = gen;
        network_info["IPV4"] = ipv4_info;
        network_info["IPV6"] = ipv6_info;
        network_info["VLAN"] = vlan;
        
        request_json["NETWORK_INFO"] = network_info;
        cout << request_json<<endl;
}

void Ipmiweb_POST::Smtp_request_json(json::value &request_json, vector<string> split_tokens){
  json::value smtp_info= json::value::object() ,smtp=json::value::object();
  cout << "split size" <<split_tokens.size()<<endl;
  vector<string> split_value;
  for (int i = 0; i<split_tokens.size();i++){
    cout << "split value" << endl;
    split_value = string_split(split_tokens[i],'=');
    if(split_value[0]== "MACHINE_NAME"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
    if(split_value[0]== "PRIMARY_SERVER_ADDRESS"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
    if(split_value[0]== "PRIMARY_USER_NAME"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
    if(split_value[0]== "PRIMARY_USER_PASSWORD"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
    if(split_value[0]== "SECONDARY_SERVER_ADDRESS"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
    if(split_value[0]== "SECONDARY_USER_NAME"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
    if(split_value[0]== "SECONDARY_USER_PASSWORD"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
    if(split_value[0]== "SENDER_ADDRESS"){
      if(split_value.size() != 2) 
        smtp[split_value[0]] = json::value::string("");
      else smtp[split_value[0]] = json::value::string(split_value[1]);
    }
  }
  request_json["SMTP"] = smtp;
  // request_json["SMTP_INFO"] = smtp_info;
}

