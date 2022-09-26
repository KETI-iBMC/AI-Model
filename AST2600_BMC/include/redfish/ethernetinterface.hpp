#ifndef __ETHERNET_HPP__
#define __ETHERNET_HPP__

#include <redfish/stdafx.hpp> 


typedef struct _Ethernet_Patch_Info
{
    bool op_enabled=false;
    bool val_enabled;
    bool op_description=false;
    string val_description;
    bool op_hostname=false;
    string val_hostname;
    bool op_mac_address=false;
    string val_mac_address;
    bool op_mtu=false;
    int val_mtu;
    bool op_v4_address=false;
    string val_v4_address;
    bool op_v4_netmask=false;
    string val_v4_netmask;
    bool op_v4_gateway=false;
    string val_v4_gateway;
} Ethernet_Patch_Info;

typedef struct _Add_Ethernet_Info
{
    string address = "";
    string netmask = "";
    string gateway = "";
    string mtu = "";
    string mac = "";

} Add_Ethernet_Info;

// 파일변경
// void read_file_line_by_line(string _filename, vector<string> &_lines);
// void write_file_line_by_line(string _filename, vector<string> _lines);
// void append_newline_and_push_back(string _str, vector<string> &_vec);
void change_hosts_file(string _target, string _new);
void change_hostname_file(string _new);
void change_interface_file(string _dev, string _keyword, string _value);
bool find_ethernet_dev_block(string _line, string _search_type);
bool find_ethernet_keyword(string _line, string _keyword, int &pos);


void change_web_app_file(string _origin, string _new);

void add_keyword_part_to_interface_file(string _keyword, string _value, vector<string> &_write);
void add_keyword_whole_to_interface_file(string _dev, string _keyword, string _value, vector<string> &_write);
void add_ethernet_to_interface_file(string _dev, Add_Ethernet_Info _info);




#endif