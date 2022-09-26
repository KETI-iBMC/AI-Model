#include "ipmi/user.hpp"
#include "redfish/stdafx.hpp"
#include <ipmi/rest_get.hpp>

extern std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;
extern std::map<uint8_t, std::map<uint8_t, Ipmifru>> fru_rec;
extern char g_firmware_build_time[50];

TempWebValues temporary;

const char *ipmi_generic_sensor_type_val[] = {
    "reserved",
    "Temperature", "Voltage", "Current", "Fan",
    "Physical Security", "Platform Security", "Processor",
    "Power Supply", "Power Unit", "Cooling Device", "Other",
    "Memory", "Drive Slot / Bay", "POST Memory Resize",
    "System Firmwares", "Event Logging Disabled", "Watchdog1",
    "System Event", "Critical Interrupt", "Button",
    "Module / Board", "Microcontroller", "Add-in Card",
    "Chassis", "Chip Set", "Other FRU", "Cable / Interconnect",
    "Terminator", "System Boot Initiated", "Boot Error",
    "OS Boot", "OS Critical Stop", "Slot / Connector",
    "System ACPI Power State", "Watchdog2", "Platform Alert",
    "Entity Presence", "Monitor ASIC", "LAN",
    "Management Subsys Health", "Battery", "Session Audit",
    "Version Change", "FRU State",
    NULL};



void Ipmiweb_GET::Get_Show_Main(int menu, json::value &response_json){
    
  cout << "menu num " << menu << endl;
     char buf[30000] = {
      0,
  };
  cout << "Enter rest_show_main" << endl;
  cout << "\t selected menu = " << menu << endl;
  // json::value obj = json::value::object();
  json::value main = json::value::object();
  json::value JHEALTH_SUMMARY = json::value::object();
  json::value JCPU0 = json::value::object();
  json::value JCPU1 = json::value::object();
  json::value JPOWER = json::value::object();
  json::value JFANS = json::value::object();
  json::value JBOARD_TEMP = json::value::object();
  json::value JSYS_INFO = json::value::object();
  json::value JEVENT_LIST;
  std::vector<json::value> VJ;

  switch (menu) {
  case ALL:
    // Ipmiweb_GET::Get_Eventlog(JEVENT_LIST);
    // rest_get_eventlog_config(buf);
    // JEVENT_LIST = json::value::parse(buf);
    // cout << "EVENT LIST " <<JEVENT_LIST<<endl;
    get_temp_cpu0(JCPU0);
    cout << "get_temp_cpu0" << endl;
    get_temp_cpu1(JCPU1);
    cout << "get_temp_cpu1" << endl;
    get_voltage_fan_power(JPOWER);
    cout << "get_voltage_fan_power" << endl;
    get_fans(JFANS);
    cout << "get_fans" << endl;
    get_temp_board(JBOARD_TEMP);
    cout << "get_temp_board" << endl;
    get_sys_info(JSYS_INFO);
    cout << "get_sys_info" << endl;
    get_health_summary(JHEALTH_SUMMARY);
    cout << "get_health_summary" << endl;
    main["EVENT_LIST"] = JEVENT_LIST["EVENT_INFO"]["SEL"];
    main["CPU1_TEMP"] = JCPU0;
    main["CPU2_TEMP"] = JCPU1;
    main["POWER"] = JPOWER;
    main["FANS"] = JFANS;
    main["BOARD_TEMP"] = JBOARD_TEMP;
    main["SYS_INFO"] = JSYS_INFO;
    main["HEALTH_SUMMARY"] = JHEALTH_SUMMARY;
    response_json["MAIN"] = main;
    break;
  case EVENT_LIST:
    cout << "select 1 " <<endl;
    rest_get_eventlog_config(buf);
    JEVENT_LIST = json::value::parse(buf);
    main["EVENT_LIST"] = JEVENT_LIST["EVENT_INFO"]["SEL"];
    response_json["MAIN"] = main;
    break;
  case CPU2_TEMP:
    get_temp_cpu1(JCPU1);
    cout << " JCPU1 " <<JCPU1.to_string()<<endl;
    main["CPU2_TEMP"] = JCPU1;
    response_json["MAIN"] = main;
    break;

  case CPU1_TEMP:
    get_temp_cpu1(JCPU1);
    cout << " JCPU1 " <<JCPU1.to_string()<<endl;
    main["CPU2_TEMP"] = JCPU1;
    get_temp_cpu0(JCPU0);
    cout << " JCPU0 " <<JCPU0.to_string()<<endl;
    main["CPU1_TEMP"] = JCPU0;
    get_temp_board(JBOARD_TEMP);
    cout << " JBOARD_TEMP " <<JBOARD_TEMP.to_string()<<endl;
    main["BOARD_TEMP"] = JBOARD_TEMP;
    response_json["MAIN"] = main;
    break;
  case BOARD_TEMP:
    get_temp_cpu1(JCPU1);
    main["CPU2_TEMP"] = JCPU1;
    get_temp_cpu0(JCPU0);
    main["CPU1_TEMP"] = JCPU0;
    get_temp_board(JBOARD_TEMP);
    main["BOARD_TEMP"] = JBOARD_TEMP;
    response_json["MAIN"] = main;
    break;
  case POWER:
    get_voltage_fan_power(JPOWER);
    main["POWER"] = JPOWER;
    response_json["MAIN"] = main;
    break;
  case FANS:
    get_fans(JFANS);
    main["FANS"] = JFANS;
    response_json["MAIN"] = main;
    break;
  case SYS:
    get_sys_info(JSYS_INFO);
    main["SYS_INFO"] = JSYS_INFO;
    response_json["MAIN"] = main;
    break;
  case SUMMARY:
    get_health_summary(JHEALTH_SUMMARY);
    main["HEALTH_SUMMARY"] = JHEALTH_SUMMARY;
    response_json["MAIN"] = main;
    break;
  default:
    cout << "rest_show_main:: not option number" << menu << endl;
    break;
  }
  
}


void Ipmiweb_GET::Get_Fru_Info(json::value &response_json){
  	cout << "Enter rest_get_flu_config" << endl;

	char buf[1024];
	unsigned char i = 0;
	unsigned char temp_number = 0;
	struct tm *strtm;
	time_t tval;

	// json::value obj = json::value::object();
  std::vector<json::value> fru_vec;
  json::value fru = json::value::object();
  json::value BOARD = json::value::object();
  json::value PRODUCT = json::value::object();
  json::value CHASSIS = json::value::object();

	// 현재 fru 0만 존재. fru 개수 늘어날 시, size만큼 반복해야 함.
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	Ipmisdr *sdr_this = &sdr_rec.find(0)->second.find(0)->second;
	sensor_thresh_t *c_sdr;
	
	c_sdr = sdr_this->sdr_get_entry();
	
	fru["ID"] = json::value::number(i);

	string fru_device_desc = "FRU Device Description : ";
	if (fru_this->fru_header.id == 0 && i == 0)
		fru_device_desc += "Builtin FRU Device";
	else if (fru_this->fru_header.id == 0 && i != 0)
		fru_device_desc += "Not Configured";
	else{
		string sdr_str(c_sdr->str);
		fru_device_desc += sdr_str;	
	}	
	fru["DEVICE"] = json::value::string(U(fru_device_desc));

	if (fru_this->fru_header.board != 0){
		// convert char [4] -> time_t (ex. 5f ee 66 00 -> 1609459200)
		tval = (((unsigned char *)fru_this->fru_board.mfg_date)[0] << 24) | (((unsigned char *)fru_this->fru_board.mfg_date)[1] << 16) | (((unsigned char *)fru_this->fru_board.mfg_date)[2] << 8) | ((unsigned char *)fru_this->fru_board.mfg_date)[3]; 
		
		strtm = localtime(&tval);
		unsigned char Dates[100];
		strftime(Dates, 100, "%Y-%m-%d %H:%M:%S", strtm);
	
		Dates[strlen(asctime(strtm)) - 1] = '\0';
		BOARD["MFG_DATE"] = json::value::string(U((char *)Dates));
		BOARD["MFG"] = json::value::string(U((char *)fru_this->fru_board.mfg));
		BOARD["PRODUCT"] = json::value::string(U((char *)fru_this->fru_board.product));
		BOARD["SERIAL"] = json::value::string(U((char *)fru_this->fru_board.serial));
		BOARD["PART_NUM"] = json::value::string(U((char *)fru_this->fru_board.part_number));	
	} else {
		BOARD["MFG_DATE"] = json::value::string(U(""));
		BOARD["MFG"] = json::value::string(U(""));
		BOARD["PRODUCT"] = json::value::string(U(""));
		BOARD["SERIAL"] = json::value::string(U(""));
		BOARD["PART_NUM"] = json::value::string(U(""));		
	}
	
	if (fru_this->fru_header.product != 0){
		PRODUCT["NAME"] = json::value::string(U((char *)fru_this->product.name));
		PRODUCT["MFG"] = json::value::string(U((char *)fru_this->product.mfg));
		PRODUCT["VERSION"] = json::value::string(U((char *)fru_this->product.version));
		PRODUCT["SERIAL"] = json::value::string(U((char *)fru_this->product.serial));
		PRODUCT["PART_NUM"] = json::value::string(U((char *)fru_this->product.part_number));
	}else {
		PRODUCT["NAME"] = json::value::string(U(""));
		PRODUCT["MFG"] = json::value::string(U(""));
		PRODUCT["VERSION"] = json::value::string(U(""));
		PRODUCT["SERIAL"] = json::value::string(U(""));
		PRODUCT["PART_NUM"] = json::value::string(U(""));
	}

	if (fru_this->fru_header.chassis != 0){
    CHASSIS["TYPE"] = json::value::string(Ipmifru::chassis_type_desc_fru[fru_this->fru_chassis.type]);
		// CHASSIS["TYPE"] = json::value::string(U(chassis_type_desc_fru[fru_this->fru_chassis.type]));
		CHASSIS["SERIAL"] = json::value::string(U((char *)fru_this->fru_chassis.serial));
		CHASSIS["PART_NUM"] = json::value::string(U((char *)fru_this->fru_chassis.part_number));	
	}else {
		CHASSIS["TYPE"] = json::value::string(U(""));
		CHASSIS["SERIAL"] = json::value::string(U(""));
		CHASSIS["PART_NUM"] = json::value::string(U(""));
	}

  fru["BOARD"] = BOARD;
	fru["PRODUCT"] = PRODUCT;
	fru["CHASSIS"] = CHASSIS;
	fru_vec.push_back(fru);

	response_json["FRU_JSON"] = json::value::array(fru_vec);

  // // [테스트 샘플] 인데 put 때문에 저장되어있는거 사용해서 안씀
  // fru["ID"] = json::value::number(1);
  // fru["DEVICE"] = json::value::string("FRU Device Description : Built in FRU Device");

  // BOARD["MFG_DATE"] = json::value::string("2022-07-15 12:00:00");
  // BOARD["MFG"] = json::value::string("MADE IN KETI Board");
  // BOARD["PRODUCT"] = json::value::string("BOARD1-KH00");
  // BOARD["SERIAL"] = json::value::string("BOARD Serial 1");
  // BOARD["PART_NUM"] = json::value::string("BOARD Part Num 1");

  // PRODUCT["NAME"] = json::value::string("PRODUCT Name 1");
  // PRODUCT["MFG"] = json::value::string("MADE IN KETI Product");
  // PRODUCT["VERSION"] = json::value::string("VERSION v1");
  // PRODUCT["SERIAL"] = json::value::string("PRODUCT Serial 1");
  // PRODUCT["PART_NUM"] = json::value::string("PRODUCT Part Num 1");

  // CHASSIS["TYPE"] = json::value::string("Chassis TYPE 1");
  // CHASSIS["SERIAL"] = json::value::string("Chassis Serial 1");
  // CHASSIS["PART_NUM"] = json::value::string("Chassis Part Num 1");

	// fru["BOARD"] = BOARD;
	// fru["PRODUCT"] = PRODUCT;
	// fru["CHASSIS"] = CHASSIS;
	// fru_vec.push_back(fru);

  // fru = json::value::null();
  
  // fru["ID"] = json::value::number(2);
  // fru["DEVICE"] = json::value::string("FRU Device Description : Not Configured");

  // BOARD["MFG_DATE"] = json::value::string("2022-07-10 15:30:00");
  // BOARD["MFG"] = json::value::string("MADE IN KETI Board");
  // BOARD["PRODUCT"] = json::value::string("BOARD2-KH00");
  // BOARD["SERIAL"] = json::value::string("BOARD Serial 2");
  // BOARD["PART_NUM"] = json::value::string("BOARD Part Num 2");

  // PRODUCT["NAME"] = json::value::string("PRODUCT Name 2");
  // PRODUCT["MFG"] = json::value::string("MADE IN KETI Product");
  // PRODUCT["VERSION"] = json::value::string("VERSION v2");
  // PRODUCT["SERIAL"] = json::value::string("PRODUCT Serial 2");
  // PRODUCT["PART_NUM"] = json::value::string("PRODUCT Part Num 2");

  // CHASSIS["TYPE"] = json::value::string("Chassis TYPE 2");
  // CHASSIS["SERIAL"] = json::value::string("Chassis Serial 2");
  // CHASSIS["PART_NUM"] = json::value::string("Chassis Part Num 2");

  

}

