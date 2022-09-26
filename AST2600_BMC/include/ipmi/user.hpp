#pragma once
#ifndef __USER_HPP__
#define __USER_HPP__

#include "ipmi/common.hpp"
#include "ipmi/ipmi.hpp"
#include "ipmi/setting_service.hpp"

#define MAX_ID 10
#define FIXED_ID 2
static int usercount = 0;
class Ipmiuser
{
    public:
        std::string name;
        std::string password;
        uint8_t enable;
        uint8_t callin;
        uint8_t link_auth;
        uint8_t ipmi;
        uint8_t priv;
        uint8_t limit;
    
    public:
        Ipmiuser()
        {
            name = "";
            password = "";
            enable = 0;
            callin = 0;
            link_auth = 0;
            ipmi = 0;
            priv = 0;
            limit = 0;
        }
        Ipmiuser(const Ipmiuser *user)
        {
            enable = user->enable;
            callin = user->callin;
            link_auth = user->link_auth;
            ipmi = user->ipmi;
            priv = user->priv;
            limit = user->limit;
            name = user->name;
            password = user->password;
        }
        void setUserPasswd(std::string _passwd);
        void setUserEnable(uint8_t _en);
        void setUserAccess(ipmi_req_t *request);
        void setUserName(std::string _name);
        void deleteUser(uint8_t index);

        uint8_t getUserPriv();
        uint8_t getUserenable();
        uint8_t getUserCallin();
        uint8_t getUserLinkAuth();
        uint8_t getUserIpmi();
        void getUserAccess(uint8_t _flag, uint8_t _val, ipmi_res_t *response, uint8_t *res_len);
        std::string getUsername();
        std::string getUserpassword();
        bool plat_user_init(uint8_t index);
        bool plat_user_init(uint8_t index, std::string _username, std::string _password);
        void printUserInfo(void);
};

int rest_make_user_json(char* res);
void app_set_user_name_params(int index, char* name);
void app_set_user_enable(int index, char enable);
void app_set_user_passwd_params(ipmi_req_t* req, unsigned char* response);
void app_set_user_access_params(ipmi_req_t* req);
void app_del_user(int index);
void plat_user_save(void);

/**
*@brief user config 파일에서 유저정보 읽어들임. 파일이 존재하지 않으면 init
*@author doyoung
*/
int user_loading(void);

/**
*@brief 현재 전체 유저 숫자 구하는 함수
*@author doyoung
*/
int get_user_cnt(void);

#endif