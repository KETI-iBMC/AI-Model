/**
 * @file sdr.cpp
 * @brief Sensor Data Record(SDR)
 */
#include <ipmi/lightning_sensor.hpp>
#include <ipmi/sdr.hpp>
#include <redfish/resource.hpp>
#include <redfish/eventservice.hpp>
#include <sys/time.h>

#include <util/dbus_system.hpp>

std::time_t erase_ts = std::time(nullptr);
std::time_t add_ts = std::time(nullptr);
extern int g_Tmax;
static int g_rsv_id = 0x1;
std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;
host_status_t host_state[SDR_RECORDS_MAX];

sdr_hdr_t g_sdr_hdr;
sdr_rec_t g_sdr_data[SDR_RECORDS_MAX];

pef_table_entry_t pef_filter_table[PEF_FILTER_ENTRY_COUNT] = {
	{
		PEF_FILTER_DISABLED,	// data1	
		PEF_CONFIG_PRECONFIGURED, //uint8_t config;
		PEF_ACTION_POWER_DOWN,	//	uint8_t action;
		0x0,	// uint8_t policy_number;
		PEF_SEVERITY_CRITICAL,	// uint8_t severity;
		0x20,						// uint8_t generator_ID_addr;
		0x00,						// uint8_t generator_ID_lun;
		0x01, 		// SENSOR_TYPE_TEMPERATURE,	// uint8_t sensor_type;
		PEB_SENSOR_NUM_BASE,		// PECI_SENSOR_NUM_BASE -> EVENT_SENSOR_NUM_BASE uint8_t sensor_number;
		PEF_EVENT_TRIGGER_THRESHOLD,	// uint8_t event_trigger;
		{0x00, 0x00},	// uint8_t event_data_1_offset_mask[2];
		0x00,			// uint8_t event_data_1_AND_mask;
		0x00,			// uint8_t event_data_1_compare_1;
		0x00,			// uint8_t event_data_1_compare_2;
		0x00,			// uint8_t event_data_2_AND_mask;
		0x00,			// uint8_t event_data_2_compare_1;
		0x00,			// uint8_t event_data_2_compare_2;
		0x00,			// uint8_t event_data_3_AND_mask;
		0x00,			// uint8_t event_data_3_compare_1;
		0x00			// uint8_t event_data_3_compare_2;
	},
	{
		PEF_FILTER_DISABLED,	// data1	
		PEF_CONFIG_PRECONFIGURED, //uint8_t config;
		PEF_ACTION_POWER_DOWN,	//	uint8_t action;
		0x0,	// uint8_t policy_number;
		PEF_SEVERITY_CRITICAL,	// uint8_t severity;
		0x20,						// uint8_t generator_ID_addr;
		0x00,						// uint8_t generator_ID_lun;
		0x01,       // SENSOR_TYPE_TEMPERATURE,	// uint8_t sensor_type;
		PEB_SENSOR_NUM_BASE + 1,		// PECI_SENSOR_NUM_BASE -> EVENT_SENSOR_NUM_BASE  uint8_t sensor_number;
		PEF_EVENT_TRIGGER_THRESHOLD,	// uint8_t event_trigger;
		{0x00, 0x00},	// uint8_t event_data_1_offset_mask[2];
		0x00,			// uint8_t event_data_1_AND_mask;
		0x00,			// uint8_t event_data_1_compare_1;
		0x00,			// uint8_t event_data_1_compare_2;
		0x00,			// uint8_t event_data_2_AND_mask;
		0x00,			// uint8_t event_data_2_compare_1;
		0x00,			// uint8_t event_data_2_compare_2;
		0x00,			// uint8_t event_data_3_AND_mask;
		0x00,			// uint8_t event_data_3_compare_1;
		0x00			// uint8_t event_data_3_compare_2;
	}
};


int generate_threshold_sensor_event(sensor_thresh_t* p_sdr, unsigned char event_dir_type, unsigned char event_data1,
							unsigned char event_reading) 
{
	sel_msg_t sel_entry;
	int rec_id, ret, i; // Just used to call plat_sel_add_entry()
#define SMTP_TITLE_LEN      1024
#define SMTP_BODY_LEN       1024
	char  smtp_title[SMTP_TITLE_LEN], smtp_body[SMTP_BODY_LEN];

	sel_entry.msg[2]  = 0x02; // System event record type 
	sel_entry.msg[7]  = 0x41; // System management software id
	sel_entry.msg[8]  = 0x00; // Channel number. System interface, IPMB or BMC
	sel_entry.msg[9]  = 0x04; // Evm Rev for IPMI v2.0
	sel_entry.msg[10] = p_sdr->sensor_type;
	sel_entry.msg[11] = p_sdr->sensor_num;
	sel_entry.msg[12] = event_dir_type;
	sel_entry.msg[13] = event_data1;
	sel_entry.msg[14] = event_reading; // This is thread unsafe. So use the prameter used by event check code
	sel_entry.msg[15] = 0xFF; // No data. Unspecified

	ret = plat_sel_add_entry (&sel_entry, &rec_id);

	memset(smtp_title, 0, SMTP_TITLE_LEN);
	memset(smtp_body,  0, SMTP_BODY_LEN);
  json::value jv;
  jv["EVENT OCCUR"]=json::value::string(smtp_body);
  send_smtp(jv);
	sprintf(smtp_title, "[EVENT OCCUR] Sensor(%s) Threshold: type[%02x]-num[%02x]-event[%02x]",
		p_sdr->str, p_sdr->sensor_type, p_sdr->sensor_num, event_data1);
	sprintf(smtp_body, "[EVENT OCCUR] Sensor(%s) Threshold: type[%02x]-num[%02x]-event[%02x]",
		p_sdr->str, p_sdr->sensor_type, p_sdr->sensor_num, event_data1);
//	smtp_client(smtp_title, smtp_body);
  jv["EVENT OCCUR"]=json::value::string(smtp_body);
  send_smtp(jv);
	if(event_data1 == 9) {
		for(i=0; i<PEF_FILTER_ENTRY_COUNT; i++) {
			if(pef_filter_table[i].config & PEF_FILTER_ENABLED) {
				if((p_sdr->sensor_num == pef_filter_table[i].sensor_number)
					&& (pef_filter_table[i].action == PEF_ACTION_POWER_DOWN))
				{

				}
			}
		}
	}

	return ret; // return with SUCCESS
}


