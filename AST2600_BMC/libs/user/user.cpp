
#include "ipmi/user.hpp"
#include <redfish/resource.hpp>

Ipmiuser ipmiUser[MAX_USER];
/**
 *@brief ipmi일경우에만 사용  아닐경우 직접 접근해야함 
 */
void Ipmiuser::setUserAccess(ipmi_req_t *request)
{
    uint8_t en = request->data[0] & 0x80 ? 1 : 0;
    if(en)
    {
        this->ipmi = request->data[0] & 0x10 ? 1 : 0;
        this->link_auth = request->data[0] & 0x20 ? 1 : 0;
        this->callin = request->data[0] & 0x40 ? 1 : 0;
        this->priv = request->data[2] & 0x0F;
    }else{
        this->priv = request->data[2] & 0x0F;    
    }
    plat_user_save();
}
/**
 *@brief ipmi일경우에만 사용  아닐경우 직접 접근해야함 
 */
void Ipmiuser::setUserPasswd(std::string _passwd)
{
    this->password = _passwd;
    char buf[500] = {
        0,
    };
    sprintf(buf, "User Set User Password User ID : %s ", this->name);
    //ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf, "User", "User"));
    plat_user_save();
}

/**
 *@brief ipmi일경우에만 사용  아닐경우 직접 접근해야함 
 */
void Ipmiuser::setUserEnable(uint8_t _en)
{
    this->enable = _en;
    char buf[500] = {
        0,
    };
    sprintf(buf, "User set User Enable User ID : %s ", this->name);
    //ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf, "User", "User"));
    plat_user_save();
}
/**
 *@brief ipmi일경우에만 사용  아닐경우 직접 접근해야함 
 */
void Ipmiuser::setUserName(std::string _name)
{
    
    this->name = _name;
    char buf[500] = {
        0,
    };
    sprintf(buf, "User set User Name User ID : %s ", this->name);
    //ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf, "User", "User"));
    plat_user_save();

}

void Ipmiuser::getUserAccess(uint8_t _flag, uint8_t _val, ipmi_res_t *response, uint8_t *res_len)
{
    uint8_t *data = &response->data[0];

    if(_flag == 1)
    {

        *data++ = MAX_ID;
        *data++ = this->enable;
        *data++ = _val;
        *data++ = (this->callin << 6) | (this->link_auth << 5) | (this->ipmi << 4) | (this->priv & 0xf);
    }
    else if(_flag == 2)
    {
        *data++ = MAX_ID;
        *data++ = (this->enable << 6) | (_val & 0x3f);
        *data++ = FIXED_ID & 0x3f;
        *data++ = (this->callin << 6) | (this->link_auth << 5) | (this->ipmi << 4) | (this->priv & 0xf);
    }
    *res_len = data - &response->data[0];
}


bool Ipmiuser::plat_user_init(uint8_t index)
{
    if(index == 0)
    {
        this->name = "root";
        this->password = "";
        this->enable = 1;
        this->ipmi = 1;
        this->priv = 4;
        this->link_auth = 1;
        this->callin = 1;
        return true;
    }
    else if(index == 1)
    {
        this->name = "admin";
        this->password = "admin";
        this->enable = 1;
        this->ipmi = 1;
        this->priv = 4;
        this->link_auth = 1;
        this->callin = 1;
        return true;
    }
    else
        return false;
}

bool Ipmiuser::plat_user_init(uint8_t index, std::string _username, std::string _password)
{
    if(index >= 2 && index <= 9)
    {
        this->name = _username;
        this->password = _password;
        this->enable = 1;
        this->ipmi = 0;
        this->priv = 0;
        this->link_auth = 0;
        callin = 0;
        return true;
    }
    else
        return false;
}

void Ipmiuser::deleteUser(uint8_t index)
{
    if (index >= 2 && index <= 9){
        this->name = "";
        this->password = "";
        this->enable = 0;
        this->ipmi = 0;
        this->priv = 0;
        this->link_auth = 0;
        this->callin = 0;
    }
    char buf[500] = {
        0,
    };
    sprintf(buf, "User Delete User User ID : %s ", this->name);
    //ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf, "User", "User"));
    return;
}

uint8_t Ipmiuser::getUserPriv()
{
    return this->priv;
}

uint8_t Ipmiuser::getUserenable()
{
    return this->enable;
}

std::string Ipmiuser::getUsername()
{
    return this->name;
}

std::string Ipmiuser::getUserpassword()
{
    return this->password;
}

uint8_t Ipmiuser::getUserCallin()
{
    return this->callin;
}

uint8_t Ipmiuser::getUserLinkAuth()
{
    return this->link_auth;
}

uint8_t Ipmiuser::getUserIpmi()
{
    return this->ipmi;
}

void Ipmiuser::printUserInfo(void)
{
    std::cout << "print user info" << std::endl;
    std::cout << "username : " << this->name << std::endl;
    std::cout << "password : " << this->password << std::endl;
    std::cout << "enable : " << to_string(this->enable) << std::endl;
    std::cout << "callin : " << to_string(this->callin) << std::endl;
    std::cout << "link_auth : " << to_string(this->link_auth) << std::endl;
    std::cout << "ipmi : " << to_string(this->ipmi) << std::endl;
    std::cout << "priv : " << to_string(this->priv) << std::endl;
    std::cout << "limit : " << to_string(this->limit) << std::endl;
}

int get_user_cnt(void)
{
    int cnt;
    for(cnt = 1; cnt < MAX_USER; cnt++){
        if(ipmiUser[cnt].getUsername() == "")
            break;
    }
    return cnt;
}

