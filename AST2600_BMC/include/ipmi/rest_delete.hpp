#pragma once
#include<string>
#include<ipmi/apps.hpp>
#include <redfish/resource.hpp>
using namespace std;

class Ipmiweb_DEL{
    public:
        static bool Del_Usb(json::value request_json);
        static bool Del_User(int index);
};