void Ipmiweb_GET::Get_Sensor_Info(json::value &response_json){
  // json::value obj = json::value::object();
  json::value sensor_info = json::value::object();
  vector<json::value> sensor_vec;
  sensor_thresh_t *p_sdr;
  float temp = 0;
  int state;

  for (auto iter = sdr_rec.begin(); iter != sdr_rec.end(); iter++) {
    json::value SENSOR = json::value::object();
    p_sdr = iter->second.find(iter->first)->second.sdr_get_entry();
    if (strlen(p_sdr->str) > 2) {
      SENSOR["NAME"] = json::value::string(U(p_sdr->str));
      switch (p_sdr->sensor_num) {
      case PEB_SENSOR_ADC_P12V_PSU1:
      case PEB_SENSOR_ADC_P12V_PSU2:
      case PEB_SENSOR_ADC_P3V3:
      case PEB_SENSOR_ADC_P5V:
      case PEB_SENSOR_ADC_PVNN_PCH:
      case PEB_SENSOR_ADC_P1V05:
      case PEB_SENSOR_ADC_P1V8:
      case PEB_SENSOR_ADC_BAT:
      case PEB_SENSOR_ADC_PVCCIN:
      case PEB_SENSOR_ADC_PVNN_PCH_CPU0:
      case PEB_SENSOR_ADC_P1V8_NACDELAY:
      case PEB_SENSOR_ADC_P1V2_VDDQ:
      case PEB_SENSOR_ADC_PVNN_NAC:
      case PEB_SENSOR_ADC_P1V05_NAC:
      case PEB_SENSOR_ADC_PVPP:
      case PEB_SENSOR_ADC_PVTT:
      // case NVA_SENSOR_PSU1_TEMP:
      // case NVA_SENSOR_PSU2_TEMP:
      // case NVA_SENSOR_PSU1_WATT:
      // case NVA_SENSOR_PSU2_WATT:
      case NVA_SYSTEM_FAN1:
      case NVA_SYSTEM_FAN2:
      case NVA_SYSTEM_FAN3:
      case NVA_SYSTEM_FAN4:
      case NVA_SYSTEM_FAN5:
      
      // case NVA_SENSOR_PSU1_FAN1:
      // case NVA_SENSOR_PSU2_FAN1:
      case PDPB_SENSOR_TEMP_CPU0:
      case PDPB_SENSOR_TEMP_CPU1:
      case PDPB_SENSOR0_TEMP_LM75:
      case PDPB_SENSOR1_TEMP_LM75:
      case PDPB_SENSOR2_TEMP_LM75:
      case PDPB_SENSOR3_TEMP_LM75:
      case PDPB_SENSOR4_TEMP_LM75:
      case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2:
      case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2:
      case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2:
      case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2:
      case PDPB_SENSOR_TEMP_REAR_RIGHT:
      case PDPB_SENSOR_TEMP_CPU_AMBIENT:
      case PDPB_SENSOR_TEMP_FRONT_RIGHT:
      case PDPB_SENSOR_TEMP_PCIE_AMBIENT:
      case PDPB_SENSOR_TEMP_FRONT_LEFT:
        SENSOR["READING"] = json::value::string(
            to_string(sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->nominal)));
        SENSOR["RB"] = json::value::string(
            to_string(sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->rb_exp)));
        temp = (p_sdr->lnc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->lnc_thresh)
                    : 0);
        SENSOR["LNC"] = json::value::string(to_string(temp));
        temp = (p_sdr->lc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->lc_thresh)
                    : 0);
        SENSOR["LC"] = json::value::string(to_string(temp));
        temp = (p_sdr->lnr_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->lnr_thresh)
                    : 0);
        SENSOR["LNR"] = json::value::string(to_string(temp));
        temp = (p_sdr->unc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->unc_thresh)
                    : 0);
        SENSOR["UNC"] = json::value::string(to_string(temp));
        temp = (p_sdr->uc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->uc_thresh)
                    : 0);
        SENSOR["UC"] = json::value::string(to_string(temp));
        temp = (p_sdr->unr_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->unr_thresh)
                    : 0);
        SENSOR["UNR"] = json::value::string(to_string(temp));

        // if (bPowerGD == 0)
        //     state = 0;
        // else
        if (p_sdr->sensor_type == 0x2 || p_sdr->sensor_type == 0x1) {
          if (p_sdr->nominal == 0)
            state = 0;
          else if (p_sdr->nominal < p_sdr->unc_thresh &&
                   p_sdr->nominal > p_sdr->lnc_thresh)
            state = 6;
          else
            state = 1;
        } else if (p_sdr->sensor_type == 0x4) {
          if (p_sdr->nominal == 0)
            state = 0;
          else if (p_sdr->nominal < p_sdr->lnc_thresh)
            state = 6;
          else
            state = 1;
        } else if (p_sdr->sensor_type == 0x9) {
          if (p_sdr->nominal == 0)
            state = 0;
          else if (p_sdr->nominal < p_sdr->unc_thresh)
            state = 6;
          else
            state = 1;
        }
        SENSOR["STATE"] = json::value::string(to_string(state));
        break;
      }
      SENSOR["NUMBER"] = json::value::string(to_string(p_sdr->sensor_num));
    }
    sensor_vec.push_back(SENSOR);
  }
  sensor_info["SENSOR"] = json::value::array(sensor_vec);
  response_json["SENSOR_INFO"] = sensor_info;
  

}

void Ipmiweb_GET::Get_Eventlog(json::value &response_json){
    // int i = 0;
    // int getfail, next;
    // sel_msg_t msg;
    // struct tm *t;
    // time_t time;
    // char buf[100];
    // char ret[30000] ={0,};
  
    // cout << " statrt eventlog " <<endl;
    // strcpy(ret, "{\n");
    // strcat(ret, "\"EVENT_INFO\": {\n  \"SEL\":\n\t[\n");
    // int eventlog_cnt = plat_sel_num_entries();
    // while (i < eventlog_cnt)
    // {
    //     (getfail = plat_sel_get_entry(i, &msg, &next));
    //     if (getfail)
    //         break;
    //     memcpy(&time, &msg.msg[3], sizeof(time_t));
    //     //    printf("time: %s\n", ipmi_sel_timestamp(time));
    //     (t = localtime(&time));

    //     strcat(ret, "\t\t{\n");

    //     if ((t->tm_year + 1900) < 2017)
    //         sprintf(buf, "\t\t \"TIME\": \"Pre-Init\",\n");
    //     else
    //         sprintf(buf, "\t\t \"TIME\": \"%04d-%02d-%02d %02d:%02d:%02d\",\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    //     strcat(ret, buf);
    //     char sName[30];
    //     int namelen = 0;

    //     string sensorname;
    //     (namelen = find_sensor_name(msg.msg[10], msg.msg[11], sensorname));
    //     strcpy(sName, sensorname.c_str());
    //     if (namelen < 0)
    //         sprintf(sName, "%s : 0x%02X", ipmi_generic_sensor_type_val[msg.msg[10]], msg.msg[11]);
    //     sprintf(buf, "\t\t \"NAME\": \"%s\",\n", sName); // need parsing through sensor enum
    //     strcat(ret, buf);

    //     sprintf(buf, "\t\t \"SEL_RD\": \"%x\",\n", msg.msg[12]); // no use
    //     strcat(ret, buf);

    //     sprintf(buf, "\t\t \"TYPE\": \"%s\",\n", ipmi_generic_sensor_type_val[msg.msg[10]]);
    //     strcat(ret, buf);

    //     char buf_des[128] = {0};
    //     char buf_ret[256] = {0};

    //     msg.msg[13] = msg.msg[13] & 0x0f;

    //     if (msg.msg[10] != 0x08 && msg.msg[10] != 0x2b && msg.msg[10] != 0x0f)
    //         msg.msg[14] = 0xff;

    //     if (msg.msg[12] != 0x6f)
    //         msg.msg[14] = 0xff;

    //     ipmi_get_event_desc(buf_des, &msg);
    //     sprintf(buf_ret, "\t\t \"DESCRIPTION\": \"%s\",\n", buf_des);
    //     strcat(ret, buf_ret);

     

    //   sprintf(buf, "\t\t \"SENSOR_ID\": \"%02X\"\n", i);
    //   strcat(ret, buf);
    //   if (++i < eventlog_cnt && strlen(ret) < 34000)
    //   {
    //       strcat(ret, "\t\t},\n");
    //   }
    //   else
    //   {
    //       strcat(ret, "\t\t}\n");
    //       break;
    //   }
    // }
    // strcat(ret, "\t]\n  }\n}\n");

    // [테스트] 임시 샘플값 주입 + json::value로 변경 윗 부분이 원본
    json::value result, event_info, SEL, BIOS, SM;

    SEL = json::value::array();
    for(int i=0; i<40; i++)
    {
        json::value tmp;

        if(i < 10)
          tmp["TIME"] = json::value::string("2022-03-1" + to_string(i) + " 12:00:00"); // 10개
        else
          tmp["TIME"] = json::value::string("2022-04-10 12:" + to_string(i) + ":00"); // 30개

        tmp["NAME"] = json::value::string("SEL Name " + to_string(i));
        tmp["SENSOR_ID"] = json::value::string("SEL Sensor " + to_string(i));
        tmp["TYPE"] = json::value::string("SEL Type");
        tmp["DESCRIPTION"] = json::value::string("SEL Description");

        SEL[SEL.size()] = tmp;
    }

    BIOS = json::value::array();
    for(int i=0; i<40; i++)
    {
        json::value tmp;

        if(i < 10)
          tmp["TIME"] = json::value::string("2022-04-1" + to_string(i) + " 22:00:00"); // 10개
        else if(i >= 10 && i < 30)
          tmp["TIME"] = json::value::string("2022-05-" + to_string(i) + " 22:00:00"); // 20개
        else
          tmp["TIME"] = json::value::string("2022-06-10 22:" + to_string(i) + ":00"); // 10개

        // tmp["TIME"] = json::value::string("2022-04-1" + to_string(i) + " 22:00:00");
        tmp["NAME"] = json::value::string("BIOS Name " + to_string(i));
        tmp["SENSOR_ID"] = json::value::string("BIOS Sensor " + to_string(i));
        tmp["TYPE"] = json::value::string("BIOS Type");
        // tmp["DESCRIPTION"] = json::value::string("BIOS Description");

        BIOS[BIOS.size()] = tmp;
    }

    SM = json::value::array();
    for(int i=0; i<60; i++)
    {
        json::value tmp;

        if(i < 10)
          tmp["TIME"] = json::value::string("2022-05-2" + to_string(i) + " 17:00:00");  
        else if(i >= 10 && i < 30)
          tmp["TIME"] = json::value::string("2022-05-21 14:" + to_string(i) + ":00");
        else if(i >= 30 && i < 40)
          tmp["TIME"] = json::value::string("2022-07-25 18:05:" + to_string(i));
        else
          tmp["TIME"] = json::value::string("2022-09-10 09:59:" + to_string(i));

        
        tmp["NAME"] = json::value::string("SM Name " + to_string(i));
        tmp["SENSOR_ID"] = json::value::string("SM Sensor " + to_string(i));
        tmp["TYPE"] = json::value::string("SM Type");
        // tmp["DESCRIPTION"] = json::value::string("BIOS Description");

        SM[SM.size()] = tmp;
    }

    event_info["SEL"] = SEL;
    event_info["BIOS"] = BIOS;
    event_info["SM"] = SM;
    result["EVENT_INFO"] = event_info;
    response_json = result;

    // response_json = json::value::parse(ret);
    // cout << "eventlog json " <<endl ;
}