static void sensor_time_function(uint8_t t_year, uint8_t t_mon, uint8_t t_days,
                                 uint8_t t_hour, uint8_t t_min, uint8_t t_sec,
                                 char *output) {
  char month[3], days[3], hour[3], min[3], sec[3] = {0};
  if ((t_mon + 1) < 10)
    sprintf(month, "0%d", t_mon + 1);
  else
    sprintf(month, "%d", t_mon + 1);

  if (t_days < 10)
    sprintf(days, "0%d", t_days);
  else
    sprintf(days, "%d", t_days);

  if (t_hour < 10)
    sprintf(hour, "0%d", t_hour);
  else
    sprintf(hour, "%d", t_hour);

  if (t_min < 10)
    sprintf(min, "0%d", t_min);
  else
    sprintf(min, "%d", t_min);

  if (t_sec < 10)
    sprintf(sec, "0%d", t_sec);
  else
    sprintf(sec, "%d", t_sec);

  sprintf(output, "%d-%s-%s %s:%s:%s", t_year + 1900, month, days, hour, min,
          sec);
}

sensor_thresh_t *Ipmisdr::sdr_get_entry() { return &this->rec; }

std::vector<uint8_t> Ipmisdr::sdr_get_rec_to_vector() {
  std::vector<uint8_t> v;

  v.push_back(this->rec_id[0]);
  v.push_back(this->rec_id[1]);
  v.push_back(this->ver);
  v.push_back(this->type);
  v.push_back(this->len);
  // v.push_back(this->rec.owner);
  // v.push_back(this->rec.lun);
  // v.push_back(this->rec.sensor_num);
  // v.push_back(this->rec.ent_id);
  // v.push_back(this->rec.ent_inst);
  // v.push_back(this->rec.sensor_init);
  // v.push_back(this->rec.sensor_caps);
  // v.push_back(this->rec.sensor_type);
  // v.push_back(this->rec.evt_read_type);
  // v.push_back(this->rec.lt_read_mask[0]);
  // v.push_back(this->rec.lt_read_mask[1]);
  // v.push_back(this->rec.ut_read_mask[0]);
  // v.push_back(this->rec.ut_read_mask[1]);
  // v.push_back(this->rec.set_thresh_mask[0]);
  // v.push_back(this->rec.set_thresh_mask[1]);
  // v.push_back(this->rec.sensor_units1);
  // v.push_back(this->rec.sensor_units2);
  // v.push_back(this->rec.sensor_units3);
  // v.push_back(this->rec.linear);
  // v.push_back(this->rec.m_val);
  // v.push_back(this->rec.m_tolerance);
  // v.push_back(this->rec.b_val);
  // v.push_back(this->rec.b_accuracy);
  // v.push_back(this->rec.accuracy_dir);
  // v.push_back(this->rec.rb_exp);
  // v.push_back(this->rec.analog_flags);
  // v.push_back(this->rec.nominal);
  // cout <<"this->rec.nominal =="<<(int)this->rec.nominal<<endl;
  // v.push_back(this->rec.normal_max);
  // v.push_back(this->rec.normal_min);
  // v.push_back(this->rec.max_reading);
  // v.push_back(this->rec.min_reading);
  // v.push_back(this->rec.unr_thresh);
  // v.push_back(this->rec.uc_thresh);
  // v.push_back(this->rec.unc_thresh);
  // v.push_back(this->rec.lnr_thresh);
  // v.push_back(this->rec.lc_thresh);
  // v.push_back(this->rec.lnc_thresh);
  // v.push_back(this->rec.pos_hyst);
  // v.push_back(this->rec.neg_hyst);
  // v.push_back(this->rec.oem);
  // v.push_back(this->rec.str_type_len);
  // v.insert(v.end(), this->rec.str,this->rec.str+SENSOR_STR_SIZE);
  // cout<<"sdr rec size ="<<v.size()<<endl;
  uint8_t *ptr = reinterpret_cast<uint8_t *>(&this->rec);
  // cout<<"copy sizeof(this->rec "<<sizeof(this->rec)<<endl;
  v.insert(v.end(), ptr, ptr + sizeof(this->rec));

  return v;
}