int rest_make_user_json(char* res) {
    json::value obj = json::value::object();
    json::value USER = json::value::object();
    vector<json::value> INFO_VEC;
    Ipmiuser temp;
    int last_index = 0;
    int user_cnt = 0;

    last_index = user_loading();
    user_cnt = get_user_cnt();
    for (int i = 0; i < user_cnt; i++){
        json::value INFO = json::value::object();
        
        INFO["INDEX"] = json::value::string(U(to_string(i+1)));
        INFO["NAME"] = json::value::string(ipmiUser[i].getUsername());
        INFO["PASSWORD"] = json::value::string(ipmiUser[i].getUserpassword());
        INFO["ENABLE_STATUS"] = json::value::string(U(to_string(ipmiUser[i].getUserenable())));
        INFO["CALLIN"] = json::value::string(U(to_string(ipmiUser[i].getUserCallin())));
        INFO["LINKAUTH"] = json::value::string(U(to_string(ipmiUser[i].getUserLinkAuth())));
        INFO["IPMIMSG"] = json::value::string(U(to_string(ipmiUser[i].getUserIpmi())));
        INFO["PRIVILEGE"] = json::value::string(U(to_string(ipmiUser[i].getUserPriv())));

        INFO_VEC.push_back(INFO);
    }
    
    USER["INFO"] = json::value::array(INFO_VEC);
    obj["USER"] = USER;
    strncpy(res, obj.serialize().c_str(), obj.serialize().length());
    return strlen(res);
}

void plat_user_save(void)
{
    cout << "plat_user_save " << endl;
    for (int i = 0; i < get_user_cnt(); i++)
    {
        Ipmiuser *user = &ipmiUser[i];
        string _uri = ODATA_ACCOUNT_ID + string("/") + to_string(i);
        
        if (record_is_exist(_uri))
        {
        
            Account *account = (Account *)g_record[_uri];
            account->user_name = user->getUsername();
            account->password = user->getUserpassword();
            account->callin = (int)user->callin;
            account->ipmi = (int)user->ipmi;
            account->link_auth = (int)user->link_auth;
            account->priv = (int)user->priv;
            resource_save_json(account);
        }

    }
    
    
}

void app_set_user_name_params(int index, char* name){
    std::string s_name(name);
    ipmiUser[index].setUserName(s_name);
    plat_user_save();
    return;
}

void app_set_user_enable(int index, char enable){
    ipmiUser[index].setUserEnable(enable);
    plat_user_save();
    return;
}

void app_set_user_passwd_params(ipmi_req_t* req, unsigned char* response){
    ipmi_res_t *res = (ipmi_res_t *)response;
    int index = (req->data[0] & 0x1F) - 1;
    int operation = req->data[1] & 0x03;
    char password[20];
	        
    switch (operation){
        case 0: 
            ipmiUser[index].setUserEnable(req->data[1]);
            res->cc = CC_SUCCESS;
            break;
        case 1:
            ipmiUser[index].setUserEnable(req->data[1]);
            res->cc = CC_SUCCESS;
            break;
        case 2:{
            memcpy(password, req->data+2, 20);	
            string s_passwd(password); 
            ipmiUser[index].setUserPasswd(s_passwd);
            res->cc = CC_SUCCESS;
            break;
        }
        case 3:{
            memcpy(password, req->data+2, 20);	
            string s_passwd(password); 
            if (s_passwd == ipmiUser[index].getUserpassword())
                res->cc = CC_SUCCESS;
            break;
        }
    }
    plat_user_save();
    return;
}

void app_set_user_access_params(ipmi_req_t* req){
    int index = req->data[1] - 1;
    ipmiUser[index].setUserAccess(req);
    return;
}

void app_del_user(int index) {
    if (index < 2)
        fprintf(stderr, "Error : user %s cannot be deleted\n", ipmiUser[index].getUsername().c_str());
    else{
        int user_cnt;
        user_cnt = get_user_cnt();
        if (index < user_cnt - 1){
            for(int i = index; i < user_cnt; i++){
                ipmiUser[i] = Ipmiuser(ipmiUser[i+1]);
            }
            ipmiUser[user_cnt-1].deleteUser(user_cnt-1);
        }else
            ipmiUser[index].deleteUser(index);
    }
    usercount =usercount-1;
    
    plat_user_save();
    return;
}
/**
 * @brief 유저읽기 
 * @bug IPMI와 redfish간에 동기화문제 발생 
 * 처음 수행시 redfish가 없을때를 대비한 소스코드 필요함
 * 읽어들이는 방식이아닌 무조건 root/ admin을 생성하지만 redfish에서 road json 수행시 redfish를 읽어서 생성함
 */
int user_loading(void){
    log(info) << "user_loading start";
    // FILE* fp = fopen(IPMI_USER_PATH, "rb");
     int last_index = 2;
    // if (fp != NULL){
    //     fseek(fp, 0, SEEK_SET);
    //     while(!feof(fp)){
    //         fread(&ipmiUser[last_index], sizeof(Ipmiuser), 1, fp);
    //         last_index++;
    //     }
    //     fclose(fp);
    // }else{
        fprintf(stderr, "\t\tWarning : NO USER CONFIG FILE\n");
    	ipmiUser[0].plat_user_init(0);
    	ipmiUser[1].plat_user_init(1);
        last_index += 2;
        //plat_user_save();
    //}

    for(int i = 0; i < get_user_cnt(); i++){
        ipmiUser[i].printUserInfo();
    }
    log(info) << "user_loading complete";
    return last_index;
}