void Ipmiweb_GET::Get_DDNS_Info(json::value &response_json){
  cout << "get dns json start "<<endl;
  char buf[128];
  unsigned char domain_name[50] = {0};
  unsigned char host_name[50] = {0};
  unsigned char nameserver_pre[30] = {0};
  unsigned char nameserver_alt[30] = {0};
  json::value redfish_dns;
  redfish_dns = record_get_json("/redfish/v1/Managers/EthernetInterfaces/NIC");
  
  cout << "redfish dns"<<redfish_dns<<endl;
  get_ddns_host_name(host_name);
  get_ddns_domain_name(domain_name);
  get_ddns_nameserver(1, nameserver_pre);
  get_ddns_nameserver(2, nameserver_alt);

  host_name[strlen(host_name) - 1] = '\0';
  cout << "get dns json start "<<endl;
  // json::value obj = json::value::object();
  json::value DNS_INFO = json::value::object();
  json::value GENERIC = json::value::object();
  
  // [테스트] dns 임시처리
  GENERIC["HOST_NAME"] = json::value::string(redfish_dns.at("HostName").as_string());
  GENERIC["DOMAIN_NAME"] = json::value::string(redfish_dns.at("FQDN").as_string());
  // GENERIC["HOST_NAME"] = json::value::string(U((char *)host_name));
  // GENERIC["DOMAIN_NAME"] = json::value::string(U((char *)domain_name));
  
  
  GENERIC["REGISTER_BMC"] = json::value::string(temporary.dns_registerBMC);
  GENERIC["REGISTER_BMC_METHOD"] = json::value::string(temporary.dns_registerBMCMethod);
  // GENERIC["REGISTER_BMC"] = json::value::string(U("1"));
  // GENERIC["REGISTER_BMC_METHOD"] = json::value::string(U("DIRECT"));

  cout << "ipv6 json "<<endl;
  json::value IPV6 = json::value::object();
  // string s_ipaddr_v6(ipmiNetwork[0].ip_addr_v6.begin(),
  //                    ipmiNetwork[0].ip_addr_v6.end());
  // cout <<"ipv6 add " <<s_ipaddr_v6 <<endl;

 

  unsigned char ddns_buf[100];
  unsigned char ddns_buf1[100];
   
  json::value nameservers;
  nameservers = redfish_dns.at("NameServers");
  
   IPV6["IPV6_PREFERRED"] = json::value::string(nameservers[2].as_string());
  IPV6["IPV6_ALTERNATE"] = json::value::string(nameservers[3].as_string());
  cout << "ipv6 pre " <<endl;
  json::value IPV4 = json::value::object();
  IPV4["IPV4_PREFERRED"] = json::value::string(nameservers[0].as_string());
  IPV4["IPV4_ALTERNATE"] = json::value::string(nameservers[1].as_string());
  cout << " ipv4 json" << endl;
  DNS_INFO["GENERIC"] = GENERIC;
  DNS_INFO["IPV6"] = IPV6;
  DNS_INFO["IPV4"] = IPV4;
  response_json["DNS_INFO"] = DNS_INFO;
  
}
void Ipmiweb_GET::Get_Sys_Info(json::value &response_json){

cout << "REST Get System Information" << endl;
  // printf("ipmi_system_information: %d\n", IPMI_SYSTEM_INFORMATION);
  // #if IPMI_SYSTEM_INFORMATION
  unsigned char net_priority = 0;

  net_priority = get_eth_priority();

  // json::value obj = json::value::object();

  json::value power_info = json::value::object();
  power_info["STATUS"] =
      json::value::string(U(ipmiChassis.get_power_status() ? "on" : "off"));

  json::value generic_info = json::value::object();
  json::value generic = json::value::object();
  generic["IPMIFW_TAG"] = json::value::string(U(ipmiApplication.g_host_name));

  char ip_str[150], mac_str[150];
  memset(ip_str, 0, sizeof(ip_str));
  memset(mac_str, 0, sizeof(mac_str));

  if (net_priority == 1) {
    sprintf(ip_str, "%u.%u.%u.%u", ipmiNetwork[0].ip_addr[0],
            ipmiNetwork[0].ip_addr[1], ipmiNetwork[0].ip_addr[2],
            ipmiNetwork[0].ip_addr[3]);
    sprintf(mac_str, "%u.%u.%u.%u.%u.%u", ipmiNetwork[0].ip_addr[0],
            ipmiNetwork[0].ip_addr[1], ipmiNetwork[0].ip_addr[2],
            ipmiNetwork[0].ip_addr[3], ipmiNetwork[0].ip_addr[4],
            ipmiNetwork[0].ip_addr[5]);

    cout << "ip 0: " << ip_str << endl;
    cout << "mac 0: " << mac_str << endl;

    generic["BMC_IP"] = json::value::string(U(ip_str));
    generic["BMC_MAC"] = json::value::string(U(mac_str));
  } else if (net_priority == 8) {
    sprintf(ip_str, "%u.%u.%u.%u", ipmiNetwork[1].ip_addr[0],
            ipmiNetwork[1].ip_addr[1], ipmiNetwork[1].ip_addr[2],
            ipmiNetwork[1].ip_addr[3]);
    sprintf(mac_str, "%u.%u.%u.%u.%u.%u", ipmiNetwork[1].ip_addr[0],
            ipmiNetwork[1].ip_addr[1], ipmiNetwork[1].ip_addr[2],
            ipmiNetwork[1].ip_addr[3], ipmiNetwork[1].ip_addr[4],
            ipmiNetwork[1].ip_addr[5]);

    cout << "ip 1: " << ip_str << endl;
    cout << "mac 1: " << mac_str << endl;

    generic["BMC_IP"] = json::value::string(U(ip_str));
    generic["BMC_MAC"] = json::value::string(U(mac_str));
  }
  generic["FRU_VERSION"] = json::value::string(U("1.31"));
  generic["IPMIFW_VERSION"] = json::value::string(U("V2.0"));
  generic["BIOS_VERSION"] = json::value::string(U("0.01"));
  generic["SDR_VERSION"] = json::value::string(U("0.51"));
  generic["WEB_VERSION"] = json::value::string(U("1.01"));
  generic["IPMIFW_BLDTIME"] = json::value::string(U(g_firmware_build_time));

  json::value kernel = json::value::object();
  kernel["VERSION"] = json::value::string(U(ipmiApplication.g_kernel_version));

  generic_info["GENERIC"] = generic;
  generic_info["KERNEL"] = kernel;
  response_json["GENERIC_INFO"] = generic_info;



}
void clear_data(uint8_t *data){
	int data_idx = strlen(data);
	
	while (data[data_idx] != '}')
		data[data_idx--] = '\0';	
	return;
}
void Ipmiweb_GET::Get_Lan_Info(json::value &response_json){
  // int ipv4_srcs = 0;
  // int vlan_enables = 0;
  // int c = 0;
  // int channel = 0;
  // uint8_t r_ip_addr6[SIZE_IP_ADDR_V6] = {
  //     0,
  // };
  // uint8_t r_netmask6[SIZE_NET_MASK_V6] = {
  //     0,
  // };
  // uint8_t r_gateway6[SIZE_IP_ADDR_V6] = {
  //     0,
  // };

  // for (int i = 0; i < MAX_NIC; i++) {
  //   if (i == 0)
  //     channel = 1;
  //   else if (i == 1)
  //     channel = 8;
  //   else
  //     channel = 0; // undefined

  //   if (get_ipv6_info(channel, (unsigned char *)r_ip_addr6,
  //                     (unsigned char *)r_netmask6,
  //                     (unsigned char *)r_gateway6) == -1) {
  //     fprintf(stderr, "get ipv6 info failed\n");
  //     return 0;
  //   }

  //   // printf("ip : %s\nnetmask : %s\ngateway : %s\n", r_ip_addr6, r_netmask6,
  //   // r_gateway6); printf("\t\t\t dy : get lan checkpoint 1 in channel
  //   // %d===========\n", channel);

  //   ipmiNetwork[i].ip_addr_v6.assign(r_ip_addr6, r_ip_addr6 + SIZE_IP_ADDR_V6);
  //   ipmiNetwork[i].net_mask_v6.assign(r_netmask6,
  //                                     r_netmask6 + SIZE_NET_MASK_V6);
  //   ipmiNetwork[i].df_gw_ip_addr_v6.assign(r_gateway6,
  //                                          r_gateway6 + SIZE_IP_ADDR_V6);

  //   // printf("\t\t\t dy : get lan checkpoint 2 ===========\n");
  //   // string ip6(ipmiNetwork[i].ip_addr_v6.begin(),
  //   // ipmiNetwork[i].ip_addr_v6.end()); string
  //   // nm6(ipmiNetwork[i].net_mask_v6.begin(),
  //   // ipmiNetwork[i].net_mask_v6.end()); string
  //   // gw6(ipmiNetwork[i].df_gw_ip_addr_v6.begin(),
  //   // ipmiNetwork[i].df_gw_ip_addr_v6.end()); printf("ip_v6 : %s\nnet_mask_v6 :
  //   // %s\ngw_ip : %s\n", ip6.c_str(), nm6.c_str(), gw6.c_str());
  // }

  // switch (ipmiNetwork[0].ip_src) {
  // case 1:
  //   ipv4_srcs = 0;
  //   break;
  // case 2:
  //   ipv4_srcs = 1;
  //   break;
  // }

  // char prior[8];
  // if (get_eth_priority() == 1) {
  //   strcpy(prior, "eth0");
  // }

  // else if (get_eth_priority() == 8) {
  //   strcpy(prior, "eth1");
  // } else {
  //   strcpy(prior, "Unknown");
  // }

  // // json::value obj = json::value::object();
  // response_json["NETWORK_PRIORITY"] = json::value::string(U(prior));
  // vector<json::value> net_info_vec;

  // char device[16];
  // for (int i = 0; i < MAX_NIC; i++) {
  //   // i == 0 ==> DEV_NAME_SHARED, i == 1 ==> DEV_NAME_DEDI
  //   json::value NETWORK_INFO = json::value::object();

  //   sprintf(device, "eth%d", i);
  //   if ((ipmiNetwork[i].vlan_enable && 0x80) == 1)
  //     vlan_enables = 1;
  //   else
  //     vlan_enables = 0;

  //   switch (ipmiNetwork[i].ip_src) {
  //   case 1:
  //     ipv4_srcs = 0;
  //     break;
  //   case 2:
  //     ipv4_srcs = 1;
  //     break;
  //   }

  //   NETWORK_INFO["LAN_INTERFACE"] = json::value::string(U(device));
  //   json::value GENERIC = json::value::object();
  //   json::value IPV4 = json::value::object();
  //   json::value IPV6 = json::value::object();
  //   json::value VLAN = json::value::object();
  //   char buf[64] = {
  //       0,
  //   };

  //   GENERIC["LAN_SETTING_ENABLE"] =
  //       json::value::string(U(to_string(ipmiNetwork[i].set_enable)));
  //   sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", ipmiNetwork[i].mac_addr[0],
  //           ipmiNetwork[i].mac_addr[1], ipmiNetwork[i].mac_addr[2],
  //           ipmiNetwork[i].mac_addr[3], ipmiNetwork[i].mac_addr[4],
  //           ipmiNetwork[i].mac_addr[5]);
  //   GENERIC["MAC_ADDRESS"] = json::value::string(U(buf));

  //   memset(buf, 0, sizeof(buf));

  //   IPV4["IPV4_PREFERRED"] = json::value::string(U(""));
  //   sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].df_gw_ip_addr[0],
  //           ipmiNetwork[i].df_gw_ip_addr[1], ipmiNetwork[i].df_gw_ip_addr[2],
  //           ipmiNetwork[i].df_gw_ip_addr[3]);
  //   IPV4["IPV4_GATEWAY"] = json::value::string(U(buf));
  //   sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].net_mask[0],
  //           ipmiNetwork[i].net_mask[1], ipmiNetwork[i].net_mask[2],
  //           ipmiNetwork[i].net_mask[3]);
  //   IPV4["IPV4_NETMASK"] = json::value::string(U(buf));
  //   sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].ip_addr[0],
  //           ipmiNetwork[i].ip_addr[1], ipmiNetwork[i].ip_addr[2],
  //           ipmiNetwork[i].ip_addr[3]);
  //   IPV4["IPV4_ADDRESS"] = json::value::string(U(buf));
  //   IPV4["IPV4_DHCP_ENABLE"] = json::value::string(U(to_string(ipv4_srcs)));

  //   cout << "[In GET IPv4] : ~~ " << endl;
  //   cout << IPV4 << endl;


  //   string s_subnet_mask_v6(ipmiNetwork[i].net_mask_v6.begin(),
  //                           ipmiNetwork[i].net_mask_v6.end());
  //   // memset(buf, 0, sizeof(buf));
  //   sprintf(buf, "%02x", ipmiNetwork[i].net_mask_v6[0]);
  //   IPV6["IPV6_SUBNET_PREFIX_LENGTH"] = json::value::string(buf);
  //   memset(buf, 0, sizeof(buf));
  //   sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", ipmiNetwork[i].ip_addr_v6[0],
  //       ipmiNetwork[i].ip_addr_v6[1], ipmiNetwork[i].ip_addr_v6[2],
  //       ipmiNetwork[i].ip_addr_v6[3], ipmiNetwork[i].ip_addr_v6[4],
  //       ipmiNetwork[i].ip_addr_v6[5]);
  //   IPV6["IPV6_ADDRESS"] = json::value::string(U(buf));
  //   memset(buf, 0, sizeof(buf));
  //   IPV6["IPV6_ENABLE"] =
  //       json::value::string(U(to_string(ipmiNetwork[i].set_enable_v6)));
  //   IPV6["IPV6_DHCP_ENABLE"] =
  //       json::value::string(U(to_string(ipmiNetwork[i].ip_src_v6)));
  //   sprintf(buf, "%02x%02x::%d", ipmiNetwork[i].df_gw_ip_addr_v6[0],
  //       ipmiNetwork[i].df_gw_ip_addr_v6[1],ipmiNetwork[i].df_gw_ip_addr_v6[2]);
  //   IPV6["IPV6_GATEWAY"] = json::value::string(U(buf));

  //   VLAN["VLAN_SETTINGS_ENABLE"] =
  //       json::value::string(U(to_string(vlan_enables)));
  //   VLAN["VLAN_ID"] = json::value::string(U(to_string(ipmiNetwork[i].vlan_id)));
  //   VLAN["VLAN_PRIORITY"] =
  //       json::value::string(U(to_string(ipmiNetwork[i].vlan_priority)));

  //   NETWORK_INFO["GENERIC"] = GENERIC;
  //   NETWORK_INFO["IPV4"] = IPV4;
  //   NETWORK_INFO["IPV6"] = IPV6;
  //   NETWORK_INFO["VLAN"] = VLAN;
  //   net_info_vec.push_back(NETWORK_INFO);
  // }
  // response_json["NETWORK_INFO"] = json::value::array(net_info_vec);

  // [테스트] network 임시처리 (redfish이용..)
  vector<json::value> network_vector;

  response_json["NETWORK_PRIORITY"] = json::value::string(temporary.net_priority);

  string eth_collection = ODATA_ETHERNET_INTERFACE_ID;
  Collection *eth_col = (Collection *)g_record[eth_collection];
  int eth_size = eth_col->members.size();
  cout << "[eth size] : " << eth_size << endl;
  for(int i=0; i<eth_size; i++)
  {
      json::value jv_networkInfo, jv_generic, jv_ipv4, jv_ipv6, jv_vlan;
      EthernetInterfaces *cur_eth = eth_col->members[i];
      cout << "111" << endl;
      
      jv_networkInfo["LAN_INTERFACE"] = json::value::string("eth" + cur_eth->id);

      if(cur_eth->interfaceEnabled)
        jv_generic["LAN_SETTING_ENABLE"] = json::value::string("1");
      else
        jv_generic["LAN_SETTING_ENABLE"] = json::value::string("0");
  cout << "112" << endl;
      jv_generic["MAC_ADDRESS"] = json::value::string(cur_eth->mac_address);

      if(cur_eth->dhcp_v4.dhcp_enabled)
        jv_ipv4["IPV4_DHCP_ENABLE"] = json::value::string("1");
      else
        jv_ipv4["IPV4_DHCP_ENABLE"] = json::value::string("0");
cout << "113" << endl;
      // json::value::string
      if(cur_eth->v_ipv4.size() == 0)
        continue;
      jv_ipv4["IPV4_ADDRESS"] = json::value::string(cur_eth->v_ipv4[0].address);
      jv_ipv4["IPV4_NETMASK"] = json::value::string(cur_eth->v_ipv4[0].subnet_mask);
      jv_ipv4["IPV4_GATEWAY"] = json::value::string(cur_eth->v_ipv4[0].gateway);
      jv_ipv4["IPV4_PREFERRED"] = json::value::string("");

      if(cur_eth->v_ipv6.size() == 0)
        continue;
  cout << "114" << endl;
      jv_ipv6["IPV6_ENABLE"] = json::value::string(temporary.net_v6_enabled);
      jv_ipv6["IPV6_DHCP_ENABLE"] = json::value::string(temporary.net_v6_dhcp);
      jv_ipv6["IPV6_ADDRESS"] = json::value::string(cur_eth->v_ipv6[0].address);
      jv_ipv6["IPV6_SUBNET_PREFIX_LENGTH"] = json::value::string(to_string(cur_eth->v_ipv6[0].prefix_length));
      jv_ipv6["IPV6_GATEWAY"] = json::value::string(cur_eth->ipv6_default_gateway);

      if(cur_eth->vlan.vlan_enable)
        jv_vlan["VLAN_SETTINGS_ENABLE"] = json::value::string("1");
      else
        jv_vlan["VLAN_SETTINGS_ENABLE"] = json::value::string("0");
      jv_vlan["VLAN_ID"] = json::value::string(to_string(cur_eth->vlan.vlan_id));
      jv_vlan["VLAN_PRIORITY"] = json::value::string(temporary.net_vlan_prior);
cout << "115" << endl;
      jv_networkInfo["GENERIC"] = jv_generic;
      jv_networkInfo["IPV4"] = jv_ipv4;
      jv_networkInfo["IPV6"] = jv_ipv6;
      jv_networkInfo["VLAN"] = jv_vlan;
cout << "116" << endl;
      network_vector.push_back(jv_networkInfo);

  }
  cout << "117" << endl;
  response_json["NETWORK_INFO"] = json::value::array(network_vector);
  cout << "118" << endl;
}


