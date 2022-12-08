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
  
  cout<<"mfg_data 0 " <<(int)mfg_dates[0] << endl;
  cout<<"mfg_data 1 " <<(int)mfg_dates[1] << endl;
  cout<<"mfg_data 2 " <<(int)mfg_dates[2] << endl;
  cout<<"mfg_data 3 " <<(int)mfg_dates[3] << endl;
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
          
        }
    }

    if(isNumber(alertPort))
    {
        int tmp_port = improved_stoi(alertPort);
       
        if(tmp_port <= 65535 && tmp_port >= 0)
        {
          set_smtp_port(alertPort);
            // temporary.setting_alertPort = alertPort;
        }
    }

    if(isNumber(sshPort))
    {
        int tmp_port = improved_stoi(sshPort);
        if(tmp_port <= 65535 && tmp_port >= 0)
        {
            set_ssh_port(sshPort);
        }
    }
    string network_odata = ODATA_MANAGER_ID;
    network_odata.append("/NetworkProtocol");
    NetworkProtocol *net = (NetworkProtocol *)g_record[network_odata];
    EventService *eventService = (EventService *)g_record[ODATA_EVENT_SERVICE_ID];
    if(alertEnabled=="1")
    {
      cout <<"alertEnabled"<<endl;
      eventService->smtp.smtp_smtp_enabled=true;
    }
    else
    {
       cout <<"alertdisalbed"<<endl;
      eventService->smtp.smtp_smtp_enabled=false;
    }
    if(sshEnabled=="1")
    {
      net->http_enabled=true;
    }
    else
    {
      net->http_enabled=false;
    }
    if(temporary.setting_webPort != webPort)
    {
        if(isNumber(webPort))
        {
            int tmp_port = improved_stoi(webPort);
            if(tmp_port <= 65535 && tmp_port >= 0)
            {
                string network_odata = ODATA_MANAGER_ID;
                network_odata.append("/NetworkProtocol");
                NetworkProtocol *net = (NetworkProtocol *)g_record[network_odata];
                net->http_port=tmp_port;
                string change_cmd = "sed -i '35s/listen.*/listen ";
                change_cmd.append(webPort).append(";/g' /etc/nginx/nginx.conf");

                system(change_cmd.c_str());

                string restart_cmd = "/etc/init.d/S50nginx restart";
                system(restart_cmd.c_str());
            }
            
        }
    }
    resource_save_json(net);
    resource_save_json(eventService);
    
   
}

void Ipmiweb_PUT::Set_Ldap(json::value request_json){

      // [테스트] ldap 임시 값 처리...
      string enabled, encryption, address, port, bindDN, bindPW, baseDN, timeout;
      AccountService *account_service = (AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID];
      get_value_from_json_key(request_json, "LDAP_EN", enabled);
       log(info)<<"enabled="<<enabled;
      if(enabled=="1")
      {
          account_service->ldap.service_enabled=true;
          get_value_from_json_key(request_json, "LDAP_SSL", encryption);
          get_value_from_json_key(request_json, "LDAP_IP", address);
          // get_value_from_json_key(request_json, "LDAP_PORT", port);
          get_value_from_json_key(request_json, "BIND_DN", bindDN);
          get_value_from_json_key(request_json, "BIND_PW", bindPW);
          get_value_from_json_key(request_json, "BASE_DN", baseDN);
          get_value_from_json_key(request_json, "TIMEOUT", timeout);
          if(account_service->ldap.service_addresses.size()>=1)
            account_service->ldap.service_addresses[0]=address;
          else
            account_service->ldap.service_addresses.push_back(address); 

          account_service->ldap.authentication.authentication_type="UsernameAndPassword";
          account_service->ldap.authentication.username=bindDN;
          account_service->ldap.authentication.password=bindPW;
          if(account_service->ldap.ldap_service.search_settings.base_distinguished_names.size()>=1)
            account_service->ldap.ldap_service.search_settings.base_distinguished_names[0]=baseDN;
          else
            account_service->ldap.ldap_service.search_settings.base_distinguished_names.push_back(baseDN); 
       account_service->wrtie_ladap_to_nclcd();   
      }
      else
      {
        log(info)<<"disabled service";
        account_service->ldap.service_enabled=false;
      }
      resource_save_json(account_service);
}