int Ipmisdr::sdr_thresh_write(uint8_t param, uint8_t *data) {
  switch (param) {
  case 0x20:
    this->rec.unr_thresh = data[7];
    cout << "sdr_thresh_write 7 =" << std::hex << (int)data[7] << endl;
    break;
  case 0x10:
    this->rec.uc_thresh = data[6];
    cout << "sdr_thresh_write 6 =" << std::hex << (int)data[6] << endl;
    break;
  case 0x08:
    this->rec.unc_thresh = data[5];
    cout << "sdr_thresh_write 5 =" << std::hex << (int)data[5] << endl;
    break;
  case 0x04:
    cout << "before sdr_thresh_write 4 =" << std::hex
         << (int)this->rec.lnr_thresh << endl;
    this->rec.lnr_thresh = data[4];
    cout << "sdr_thresh_write 4 =" << std::hex << (int)data[4] << endl;
    break;
  case 0x02:
    cout << "before sdr_thresh_write 3 =" << std::hex
         << (int)this->rec.lc_thresh << endl;
    this->rec.lc_thresh = data[3];
    cout << "sdr_thresh_write 3 =" << std::hex << (int)data[3] << endl;
    break;
  case 0x01:
    cout << "before sdr_thresh_write 2 =" << std::hex
         << (int)this->rec.lnc_thresh << endl;
    this->rec.lnc_thresh = data[2];
    cout << "sdr_thresh_write 2 =" << std::hex << (int)data[2] << endl;
    break;
  default:
    return -1;
    break;
  }
  return 0;
}

uint8_t Ipmisdr::sdr_get_sensornum() { return this->sensor_num; }

uint8_t Ipmisdr::sdr_get_thresh_flag() {
  uint8_t flag = 0;

  if (this->rec.unr_thresh != THRESH_NOT_AVAILABLE)
    flag |= SENSOREV_UPPER_NONRECOVER_THR_SETREAD_MASK;
  if (this->rec.uc_thresh != THRESH_NOT_AVAILABLE)
    flag |= SENSOREV_UPPER_CRITICAL_THR_SETREAD_MASK;
  if (this->rec.unc_thresh != THRESH_NOT_AVAILABLE)
    flag |= SENSOREV_UPPER_NONCRITICAL_THR_SETREAD_MASK;
  if (this->rec.lnr_thresh != THRESH_NOT_AVAILABLE)
    flag |= SENSOREV_LOWER_NONRECOVER_THR_SETREAD_MASK;
  if (this->rec.lc_thresh != THRESH_NOT_AVAILABLE)
    flag |= SENSOREV_LOWER_CRITICAL_THR_SETREAD_MASK;
  if (this->rec.lnc_thresh != THRESH_NOT_AVAILABLE)
    flag |= SENSOREV_LOWER_NONCRITICAL_THR_SETREAD_MASK;

  return flag;
}

uint8_t Ipmisdr::sdr_get_thresh_param(uint8_t param) {
  switch (param) {
  case UNR_THRESH:
    return this->rec.unr_thresh;
    break;
  case UCR_THRESH:
    return this->rec.uc_thresh;
    break;
  case UNC_THRESH:
    return this->rec.unc_thresh;
    break;
  case LNR_THRESH:
    return this->rec.lnr_thresh;
    break;
  case LCR_THRESH:
    return this->rec.lc_thresh;
    break;
  case LNC_THRESH:
    return this->rec.lnc_thresh;
    break;
  default:
    cout << "in sdr_get_thresh_param, param doesn't match anything" << endl;
    return 0;
  }
}

uint8_t Ipmisdr::sdr_sensor_read() { return this->rec.nominal; }

uint8_t Ipmisdr::sdr_get_analog_flag() { return this->rec.analog_flags; }
/**
 *@brief
 Lower Non-Recoverable (LNR)
 Upper Non-Recoverable (UNR)
 Lower Critical (LC)
 Upper Critical (UC)
 Lower Non-Critical (LNC)
 Upper Non-Critical (UNC)
 */
void Ipmisdr::print_sensor_info() {
  printf("\t ==== sensor_name : %s ====\n", this->rec.str);
  printf("\t - sensor num : %d\n", this->sensor_num);
  printf("\t - sensor reading : %f\n",
         sdr_convert_raw_to_sensor_value(&(this->rec), this->rec.nominal));
  printf("\t == thresh info == \n");
  printf("\t lnr : %f\n",
         sdr_convert_raw_to_sensor_value(&(this->rec), this->rec.lnr_thresh));
  printf("\t lc : %f\n",
         sdr_convert_raw_to_sensor_value(&(this->rec), this->rec.lc_thresh));
  printf("\t lnc : %f\n",
         sdr_convert_raw_to_sensor_value(&(this->rec), this->rec.lnc_thresh));
  printf("\t unr : %f\n",
         sdr_convert_raw_to_sensor_value(&(this->rec), this->rec.unr_thresh));
  printf("\t uc : %f\n",
         sdr_convert_raw_to_sensor_value(&(this->rec), this->rec.uc_thresh));
  printf("\t unc : %f\n",
         sdr_convert_raw_to_sensor_value(&(this->rec), this->rec.unc_thresh));
}

int plat_sdr_get_entry(int rsv_id, int read_rec_id, std::vector<uint8_t> *v_rec,
                       int *next_rec_id) {
  int index = 0;

  if (rsv_id != g_rsv_id)
    return -1;

  std::map<uint8_t, std::map<uint8_t, Ipmisdr>>::iterator iter;

  if (read_rec_id == SDR_RECID_FIRST)
    index = sdr_rec.begin()->first;
  else if (read_rec_id == SDR_RECID_LAST)
    index = (sdr_rec.end()--)->first;
  else
    index = read_rec_id - 1;
  if (plat_sdr_num_entries() == 0)
    return -1;
  if ((index < SDR_INDEX_MIN) || (index > SDR_INDEX_MAX))
    return -1;
  if (index < sdr_rec.begin()->first || index >= sdr_rec.end()--->first)
    return -1;
  for (auto ptr = sdr_rec.begin(); ptr != sdr_rec.end(); ptr++) {
    if (ptr->first == index) {
      sensor_thresh_t *test = ptr->second.find(index)->second.sdr_get_entry();
      // printf("ptr : %d / sensor name : %s / index : %d\n", ptr->first,
      // test->str, index);
      // cout<<"str="<<test->str<<endl;
      // cout<<"str="<<test->analog_flags<<endl;
      // test->analog_flags=0;
      // cout<<test->str<<"= nominal"<<(int)test->nominal<<endl;
      *v_rec = ptr->second.find(index)->second.sdr_get_rec_to_vector();
      break;
    }
  }
  if (read_rec_id == SDR_RECID_FIRST)
    *next_rec_id = ++read_rec_id + 1;
  else
    *next_rec_id = ++read_rec_id;
  if (*next_rec_id > (sdr_rec.end()--)->first)
    *next_rec_id = SDR_RECID_LAST;
  return 0;
}

