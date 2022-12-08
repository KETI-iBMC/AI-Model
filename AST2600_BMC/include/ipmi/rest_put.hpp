#pragma once
#include<string>
#include<iostream>
#include<ipmi/apps.hpp>
#include<ipmi/user.hpp>
#include<ipmi/fru.hpp>
#include<ipmi/sdr.hpp>
#include<redfish/resource.hpp>

using namespace std;
#define QSIZE 35000

class Ipmiweb_PUT{
    public:
        static void Set_Fru(json::value request_json);
        static void Set_Sensor(json::value request_json);
        static void Set_Ldap(json::value request_json);
        static void Set_Usb(json::value request_json, json::value &response_json);
        static void Set_Setting(json::value request_json);

        // static void rest_set_fru_board(int id,char f_mfg_date[4], string f_mfg, string f_product, string f_serial, string f_part_num);
};