void Ipmiweb_GET::Get_Ntp_Info(json::value &response_json) {
  // const char *etables[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
  //                       "Aug", "Sep", "Oct", "Nov", "Dec", NULL};
  // // json::value obj = json::value::object();
  // json::value NTP_INFO = json::value::object();
  // json::value NTP = json::value::object();

  // if (access(NTPFILE, F_OK) == 0) {
  //   char buf[64];
  //   char server[32];
  //   char cmds[100] = {
  //       0,
  //   };

  //   NTP["AUTO_SYNC"] = json::value::string(U("1"));

  //   // ntp server 백업 주소 가져오기.
  //   if (access("/etc/ntp.conf.bak", F_OK) == 0) {
  //     sprintf(cmds, "mv /etc/ntp.conf.bak /etc/ntp.conf");
  //     system(cmds);
  //   }
  //   sprintf(buf, "awk '$1 == \"server\" {print $2}' %s", NTPFILE);

  //   FILE *fp = popen(buf, "r");

  //   fgets(server, 32, fp);
  //   server[strlen(server) - 1] = '\0';

  //   pclose(fp);

  //   NTP["NTP_SERVER"] = json::value::string(U(server));
  //   NTP["TIME_ZONE"] = json::value::string(U(""));
  //   NTP["YEAR"] = json::value::string(U(""));
  //   NTP["MONTH"] = json::value::string(U(""));
  //   NTP["DAY"] = json::value::string(U(""));
  //   NTP["HOUR"] = json::value::string(U(""));
  //   NTP["MIN"] = json::value::string(U(""));
  //   NTP["SEC"] = json::value::string(U(""));
  // } else {
  //   char date[32];
  //   char buf[8];

  //   FILE *fp = popen("date -R", "r");
  //   fgets(date, 32, fp);
  //   pclose(fp);

  //   char *token;
  //   token = strtok(date, " ");
  //   token = strtok(NULL, " ");

  //   NTP["AUTO_SYNC"] = json::value::string(U("0"));
  //   NTP["NTP_SERVER"] = json::value::string(U(""));
  //   NTP["DAY"] = json::value::string(U(token));

  //   token = strtok(NULL, " ");
  //   for (int i = 0; etables[i] != NULL; i++)
  //     if (!strcmp(token, etables[i]))
  //       NTP["MONTH"] = json::value::string(to_string(i + 1));

  //   token = strtok(NULL, " ");
  //   NTP["YEAR"] = json::value::string(U(token));

  //   token = strtok(NULL, " :");
  //   NTP["HOUR"] = json::value::string(U(token));

  //   token = strtok(NULL, " :");
  //   NTP["MIN"] = json::value::string(U(token));

  //   token = strtok(NULL, " :");
  //   NTP["SEC"] = json::value::string(U(token));

  //   token = strtok(NULL, " ");

  //   if (token[0] == '-')
  //     sprintf(buf, "GMT-%d", atoi(token + 1));
  //   else
  //     sprintf(buf, "GMT+%d", atoi(token + 1) / 100);

  //   NTP["TIME_ZONE"] = json::value::string(U(buf));
  // }

  // NTP_INFO["NTP"] = NTP;
  // response_json["NTP_INFO"] = NTP_INFO;

    // [테스트] ntp 임시처리 (redfish이용)
    json::value jv_ntp_info, jv_ntp;
    string network_odata = ODATA_MANAGER_ID;
    network_odata.append("/NetworkProtocol");
    NetworkProtocol *net = (NetworkProtocol *)g_record[network_odata];

    if(net->ntp.protocol_enabled)
      jv_ntp["AUTO_SYNC"] = json::value::string("1");
    else
      jv_ntp["AUTO_SYNC"] = json::value::string("0");
    
    string date_str, time_str;
    get_current_date_info(date_str);
    get_current_time_info(time_str);

    vector<string> split_date, split_time;
    split_date = string_split(date_str, '-');
    jv_ntp["YEAR"] = json::value::string(split_date[0]);
    jv_ntp["MONTH"] = json::value::string(split_date[1]);
    jv_ntp["DAY"] = json::value::string(split_date[2]);
    net->ntp.date_str = date_str;

    split_time = string_split(time_str, ':');
    jv_ntp["HOUR"] = json::value::string(split_time[0]);
    jv_ntp["MIN"] = json::value::string(split_time[1]);
    jv_ntp["SEC"] = json::value::string(split_time[2]);
    net->ntp.time_str = time_str;

    jv_ntp["TIME_ZONE"] = json::value::string(net->ntp.timezone);
    jv_ntp["NTP_SERVER"] = json::value::string(net->ntp.primary_ntp_server);

    jv_ntp_info["NTP"] = jv_ntp;
    response_json["NTP_INFO"] = jv_ntp_info;

  
}
void Ipmiweb_GET::Get_Smtp_Info(json::value &response_json, int flag){
  // char machine_name[50] = "\0", sender_address[50] = "\0",
  //      primary_server_address[50] = "\0", primary_user_name[50] = "\0";
  // char primary_user_password[50] = "\0", secondary_server_address[50] = "\0",
  //      secondary_user_name[50] = "\0", secondary_user_password[50] = "\0";
  // smtp_config_t smtp_config;
  // char res[3000]={0,};

  // if (access(SMTP_BIN, F_OK) == -1) {
  //   fprintf(stderr, "\t\tWarning : No SMTP configured.\n");
  // }

  // FILE *fp = fopen(SMTP_BIN, "r");

  // if (fp != NULL) {
  //   if (fread(&smtp_config, sizeof(smtp_config_t), 1, fp) < 1) {
  //     fprintf(stderr, "\t\tError : fread smtp_config for parsing failed\n");
  //     fclose(fp);
  //     return;
  //   }

  //   strcpy(machine_name, smtp_config.machine);
  //   strcpy(sender_address, smtp_config.sender);
  //   strcpy(primary_server_address, smtp_config.server1);
  //   strcpy(primary_user_name, smtp_config.id1);
  //   strcpy(primary_user_password, smtp_config.pwd1);
  //   strcpy(secondary_server_address, smtp_config.server2);
  //   strcpy(secondary_user_name, smtp_config.id2);
  //   strcpy(secondary_user_password, smtp_config.pwd2);
  //   fclose(fp);
  // }
  // primary_user_password[strlen(primary_user_password)] = '\0';


  //   cout << "machine_name"<<machine_name<<endl;
  // cout << "sender_address"<<sender_address<<endl;
  // cout << "primary_server_address"<<primary_server_address<<endl;
  // cout << "primary_user_name"<<primary_user_name<<endl;
  // cout << "primary_user_password"<<primary_user_password<<endl;
  // cout << "secondary_server_address"<<secondary_server_address<<endl;
  // cout << "secondary_user_name"<<secondary_user_name<<endl;
  // cout << "secondary_user_password"<<secondary_user_password<<endl;
  // char buf[200];
  // // json::value obj = json::value::object();
  // json::value SMTP_INFO = json::value::object();

  // if (flag == 0) {
  //   cout <<"flag" <<flag<<endl;
  //   json::value DEVICE = json::value::object();
  //   DEVICE["MACHINE_NAME"] = json::value::string(U(machine_name));
  //   DEVICE["SENDER_ADDRESS"] = json::value::string(U(sender_address));

  //   json::value PRIMARY = json::value::object();
  //   PRIMARY["PRIMARY_SERVER_ADDRESS"] =
  //       json::value::string(U(primary_server_address));
  //   PRIMARY["PRIMARY_USER_NAME"] = json::value::string(U(primary_user_name));
  //   PRIMARY["PRIMARY_USER_PASSWORD"] =
  //       json::value::string(U(primary_user_password));

  //   json ::value SECONDARY = json::value::object();
  //   SECONDARY["SECONDARY_SERVER_ADDRESS"] =
  //       json::value::string(U(secondary_server_address));
  //   SECONDARY["SECONDARY_USER_NAME"] =
  //       json::value::string(U(secondary_user_name));
  //   SECONDARY["SECONDARY_USER_PASSWORD"] =
  //       json::value::string(U(secondary_user_password));

  //   SMTP_INFO["DEVICE"] = DEVICE;
  //   SMTP_INFO["PRIMARY"] = PRIMARY;
  //   SMTP_INFO["SECONDARY"] = SECONDARY;
  //   response_json["SMTP_INFO"] = SMTP_INFO;
  //   // strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  // } else if (flag == 1) { // get primary receiver
  //   if (strcmp(primary_server_address, "\0") == 0) {
  //     fprintf(stderr, "No primary receiver\n");
  //     // return 0;
  //   }
  //   sscanf(primary_server_address, "smtp.%s", buf);
  //   sprintf(res, "%s@%s", primary_user_name, buf);
  // } else if (flag == 2) { // get secondary receiver
  //   if (strcmp(secondary_server_address, "\0") == 0) {
  //     fprintf(stderr, "No secondary receiver\n");
  //     // return 0;
  //   }
  //   sscanf(secondary_server_address, "smtp.%s", buf);
  //   sprintf(res, "%s@%s", secondary_user_name, buf);
  // response_json = json::value::parse(res);
  // }

  // [테스트] smtp 임시처리 (redfish이용)
  json::value jv_smtpInfo, jv_device, jv_primary, jv_secondary;

  EventService *eventService = (EventService *)g_record[ODATA_EVENT_SERVICE_ID];

  jv_device["MACHINE_NAME"] = json::value::string(temporary.smtp_machineName);
  jv_device["SENDER_ADDRESS"] = json::value::string(eventService->smtp.smtp_sender_address);

  jv_primary["PRIMARY_SERVER_ADDRESS"] = json::value::string(eventService->smtp.smtp_server);
  jv_primary["PRIMARY_USER_NAME"] = json::value::string(eventService->smtp.smtp_username);
  jv_primary["PRIMARY_USER_PASSWORD"] = json::value::string(eventService->smtp.smtp_password);

  jv_secondary["SECONDARY_SERVER_ADDRESS"] = json::value::string(temporary.smtp_second_server);
  jv_secondary["SECONDARY_USER_NAME"] = json::value::string(temporary.smtp_second_username);
  jv_secondary["SECONDARY_USER_PASSWORD"] = json::value::string(temporary.smtp_second_password);

  jv_smtpInfo["DEVICE"] = jv_device;
  jv_smtpInfo["PRIMARY"] = jv_primary;
  jv_smtpInfo["SECONDARY"] = jv_secondary;

  response_json["SMTP_INFO"] = jv_smtpInfo;
  
}