int plat_sdr_num_entries(void) { return sdr_rec.size(); }

int plat_sdr_free_space(void) {
  int total_space;
  int used_space;

  total_space = SDR_RECORDS_MAX * 64; // Record size is 64 Byte
  used_space = plat_sdr_num_entries() * 64;

  return (total_space - used_space);
}

int plat_sdr_rsv_id() {
  if (g_rsv_id++ == SDR_RSVID_MAX) {
    g_rsv_id = SDR_RSVID_MIN;
  }

  return g_rsv_id;
}

uint8_t plat_find_sdr_index(uint8_t s_num) {
  for (uint8_t i = 0; i < plat_sdr_num_entries(); i++) {
    if (sdr_rec[i].find(i)->second.sdr_get_sensornum() == s_num) {
      return i;
      break;
    }
  }
  cout << "cannot find sdr_index in plat_find_sdr_index" <<(int)s_num<< endl;
  return 0;
}
uint8_t plat_find_sdr_name(string name) {
  for (uint8_t i = 0; i < plat_sdr_num_entries(); i++) {
    if (sdr_rec[i].find(i)->second.get_sensor_thresh_t().str == name) {
      log(info) << "sdr_rec["<<i<<"]"<<" = " << sdr_rec[i].find(i)->second.get_sensor_thresh_t().str;
      return i;
      break;
    }
  }
  log(info) << "cannot find sdr_index in plat_find_sdr_index";
  return 0;
}

void storage_clear_sdr_repository(ipmi_res_t *response, uint8_t *res_len) {
  uint8_t *data = &response->data[0];

  response->cc = CC_SUCCESS;
  *data++ = 0x01;

  sdr_rec.clear();

  *res_len = data - &response->data[0];
}

