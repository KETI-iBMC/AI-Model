#include "ipmi/rest_get.hpp"
#include "redfish/resource.hpp"
#include "redfish/stdafx.hpp"
#include <cstddef>
#include <cstring>
#include <ipmi/rest_put.hpp>
#include <time.h>
extern std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;

extern TempWebValues temporary;


void Ipmiweb_PUT::Set_Fru(json::value request_json){
  json::value myobj, board_obj, product_obj, chassis_obj;
  string mfg, mfg_date, part_num, serial, product;
  string p_mfg, p_name, p_part_num, p_serial, p_version;
  string c_part_num, c_serial, c_type;
  int years, month, days, hours, minutes, secs;
  int id;
  unsigned char datetime[30];
  unsigned char mfg_dates[4];
  // struct tm time;
  // time_t c_time;

  // [테스트] Date 변경 오류 수정
  time_t init_time;
  struct tm *time_tm;
  std::time(&init_time);
  time_tm = localtime(&init_time);
  
  // [테스트] ID가 integer/string 예외처리
  int id_int;
  string id_str;
  if(get_value_from_json_key(request_json, "ID", id_int) == true)
    id = id_int;

  if(get_value_from_json_key(request_json, "ID", id_str) == true)
    id = improved_stoi(id_str);

  // id = request_json.at("ID").as_integer();

  board_obj = request_json.at("BOARD");
  mfg = board_obj.at("MFG").as_string();
  mfg_date = board_obj.at("MFG_DATE").as_string();
  part_num = board_obj.at("PART_NUM").as_string();
  serial = board_obj.at("SERIAL").as_string();
  product = board_obj.at("PRODUCT").as_string();

  cout << "mfg " << mfg<<endl;

  product_obj = request_json.at("PRODUCT");
  p_mfg = product_obj.at("MFG").as_string();
  p_name = product_obj.at("NAME").as_string();
  p_part_num = product_obj.at("PART_NUM").as_string();
  p_serial = product_obj.at("SERIAL").as_string();
  p_version = product_obj.at("VERSION").as_string();

  chassis_obj = request_json.at("CHASSIS");
  c_part_num = chassis_obj.at("PART_NUM").as_string();
  c_serial = chassis_obj.at("SERIAL").as_string();
  c_type = chassis_obj.at("TYPE").as_string();

  cout<< "mfg" << mfg<<endl; 
  cout<< "mfg_date " << mfg_date<<endl;
  cout << "part_num "<<part_num<<endl;
  cout << "serial " << serial<<endl;
	cout << "product " << product<<endl;
  cout<< "p_mfg " << p_mfg<<endl;
  cout << "p_part_num "<<p_part_num<<endl;
  cout << "p_serial " << p_serial<<endl;
	cout << "p_version " << p_version<<endl;
	cout << "c_part_num " << c_part_num<<endl;
	cout << "c_serial " << c_serial<<endl;
	cout << "c_type " << c_type<<endl;


  // struct tm temp;
  // memset(&temp, 0, sizeof(struct tm));
  // if (strptime(mfg_date.c_str(), "%F %T", &temp) == NULL) {
  if (strptime(mfg_date.c_str(), "%F %T", time_tm) == NULL) {
    fprintf(stderr, "\t\tWarning : Date Format is invalid\n");
  }

  // [테스트] 불필요 코드 제거하고 여기서 구한 time_t 바로 사용
  time_t time_tt = mktime(time_tm);

  // cout << "TMTM >>>>>>>>>> " << endl;
  // cout <<"year " <<time_tm->tm_year <<endl;
  // cout <<"month " <<time_tm->tm_mon+1 <<endl;
  // cout <<"days " <<time_tm->tm_mday <<endl;
  // cout <<"hours " <<time_tm->tm_hour <<endl;
  // cout <<"minutes " <<time_tm->tm_min <<endl;
  // cout <<"secs " <<time_tm->tm_sec <<endl;

  // cout << "TIME_T >>>>>>>>>>> " << endl;
  // cout << time_tt << endl;

  string hdr_board ="4" ,hdr_product="2" ,hdr_chassis="3" ;

  rest_set_fru_header(id, hdr_board.c_str(), hdr_product.c_str(), hdr_chassis.c_str());

  // // // convert time_t -> char [4] (ex. 1609459200 -> 5f ee 66 00)
  // mfg_dates[0] = c_time >> 24;
  // mfg_dates[1] = (c_time >> 16) & 0xff;
  // mfg_dates[2] = (c_time >> 8) & 0xff;
  // mfg_dates[3] = c_time & 0xff;

  mfg_dates[0] = time_tt >> 24;
  mfg_dates[1] = (time_tt >> 16) & 0xff;
  mfg_dates[2] = (time_tt >> 8) & 0xff;
  mfg_dates[3] = time_tt & 0xff;
  
  cout<<"mfg_data 0 " <<mfg_dates[0] << endl;
  cout<<"mfg_data 1 " <<mfg_dates[1] << endl;
  cout<<"mfg_data 2 " <<mfg_dates[2] << endl;
  cout<<"mfg_data 3 " <<mfg_dates[3] << endl;
  // vector<string> split_mfg = string_split(mfg,' ');
  // vector<string> split_product = string_split(product,'.');
  // // vector<string> split_mfg = string_split(mfg,'.');
  // split_mfg[0]+=".";
  // split_product[0]+=".";


  rest_set_fru_board(id, mfg_dates, mfg.c_str(), product.c_str(), serial.c_str(), part_num.c_str());
  cout <<"end fru board" << endl;

  rest_set_fru_product(id, p_name.c_str(), p_mfg.c_str(), p_version.c_str(),
                    p_serial.c_str(), p_part_num.c_str());
  rest_set_fru_chassis(id, c_type.c_str(), c_serial.c_str(), c_part_num.c_str());
  plat_fru_device_save();
  cout << " set fru end " << endl;
  
}