// [테스트] certificate 만들면 그곳으로 이동 / 임시사용
string parse_validate_info_string(string _info)
{
    // notBefore=Jul 26 07:01:16 2022 GMT
    string result;
    string back = get_current_object_name(_info, "=");
    vector<string> part = string_split(back, ' ');

    // for(int i=0; i<part.size(); i++)
    // {
    //     cout << part[i] << " / ";
    //     // log(trace) << part[i] << " / ";
    // }
    // cout << endl;

    result = part[3];
    result.append("-");

    if(part[0] == "Jan")
        result.append("01-");
    else if(part[0] == "Feb")
        result.append("02-");
    else if(part[0] == "Mar")
        result.append("03-");
    else if(part[0] == "Apr")
        result.append("04-");
    else if(part[0] == "May")
        result.append("05-");
    else if(part[0] == "Jun")
        result.append("06-");
    else if(part[0] == "Jul")
        result.append("07-");
    else if(part[0] == "Aug")
        result.append("08-");
    else if(part[0] == "Sep")
        result.append("09-");
    else if(part[0] == "Oct")
        result.append("10-");
    else if(part[0] == "Nov")
        result.append("11-");
    else if(part[0] == "Dec")
        result.append("12-");

    if(part[1].length() < 2)
        result.append("0");
    
    result.append(part[1]).append(" ").append(part[2]);

    // cout << "RESULT : " << result << endl;

    return result;

}

void Ipmiweb_GET::Get_Ssl_Info(json::value &response_json){  
  // char country[3] = "\0", state_province[32] = "\0", city_locality[32] = "\0",
  //      organ[32] = "\0";
  // char organ_unit[32] = "\0", common[32] = "\0", email[32] = "\0",
  //      keylen[5] = "\0";
  // char valid_from[16] = "\0", valid_to[16] = "\0";
  // int valid_for = 0;
  // FILE *fp_read = fopen(SSL_BIN, "r");
  // CertificateService *certificate =
  //     ((CertificateService *)g_record[ODATA_CERTIFICATE_SERVICE_ID]);

  // if (fp_read == NULL) {
  //   fprintf(stderr, "\t\tWarning : No ssl_bin.\n");
  // } else {
  //   ssl_config_t ssl_config;
  //   if (fread(&ssl_config, sizeof(ssl_config_t), 1, fp_read) < 1) {
  //     fprintf(stderr, "\t\tError : fread ssl_config for parsing failed\n");
  //     fclose(fp_read);
  //     return 0;
  //   }

  //   fclose(fp_read);

  //   strcpy(country, ssl_config.country);
  //   strcpy(state_province, ssl_config.state_province);
  //   strcpy(city_locality, ssl_config.city_locality);
  //   strcpy(organ, ssl_config.organ);
  //   strcpy(organ_unit, ssl_config.organ_unit);
  //   strcpy(common, ssl_config.common);
  //   strcpy(email, ssl_config.email);
  //   strcpy(keylen, ssl_config.keylen);
  //   strcpy(valid_from, ssl_config.valid_from);
  //   strcpy(valid_to, ssl_config.valid_to);
  //   valid_for = ssl_config.valid_for;
  // }

  // // json::value obj = json::value::object();
  // json::value SSL_INFO = json::value::object();

  // json::value BASIC = json::value::object();
  // BASIC["VERSION"] = json::value::string(U("1.0.2e"));
  // BASIC["SERIAL_NUMBER"] = json::value::string(U("9FF7A"));
  // BASIC["SIGNATURE_ALGORITHM"] = json::value::string(U("RSA"));

  // json::value ISSUED_FROM = json::value::object();
  // ISSUED_FROM["COMMON_NAME"] = json::value::string(U(common));
  // ISSUED_FROM["ORGANIZATION"] = json::value::string(U(organ));
  // ISSUED_FROM["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));
  // ISSUED_FROM["CITY_OR_LOCALITY"] = json::value::string(U(city_locality));
  // ISSUED_FROM["STATE_OR_PROVINCE"] = json::value::string(U(state_province));
  // ISSUED_FROM["COUNTRY"] = json::value::string(U(country));
  // ISSUED_FROM["EMAIL_ADDRESS"] = json::value::string(U(email));
  // ISSUED_FROM["VALID_FOR"] = json::value::string(to_string(valid_for));
  // ISSUED_FROM["KEY_LENGTH"] = json::value::string(U("1024"));

  // json::value VALIDITY_INFORMATION = json::value::object();
  // VALIDITY_INFORMATION["VALID_FROM"] = json::value::string(U(valid_from));
  // VALIDITY_INFORMATION["VALID_FOR"] = json::value::string(U(valid_to));

  // json::value ISSUED_TO = json::value::object();
  // ISSUED_TO["COMMON_NAME"] = json::value::string(U(common));
  // ISSUED_TO["ORGANIZATION"] = json::value::string(U(organ));
  // ISSUED_TO["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));

  // SSL_INFO["BASIC"] = BASIC;
  // SSL_INFO["ISSUED_FROM"] = ISSUED_FROM;
  // SSL_INFO["ISSUED_TO"] = ISSUED_TO;
  // SSL_INFO["VALIDITY_INFORMATION"] = VALIDITY_INFORMATION;
  // response_json["SSL_INFO"] = SSL_INFO;

  // strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  // response_json = json::value::parse(res);

    // [테스트] ssl 변경 및 임시적용
    // certificate 리소스를 사용하진 않고 (이건 cmm쪽 가져오면서 바꾸기)
    // 일단 temporary에다가 만든걸 이용하는데 키 발급하고 config만들어서 cert를 생성하는거까지는 넣을거임
    // 그 cert에서 읽어오는 값들로 temporary를 갱신하고 그걸로 get 보여주기
    // set하나도 안되니까 다시 해놔야함
    json::value jv_ssl, jv_basic, jv_issued_from, jv_issued_to, jv_validity;
    fs::path certFile("/conf/ssl/edge_server.crt");

    if(!fs::exists(certFile))
    {
        log(warning) << "[Get SSL Info] : No Cert File";
        return ;
    }
    

    temporary.ssl_keyLength = "1024";

    
    // 따와야 하는거
    // 베이직 - 시리얼 넘버
    // 이슈프롬 - 풀세트
    // 이슈 투 - 똑같은데 거기서빼서씀
    // 밸리드 - notafter/before
    
    // [step 1] Get Issuer Information
    //issuer=C = US, ST = Gyeonggi-do, L = Seong-Nam, O = KETI, OU = HumanIT, CN = Choi, emailAddress = qwer@naver.com
    string cmd_issuer = "openssl x509 -in " + certFile.string() + " -issuer -noout";
    string issuer_result = get_popen_string(cmd_issuer);

    //C = US, ST = Gyeonggi-do, L = Seong-Nam, O = KETI, OU = HumanIT, CN = Choi, emailAddress = qwer@naver.com
    string removed_issuer = issuer_result.substr(issuer_result.find("=")+1);

    cout << "Issuer >>>>>>>>>>> " << endl;
    cout << "origin : " << issuer_result << endl;
    cout << "Removed : " << removed_issuer << endl;

    vector<string> block = string_split(removed_issuer, ',');
    for(int i=0; i<block.size(); i++)
    {
        vector<string> component = string_split(block[i], '=');
        if(component.size() != 2)
        {
            log(warning) << "why this component size not 2?";
            continue;
        }

        string key = component[0], value = component[1];
        string parse_key, parse_value;

        // remove space >> CMM 이랑 다른 점임
        int key_firstIndex, key_lastIndex, value_firstIndex, value_lastIndex;
        key_firstIndex = key.find_first_not_of(' ');
        key_lastIndex = key.find_last_not_of(' ');
        parse_key = key.substr(key_firstIndex, key_lastIndex-key_firstIndex+1);

        value_firstIndex = value.find_first_not_of(' ');
        value_lastIndex = value.find_last_not_of(' ');
        parse_value = value.substr(value_firstIndex, value_lastIndex-value_firstIndex+1);

        // cout << "key/value/parse_key/parse_value" << endl;
        // cout << key << "/" << value << "/" << parse_key << "/" << parse_value << endl;

        if(parse_key == "C")
            temporary.ssl_countryName = parse_value;
        else if(parse_key == "ST")
            temporary.ssl_state = parse_value;
        else if(parse_key == "L")
            temporary.ssl_city = parse_value;
        else if(parse_key == "O")
            temporary.ssl_org = parse_value;
        else if(parse_key == "OU")
            temporary.ssl_orgUnit = parse_value;
        else if(parse_key == "CN")
            temporary.ssl_commonName = parse_value;
        else if(parse_key == "emailAddress")
            temporary.ssl_email = parse_value;
          
        
    }

    // [step 2] Get Validity Information
    string cmd_notBefore, cmd_notAfter;
    cmd_notBefore = "openssl x509 -in " + certFile.string() + " -startdate -noout";
    cmd_notAfter = "openssl x509 -in " + certFile.string() + " -enddate -noout";

    string notBefore_result = get_popen_string(cmd_notBefore);
    string notAfter_result = get_popen_string(cmd_notAfter);

    temporary.ssl_validBefore = parse_validate_info_string(notBefore_result);
    temporary.ssl_validAfter = parse_validate_info_string(notAfter_result);

    // [step 3] Get Serial Information
    // serial=77E6D490123270E68BA19D5A40994EEE6D0A51C6
    string cmd_serial = "openssl x509 -in " + certFile.string() + " -serial -noout";
    string serial_result = get_popen_string(cmd_serial);
    temporary.ssl_serialNumber = get_current_object_name(serial_result, "=");

    jv_basic["VERSION"] = json::value::string("3");
    jv_basic["SERIAL_NUMBER"] = json::value::string(temporary.ssl_serialNumber);
    jv_basic["SIGNATURE_ALGORITHM"] = json::value::string("RSA");

    jv_issued_from["COMMON_NAME"] = json::value::string(temporary.ssl_commonName);
    jv_issued_from["ORGANIZATION"] = json::value::string(temporary.ssl_org);
    jv_issued_from["ORGANIZATION_UNIT"] = json::value::string(temporary.ssl_orgUnit);
    jv_issued_from["CITY_OR_LOCALITY"] = json::value::string(temporary.ssl_city);
    jv_issued_from["STATE_OR_PROVINCE"] = json::value::string(temporary.ssl_state);
    jv_issued_from["COUNTRY"] = json::value::string(temporary.ssl_countryName);
    jv_issued_from["EMAIL_ADDRESS"] = json::value::string(temporary.ssl_email);
    jv_issued_from["VALID_FOR"] = json::value::string(temporary.ssl_validAfter);
    jv_issued_from["KEY_LENGTH"] = json::value::string(temporary.ssl_keyLength);

    jv_validity["VALID_FROM"] = json::value::string(temporary.ssl_validBefore);
    jv_validity["VALID_FOR"] = json::value::string(temporary.ssl_validAfter);

    jv_issued_to["COMMON_NAME"] = json::value::string(temporary.ssl_commonName);
    jv_issued_to["ORGANIZATION"] = json::value::string(temporary.ssl_org);
    jv_issued_to["ORGANIZATION_UNIT"] = json::value::string(temporary.ssl_orgUnit);
    jv_issued_to["CITY_OR_LOCALITY"] = json::value::string(temporary.ssl_city);

    


    

    // json::value BASIC = json::value::object();
  // BASIC["VERSION"] = json::value::string(U("1.0.2e"));
  // BASIC["SERIAL_NUMBER"] = json::value::string(U("9FF7A"));
  // BASIC["SIGNATURE_ALGORITHM"] = json::value::string(U("RSA"));

  // json::value ISSUED_FROM = json::value::object();
  // ISSUED_FROM["COMMON_NAME"] = json::value::string(U(common));
  // ISSUED_FROM["ORGANIZATION"] = json::value::string(U(organ));
  // ISSUED_FROM["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));
  // ISSUED_FROM["CITY_OR_LOCALITY"] = json::value::string(U(city_locality));
  // ISSUED_FROM["STATE_OR_PROVINCE"] = json::value::string(U(state_province));
  // ISSUED_FROM["COUNTRY"] = json::value::string(U(country));
  // ISSUED_FROM["EMAIL_ADDRESS"] = json::value::string(U(email));
  // ISSUED_FROM["VALID_FOR"] = json::value::string(to_string(valid_for));
  // ISSUED_FROM["KEY_LENGTH"] = json::value::string(U("1024"));

  // json::value VALIDITY_INFORMATION = json::value::object();
  // VALIDITY_INFORMATION["VALID_FROM"] = json::value::string(U(valid_from));
  // VALIDITY_INFORMATION["VALID_FOR"] = json::value::string(U(valid_to));

  // json::value ISSUED_TO = json::value::object();
  // ISSUED_TO["COMMON_NAME"] = json::value::string(U(common));
  // ISSUED_TO["ORGANIZATION"] = json::value::string(U(organ));
  // ISSUED_TO["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));





    jv_ssl["BASIC"] = jv_basic;
    jv_ssl["ISSUED_FROM"] = jv_issued_from;
    jv_ssl["ISSUED_TO"] = jv_issued_to;
    jv_ssl["VALIDITY_INFORMATION"] = jv_validity;

    response_json["SSL_INFO"] = jv_ssl;


}