int check_event_occured(sensor_thresh_t* p_sdr, int sensor_id)
{
	// get current reading and thresholds
	unsigned char nominal = p_sdr->nominal;
	unsigned char uc_thresh = p_sdr->uc_thresh;
	unsigned char unc_thresh = p_sdr->unc_thresh;
	unsigned char lc_thresh = p_sdr->lc_thresh;
	unsigned char lnc_thresh = p_sdr->lnc_thresh;
	unsigned char neg_hyst = p_sdr->neg_hyst;
	unsigned char* event_mask = p_sdr->lt_read_mask; 
	// get calculation parameters
	int m = ABBM(BM(p_sdr->m_tolerance,p_sdr->m_val));
	int b = ABBM(BM(p_sdr->b_accuracy,p_sdr->b_val));
	int k1 = ABEXP(BEXP(p_sdr->rb_exp));
	int k2 = ABEXP(REXP(p_sdr->rb_exp));
	int ret = -1;

	// current reaidng and thresholds calculations
	double d_nominal, d_uc_thresh, d_unc_thresh, d_lc_thresh;
	double d_lnc_thresh, d_neg_hyst, d_neg_plus;
	switch(p_sdr->sensor_units1&0xC0)
	{
		case 0x80: // Analog data format == 2's complement
			d_nominal 		= (double) (((m * (int8_t)nominal) + (b * pow(10, k1))) * pow(10, k2));
			d_uc_thresh 	= (double) (((m * (int8_t)uc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_unc_thresh 	= (double) (((m * (int8_t)unc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_lc_thresh 	= (double) (((m * (int8_t)lc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_lnc_thresh 	= (double) (((m * (int8_t)lnc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_neg_hyst 		= (double) (((m * neg_hyst) /*+ (b * pow(10, k1))*/) * pow(10, k2));
			d_neg_plus 		= (double) (((m * 1) /*+ (b * pow(10, k1))*/) * pow(10, k2));
		break;
		case 0x00: // Analog data format == unsigned
		default:
			d_nominal 		= (double) (((m * nominal) + (b * pow(10, k1))) * pow(10, k2));
			d_uc_thresh 	= (double) (((m * uc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_unc_thresh 	= (double) (((m * unc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_lc_thresh 	= (double) (((m * lc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_lnc_thresh 	= (double) (((m * lnc_thresh) + (b * pow(10, k1))) * pow(10, k2));
			d_neg_hyst 		= (double) (((m * neg_hyst) /*+ (b * pow(10, k1))*/) * pow(10, k2));
			d_neg_plus 		= (double) (((m * 1) /*+ (b * pow(10, k1))*/) * pow(10, k2));
		break;
	}

	if( p_sdr->uc_thresh != THRESH_NOT_AVAILABLE && IS_UCH_MASKED(event_mask) &&
		d_nominal >= (d_uc_thresh+d_neg_hyst+d_neg_plus) && d_nominal != 0){ // Upper critical going high
		if(host_state[sensor_id].state != 1) {
			ret = generate_threshold_sensor_event(p_sdr, 0x01, 0x09, nominal);
			//prepare_SMTP(p_sdr);
      
		}
		host_state[sensor_id].state = 1;
	} else if(p_sdr->unc_thresh != THRESH_NOT_AVAILABLE && IS_UNCH_MASKED(event_mask) &&
			d_nominal >= (d_unc_thresh+d_neg_hyst+d_neg_plus) && d_nominal != 0){ // Upper non-critical going high
		if(host_state[sensor_id].state != 2) {
			ret = generate_threshold_sensor_event(p_sdr, 0x01, 0x07, nominal);
			//prepare_SMTP(p_sdr);
			printf("Upper non-critical event occured\n");

		}
		host_state[sensor_id].state = 2;
	} else if(p_sdr->lc_thresh != THRESH_NOT_AVAILABLE && IS_LCL_MASKED(event_mask) &&
			d_nominal <= (d_lc_thresh) && d_nominal != 0){ // Lower critical going low
		if(host_state[sensor_id].state != 3) {
			ret = generate_threshold_sensor_event(p_sdr, 0x01, 0x02, nominal);
			//prepare_SMTP(p_sdr);

			printf("Lower critical event occured\n");
		}
		host_state[sensor_id].state = 3;
	} else if(p_sdr->lnc_thresh != THRESH_NOT_AVAILABLE && IS_LNCL_MASKED(event_mask) &&
			d_nominal <= (d_lnc_thresh) && d_nominal != 0){ // Lower non-critical going low
		if(host_state[sensor_id].state != 4) {
			ret = generate_threshold_sensor_event(p_sdr, 0x01, 0x00, nominal);

			printf("Lower non-critical event occured\n");

		}
		host_state[sensor_id].state = 4;
	} else{
	// Do nothing in general
		ret = 0; // SUCCESS
		host_state[sensor_id].state = 0;
	}
	return ret;
}

/**
 * @brief Sensor정보 업데이트
 * @details AST2600A3 KTNF 기반 포팅(ADC reading 값 문제 해결중 )
 * @author 기철
 *
 */
void update_sensor_reading() {
  int ret, rVal;
  int cpu0_flag = 0, cpu1_flag = 0;
  sensor_thresh_t *p_sdr, *t_sdr;
  DBus::BusDispatcher dispatcher;
  DBus::default_dispatcher = &dispatcher;
  DBus::Connection conn_n = DBus::Connection::SystemBus();
  DBus_Sensor sdr_dbus_sensor = DBus_Sensor(conn_n,SERVER_PATH_1,SERVER_NAME_1);
  for (auto iter = sdr_rec.begin(); iter != sdr_rec.end(); iter++) {
    p_sdr = iter->second.find(iter->first)->second.sdr_get_entry();
    rVal = sdr_dbus_sensor.lightning_sensor_read(p_sdr->oem, p_sdr->sensor_num);
    p_sdr->nominal = rVal;
    p_sdr->analog_flags = PSDR_ANALOG_DISABLE;
    cpu0_flag = 1;
    check_event_occured(p_sdr,p_sdr->sensor_num);
    redfish_seonsor_sync(p_sdr);
  }
  dispatcher.leave();
  time_t ltime = time(NULL);
  struct tm tm_local = *localtime(&ltime);
}
/**
 * @brief power reading 부분이 들어감 power status가 false인경우 sensor와 sdr
 * 값을 읽어올수 없음 data[0] reading 값 data[1] 192 data[2] 255 완료 코드
 *
 * @param request
 * @param response
 * @param res_len
 */
void sensor_get_reading(ipmi_req_t *request, ipmi_res_t *response,
                        uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  uint8_t param = request->data[0];
  std::map<uint8_t, Ipmisdr>::iterator ptr;
  int val = 0;
  DBus::BusDispatcher dispatcher;
  DBus::default_dispatcher = &dispatcher;
  DBus::Connection conn_n = DBus::Connection::SystemBus();
  DBus_Sensor sdr_dbus_sensor = DBus_Sensor(conn_n,SERVER_PATH_1,SERVER_NAME_1);
  val = sdr_dbus_sensor.lightning_sensor_read(FRU_PEB, PEB_SENSOR_ADC_P12V_PSU1);
    dispatcher.leave();

  if (val >= 150) {
    uint8_t idx = plat_find_sdr_index(param);

    response->cc = CC_SUCCESS;
    *data++ = sdr_rec[idx].find(idx)->second.sdr_sensor_read();
    if (sdr_rec[idx].find(idx)->second.sdr_get_analog_flag() !=
        PSDR_ANALOG_DISABLE)
      *data++ = 0xc0;
    else
      *data++ = 0x0;

    if (response->cc != CC_SUCCESS)
      response->cc = CC_UNSPECIFIED_ERROR;
  } else {
    response->cc = CC_SUCCESS;
    *data++ = 0x0;
    *data++ = 0x0;
  }

  *res_len = data - &response->data[0];
}

void sensor_get_threshod(ipmi_req_t *request, ipmi_res_t *response,
                         uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  uint8_t param = request->data[0];

  std::map<uint8_t, Ipmisdr>::iterator ptr;

  uint8_t idx = plat_find_sdr_index(param);

  response->cc = CC_SUCCESS;
  *data++ = sdr_rec[idx].find(idx)->second.sdr_get_thresh_flag();
  *data++ = sdr_rec[idx].find(idx)->second.sdr_get_thresh_param(LNC_THRESH);
  *data++ = sdr_rec[idx].find(idx)->second.sdr_get_thresh_param(LCR_THRESH);
  *data++ = sdr_rec[idx].find(idx)->second.sdr_get_thresh_param(LNR_THRESH);
  *data++ = sdr_rec[idx].find(idx)->second.sdr_get_thresh_param(UNC_THRESH);
  *data++ = sdr_rec[idx].find(idx)->second.sdr_get_thresh_param(UCR_THRESH);
  *data++ = sdr_rec[idx].find(idx)->second.sdr_get_thresh_param(UNR_THRESH);
  *res_len = data - &response->data[0];

  if (response->cc != CC_SUCCESS)
    response->cc = CC_UNSPECIFIED_ERROR;
}

void sensor_set_threshold(ipmi_req_t *request, ipmi_res_t *response,
                          uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  uint8_t param = request->data[1];
  uint8_t id = request->data[0];

  std::map<uint8_t, Ipmisdr>::iterator ptr;

  for (auto iter = sdr_rec.begin(); iter != sdr_rec.end(); iter++) {
    ptr = iter->second.begin();
    if (id == ptr->second.sdr_get_sensornum()) {
      response->cc = CC_SUCCESS;
      cout << "sensor_set_threshold sensor name = "
           << ptr->second.get_sensor_thresh_t().str << endl;
      ptr->second.sdr_thresh_write(param, request->data);

      char buf[500] = {
          0,
      };
      sprintf(buf, "SDR Set Sensor Threshold Sensor Number : %d ",
              ptr->second.sdr_get_sensornum());
      // ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf, "Sensor",
      // "System"));
      break;
    }
  }

  if (response->cc != CC_SUCCESS)
    response->cc = CC_UNSPECIFIED_ERROR;
}

void storage_get_sdr(ipmi_req_t *request, ipmi_res_t *response,
                     uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  std::vector<uint8_t> entry; // SDR record entry

  int rsv_id =
      (request->data[1] >> 8) | request->data[0]; // record ID to be read
  int read_rec_id = (request->data[3] >> 8) |
                    request->data[2]; // record ID for the next entry
  int rec_offset = request->data[4];  // Read offset into the record
  int rec_bytes = request->data[5];   // Number of bytes to be read
  int next_rec_id = 0;
  int ret = 0;
  // Use platform API to read the record Id and get next ID
  ret = plat_sdr_get_entry(rsv_id, read_rec_id, &entry, &next_rec_id);
  if (ret) {
    response->cc = CC_UNSPECIFIED_ERROR;
    return;
  }

  response->cc = CC_SUCCESS;
  *data++ = next_rec_id & 0xff;
  *data++ = (next_rec_id >> 8) & 0xff;

  std::copy(entry.begin() + rec_offset, entry.end(), data);
  data += rec_bytes;
  cout << ("Get SDR Entry----") << endl;
  printf("SDR Reservation ID : 0x%02x \n", rsv_id);
  printf("SDR Read ID : 0x%02x\n", read_rec_id);
  printf("SDR Record Offset : 0x%02x\n", rec_offset);
  printf("SDR Read Number of byte : 0x%02x\n", rec_bytes);

  *res_len = data - &response->data[0];
}

void storage_rsv_sdr(ipmi_res_t *response, uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  int rsv_id = 0;

  rsv_id = plat_sdr_rsv_id();
  if (rsv_id < 0) {
    response->cc = CC_UNSPECIFIED_ERROR;
    return;
  }

  response->cc = CC_SUCCESS;
  *data++ = rsv_id & 0xff;
  *data++ = (rsv_id >> 8) & 0xff;

  *res_len = data - &response->data[0];
}

void storage_get_sdr_info(ipmi_res_t *response, uint8_t *res_len) {
  uint8_t *data = &response->data[0];
  std::vector<uint8_t> v_ts;
  int num_entries = 0;
  int free_space = 0;

  num_entries = plat_sdr_num_entries();
  free_space = plat_sdr_free_space();

  response->cc = CC_SUCCESS;

  *data++ = IPMI_SDR_VERSION;
  *data++ = num_entries & 0xff;
  *data++ = (num_entries >> 8) & 0xff;
  *data++ = free_space & 0xff;
  *data++ = (free_space >> 8) & 0xff;

  for (int i = 0; i < SIZE_TIME_STAMP; i++)
    v_ts.push_back((add_ts >> (8 * i)) & 0xff);

  std::copy(v_ts.begin(), v_ts.end(), data);
  data += SIZE_TIME_STAMP;

  v_ts.clear();

  for (int i = 0; i < SIZE_TIME_STAMP; i++)
    v_ts.push_back((erase_ts >> (8 * i)) & 0xff);

  std::copy(v_ts.begin(), v_ts.end(), data);
  data += SIZE_TIME_STAMP;

  *data++ = 0x02;

  *res_len = data - &response->data[0];
}

/**
 * @brief KETI-WEB에 전송하는 API 리스트
 *
 * @param res
 * @return int
 */
int rest_get_sensor_config(char *res) {
  json::value obj = json::value::object();
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
  obj["SENSOR_INFO"] = sensor_info;
  strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  return strlen(res);
}

float sdr_convert_raw_to_sensor_value(sensor_thresh_t *sensor, uint8_t val) {
  int m, b, k1, k2;
  double result;
  // m = __TO_M(sensor->m_tolerance);
  // b = __TO_B(sensor->rb_exp);//__TO_B(sensor->bacc);
  m = sensor->m_val; 
  b = sensor->b_val;//__TO_B(sensor->bacc);
  k1 = __TO_B_EXP(sensor->rb_exp);
  k2 = __TO_R_EXP(sensor->rb_exp);
  /* don't divide by zero */
  if (m == 0)
    return 0;

  result = (((m * val) + (b * pow(10, k1))) * (pow(10, k2)));
  return result;
}
/**
 * @brief sensor data를 바꾸어주는 함수 sesnor threshold같이 값이 바뀌는함수를
 * 지정한다.
 *
 * @param sensor
 * @param val
 * @return uint8_t
 */
uint8_t sdr_convert_sensor_value_to_raw(sensor_thresh_t *sensor, double val) {
  int m, b, k1, k2;
  double result;

  m = sensor->m_val;
  b = sensor->b_val;
  k1 = __TO_B_EXP(sensor->rb_exp); // b_val
  k2 = __TO_R_EXP(sensor->rb_exp);
  /* don't divide by zero */
  if (m == 0)
    return 0;

  result = (((val / pow(10, k2)) - (b * pow(10, k1))) / m);
  if ((result - (int)result) >= .5)
    return (uint8_t)ceil(result);
  else
    return (uint8_t)result;
}

int plat_sdr_init(void) {
  int num, i = 0;
  sensor_thresh_t *p_thresh;

  FILE *fp;
  // Populate SDR Header
  if (access(SDR_LOG_FILE, F_OK) == 0) {
    if (access(SDR_HEADER_FILE, F_OK) == 0) {
      if (file_get_sdr_hdr()) {
        fprintf(stderr, "plat_init_sdr: file_get_sdr_hdr failed\n");
        return -1;
      }
    }
    if (file_get_sdr_data()) {
      fprintf(stderr, "plat_init_sdr: file_get_sdr_data failed\n");
      return -1;
    }
    return 0;
  } else {
    fp = fopen(SDR_LOG_FILE, "w+");
    if (fp == NULL) {
      fprintf(stderr, "plat_init_sdr: fopen failed\n");
      return -1;
    }
    fclose(fp);

    fp = fopen(SDR_HEADER_FILE, "w+");
    if (fp == NULL) {
      fprintf(stderr, "plat_init_sdr: fopen failed\n");
    }
    fclose(fp);

    g_sdr_hdr.magic = SDR_HDR_MAGIC;
    g_sdr_hdr.version = SDR_HDR_VERSION;
    g_sdr_hdr.begin = SDR_INDEX_MIN;
    g_sdr_hdr.end = SDR_INDEX_MIN;
    memset(g_sdr_hdr.ts_add.ts, 0x0, 4);
    memset(g_sdr_hdr.ts_erase.ts, 0x0, 4);

    sensor_thresh_t *p_sdr;
    sdr_rec_t sdr;
    for (auto iter = sdr_rec.begin(); iter != sdr_rec.end(); iter++) {
      printf("\t\t\tdy: sdr rec count++\n");
      p_sdr = iter->second.find(iter->first)->second.sdr_get_entry();
      memcpy(sdr.rec, p_sdr, SDR_LEN_MAX);
      int rec_id = iter->second.find(iter->first)->first;
      if (plat_sdr_add_entry(&sdr, &rec_id)) {
        fprintf(stderr, "plat_sdr_init : plat_sdr_add_entry failed\n");
        return -1;
      }
    }

    if (file_store_sdr_hdr()) {
      fprintf(stderr, "plat_sdr_add_entry: file_store_sdr_hdr failed\n");
      return -1;
    }
  }
  return 0;
}

static int plat_sdr_add_entry(sdr_rec_t *rec, int *rec_id) {
  if (plat_sdr_num_entries() == SDR_RECORDS_MAX) {
    fprintf(stderr, "plat_sdr_add_entry: SDR full\n");
    return -1;
  }

  rec->rec[0] = g_sdr_hdr.end + 1;

  memcpy(g_sdr_data[g_sdr_hdr.end].rec, rec->rec, sizeof(sdr_rec_t));

  *rec_id = g_sdr_hdr.end + 1;

  ++g_sdr_hdr.end;

  time_stamp_fill(g_sdr_hdr.ts_add.ts);
  if (file_store_sdr_data((*rec_id - 1), rec)) {
    fprintf(stderr, "plat_sdr_add_entry: file_store_sdr_data failed\n");
    return -1;
  }
  return 0;
}

int file_store_sdr_data(int recId, sdr_rec_t *data) {
  printf("save sdr data\n");
  FILE *fp;
  int index;

  fp = fopen(SDR_LOG_FILE, "r+");
  if (fp == NULL) {
    fprintf(stderr, "file_store_sdr_data: fopen\n");
    return -1;
  }

  index = recId * sizeof(sdr_rec_t);

  if (fseek(fp, index, SEEK_SET)) {
    fprintf(stderr, "file_store_sdr_data: fseek\n");
    fclose(fp);
    return -1;
  }

  if (fwrite(data->rec, sizeof(sdr_rec_t), 1, fp) <= 0) {
    fprintf(stderr, "file_store_sdr_data: fwrite\n");
    fclose(fp);
    return -1;
  }

  fclose(fp);

  char save[500] = {
      0,
  };
  sprintf(save, "cp %s /conf/ipmi/", SDR_LOG_FILE);
  system(save);

  return 0;
}

int file_store_sdr_hdr() {
  FILE *fp;

  fp = fopen(SDR_HEADER_FILE, "w+");
  if (fp == NULL) {
    fprintf(stderr, "file_store_sdr_hdr: fopen failed\n");
    return -1;
  }

  if (fwrite(&g_sdr_hdr, sizeof(sdr_hdr_t), 1, fp) <= 0) {
    fprintf(stderr, "file_store_sdr_hdr: fwrite failed\n");
    fclose(fp);
    return -1;
  }

  fclose(fp);

  char save[500] = {
      0,
  };
  sprintf(save, "cp %s /backup_conf/ipmi/", SDR_HEADER_FILE);
  system(save);

  return 0;
}

int file_get_sdr_hdr() {
  FILE *fp;

  fp = fopen(SDR_HEADER_FILE, "r");
  if (fp == NULL)
    return -1;

  if (fread(&g_sdr_hdr, sizeof(sdr_hdr_t), 1, fp) <= 0) {
    fprintf(stderr, "file_get_sdr_hdr: fread failed\n");
    fclose(fp);
    return -1;
  }
  fclose(fp);
  return 0;
}

int file_get_sdr_data() {
  FILE *fp;
  int count_read = 0, i, j;
  unsigned char buf[SDR_RECORDS_MAX * sizeof(sdr_rec_t)];
  unsigned char temp[sizeof(sdr_rec_t)];

  fp = fopen(SDR_LOG_FILE, "r");
  if (fp == NULL) {
    fprintf(stderr, "file_get_sdr_data: fopen failed\n");
    return -1;
  }

  for (i = 0; i < SDR_RECORDS_MAX; i++) {
    if ((count_read = fread(temp, 1, sizeof(sdr_rec_t), fp)) <= 0)
      break;
    else {
      for (j = 0; j < SDR_LEN_MAX; j++)
        g_sdr_data[i].rec[j] = temp[j];
    }
  }
  fclose(fp);

  return 0;
}

unsigned char sdr_sensor_read(int sensor_num) {
  u_int8_t idx = plat_find_sdr_index(sensor_num);
  int value = 0;

  sensor_thresh_t *p_sdr = sdr_rec[idx].find(idx)->second.sdr_get_entry();
  DBus::BusDispatcher dispatcher;
  DBus::default_dispatcher = &dispatcher;
  DBus::Connection conn_n = DBus::Connection::SystemBus();
  DBus_Sensor sdr_dbus_sensor = DBus_Sensor(conn_n,SERVER_PATH_1,SERVER_NAME_1);
  value = sdr_dbus_sensor.lightning_sensor_read(p_sdr->oem, p_sdr->sensor_num);
    dispatcher.leave();

  unsigned char data = (unsigned char)(value & 0x000000ff);
  return data;
}
/**
 * @brief ipmi 기준으로 된 sensor type 을 redfish 타입으로 변경 하기 위한 함수
 */
string sensor_tpye2string(uint8_t ipmisensor_type) {
  if (ipmisensor_type <= PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2)
    return "Temperature";
  else if (ipmisensor_type >= PEB_SENSOR_ADC_P12V_PSU1 &&
           ipmisensor_type <= PEB_SENSOR_ADC_PVTT)
    return "Voltage";
  else if (ipmisensor_type >= PDPB_SENSOR_TEMP_REAR_RIGHT &&
           ipmisensor_type <= PDPB_SENSOR_TEMP_FRONT_LEFT)
    return "Temperature";
  // else if (ipmisensor_type >= NVA_SENSOR_TEMP1 && ipmisensor_type <=
  // NVA_SENSOR_PSU2_TEMP)
  //     return "Temperature";
  // else if (ipmisensor_type >= NVA_SENSOR_PSU1_FAN1 && ipmisensor_type <=
  // NVA_SENSOR_PSU1_FAN2)
  //     return "AirFlow";
  // else if (ipmisensor_type >= NVA_SENSOR_PSU1_WATT && ipmisensor_type <=
  // NVA_SENSOR_PSU2_WATT)
  //     return "Power";
  // else if (ipmisensor_type >= NVA_SENSOR_PSU1_TEMP2 && ipmisensor_type <=
  // NVA_SENSOR_PSU2_TEMP2)
  //     return "Temperature";
  else if (ipmisensor_type >= NVA_SYSTEM_FAN1 &&
           ipmisensor_type <= NVA_SYSTEM_FAN5)
    return "AirFlow";
}
/**
 * @brief
 * @bug hex값
 * @param rec
 * @return true
 * @return false
 */
bool redfish_seonsor_sync(sensor_thresh_t *rec) {

  try {

    time_t ltime = time(NULL);
    struct tm tm_local = *localtime(&ltime);
    double reading, lnr, lc, lnc, unr, uc, unc;

    reading = sdr_convert_raw_to_sensor_value((rec), rec->nominal);
    lnr = sdr_convert_raw_to_sensor_value((rec), rec->lnr_thresh);
    lc = sdr_convert_raw_to_sensor_value((rec), rec->lc_thresh);
    lnc = sdr_convert_raw_to_sensor_value((rec), rec->lnc_thresh);
    unr = sdr_convert_raw_to_sensor_value((rec), rec->unr_thresh);
    uc = sdr_convert_raw_to_sensor_value((rec), rec->uc_thresh);
    unc = sdr_convert_raw_to_sensor_value((rec), rec->unc_thresh);

    SensorMake se;
    se.id = rec->str;
    se.reading = reading;
    se.thresh.lower_caution.reading = lnc;
    se.thresh.lower_critical.reading = lc;
    se.thresh.upper_critical.reading = uc;
    se.thresh.upper_caution.reading = unc;
    se.thresh.upper_fatal.reading = unr;
    se.thresh.lower_fatal.reading = lnr;
    uint16_t flag;
    flag |= 0x1; // reading_units
    // flag|=0x40; //sensing_interval
    flag |= 0x80;   // lower_caution
    flag |= 0x100;  // lower_critical
    flag |= 0x200;  // lower_fatal
    flag |= 0x400;  // upper_caution
    flag |= 0x800;  // upper_critical
    flag |= 0x1000; // upper_fatal
    char time_string[100] = {0};
    sensor_time_function(tm_local.tm_year, tm_local.tm_mon, tm_local.tm_mday,
                         tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec,
                         time_string);
    string temp = string(time_string);
    se.reading_time = temp;
    se.reading_type = sensor_tpye2string(rec->sensor_type);
    make_sensor(se, flag);
  } catch (const std::exception &) {
    return false;
  }
  return true;
}