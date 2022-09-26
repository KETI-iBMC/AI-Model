#pragma once
#include<string>
#include<ipmi/apps.hpp>
#include<ipmi/user.hpp>
#include<ipmi/fru.hpp>
#include<ipmi/sdr.hpp>
#include<ipmi/network.hpp>
#include<redfish/resource.hpp>
#include<ipmi/user.hpp>
#include<ipmi/ipmi.hpp>
using namespace std;
#define QSIZE 35000
#define IPMI_PASSWORD_ENABLE_USER   0x01
#define IPMI_PASSWORD_SET_PASSWORD  0x02

#define UPDATE_FILE_STORED_DIRECTORY_PATH "/tmp/tmp_update"
class Ipmiweb_POST{
    public:
        static int Try_Login(string username , string pwd);
        static void Set_Ddns(json::value request_json);
        static void Set_Network(json::value request_json);
        static void Set_Ntp(json::value request_json);
        static void Set_Ssl(json::value request_json);
        static void Set_AD(json::value request_json);
        static void Set_Smtp(json::value request_json);
        static void Set_Radius(json::value request_json);
        static void Set_Power(json::value request_json);
        static bool Set_Usb(json::value request_json);
        static void Set_BMC_Reset();
        static void Set_Warm_Reset();
        static void Set_Upload(http_request request);
        static bool Set_User(json::value request_json);
        

        static void Ddns_request_json(json::value &request_json, vector<string> split_tokens);
        static void Radius_request_json(json::value &request_json, vector<string> split_tokens);
        static void Ssl_request_json(json::value &request_json, vector<string> split_tokens);
        static void Ntp_request_json(json::value &request_json, vector<string> split_tokens);
        static void Network_request_json(json::value &request_json, vector<string> split_tokens);
        static void Smtp_request_json(json::value &request_json, vector<string> split_tokens);



};      


static int umount_cmd() {
  uint8_t cmds[100] = {0};
  uint8_t response[10000], result[10000] = {0};
  sprintf(cmds, "mount | grep /nfs_check");
  FILE *p = popen(cmds, "r");
  if (p != NULL) {
    while (fgets(result, sizeof(result), p) != NULL)
      strncat(response, result, strlen(result));
    pclose(p);
  }

  if (strlen(response) > 0) {
    // umount의 표준 에러 를 출력으로 redirection 후 띄우지않음.
    sprintf(cmds, "umount -l /nfs_check > /dev/null 2>&1");
    int rets = system(cmds);
    if (rets == 0)
      return 0;
    else
      return -1;
  } else
    return 0;
}