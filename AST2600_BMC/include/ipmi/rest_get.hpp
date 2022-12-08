#pragma once
#include<string>
#include<ipmi/apps.hpp>
#include<ipmi/user.hpp>
#include<ipmi/fru.hpp>
#include<ipmi/sdr.hpp>
#include<ipmi/sel.hpp>
#include<redfish/resource.hpp>
#include<sstream>
#include <sys/msg.h>
using namespace std;
// extern string chassis_type_desc_fru[100];
#define MAX_NIC 2
#define QSIZE 30000

class Ipmiweb_GET{
    public:
        static void Get_Show_Main(int menu, json::value &response_json);
        static void Get_Fru_Info(json::value &response_json);
        static void Get_Sensor_Info(json::value &response_json);
        static void Get_Eventlog(json::value &response_json);
        static void Get_DDNS_Info(json::value &response_json);
        static void Get_Lan_Info(json::value &response_json);
        static void Get_Ntp_Info(json::value &response_json);
        static void Get_Smtp_Info(json::value &response_json, int flag);
        static void Get_Ssl_Info(json::value &response_json);
        static void Get_Active_Dir(json::value &response_json);
        static void Get_Ldap(json::value &response_json);
        static void Get_Radius(json::value &response_json);
        static void Get_Setting_Service(json::value &response_json);
        static void Get_KVM(json::value &response_json);
        static void Get_Power_Satus(json::value &response_json);
        static void Get_Usb_Info(json::value &response_json);
        static void Get_Sys_Info(json::value &response_json);
        // static void Get_Sol(http_response &response);
        static void Get_User_List(json::value &response_json);
        static void Get_Watt_Info(int menu,json::value &response_json);
        static void EXIT_KVM_Cleanup();
        
        // static void signalHandler(int signo);
        
};  
static int rest_send_to_kvm();


// 웹 임시 변수
typedef struct _Temporary_Web_Values
{
    // dns
    string dns_registerBMC = "1";
    string dns_registerBMCMethod = "DIRECT";

    // network
    string net_priority = "eth0";
    string net_v6_enabled = "0";
    string net_v6_dhcp = "0";
    string net_vlan_prior = "0";

    // smtp
    string smtp_machineName = "SMTP_Machine";
    string smtp_second_server = "";
    string smtp_second_username = "";
    string smtp_second_password = "";

    // ssl
    string ssl_serialNumber = "";
    string ssl_countryName = "";
    string ssl_state = "";
    string ssl_city = "";
    string ssl_org = "";
    string ssl_orgUnit = "";
    string ssl_commonName = "";
    string ssl_email = "";
    string ssl_validBefore = "";
    string ssl_validAfter = "";
    string ssl_keyLength = "";

    // setting
    string setting_webPort = "80";
    string setting_kvmPort = "7887";
    string setting_kvmProxyPort = "8877";
    string setting_alertEnabled = "0";
    string setting_alertPort = "25";
    string setting_sshEnabled = "0";
    string setting_sshPort = "22";

    // AD
    string ad_enabled = "0";
    string ad_username = "";
    string ad_password = "";
    string ad_domain = "";
    string ad_domainControllerAddress = "";

    // LDAP
    string ldap_enabled = "0";
    string ldap_encryptionType = "0";
    string ldap_serverAddress = "";
    string ldap_port = "";
    string ldap_bindDN = "";
    string ldap_bindPW = "";
    string ldap_baseDN = "";
    string ldap_timeout = "";

    // Radius
    string radius_enabled = "0";
    string radius_serverAddress = "";
    string radius_port = "";
    string radius_secret = "";



} TempWebValues;