void Ipmiweb_GET::Get_Active_Dir(json::value &response_json){
//  int enable = 0;
//   char ip[16] = {
//       0,
//   };
//   char domain[64] = {
//       0,
//   };
//   char s_username[64] = {
//       0,
//   };
//   char s_password[32] = {
//       0,
//   };

//   FILE *fp_read = fopen(AD_BIN, "rb");
//   if (fp_read != NULL) {
//     ad_config_t ad_config;
//     if (fread(&ad_config, sizeof(ad_config_t), 1, fp_read) < 1) {
//       fprintf(stderr, "Error : fread ad_config for GET failed\n");
//     }
//     enable = ad_config.enable;
//     if (enable) {
//       strcpy(ip, ad_config.dc_ip);
//       strcpy(domain, ad_config.domain);
//       strcpy(s_username, ad_config.secret_name);
//       strcpy(s_password, ad_config.secret_pwd);
//     }
//     fclose(fp_read);
//   }

//   // json::value obj = json::value::object();
//   json::value ACTIVE_DIRECTORY = json::value::object();
//   ACTIVE_DIRECTORY["ENABLE"] = json::value::string(U(std::to_string(enable)));
//   ACTIVE_DIRECTORY["IP"] = json::value::string(U(ip));
//   ACTIVE_DIRECTORY["DOMAIN"] = json::value::string(U(domain));
//   ACTIVE_DIRECTORY["SECRET_NAME"] = json::value::string(U(s_username));
//   ACTIVE_DIRECTORY["SECRET_PWD"] = json::value::string(U(s_password));
//   response_json["ACTIVE_DIRECTORY"] = ACTIVE_DIRECTORY;

    // [테스트] ad 임시 값 처리...
    json::value jv_ad;

    jv_ad["ENABLE"] = json::value::string(temporary.ad_enabled);
    jv_ad["SECRET_NAME"] = json::value::string(temporary.ad_username);
    jv_ad["SECRET_PWD"] = json::value::string(temporary.ad_password);
    jv_ad["DOMAIN"] = json::value::string(temporary.ad_domain);
    jv_ad["IP"] = json::value::string(temporary.ad_domainControllerAddress);

    response_json["ACTIVE_DIRECTORY"] = jv_ad;

}


void Ipmiweb_GET::Get_Ldap(json::value &response_json){
// char ip[16] =
//       {
//           0,
//       },
//        port[6] =
//            {
//                0,
//            },
//        searchbase[32] =
//            {
//                0,
//            },
//        binddn[32] =
//            {
//                0,
//            },
//        pwd[32] = {
//            0,
//        };
//   int timelimit = 0;
//   int enable = 0;
//   int ssl = 0;
//   ldap_config_t ldap_config;

//   memset(&ldap_config, 0, sizeof(ldap_config_t));
//   FILE *fp_read = fopen(LDAP_BIN, "rb");
//   if (fp_read != NULL) {
//     if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
//       fprintf(stderr, "Error : fread ldap_config\n");
//     }
//     enable = ldap_config.enable;
//     printf("\t\t\t\tcheckpoint2 : ldap en : %d\n", enable);

//     if (enable) {
//       strcpy(ip, ldap_config.ip);
//       strcpy(port, ldap_config.port);
//       strcpy(searchbase, ldap_config.basedn);
//       strcpy(binddn, ldap_config.binddn);
//       strcpy(pwd, ldap_config.bindpw);
//       ssl = ldap_config.ssl;
//       timelimit = ldap_config.timelimit;
//     }
//     fclose(fp_read);
//   }

//   // json::value obj = json::value::object();
//   json::value LDAP_INFO = json::value::object();
//   json::value LDAP = json::value::object();

//   LDAP["LDAP_EN"] = json::value::string(U(to_string(enable)));
//   LDAP["BIND_PW"] = json::value::string(U(pwd));
//   LDAP["LDAP_IP"] = json::value::string(U(ip));
//   LDAP["LDAP_PORT"] = json::value::string(U(port));
//   LDAP["TIMEOUT"] = json::value::string(U(to_string(timelimit)));
//   LDAP["BASE_DN"] = json::value::string(U(searchbase));
//   LDAP["LDAP_SSL"] = json::value::string(U(to_string(ssl)));
//   LDAP["BIND_DN"] = json::value::string(U(binddn));
//   LDAP_INFO["LDAP"] = LDAP;
//   response_json["LDAP_INFO"] = LDAP_INFO;

    // [테스트] ldap 임시 값 처리...
    json::value jv_ldap, jv_return;

    jv_ldap["LDAP_EN"] = json::value::string(temporary.ldap_enabled);
    jv_ldap["LDAP_SSL"] = json::value::string(temporary.ldap_encryptionType);
    jv_ldap["LDAP_IP"] = json::value::string(temporary.ldap_serverAddress);
    jv_ldap["LDAP_PORT"] = json::value::string(temporary.ldap_port);
    jv_ldap["BIND_DN"] = json::value::string(temporary.ldap_bindDN);
    jv_ldap["BIND_PW"] = json::value::string(temporary.ldap_bindPW);
    jv_ldap["BASE_DN"] = json::value::string(temporary.ldap_baseDN);
    jv_ldap["TIMEOUT"] = json::value::string(temporary.ldap_timeout);

    jv_return["LDAP"] = jv_ldap;
    response_json["LDAP_INFO"] = jv_return;
}

void Ipmiweb_GET::Get_Radius(json::value &response_json){
  // char ip[16] = "\0", port[6] = "\0", secret[32] = "\0";
  // FILE *fp = fopen(RAD_BIN, "r");
  // rad_config_t rad_config;
  // int enable;
  // if (fp != NULL) {
  //   if (fread(&rad_config, sizeof(rad_config_t), 1, fp) < 1) {
  //     fprintf(stderr, "\t\tError : fread error radc_config");
  //     fclose(fp);
  //     return 0;
  //   }
  //   enable = rad_config.enable;
  //   strcpy(ip, rad_config.ip);
  //   strcpy(port, rad_config.port);
  //   strcpy(secret, rad_config.secret);
  //   fclose(fp);
  // }

  // // json::value obj = json::value::object();
  // json::value RADIUS_INFO = json::value::object();
  // json::value RADIUS = json::value::object();

  // RADIUS["RADIUS_ENABLE"] = json::value::string(to_string(enable));
  // RADIUS["IP"] = json::value::string(U(ip));
  // RADIUS["PORT"] = json::value::string(U(port));
  // RADIUS["SECRET"] = json::value::string(U(secret));
  // RADIUS_INFO["RADIUS"] = RADIUS;
  // response_json["RADIUS_INFO"] = RADIUS_INFO;

      // [테스트] radius 임시 값 처리...
      json::value jv_radius, jv_return;

      jv_radius["RADIUS_ENABLE"] = json::value::string(temporary.radius_enabled);
      jv_radius["IP"] = json::value::string(temporary.radius_serverAddress);
      jv_radius["PORT"] = json::value::string(temporary.radius_port);
      jv_radius["SECRET"] = json::value::string(temporary.radius_secret);

      jv_return["RADIUS"] = jv_radius;
      response_json["RADIUS_INFO"] = jv_return;
}


void Ipmiweb_GET::Get_Setting_Service(json::value &response_json){	
  // json::value obj = json::value::object();
	// json::value SETTING_SERVICE = json::value::object();

	// if ((access(SSH_SERVICE_BIN, F_OK) != 0) || (access(KVM_PORT_BIN, F_OK) != 0) || (access(ALERT_PORT_BIN, F_OK) != 0) || (access(WEB_PORT_BIN, F_OK) != 0)) {	
	// 	if (init_setting_service() != 0)
	// 		return FAIL;
	// }

	// SETTING_SERVICE["WEB_PORT"] = json::value::string(U(g_setting.web_port));
	// SETTING_SERVICE["SSH_ENABLES"] = json::value::number(g_setting.ssh_enables);
	// SETTING_SERVICE["SSH_PORT"] = json::value::string(U(g_setting.ssh_port));
	// SETTING_SERVICE["ALERT_ENABLES"] = json::value::number(g_setting.alert_enables);
	// SETTING_SERVICE["ALERT_PORT"] = json::value::string(U(g_setting.alert_port));
	// SETTING_SERVICE["KVM_ENABLES"] = json::value::number(g_setting.kvm_enables);
	// SETTING_SERVICE["KVM_PORT"] = json::value::string(U(g_setting.kvm_port));
	// SETTING_SERVICE["KVM_PROXY_PORT"] = json::value::string(U(g_setting.kvm_proxy_port));
	// response_json["SETTING_SERVICE"] = SETTING_SERVICE;

    // [테스트] setting 임시 처리...
    json::value jv_setting;

    string stringListen, webPort;
    int pos_semicolon, pos_startingNumber;
  
    //         listen 80;
    stringListen = get_popen_string("sed -n 35p /etc/nginx/nginx.conf");
    pos_semicolon = stringListen.find_first_of(";");
    pos_startingNumber = stringListen.find_first_of("0123456789");
    webPort = stringListen.substr(pos_startingNumber, pos_semicolon-pos_startingNumber);
    
    
    // cout << "READING PORT !! : " << webPort << endl; // 80;
    
    temporary.setting_webPort = webPort;
    jv_setting["WEB_PORT"] = json::value::string(temporary.setting_webPort);
    jv_setting["KVM_PORT"] = json::value::string(temporary.setting_kvmPort);
    jv_setting["KVM_PROXY_PORT"] = json::value::string(temporary.setting_kvmProxyPort);
    jv_setting["ALERT_ENABLES"] = json::value::string(temporary.setting_alertEnabled);
    jv_setting["ALERT_PORT"] = json::value::string(temporary.setting_alertPort);
    jv_setting["SSH_ENABLES"] = json::value::string(temporary.setting_sshEnabled);
    jv_setting["SSH_PORT"] = json::value::string(temporary.setting_sshPort);
    response_json["SETTING_SERVICE"] = jv_setting;

  }


  void Ipmiweb_GET::Get_KVM(json::value &response_json){
    uint8_t cmds[100] = {0};
    sprintf(cmds, "ps | grep KETI-KVM | grep -v grep");
    FILE *p = popen(cmds, "r");
    uint8_t response[500], result[500];
    if (p != NULL) {
      while (fgets(result, sizeof(result), p) != NULL)
        strncat(response, result, strlen(result));
      pclose(p);
    }

    if (strlen(response) == 0) {
      memset(cmds, 0, sizeof(cmds));
      sprintf(cmds, "KETI-KVM &");
      system(cmds);
      printf("\t\t\tdy : kvm 실행\n");
     
    } else {
      int ret = 0;

      ret = rest_send_to_kvm();
      printf("\t\t\tdy : kvm 종료 요청 전송 완료\n");

      if (ret == 0) {

        json::value response_json = json::value::object();
        json::value SETTING_SERVICE = json::value::object();

        if ((access(SSH_SERVICE_BIN, F_OK) != 0) || (access(KVM_PORT_BIN, F_OK) != 0) || (access(ALERT_PORT_BIN, F_OK) != 0) || (access(WEB_PORT_BIN, F_OK) != 0)) {	
          if (init_setting_service() != 0)
            return FAIL;
        }

        SETTING_SERVICE["WEB_PORT"] = json::value::string(U(g_setting.web_port));
        SETTING_SERVICE["SSH_ENABLES"] = json::value::number(g_setting.ssh_enables);
        SETTING_SERVICE["SSH_PORT"] = json::value::string(U(g_setting.ssh_port));
        SETTING_SERVICE["ALERT_ENABLES"] = json::value::number(g_setting.alert_enables);
        SETTING_SERVICE["ALERT_PORT"] = json::value::string(U(g_setting.alert_port));
        SETTING_SERVICE["KVM_ENABLES"] = json::value::number(g_setting.kvm_enables);
        SETTING_SERVICE["KVM_PORT"] = json::value::string(U(g_setting.kvm_port));
        SETTING_SERVICE["KVM_PROXY_PORT"] = json::value::string(U(g_setting.kvm_proxy_port));
        response_json["SETTING_SERVICE"] = SETTING_SERVICE;

        // uint8_t res_msg[QSIZE];
        int res_len = 0;
        // request_get_setting_service(res_msg, &res_len);
        // printf("\t\t\tdy : setting service result : %s, %d\n", res_msg,
        //        res_len);
        if (res_len != 0) {
          json::value  service;
          string kvm_proxy, kvm_port;
          // refine_data(res_msg);
          // string sdata((char *)res_msg);

          // myobj = json::value::parse(sdata);
          service = response_json.at("SETTING_SERVICE");

          kvm_port = service.at("KVM_PORT").as_string();
          kvm_proxy = service.at("KVM_PROXY_PORT").as_string();

          memset(cmds, 0, sizeof(cmds));
          sprintf(cmds, "KETI-KVM %s %s &", kvm_port.c_str(),
                  kvm_proxy.c_str());
          system(cmds);

         
        }
      }
    }
  }


  void Ipmiweb_GET::Get_Power_Satus(json::value &response_json){
  // json::value obj = json::value::object();
  json::value power = json::value::object();
  unsigned char status = 0;

  status = ipmiChassis.get_power_status();

  if (status == 1)
    power["STATUS"] = json::value::string(U("1"));
  else if (status == 0)
    power["STATUS"] = json::value::string(U("0"));
  else
    power["STATUS"] = json::value::string(U("Unknown")); // error

  response_json["POWER"] = power;


  }

  void Ipmiweb_GET::Get_Usb_Info(json::value &response_json){
    // uint8_t *file = "/conf/nfs_save.sh";
    // json::value obj = json::value::object();
    // json::value usb = json::value::object();

    // if (access(file, F_OK) == 0) {
    //   FILE *p = popen("cat /conf/nfs_save.sh", "r");
    //   uint8_t result[150], nfs_option[1500], ippath[1500], ipaddr[1500],
    //       path_np[1500], path[1500] = {0};

    //   if (p != NULL) {
    //     memset(result, 0, sizeof(result));
    //     memset(nfs_option, 0, sizeof(nfs_option));
    //     while (fgets(result, sizeof(result), p) != NULL)
    //       strncat(nfs_option, result, strlen(result));
    //     pclose(p);
    //   }

    //   if (strlen(nfs_option) > 0) {

    //     p = popen("cat /conf/nfs_save.sh | grep mount | awk \'{print $4}\'",
    //               "r");
    //     if (p != NULL) {
    //       memset(ippath, 0, sizeof(ippath));
    //       memset(ipaddr, 0, sizeof(ipaddr));
    //       memset(result, 0, sizeof(result));
    //       while (fgets(result, sizeof(result), p) != NULL)
    //         strncat(ippath, result, strlen(result));
    //       pclose(p);
    //     }
    //     uint8_t *r_index;

    //     r_index = strrchr((char *)ippath, ':');
    //     strncpy(ipaddr, ippath, strlen(ippath));
    //     int loop = 0;
    //     for (loop = (strlen(ippath) - strlen(r_index)); loop < strlen(ippath);
    //          loop++)
    //       ipaddr[loop] = 0;

    //     p = popen("cat /conf/nfs_save.sh | grep modprobe | awk \'{print $3}\'",
    //               "r");
    //     if (p != NULL) {
    //       memset(ippath, 0, sizeof(ippath));
    //       memset(result, 0, sizeof(result));
    //       while (fgets(result, sizeof(result), p) != NULL)
    //         strncat(ippath, result, strlen(result));
    //       pclose(p);
    //     }

    //     r_index = rindex((char *)ippath, '=');
    //     r_index[strlen(r_index) - 1] = '\0';

    //     usb["IP_ADDRESS"] = json::value::string(U((char *)ipaddr));
    //     usb["PATH"] = json::value::string(U((char *)(r_index + 1)));
    //     response_json["USB"] = usb;
    //   } else {
    //     usb["IP_ADDRESS"] = json::value::string(U(""));
    //     usb["PATH"] = json::value::string(U(""));
    //     response_json["USB"] = usb;
    //   }
    //   uint8_t header[1000], response[1000] = {0};

    // } else {
    //   uint8_t header[1000], response[1000] = {0};

    //   usb["IP_ADDRESS"] = json::value::string(U(""));
    //   usb["PATH"] = json::value::string(U(""));
    //   response_json["USB"] = usb;
    // }

      // [테스트] redfish로..
      // redfish Manager 밑에 virtual로 읽고 없으면 빈칸
      // post로 insertMedia...
      string odata_id = ODATA_MANAGER_ID;
      odata_id.append("/VirtualMedia");
      json::value jv_vm_list, jv_vm;
      jv_vm_list = json::value::array();

      Collection *vm_col = (Collection *)g_record[odata_id];
      for(int i=0; i<vm_col->members.size(); i++)
      {
          jv_vm = json::value::object();
          VirtualMedia *vm = (VirtualMedia *)(vm_col->members[i]);

          jv_vm["ID"] = json::value::string(vm->id);
          jv_vm["SIZE"] = json::value::string(vm->size);
          jv_vm["CREATE_TIME"] = json::value::string(vm->create_time);
          jv_vm["NAME"] = json::value::string(vm->image_name);

          string host = get_parent_object_uri(vm->image, ":");
          jv_vm["IP_ADDRESS"] = json::value::string(host);
          jv_vm["PATH"] = json::value::string(vm->image_name);
          jv_vm["USER"] = json::value::string(vm->user_name);
          jv_vm["PASSWORD"] = json::value::string(vm->password);

          if(vm->inserted)
            jv_vm["INSERTED"] = json::value::string("O");
          else
            jv_vm["INSERTED"] = json::value::string("X");

          jv_vm_list[i] = jv_vm;
      }

      response_json["VM"] = jv_vm_list;
  }

  // void Ipmiweb_GET::Get_Sol(http_response &response){
  //   response.set_status_code(status_codes::OK);
    

  // }