void Ipmiweb_PUT::Set_Sensor(json::value request_json){
   string s_name, thresh;
      int num = 0;
      json::value temp;
      float unr, uc, unc, lnc, lc, lnr;
      float th_data[10] = {0};
      json::value myobj;
      // strncpy(data, hm->body.p, b_len);
      // string sdata((char *)data);
      // cout << "sdata in sensor_call : " << sdata << endl;

      // myobj = json::value::parse(sdata);
      
      s_name = request_json.at("SENSOR").as_string();
      unr = stof(request_json.at("UNR").as_string());
      uc = stof(request_json.at("UC").as_string());
      unc = stof(request_json.at("UNC").as_string());
      lnc = stof(request_json.at("LNC").as_string());
      lc = stof(request_json.at("LC").as_string());
      lnr = stof(request_json.at("LNR").as_string());

      std::map<uint8_t, Ipmisdr>::iterator ptr;


      json::value response_json;
      Ipmiweb_GET::Get_Sensor_Info(response_json);
      cout << "get sensor info" <<endl;
      json::value sensor_list, sensor;
      string sensor_name;

      sensor_list = response_json.at("SENSOR_INFO").at("SENSOR");
      
      for (auto sensor : sensor_list.as_array()) {
        sensor_name = sensor.at("NAME").as_string();
        if (sensor_name == s_name) {
          num = stoi(sensor.at("NUMBER").as_string());
          cout << "sensor num insert"<<num <<endl;
          break;
        }
      }
   
    th_data[0] = uc;
    th_data[1] = unc;
    th_data[2] = unr;
    th_data[3] = lc;
    th_data[4] = lnc;
    th_data[5] = lnr;
    th_data[6] = (float)num;

    uint8_t s_num = num;
    int s_index = 0;
    uint8_t sdr_idx;
    sensor_thresh_t *p_sdr;


    sdr_idx = plat_find_sdr_index(num);
    p_sdr = sdr_rec[sdr_idx].find(sdr_idx)->second.sdr_get_entry();

    for (int i = 0; i < 10; i++) {
      printf("th_data %d : %f\n", i, th_data[i]);
      printf("convert_data %d : %f\n", i,
      sdr_convert_sensor_value_to_raw(p_sdr, th_data[i]));
    }

    printf("\t\tdy : before sensor\n");
    sdr_rec[sdr_idx].find(sdr_idx)->second.print_sensor_info();
    if (p_sdr != NULL) {
      if (p_sdr->uc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->uc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[0]);
        printf("\t\t uc updated\n");
      }
      if (p_sdr->unc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->unc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[1]);
        printf("\t\t unc updated\n");
      }
      if (p_sdr->unr_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->unr_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[2]);
        printf("\t\t unr updated\n");
      }
      if (p_sdr->lc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->lc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[3]);
        printf("\t\t lc updated\n");
      }
      if (p_sdr->lnc_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->lnc_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[4]);
        printf("\t\t lnc updated\n");
      }
      if (p_sdr->lnr_thresh != THRESH_NOT_AVAILABLE) {
        p_sdr->lnr_thresh = sdr_convert_sensor_value_to_raw(p_sdr, th_data[5]);
        printf("\t\t lnr updated\n");
      }
      file_store_sdr_data(s_index, (sdr_rec_t *)p_sdr);
    }
      sdr_rec[sdr_idx].find(sdr_idx)->second.print_sensor_info();
    
}
static int post_umount() {
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

void Ipmiweb_PUT::Set_Usb(json::value request_json,json::value &response_json){
      // uint8_t ip_addr[50], path[10], user[20], pwd[20] = {0};
      // uint8_t result[1000], response[100][3000] = {0};
      // uint8_t userset = 0 , cmds[300]={0,};
      // json::value myobj;
      // FILE *p = NULL;
      // string u_ip, u_path, u_user, u_pwd;
      // // string sdata((char *)data);

      // // myobj = json::value::parse(sdata);

      // u_ip = request_json.at("IP_ADDRESS").as_string();
      // u_path = request_json.at("PATH").as_string();
      // u_user = request_json.at("USER").as_string();
      // u_pwd = request_json.at("PASSWORD").as_string();

      // strcpy(ip_addr, u_ip.c_str());
      // strcpy(path, u_path.c_str());
      // strcpy(user, u_user.c_str());
      // strcpy(pwd, u_pwd.c_str());

      // if (strlen(user) > 0)
      //   userset = 1;
      // else if (strlen(user) == 0)
      //   userset = 0;
      // int u_ret, lp = 0;

      // uint8_t u_lp = 0;
      // u_ret = post_umount();

      // sprintf(cmds, "mount -t nfs %s:%s /nfs_check", ip_addr, path);
      // u_ret = system(cmds);

      // sprintf(cmds, "ls -lphc /nfs_check | grep -v / | grep .iso");

      // p = popen(cmds, "r");

      // if (p != NULL) {
      //   while (fgets(result, sizeof(result), p) != NULL) {
      //     strcpy(response[lp], result);
      //     response[lp][strlen(response[lp]) - 1] = '\0';
      //     lp++;
      //   }
      //   for (u_lp = 0; u_lp < lp; u_lp++) {
      //   }
      //   pclose(p);
      // }

      // json::value obj = json::value::object();
      // std::vector<json::value> files_vec;

      // uint8_t query_string[20000], temp_string[20000] = {0};
      // uint8_t temp_data[12][200] = {0};
      // uint8_t u_year[5], u_time[10], u_month[5], u_date[3] = {0};
      // uint8_t u_index[lp][3], u_size[lp][20], u_ctime[lp][150], u_name[lp][50];
      // uint8_t *ptr = NULL;
      // uint8_t header[1000] = {0};
      // uint8_t u_lt = 0;
      // u_lp = 0;

      // for (u_lp = 0; u_lp < lp; u_lp++) {
      //   json::value files = json::value::object();

      //   ptr = strtok(response[u_lp], " ");
      //   u_lt = 0;
      //   while (ptr != NULL) {
      //     if (u_lt == 4)
      //       strcpy(u_size[u_lp], ptr);
      //     if (u_lt == 6)
      //       strcpy(u_month, ptr);
      //     if (u_lt == 7)
      //       strcpy(u_date, ptr);
      //     if (u_lt == 8)
      //       strcpy(u_time, ptr);
      //     if (u_lt == 9)
      //       strcpy(u_year, ptr);
      //     if (u_lt == 10)
      //       strcpy(u_name[u_lp], ptr);

      //     ptr = strtok(NULL, " ");
      //     u_lt++;
      //   }
      //   sprintf(u_ctime[u_lp], "%s %s %s %s", u_year, u_month, u_date, u_time);

      //   files["INDEX"] = json::value::number(u_lp);
      //   files["SIZE"] = json::value::string(U((char *)u_size[u_lp]));
      //   files["CREATE_TIME"] = json::value::string(U((char *)u_ctime[u_lp]));
      //   files["NAME"] = json::value::string(U((char *)u_name[u_lp]));
      //   files_vec.push_back(files);
      // }
      // response_json["FILES"] = json::value::array(files_vec);

      // [테스트] PUT usb를 VM 리소스 생성으로 변경
      string host, directoryPath, user, password;
      string image;

      get_value_from_json_key(request_json, "IP_ADDRESS", host);
      get_value_from_json_key(request_json, "PATH", directoryPath);
      get_value_from_json_key(request_json, "USER", user);
      get_value_from_json_key(request_json, "PASSWORD", password);

      // put 할때 host, directoryPath 유효검사
      if(!validateIPv4(host)){
        // cout << " 11 ? " << endl;
          response_json["CODE"] = json::value::string("400");
          return ;
      }
      if(directoryPath[0] != '/'){
        // cout << " 22 ? " << endl;
          response_json["CODE"] = json::value::string("400");
          return ;
      }

      image = host + ":" + directoryPath;

      cout << "PUT USB INPUT CHECK START!!!!! " << endl;
      cout << host << endl;
      cout << directoryPath << endl;
      cout << user << endl;
      cout << password << endl;
      cout << image << endl;
      cout << "PUT USB INPUT CHECK END!!!!! " << endl;

      // VM 만들고 값 넣어줘야하는것들
      // image, username, password, inserted, write_protected, create_time, 정도만 일단 넣어두고
      // size랑 image_name 은 마운트 후에..
      // ID가 어쨌든 ALLOCATE를 거쳐야해서 먼저 그거부터 수정해야할거같은데? 아니면 걍 만들어보고 출력되는걸 보든가
      string odata_id = ODATA_MANAGER_ID;
      odata_id.append("/VirtualMedia");
      Collection *vm_col = (Collection *)g_record[odata_id];
      
      string vm_id = to_string(allocate_numset_num(ALLOCATE_VM_USB_NUM));
      odata_id.append("/USB").append(vm_id);
      VirtualMedia *vm = new VirtualMedia(odata_id);

      vm->id = get_current_object_name(odata_id, "/");
      vm->media_type.push_back("USBStick");
      vm->create_time = currentDateTime();
      vm->image = image;
      vm->user_name = user;
      vm->password = password;
      vm->inserted = false;
      vm->write_protected = true;
      vm->connected_via = "URI";
      vm->image_name = directoryPath; // get_current_object_name(image, "/");
      // vm->size = "10MB";
      // vm->image_name = "Mount is Mountain";

      vm_col->add_member(vm);

      resource_save_json(vm_col);
      resource_save_json(vm);

      response_json["CODE"] = json::value::string("200");
      response_json["MESSAGE"] = json::value::string("SUCCESS");
}


void Ipmiweb_PUT::Set_Setting(json::value request_json){
  //  uint8_t res_msg[QSIZE], response[QSIZE];
  //     int rets, res_len = 0;

  //     json::value response_json = json::value::object();
  //     json::value SETTING_SERVICE = json::value::object();

  //     if ((access(SSH_SERVICE_BIN, F_OK) != 0) || (access(KVM_PORT_BIN, F_OK) != 0) || (access(ALERT_PORT_BIN, F_OK) != 0) || (access(WEB_PORT_BIN, F_OK) != 0)) {	
  //       if (init_setting_service() != 0)
  //         return FAIL;
  //     }

  //     SETTING_SERVICE["WEB_PORT"] = json::value::string(U(g_setting.web_port));
  //     SETTING_SERVICE["SSH_ENABLES"] = json::value::number(g_setting.ssh_enables);
  //     SETTING_SERVICE["SSH_PORT"] = json::value::string(U(g_setting.ssh_port));
  //     SETTING_SERVICE["ALERT_ENABLES"] = json::value::number(g_setting.alert_enables);
  //     SETTING_SERVICE["ALERT_PORT"] = json::value::string(U(g_setting.alert_port));
  //     SETTING_SERVICE["KVM_ENABLES"] = json::value::number(g_setting.kvm_enables);
  //     SETTING_SERVICE["KVM_PORT"] = json::value::string(U(g_setting.kvm_port));
  //     SETTING_SERVICE["KVM_PROXY_PORT"] = json::value::string(U(g_setting.kvm_proxy_port));
  //     response_json["SETTING_SERVICE"] = SETTING_SERVICE;
      
  //     char result[20];
  //     uint8_t cmds[300]={0,};
  //     json::value setting = response_json.at("SETTING_SERVICE");
  //     string origin_value, req_value;

  //     origin_value = setting.at("WEB_PORT").as_string();
  //     req_value = request_json.at("WEB_PORT").as_string();

  //     if (origin_value != req_value) {
  //       rets = set_setting_service(0, req_value.c_str());

  //     }

  //     origin_value = to_string(setting.at("SSH_ENABLES").as_integer());
  //     req_value = request_json.at("SSH_ENABLES").as_string();

  //     if (req_value == "0") {
  //       rets = set_setting_service(1, req_value.c_str());

  //     } else {
  //       req_value = request_json.at("SSH_PORT").as_string();

  //       rets = set_setting_service(1, req_value.c_str());

  //     }

  //     origin_value = to_string(setting.at("ALERT_ENABLES").as_integer());
  //     req_value = request_json.at("ALERT_ENABLES").as_string();

  //     if (req_value == "0") {
  //       rets = set_setting_service(2, req_value.c_str());

  //     } else {
  //       memset(cmds, 0, sizeof(cmds));
  //       req_value = request_json.at("ALERT_PORT").as_string();

  //       rets = set_setting_service(2, req_value.c_str());

  //     }

  //     origin_value = setting.at("ALERT_PORT").as_string();
  //     req_value = request_json.at("ALERT_PORT").as_string();

  //     if (strcmp(origin_value.c_str(), req_value.c_str()) != 0) {
  //       rets = set_setting_service(2, req_value.c_str());
     
  //     }

  //     string d_port, d_proxy, o_port, o_proxy;

  //     o_proxy = setting.at("KVM_PROXY_PORT").as_string();
  //     d_proxy = request_json.at("KVM_PROXY_PORT").as_string();

  //     o_port = setting.at("KVM_PORT").as_string();
  //     d_port = request_json.at("KVM_PORT").as_string();

  //     if ((o_port != d_port) && (o_proxy == d_proxy)) {
  //       rets = set_setting_service(3, d_port.c_str());

  //     }
  //     if ((o_port == d_port) && (o_proxy != d_proxy)) {
  //       rets = set_setting_service(4, d_proxy.c_str());
  
  //     }
  //     if ((o_port != d_port) && (o_proxy != d_proxy)) {
  //       memset(cmds, 0, sizeof(cmds));
  //       sprintf(cmds, "req setting set 5 %d %d %s %s", strlen(d_port.c_str()),
  //               strlen(d_proxy.c_str()), d_port.c_str(), d_proxy.c_str());
  //       rets = system(cmds);
       
  //     }

    // [테스트] setting 임시 처리
    string webPort, kvmPort, kvmProxyPort, alertEnabled, alertPort, sshEnabled, sshPort;

    get_value_from_json_key(request_json, "WEB_PORT", webPort);
    get_value_from_json_key(request_json, "KVM_PORT", kvmPort);
    get_value_from_json_key(request_json, "KVM_PROXY_PORT", kvmProxyPort);
    get_value_from_json_key(request_json, "ALERT_ENABLES", alertEnabled);
    get_value_from_json_key(request_json, "ALERT_PORT", alertPort);
    get_value_from_json_key(request_json, "SSH_ENABLES", sshEnabled);
    get_value_from_json_key(request_json, "SSH_PORT", sshPort);

    if(isNumber(kvmPort))
    {
        int tmp_port = improved_stoi(kvmPort);
        if(tmp_port <= 65535 && tmp_port >= 0)
        {
            temporary.setting_kvmPort = kvmPort;
        }
    }

    if(isNumber(kvmProxyPort))
    {
        int tmp_port = improved_stoi(kvmProxyPort);
        if(tmp_port <= 65535 && tmp_port >= 0)
        {
            temporary.setting_kvmProxyPort = kvmProxyPort;
        }
    }

    if(isNumber(alertPort))
    {
        int tmp_port = improved_stoi(alertPort);
        if(tmp_port <= 65535 && tmp_port >= 0)
        {
            temporary.setting_alertPort = alertPort;
        }
    }

    if(isNumber(sshPort))
    {
        int tmp_port = improved_stoi(sshPort);
        if(tmp_port <= 65535 && tmp_port >= 0)
        {
            temporary.setting_sshPort = sshPort; 
        }
    }

    temporary.setting_alertEnabled = alertEnabled;
    temporary.setting_sshEnabled = sshEnabled;


    // web port apply
    if(temporary.setting_webPort != webPort)
    {
        if(isNumber(webPort))
        {
            int tmp_port = improved_stoi(webPort);
            if(tmp_port <= 65535 && tmp_port >= 0)
            {
                temporary.setting_webPort = webPort;
                // 적용

                string change_cmd = "sed -i '35s/listen.*/listen ";
                change_cmd.append(webPort).append(";/g' /etc/nginx/nginx.conf");

                system(change_cmd.c_str());

                string restart_cmd = "/etc/init.d/S50nginx restart";
                system(restart_cmd.c_str());
            }
            
        }
    }
    
   
}

void Ipmiweb_PUT::Set_Ldap(json::value request_json){
  //  uint8_t response[QSIZE];

  // uint8_t header[1000];
  // string ldap_en, bind_dn, ldap_ip, ldap_port, ldap_ssl, base_dn, bind_pw,
  //     timeout;
  // uint8_t data[1000];
  // json::value myobj;

  // // strcpy(data, hm->body.p);
  // // refine_data(data);
  // // string sdata((char *)data);
  // // cout << "sdata in ldap_call : " << sdata << endl;

  // // myobj = json::value::parse(sdata);
  // cout << "set ldap" <<endl;
  // cout << request_json <<endl;
  // myobj = request_json;
  // // cout << myobj<<endl;
  // cout << "ldap en st"<<endl;
  // ldap_en = request_json.at("LDAP_EN").as_string();
  // cout << "ldap en end "<<endl;
  // int rets = 0;
  // //바꾼 부분

  // cout << "b end"<<endl;
  // cout << "bind_dn" <<bind_dn<<endl;
  // cout << "base_dn" <<base_dn<<endl;
  // cout << "bind_pw" <<bind_pw<<endl;
  
  // if (ldap_en == "1") {

  //   //원본
  //   // bind_dn = request_json.at("BIND_DN").as_string();
  //   // ldap_ip = request_json.at("LDAP_IP").as_string();
  //   // ldap_port = request_json.at("LDAP_PORT").as_string();
  //   // ldap_ssl = request_json.at("LDAP_SSL").as_string();
  //   // base_dn = request_json.at("BASE_DN").as_string();
  //   // bind_pw = request_json.at("BIND_PW").as_string();
  //   // timeout = request_json.at("TIMEOUT").as_string();
  //   get_value_from_json_key(request_json,"LDAP_IP",ldap_ip);
  //   get_value_from_json_key(request_json,"LDAP_PORT",ldap_port);
  //   get_value_from_json_key(request_json,"LDAP_SSL",ldap_ssl);
  //   get_value_from_json_key(request_json,"BIND_DN",bind_dn);
  //   get_value_from_json_key(request_json,"BASE_DN",base_dn);
  //   get_value_from_json_key(request_json,"BIND_PW",bind_pw);
  //   get_value_from_json_key(request_json,"TIMEOUT",timeout);

  //   cout << "ldap json endl " <<endl;
  //   rets = set_ldap_enable(1);
  //   if (ldap_ip.size() != 0)
  //   rets += set_ldap_ip(ldap_ip.c_str());
    
  //   // rets += set_ldap_port(ldap_port.c_str());
  //   if (ldap_port.size() != 0)
  //   rets += set_ldap_searchbase(base_dn.c_str());
  //   if (!bind_dn.size() != 0)
  //   rets += set_ldap_binddn(bind_dn.c_str());
  //   if (!bind_pw.size() != 0)
  //   rets += set_ldap_bindpw(bind_pw.c_str());
  //   rets += set_ldap_ssl(ldap_ssl.c_str());
  //   rets += set_ldap_timelimit(timeout.c_str());
  //   set_ad_enable("0");

  // } 
  // // else 
  // //   rets = set_ldap_enable("0");

      // [테스트] ldap 임시 값 처리...
      string enabled, encryption, address, port, bindDN, bindPW, baseDN, timeout;

      get_value_from_json_key(request_json, "LDAP_EN", enabled);
      get_value_from_json_key(request_json, "LDAP_SSL", encryption);
      get_value_from_json_key(request_json, "LDAP_IP", address);
      get_value_from_json_key(request_json, "LDAP_PORT", port);
      get_value_from_json_key(request_json, "BIND_DN", bindDN);
      get_value_from_json_key(request_json, "BIND_PW", bindPW);
      get_value_from_json_key(request_json, "BASE_DN", baseDN);
      get_value_from_json_key(request_json, "TIMEOUT", timeout);

      temporary.ldap_enabled = enabled;
      temporary.ldap_encryptionType = encryption;
      temporary.ldap_serverAddress = address;
      temporary.ldap_port = port;
      temporary.ldap_bindDN = bindDN;
      temporary.ldap_bindPW = bindPW;
      temporary.ldap_baseDN = baseDN;
      temporary.ldap_timeout = timeout;

}