void Ipmiweb_GET::Get_User_List(json::value &response_json){
//  json::value obj = json::value::object();
//     json::value USER = json::value::object();
//     vector<json::value> INFO_VEC;
//     Ipmiuser temp;
//     int last_index = 0;
//     int user_cnt = 0;

//     last_index = user_loading();
//     user_cnt = get_user_cnt();
//     cout<<"user_cnt " << get_user_cnt()<<endl;
//     for (int i = 0; i < user_cnt; i++){
//         json::value INFO = json::value::object();
        
//         INFO["INDEX"] = json::value::string(U(to_string(i+1)));
//         INFO["NAME"] = json::value::string(ipmiUser[i].getUsername());
//         INFO["PASSWORD"] = json::value::string(ipmiUser[i].getUserpassword());
//         INFO["ENABLE_STATUS"] = json::value::string(U(to_string(ipmiUser[i].getUserenable())));
//         INFO["CALLIN"] = json::value::string(U(to_string(ipmiUser[i].getUserCallin())));
//         INFO["LINKAUTH"] = json::value::string(U(to_string(ipmiUser[i].getUserLinkAuth())));
//         INFO["IPMIMSG"] = json::value::string(U(to_string(ipmiUser[i].getUserIpmi())));
//         INFO["PRIVILEGE"] = json::value::string(U(to_string(ipmiUser[i].getUserPriv())));

//         INFO_VEC.push_back(INFO);
//     }
    
//     USER["INFO"] = json::value::array(INFO_VEC);
//     response_json["USER"] = USER;

      // [테스트] Account 레드피시로 적용
      json::value jv_user, jv_infoArray, jv_account;
      jv_infoArray = json::value::array();

      Collection *account_col = (Collection *)g_record[ODATA_ACCOUNT_ID];
      for(int i=0; i<account_col->members.size(); i++)
      {
          Account *account = (Account *)account_col->members[i];

          jv_account = json::value::object();
          
          // Q.값 의미?
          // jv_account["CALLIN"] = json::value::string("1");
          // jv_account["IPMIMSG"] = json::value::string("1");
          // jv_account["LINKAUTH"] = json::value::string("1");
          jv_account["CALLIN"] = json::value::string(to_string(account->callin));
          jv_account["IPMIMSG"] = json::value::string(to_string(account->ipmi));
          jv_account["LINKAUTH"] = json::value::string(to_string(account->link_auth));
          jv_account["PRIVILEGE"] = json::value::string(to_string(account->priv));

          // string index = to_string(improved_stoi(account->id) + 1);
          jv_account["INDEX"] = json::value::string(account->id);

          if(account->enabled)
            jv_account["ENABLE_STATUS"] = json::value::string("1");
          else
            jv_account["ENABLE_STATUS"] = json::value::string("0");

          jv_account["NAME"] = json::value::string(account->user_name);
          jv_account["PASSWORD"] = json::value::string(account->password);
          
          

          jv_infoArray[i] = jv_account;
      }

      jv_user["INFO"] = jv_infoArray;
      response_json["USER"] = jv_user;

  }



int rest_send_to_kvm() {
	// signal(SIGINT, Ipmiweb_GET::signalHandler);

    //pid_t pid = getpid();
	
	int msqid_req, msqid_rsp;

	if ( -1 == ( msqid_req = msgget( (key_t)1111, IPC_CREAT | 0666)))  // 요청 큐 생성
    {
       perror( "msgget() in req 실패");
       exit( 1);
    }
	
	if ( -1 == ( msqid_rsp = msgget( (key_t)2222, IPC_CREAT | 0666)))  // 요청 큐 생성
    {
       perror( "msgget() in req 실패");
       exit( 1);
    }
	
    kvm_msq_t msq_req, msq_rsp;

    msq_req.type = 1;
    msq_req.ccode = 0; 

	if ( -1 == msgsnd(msqid_req, &msq_req, sizeof(kvm_msq_t)-sizeof(long), 0))
    {
        perror( "msgsnd() in req실패");
        exit( 1);
    }
	
	   puts("Sent request. Waiting for response...");
	if ( -1 == msgrcv( msqid_rsp, &msq_rsp, sizeof(kvm_msq_t)-sizeof(long), 0, 0))
    {
		perror("msgrcv in req failed");
        exit(1); // while문 돌면서 request msg가 오기를 기다림
    }	

	   printf("ccode : %d\n", msq_rsp.ccode);  // python에서 popen으로 응답 json을 읽으려면 출력 부분 필요

    if (msq_rsp.ccode == 0)    
    	return 0;
    else {
    	printf("err: ccode is not 0 : %d\n", msq_rsp.ccode);
    }
}


void Ipmiweb_GET::Get_Watt_Info(int menu, json::value &response_json){
    // char cli_bug[100] = {0};
    // mg_sock_to_str(nc->sock, cli_bug, sizeof(cli_bug),
    //                MG_SOCK_STRINGIFY_REMOTE | MG_SOCK_STRINGIFY_IP |
    //                    MG_SOCK_STRINGIFY_PORT);
    // if (q_len != 0) {
      // index = data[q_len - 1];
      // uint8_t res_msg[QSIZE];
      char response[30000] = {0,};
      // try
      // {
      // 	cout<<"handle_watt_call:STEP0 data:"<<(char *)data<<endl;
      // 	//refine_data(data);
      // }
      // catch (const std::exception&)
      // {
      // 	cout<<"handle_watt_call:refine_data error "<<endl;
      // }
      // cout << "handle_watt_call:STEP0 data:" << (char *)data << endl;
      // cout << "handle_watt_call:STEP1 data:" << (char *)data << endl;
      // int res_len = 0;
      // memset((void *)res_msg, 0, QSIZE);
      // memset((void *)response, 0, QSIZE);
      
      // [수정]잠시 빼놓음
      // get_min_power_usage(menu);

      // cout << "handle_watt_call:STEP3 res_msg:" << (char *)res_msg << endl;
      // cout << "handle_watt_call:STEP4 response:" << (char *)response << endl;

      // [수정]잠시 빼놓음
      get_power_response(menu, response_json);

      // get_power_response(menu,response);
      // log(info)<<"debug get_power_response=>" <<response;
      // response_json = json::value::parse(response);

      // [테스트]임시 샘플 값
      switch (menu)
      {
          case WATT_ALL: {
              response_json["POWER_USAGE"]["HIGHEST_PEAK"]["VALUE"] = json::value::string("50000");
              response_json["POWER_USAGE"]["HIGHEST_PEAK"]["DATETIME"] = json::value::string("2022-07-05 13:00:00");

              response_json["POWER_USAGE"]["LOWEST_PEAK"]["VALUE"] = json::value::string("10000");
              response_json["POWER_USAGE"]["LOWEST_PEAK"]["DATETIME"] = json::value::string("2022-07-05 03:00:00");

              response_json["POWER_USAGE"]["HOUR_TOTAL_PEAK"] = json::value::string("55000");
              response_json["POWER_USAGE"]["MIN_TOTAL_PEAK"] = json::value::string("15000");

              response_json["POWER_USAGE"]["PAST_MIN"]["HIGHEST"]["VALUE"] = json::value::string("40000");
              response_json["POWER_USAGE"]["PAST_MIN"]["HIGHEST"]["DATETIME"] = json::value::string("2022-07-05 12:05:00");
              response_json["POWER_USAGE"]["PAST_MIN"]["LOWEST"]["VALUE"] = json::value::string("12000");
              response_json["POWER_USAGE"]["PAST_MIN"]["LOWEST"]["DATETIME"] = json::value::string("2022-07-05 02:05:00");

              response_json["POWER_USAGE"]["PAST_HOUR"]["HIGHEST"]["VALUE"] = json::value::string("35000");
              response_json["POWER_USAGE"]["PAST_HOUR"]["HIGHEST"]["DATETIME"] = json::value::string("2022-07-05 11:00:00");
              response_json["POWER_USAGE"]["PAST_HOUR"]["LOWEST"]["VALUE"] = json::value::string("18000");
              response_json["POWER_USAGE"]["PAST_HOUR"]["LOWEST"]["DATETIME"] = json::value::string("2022-07-05 01:00:00");

              response_json["POWER_USAGE"]["PAST_DAY"]["HIGHEST"]["VALUE"] = json::value::string("70000");
              response_json["POWER_USAGE"]["PAST_DAY"]["HIGHEST"]["DATETIME"] = json::value::string("2022-07-04 14:00:00");
              response_json["POWER_USAGE"]["PAST_DAY"]["LOWEST"]["VALUE"] = json::value::string("9000");
              response_json["POWER_USAGE"]["PAST_DAY"]["LOWEST"]["DATETIME"] = json::value::string("2022-07-04 02:00:00");

              for(int i=0; i<10; i++)
              {
                  response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][i]["ID"] = json::value::string(to_string(i+1));
                  // response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][i]["WATT"] = json::value::string(to_string(i*300));
                  response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][i]["ID"] = json::value::string(to_string(i+1));
                  // response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][i]["WATT"] = json::value::string(to_string(i*500));
              }

              // WATT
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][0]["WATT"] = json::value::string(to_string(300));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][1]["WATT"] = json::value::string(to_string(400));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][2]["WATT"] = json::value::string(to_string(800));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][3]["WATT"] = json::value::string(to_string(1000));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][4]["WATT"] = json::value::string(to_string(1500));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][5]["WATT"] = json::value::string(to_string(2000));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][6]["WATT"] = json::value::string(to_string(2300));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][7]["WATT"] = json::value::string(to_string(1500));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][8]["WATT"] = json::value::string(to_string(700));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][9]["WATT"] = json::value::string(to_string(3000));

              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][0]["WATT"] = json::value::string(to_string(0));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][1]["WATT"] = json::value::string(to_string(400));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][2]["WATT"] = json::value::string(to_string(800));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][3]["WATT"] = json::value::string(to_string(1500));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][4]["WATT"] = json::value::string(to_string(4000));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][5]["WATT"] = json::value::string(to_string(300));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][6]["WATT"] = json::value::string(to_string(300));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][7]["WATT"] = json::value::string(to_string(300));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][8]["WATT"] = json::value::string(to_string(300));
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][9]["WATT"] = json::value::string(to_string(2000));

              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][1]["DATETIME"] = json::value::string("2022-07-05 13:00:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][2]["DATETIME"] = json::value::string("2022-07-05 13:01:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][3]["DATETIME"] = json::value::string("2022-07-05 13:01:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][4]["DATETIME"] = json::value::string("2022-07-05 13:02:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][5]["DATETIME"] = json::value::string("2022-07-05 13:02:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][6]["DATETIME"] = json::value::string("2022-07-05 13:03:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][7]["DATETIME"] = json::value::string("2022-07-05 13:03:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][8]["DATETIME"] = json::value::string("2022-07-05 13:04:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_1"][9]["DATETIME"] = json::value::string("2022-07-05 13:04:30");

              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][1]["DATETIME"] = json::value::string("2022-07-05 13:00:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][2]["DATETIME"] = json::value::string("2022-07-05 13:01:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][3]["DATETIME"] = json::value::string("2022-07-05 13:01:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][4]["DATETIME"] = json::value::string("2022-07-05 13:02:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][5]["DATETIME"] = json::value::string("2022-07-05 13:02:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][6]["DATETIME"] = json::value::string("2022-07-05 13:03:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][7]["DATETIME"] = json::value::string("2022-07-05 13:03:30");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][8]["DATETIME"] = json::value::string("2022-07-05 13:04:00");
              response_json["POWER_USAGE"]["LAST_MIN_GRAPH_2"][9]["DATETIME"] = json::value::string("2022-07-05 13:04:30");

              for(int i=0; i<5; i++)
              {
                  response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_1"][i]["ID"] = json::value::string(to_string(i+1));
                  response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_1"][i]["WATT"] = json::value::string(to_string((5-i)*400));
                  response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_2"][i]["ID"] = json::value::string(to_string(i+1));
                  response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_2"][i]["WATT"] = json::value::string(to_string(i*600));
              }

              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_1"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_1"][1]["DATETIME"] = json::value::string("2022-07-05 14:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_1"][2]["DATETIME"] = json::value::string("2022-07-05 15:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_1"][3]["DATETIME"] = json::value::string("2022-07-05 16:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_1"][4]["DATETIME"] = json::value::string("2022-07-05 17:00:00");

              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_2"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_2"][1]["DATETIME"] = json::value::string("2022-07-05 14:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_2"][2]["DATETIME"] = json::value::string("2022-07-05 15:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_2"][3]["DATETIME"] = json::value::string("2022-07-05 16:00:00");
              response_json["POWER_USAGE"]["LAST_HOUR_GRAPH_2"][4]["DATETIME"] = json::value::string("2022-07-05 17:00:00");

              break;
          }
          case WATT_BOTTOM_HOUR: {
              for(int i=0; i<5; i++)
              {
                  response_json["LAST_HOUR_GRAPH_1"][i]["ID"] = json::value::string(to_string(i+1));
                  response_json["LAST_HOUR_GRAPH_1"][i]["WATT"] = json::value::string(to_string((5-i)*400));
                  response_json["LAST_HOUR_GRAPH_2"][i]["ID"] = json::value::string(to_string(i+1));
                  response_json["LAST_HOUR_GRAPH_2"][i]["WATT"] = json::value::string(to_string(i*600));
              }

              response_json["LAST_HOUR_GRAPH_1"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["LAST_HOUR_GRAPH_1"][1]["DATETIME"] = json::value::string("2022-07-05 14:00:00");
              response_json["LAST_HOUR_GRAPH_1"][2]["DATETIME"] = json::value::string("2022-07-05 15:00:00");
              response_json["LAST_HOUR_GRAPH_1"][3]["DATETIME"] = json::value::string("2022-07-05 16:00:00");
              response_json["LAST_HOUR_GRAPH_1"][4]["DATETIME"] = json::value::string("2022-07-05 17:00:00");

              response_json["LAST_HOUR_GRAPH_2"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["LAST_HOUR_GRAPH_2"][1]["DATETIME"] = json::value::string("2022-07-05 14:00:00");
              response_json["LAST_HOUR_GRAPH_2"][2]["DATETIME"] = json::value::string("2022-07-05 15:00:00");
              response_json["LAST_HOUR_GRAPH_2"][3]["DATETIME"] = json::value::string("2022-07-05 16:00:00");
              response_json["LAST_HOUR_GRAPH_2"][4]["DATETIME"] = json::value::string("2022-07-05 17:00:00");

              break;
          }
          case WATT_BOTTOM_MIN: {
              for(int i=0; i<10; i++)
              {
                  response_json["LAST_MIN_GRAPH_1"][i]["ID"] = json::value::string(to_string(i+1)); 
                  // response_json["LAST_MIN_GRAPH_1"][i]["WATT"] = json::value::string(to_string(i*300));
                  response_json["LAST_MIN_GRAPH_2"][i]["ID"] = json::value::string(to_string(i+1));
                  // response_json["LAST_MIN_GRAPH_2"][i]["WATT"] = json::value::string(to_string(i*500));
              }


              response_json["LAST_MIN_GRAPH_1"][0]["WATT"] = json::value::string(to_string(300));
              response_json["LAST_MIN_GRAPH_1"][1]["WATT"] = json::value::string(to_string(400));
              response_json["LAST_MIN_GRAPH_1"][2]["WATT"] = json::value::string(to_string(800));
              response_json["LAST_MIN_GRAPH_1"][3]["WATT"] = json::value::string(to_string(1000));
              response_json["LAST_MIN_GRAPH_1"][4]["WATT"] = json::value::string(to_string(1500));
              response_json["LAST_MIN_GRAPH_1"][5]["WATT"] = json::value::string(to_string(2000));
              response_json["LAST_MIN_GRAPH_1"][6]["WATT"] = json::value::string(to_string(2300));
              response_json["LAST_MIN_GRAPH_1"][7]["WATT"] = json::value::string(to_string(1500));
              response_json["LAST_MIN_GRAPH_1"][8]["WATT"] = json::value::string(to_string(700));
              response_json["LAST_MIN_GRAPH_1"][9]["WATT"] = json::value::string(to_string(3000));

              response_json["LAST_MIN_GRAPH_2"][0]["WATT"] = json::value::string(to_string(0));
              response_json["LAST_MIN_GRAPH_2"][1]["WATT"] = json::value::string(to_string(400));
              response_json["LAST_MIN_GRAPH_2"][2]["WATT"] = json::value::string(to_string(800));
              response_json["LAST_MIN_GRAPH_2"][3]["WATT"] = json::value::string(to_string(1500));
              response_json["LAST_MIN_GRAPH_2"][4]["WATT"] = json::value::string(to_string(4000));
              response_json["LAST_MIN_GRAPH_2"][5]["WATT"] = json::value::string(to_string(300));
              response_json["LAST_MIN_GRAPH_2"][6]["WATT"] = json::value::string(to_string(300));
              response_json["LAST_MIN_GRAPH_2"][7]["WATT"] = json::value::string(to_string(300));
              response_json["LAST_MIN_GRAPH_2"][8]["WATT"] = json::value::string(to_string(300));
              response_json["LAST_MIN_GRAPH_2"][9]["WATT"] = json::value::string(to_string(2000));

              response_json["LAST_MIN_GRAPH_1"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["LAST_MIN_GRAPH_1"][1]["DATETIME"] = json::value::string("2022-07-05 13:00:30");
              response_json["LAST_MIN_GRAPH_1"][2]["DATETIME"] = json::value::string("2022-07-05 13:01:00");
              response_json["LAST_MIN_GRAPH_1"][3]["DATETIME"] = json::value::string("2022-07-05 13:01:30");
              response_json["LAST_MIN_GRAPH_1"][4]["DATETIME"] = json::value::string("2022-07-05 13:02:00");
              response_json["LAST_MIN_GRAPH_1"][5]["DATETIME"] = json::value::string("2022-07-05 13:02:30");
              response_json["LAST_MIN_GRAPH_1"][6]["DATETIME"] = json::value::string("2022-07-05 13:03:00");
              response_json["LAST_MIN_GRAPH_1"][7]["DATETIME"] = json::value::string("2022-07-05 13:03:30");
              response_json["LAST_MIN_GRAPH_1"][8]["DATETIME"] = json::value::string("2022-07-05 13:04:00");
              response_json["LAST_MIN_GRAPH_1"][9]["DATETIME"] = json::value::string("2022-07-05 13:04:30");

              response_json["LAST_MIN_GRAPH_2"][0]["DATETIME"] = json::value::string("2022-07-05 13:00:00");
              response_json["LAST_MIN_GRAPH_2"][1]["DATETIME"] = json::value::string("2022-07-05 13:00:30");
              response_json["LAST_MIN_GRAPH_2"][2]["DATETIME"] = json::value::string("2022-07-05 13:01:00");
              response_json["LAST_MIN_GRAPH_2"][3]["DATETIME"] = json::value::string("2022-07-05 13:01:30");
              response_json["LAST_MIN_GRAPH_2"][4]["DATETIME"] = json::value::string("2022-07-05 13:02:00");
              response_json["LAST_MIN_GRAPH_2"][5]["DATETIME"] = json::value::string("2022-07-05 13:02:30");
              response_json["LAST_MIN_GRAPH_2"][6]["DATETIME"] = json::value::string("2022-07-05 13:03:00");
              response_json["LAST_MIN_GRAPH_2"][7]["DATETIME"] = json::value::string("2022-07-05 13:03:30");
              response_json["LAST_MIN_GRAPH_2"][8]["DATETIME"] = json::value::string("2022-07-05 13:04:00");
              response_json["LAST_MIN_GRAPH_2"][9]["DATETIME"] = json::value::string("2022-07-05 13:04:30");
              

              break;
          }
          case WATT_BOTTOM_DAY: {

                  response_json["LAST_DAY_GRAPH_1"][0]["ID"] = json::value::string(to_string(1));
                  response_json["LAST_DAY_GRAPH_1"][0]["WATT"] = json::value::string(to_string(8000));
                  response_json["LAST_DAY_GRAPH_1"][0]["DATETIME"] = json::value::string("2022-07-03 00:00:00");
                  response_json["LAST_DAY_GRAPH_1"][1]["ID"] = json::value::string(to_string(2));
                  response_json["LAST_DAY_GRAPH_1"][1]["WATT"] = json::value::string(to_string(3000));
                  response_json["LAST_DAY_GRAPH_1"][1]["DATETIME"] = json::value::string("2022-07-04 00:00:00");
                  response_json["LAST_DAY_GRAPH_1"][2]["ID"] = json::value::string(to_string(3));
                  response_json["LAST_DAY_GRAPH_1"][2]["WATT"] = json::value::string(to_string(5500));
                  response_json["LAST_DAY_GRAPH_1"][2]["DATETIME"] = json::value::string("2022-07-05 00:00:00");


                  response_json["LAST_DAY_GRAPH_2"][0]["ID"] = json::value::string(to_string(1));
                  response_json["LAST_DAY_GRAPH_2"][0]["WATT"] = json::value::string(to_string(2000));
                  response_json["LAST_DAY_GRAPH_2"][0]["DATETIME"] = json::value::string("2022-07-04 00:00:00");
                  response_json["LAST_DAY_GRAPH_2"][1]["ID"] = json::value::string(to_string(2));
                  response_json["LAST_DAY_GRAPH_2"][1]["WATT"] = json::value::string(to_string(7000));
                  response_json["LAST_DAY_GRAPH_2"][1]["DATETIME"] = json::value::string("2022-07-05 00:00:00");
                  response_json["LAST_DAY_GRAPH_2"][2]["ID"] = json::value::string(to_string(3));
                  response_json["LAST_DAY_GRAPH_2"][2]["WATT"] = json::value::string(to_string(5500));
                  response_json["LAST_DAY_GRAPH_2"][2]["DATETIME"] = json::value::string("2022-07-06 00:00:00");

                  break;
          }
      }
      log(info)<<"parser="<<response_json;
   
    // }
}