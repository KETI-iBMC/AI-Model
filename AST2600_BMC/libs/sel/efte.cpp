#include "ipmi/common.hpp"
#include "ipmi/efte.hpp"
#include "ipmi/sensor_define.hpp"

extern pef_table_entry_t g_eft_data[EVENT_FILTER_TABLE_ELEMS_MAX];
extern char *Alert_Strings[4];
struct bit_desc_map severity_name = {{{"Unspecified", 0x00},
                                      {"Monitor", 0x01},
                                      {"Information", 0x02},
                                      {"Normal", 0x04},
                                      {"Warning", 0x08},
                                      {"Critical", 0x10},
                                      {"Non recoverable", 0x20},
                                      {NULL}}};

struct bit_desc_map voltage_sensor_name = {
    {{"PEB_ADC_P12V_PSU1", PEB_SENSOR_ADC_P12V_PSU1},
     {"PEB_ADC_P12V_PSU2", PEB_SENSOR_ADC_P12V_PSU2},
     {"PEB_ADC_P3V3", PEB_SENSOR_ADC_P3V3},
     {"PEB_ADC_P5V", PEB_SENSOR_ADC_P5V},
     {"PEB_ADC_PVNN_PCH", PEB_SENSOR_ADC_PVNN_PCH},
     {"PEB_ADC_P1V05", PEB_SENSOR_ADC_P1V05},
     {"PEB_ADC_P1V8", PEB_SENSOR_ADC_P1V8},
     {"PEB_ADC_BAT", PEB_SENSOR_ADC_BAT},
     {"PEB_ADC_PVCCIN", PEB_SENSOR_ADC_PVCCIN},
     {"PEB_ADC_PVNN_PCH_CPU0", PEB_SENSOR_ADC_PVNN_PCH_CPU0},
     {"PEB_ADC_P1V8_NACDELAY", PEB_SENSOR_ADC_P1V8_NACDELAY},
     {"PEB_ADC_P1V2_VDDQ", PEB_SENSOR_ADC_P1V2_VDDQ},
     {"PEB_ADC_PVNN_NAC", PEB_SENSOR_ADC_PVNN_NAC},
     {"PEB_P1V05_NAC", PEB_SENSOR_ADC_P1V05_NAC},
     {"PEB_ADC_PVPP", PEB_SENSOR_ADC_PVPP},
     {"PEB_ADC_PVTT", PEB_SENSOR_ADC_PVTT},
     {NULL}}};

struct bit_desc_map temperature_sensor_name = {
    {{"REAR_RIGHT_TEMP", PDPB_SENSOR_TEMP_REAR_RIGHT},
     {"CPU_AMBIENT_TEMP", PDPB_SENSOR_TEMP_CPU_AMBIENT},
     {"FRONT_RIGHT_TEMP", PDPB_SENSOR_TEMP_FRONT_RIGHT},
     {"PCIE_AMBIENT_TEMP", PDPB_SENSOR_TEMP_PCIE_AMBIENT},
     {"FRONT_LEFT_TEMP", PDPB_SENSOR_TEMP_FRONT_LEFT},
     {"CPU0_TEMP", PDPB_SENSOR_TEMP_CPU0},
     {"CPU1_TEMP", PDPB_SENSOR_TEMP_CPU1},
     {"LM75_TEMP0", PDPB_SENSOR0_TEMP_LM75},
     {"LM75_TEMP1", PDPB_SENSOR1_TEMP_LM75},
     {"LM75_TEMP2", PDPB_SENSOR2_TEMP_LM75},
     {"LM75_TEMP3", PDPB_SENSOR3_TEMP_LM75},
     {"LM75_TEMP4", PDPB_SENSOR4_TEMP_LM75},
     // {"NVA_TEMP1", NVA_SENSOR_TEMP1},
     // {"NVA_TEMP2", NVA_SENSOR_TEMP2},
     // {"NVA_TEMP3", NVA_SENSOR_TEMP3},
     // {"NVA_TEMP4", NVA_SENSOR_TEMP4},
     {"CPU0_CH0_DIMM0_T", PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0},
     {"CPU0_CH0_DIMM1_T", PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1},
     {"CPU0_CH0_DIMM2_T", PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2},
     {"CPU0_CH1_DIMM0_T", PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0},
     {"CPU0_CH1_DIMM1_T", PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1},
     {"CPU0_CH1_DIMM2_T", PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2},
     {"CPU0_CH2_DIMM0_T", PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0},
     {"CPU0_CH2_DIMM1_T", PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1},
     {"CPU0_CH2_DIMM2_T", PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2},
     {"CPU0_CH3_DIMM0_T", PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0},
     {"CPU0_CH3_DIMM1_T", PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1},
     {"CPU0_CH3_DIMM2_T", PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2},
     {"CPU1_CH0_DIMM0_T", PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0},
     {"CPU1_CH0_DIMM1_T", PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1},
     {"CPU1_CH0_DIMM2_T", PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2},
     {"CPU1_CH1_DIMM0_T", PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0},
     {"CPU1_CH1_DIMM1_T", PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1},
     {"CPU1_CH1_DIMM2_T", PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2},
     {"CPU1_CH2_DIMM0_T", PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0},
     {"CPU1_CH2_DIMM1_T", PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1},
     {"CPU1_CH2_DIMM2_T", PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2},
     {"CPU1_CH3_DIMM0_T", PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0},
     {"CPU1_CH3_DIMM1_T", PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1},
     {"CPU1_CH3_DIMM2_T", PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2},
     //{"NVA_PSU1_TEMP", NVA_SENSOR_PSU1_TEMP},
     //{"NVA_PSU2_TEMP", NVA_SENSOR_PSU2_TEMP},
     {NULL}}};

struct bit_desc_map current_sensor_name = {
    {//{"NVA_PSU1_WATT", NVA_SENSOR_PSU1_WATT},
     //{"NVA_PSU2_WATT", NVA_SENSOR_PSU2_WATT},
     {NULL}}};

struct bit_desc_map fan_sensor_name = {
    {//{"NVA_PSU1_FAN", NVA_SENSOR_PSU1_FAN1},
     //{"NVA_PSU2_FAN", NVA_SENSOR_PSU2_FAN1},
     {"SYSTEM_FAN1", NVA_SYSTEM_FAN1},
     {"SYSTEM_FAN2", NVA_SYSTEM_FAN2},
     {"SYSTEM_FAN3", NVA_SYSTEM_FAN3},
     {"SYSTEM_FAN4", NVA_SYSTEM_FAN4},
     {"SYSTEM_FAN5", NVA_SYSTEM_FAN5},
     {NULL}}};

int set_pef_enable(unsigned char recid, unsigned char enable) {
  uint8_t zero = 0;
  g_eft_data[recid].config = zero;
  if (enable == 0) { // disable
    g_eft_data[recid].config = PEF_FILTER_DISABLED;
  } else { // enable
    g_eft_data[recid].config = PEF_FILTER_ENABLED;
  }
  if (set_eft_entry(recid, &(g_eft_data[recid].config)) < 0) {
    return -1;
  }
  return 0;
}

int set_pef_cfg(unsigned char recid, void *entry) {
  g_eft_data[recid].config = *(unsigned char *)entry;

  if (set_eft_entry(recid, &g_eft_data[recid]) < 0) {
    return -1;
  }
  return 0;
}

/*
 * get_pef_supported_number
 * Param : unsigned char *data
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Get PEF Supported Number
 */
int get_pef_supported_number(unsigned char *data) {
  *data = EVENT_FILTER_TABLE_ELEMS_MAX;
  return 0;
}

/*
 * get_pef_ctrl
 * Param : unsigned char *data
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Get PEF Control (PEF Configuration Parameters #1)
 */
int get_pef_ctrl(unsigned char *data) {
  if (etf_init() < 0) {
    return -1;
  }
  if (get_pef_ctrl_from_file(data) < 0) {
    return -1;
  }
  // success
  return 0;
}

int get_pef_ctrl_from_file(unsigned char *data) {
  FILE *fp;
  fp = fopen(EVENT_FILTER_TABLE_FILE, "r");
  if (fp == NULL) {
    printf("get_pef_ctrl: fopen\n");
    return -1;
  }

  if (fseek(fp, EFT_PEF_CTRL_OFFSET, SEEK_SET)) {
    printf("get_pef_ctrl: fseek\n");
    fclose(fp);
    return -1;
  }

  if (fread(data, sizeof(unsigned char), 1, fp) <= 0) {
    printf("get_pef_ctrl: fread\n");
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);
  return 0;
}

/*
 * set_pef_ctrl
 * Param : unsigned char *data
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Set PEF Control (PEF Configuration Parameters #1)
 */
int set_pef_ctrl(unsigned char *data) {
  if (etf_init() < 0) {
    return -1;
  }
  if (set_pef_ctrl_to_file(data) < 0) {
    return -1;
  }
  // success
  return 0;
}

int set_pef_ctrl_to_file(unsigned char *data) {
  FILE *fp;
  int index = 0;
  fp = fopen(EVENT_FILTER_TABLE_FILE, "r+");
  if (fp == NULL) {
    printf("set_pef_ctrl: fopen\n");
    return -1;
  }

  if (fseek(fp, EFT_PEF_CTRL_OFFSET, SEEK_SET)) {
    printf("set_pef_ctrl: fseek\n");
    fclose(fp);
    return -1;
  }

  if (fwrite(data, sizeof(unsigned char), 1, fp) <= 0) {
    printf("set_pef_ctrl: fwrite\n");
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);

  // char save[500] = {0,};
  // sprintf(save, "cp %s /backup_conf/ipmi/", EVENT_FILTER_TABLE_FILE);
  // system(save);

  return 0;
}

/*
 * get_pef_act_glb_ctrl
 * Param : unsigned char *data
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Get PEF Action global control (PEF Configuration Parameters #2)
 */
int get_pef_act_glb_ctrl(unsigned char *data) {
  if (etf_init() < 0) {
    return -1;
  }
  if (get_pef_act_glb_ctrl_from_file(data) < 0) {
    return -1;
  }
  // success
  return 0;
}

int get_pef_act_glb_ctrl_from_file(unsigned char *data) {
  FILE *fp;
  fp = fopen(EVENT_FILTER_TABLE_FILE, "r");
  if (fp == NULL) {
    printf("get_pef_act_glb_ctrl: fopen\n");
    return -1;
  }

  if (fseek(fp, EFT_PEF_ACT_GLB_CTRL_OFFSET, SEEK_SET)) {
    printf("get_pef_act_glb_ctrl: fseek\n");
    fclose(fp);
    return -1;
  }

  if (fread(data, sizeof(unsigned char), 1, fp) <= 0) {
    printf("get_pef_act_glb_ctrl: fread\n");
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);
  return 0;
}

/*
 * set_pef_act_glb_ctrl
 * Param : unsigned char *data
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Set PEF Action global control (PEF Configuration Parameters #2)
 */
int set_pef_act_glb_ctrl(unsigned char *data) {
  if (etf_init() < 0) {
    return -1;
  }
  if (set_pef_act_glb_ctrl_to_file(data) < 0) {
    return -1;
  }
  // success
  return 0;
}

int set_pef_act_glb_ctrl_to_file(unsigned char *data) {
  FILE *fp;
  int index = 0;
  fp = fopen(EVENT_FILTER_TABLE_FILE, "r+");
  if (fp == NULL) {
    printf("set_pef_act_glb_ctrl: fopen\n");
    return -1;
  }

  if (fseek(fp, EFT_PEF_ACT_GLB_CTRL_OFFSET, SEEK_SET)) {
    printf("set_pef_act_glb_ctrl: fseek\n");
    fclose(fp);
    return -1;
  }

  if (fwrite(data, sizeof(unsigned char), 1, fp) <= 0) {
    printf("set_pef_act_glb_ctrl: fwrite\n");
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);

  // char save[500] = {0,};
  // sprintf(save, "cp %s /backup_conf/ipmi/", EVENT_FILTER_TABLE_FILE);
  // system(save);

  return 0;
}

/*
 * get_eft_entry_num
 * Param : unsigned char *size
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Get Number of Event Filter Table's entries
 */
int get_eft_entry_num(unsigned char *size) {
  if (etf_init() < 0) {
    return -1;
  }
  if (get_eft_entry_num_from_file(size) < 0) {
    return -1;
  }
  // success
  return 0;
}

int get_eft_entry_num_from_file(unsigned char *size) {
  FILE *fp;
  fp = fopen(EVENT_FILTER_TABLE_FILE, "r");
  if (fp == NULL) {
    printf("get_eft_entry_num: fopen\n");
    return -1;
  }

  if (fread(size, sizeof(unsigned char), 1, fp) <= 0) {
    printf("get_eft_entry_num: fread");
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);
  return 0;
}

/*
 * set_eft_entry_num
 * Param : unsigned char *size
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Set Number of Event Filter Table's entries
 */
int set_eft_entry_num(unsigned char *size) {
  if (etf_init() < 0) {
    return -1;
  }

  if (set_eft_entry_num_to_file(size) < 0) {
    return -1;
  }

  return 0;
}

int set_eft_entry_num_to_file(unsigned char *size) {
  FILE *fp;
  fp = fopen(EVENT_FILTER_TABLE_FILE, "r+");
  if (fp == NULL) {
    printf("get_eft_entry_num: fopen\n");
    return -1;
  }

  if (fwrite(size, sizeof(unsigned char), 1, fp) <= 0) {
    printf("set_eft_entry_num: fwrite");
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);

  // char save[500] = {0,};
  // sprintf(save, "cp %s /backup_conf/ipmi/", EVENT_FILTER_TABLE_FILE);
  // system(save);

  return 0;
}

/**
 * @brief set_eft_entry
 * @author kys
 * @param void
 * @return 0 (Success) -1 (Wrong)
 * @details Set Event Filter Table's entry init
 */
int set_eft_init(void) {
  FILE *fp;
  pef_table_entry_t temp_pef_data[80];
  unsigned char index[80];
  unsigned char size;
  uint8_t pef_ctrl, pef_act_glb_ctrl;
  unsigned char i;
  uint8_t r1, r2, r3;
  int j;
  int vol = 15, temp = 38, cur = 2, fan = 17, dp = 0;
  int pef_index = 0;
  int pef_eft_table_size = 4;
  cout << "pef table init start " << endl;
  pef_table_entry_t pef_init_table[pef_eft_table_size] = {
      {PEF_FILTER_ENABLED,
       EVENT_FILTER_ACTION_POWER_OFF,
       PEF_POLICY_NUMBER_DONT_CARE,
       EVENT_SEVERITY_MONITOR,
       0x02,
       0x00,
       0x08,
       0x24,
       0xff,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00},

      {PEF_FILTER_DISABLED,
       PEF_ACTION_RESET,
       PEF_POLICY_NUMBER_DONT_CARE,
       EVENT_SEVERITY_INFORMATION,
       0x02,
       0x00,
       0x09,
       NVA_SYSTEM_FAN1,
       PEF_EVENT_TRIGGER_SENSOR_SPECIFIC,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00},

      {PEF_FILTER_DISABLED,
       EVENT_FILTER_ACTION_ALERT,
       PEF_POLICY_NUMBER_DONT_CARE,
       EVENT_SEVERITY_NON_CRITICAL,
       0x02,
       0x00,
       0x0c,
       PDPB_SENSOR_TEMP_CPU0,
       PEF_EVENT_TRIGGER_SENSOR_SPECIFIC,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00},

      {PEF_FILTER_DISABLED,
       PEF_ACTION_RESET_AND_POWER_CYCLE,
       PEF_POLICY_NUMBER_DONT_CARE,
       PEF_SEVERITY_OK,
       0x02,
       0x00,
       0x04,
       NVA_SYSTEM_FAN2,
       PEF_EVENT_TRIGGER_THRESHOLD,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00,
       0x00},
  };

  for (pef_index = 1; pef_index <= pef_eft_table_size; pef_index++) {
    cout << "pef index : " << pef_index << endl;
    set_eft_entry(pef_index, &pef_init_table[pef_index - 1]);
  }

  if (access(EVENT_FILTER_TABLE_FILE, F_OK) < 0) {
    pef_ctrl = CONTROL_PEF_ENABLE | CONRTOL_PEF_ALERT_STARTUP_DELAY;
    set_pef_ctrl(&pef_ctrl);

    pef_act_glb_ctrl =
        EVENT_FILTER_ACTION_ALERT | EVENT_FILTER_ACTION_POWER_OFF |
        EVENT_FILTER_ACTION_RESET | EVENT_FILTER_ACTION_POWER_CYCLE;

    set_pef_act_glb_ctrl(&pef_act_glb_ctrl);

    for (j = 0; j < vol; j++) {
      index[j + dp] = j + 1 + dp;
      temp_pef_data[j + dp].config = 0x80;

#if BR2_PACKAGE_NETSNMP
      temp_pef_data[j + dp].action = EVENT_FILTER_ACTION_ALERT;
#else
      temp_pef_data[j + dp].action = 0x00;
#endif
      temp_pef_data[j + dp].policy_number = 0x01;
      temp_pef_data[j + dp].severity = PEF_SEVERITY_CRITICAL;
      temp_pef_data[j + dp].event_trigger = PEF_EVENT_TRIGGER_MATCH_ANY;
      temp_pef_data[j + dp].sensor_type = SENSOR_TYPE_VOLTAGE;
      temp_pef_data[j + dp].sensor_number = voltage_sensor_name.list[j].mask;
      set_eft_entry(index[j + dp], (unsigned char *)&temp_pef_data[j + dp]);
    }
    dp += vol;
    for (j = 0; j < temp; j++) {
      index[j + dp] = j + 1 + dp;
      temp_pef_data[j + dp].config = 0x80;
#if BR2_PACKAGE_NETSNMP
      temp_pef_data[j + dp].action = EVENT_FILTER_ACTION_ALERT;
#else
      temp_pef_data[j + dp].action = 0x00;
#endif
      temp_pef_data[j + dp].policy_number = 0x01;
      temp_pef_data[j + dp].severity = PEF_SEVERITY_CRITICAL;
      temp_pef_data[j + dp].event_trigger = PEF_EVENT_TRIGGER_MATCH_ANY;
      temp_pef_data[j + dp].sensor_type = SENSOR_TYPE_TEMPERATURE;
      temp_pef_data[j + dp].sensor_number =
          temperature_sensor_name.list[j].mask;
      set_eft_entry(index[j + dp], (unsigned char *)&temp_pef_data[j + dp]);
    }

    dp += temp;
    for (j = 0; j < cur; j++) {
      index[j + dp] = j + 1 + dp;
      temp_pef_data[j + dp].config = 0x80;
#if BR2_PACKAGE_NETSNMP
      temp_pef_data[j + dp].action = EVENT_FILTER_ACTION_ALERT;
#else
      temp_pef_data[j + dp].action = 0x00;
#endif
      temp_pef_data[j + dp].policy_number = 0x01;
      temp_pef_data[j + dp].severity = PEF_SEVERITY_CRITICAL;
      temp_pef_data[j + dp].event_trigger = PEF_EVENT_TRIGGER_MATCH_ANY;
      temp_pef_data[j + dp].sensor_type = SENSOR_TYPE_CURRENT;
      temp_pef_data[j + dp].sensor_number = current_sensor_name.list[j].mask;
      set_eft_entry(index[j + dp], (unsigned char *)&temp_pef_data[j + dp]);
    }

    dp += cur;
    for (j = 0; j < fan; j++) {
      index[j + dp] = j + 1 + dp;
      temp_pef_data[j + dp].config = 0x80;
#if BR2_PACKAGE_NETSNMP
      temp_pef_data[j + dp].action = EVENT_FILTER_ACTION_ALERT;
#else
      temp_pef_data[j + dp].action = 0x00;
#endif
      temp_pef_data[j + dp].policy_number = 0x01;
      temp_pef_data[j + dp].severity = PEF_SEVERITY_CRITICAL;
      temp_pef_data[j + dp].event_trigger = PEF_EVENT_TRIGGER_MATCH_ANY;
      temp_pef_data[j + dp].sensor_type = SENSOR_TYPE_FAN;
      temp_pef_data[j + dp].sensor_number = fan_sensor_name.list[j].mask;
      set_eft_entry(index[j + dp], (unsigned char *)&temp_pef_data[j + dp]);
    }

  } else {
    if (get_eft_entry_num_from_file(&size) < 0) {
      return -1;
    }

    for (i = 1; i <= size; i++) {
      get_eft_entry(i, &g_eft_data[i]);
    }
  }

  return 0;
}

/*
 * etf_init
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Check to exist EVENT_FILTER_TABLE_FILE
 */
int etf_init(void) {
  FILE *fp;
  unsigned char zero = 0;
  unsigned char tmp[20];
  if (access(EVENT_FILTER_TABLE_FILE, F_OK) < 0) {
    fp = fopen(EVENT_FILTER_TABLE_FILE, "w+");
    if (fp == NULL) {
      printf("etf_init: fopen\n");
      return -1;
    }
    fclose(fp);
    if (set_eft_entry_num_to_file(&zero) < 0) {
      return -1;
    }
  }
  return 0;
}

/*
 * get_eft_entry
 * Param : unsigned char recid
           void *entry
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Get Event Filter Table's entry
 */
int get_eft_entry(unsigned char recid, void *entry) {
  unsigned char size;
  int index;
  if (etf_init() < 0) {
    return -1;
  }

  if (get_eft_entry_num_from_file(&size) < 0) {
    return -1;
  }

  if (recid > size) {
    printf("Inaccessible Index\n");
    return -1;
  }

  if (get_eft_entry_from_file(recid, entry) < 0) {
    return -1;
  }
  return 0;
}

int get_eft_entry_from_file(unsigned char recid, void *entry) {
  FILE *fp;
  int index;
  fp = fopen(EVENT_FILTER_TABLE_FILE, "r");
  if (fp == NULL) {
    printf("get_eft_entry: fopen\n");
    return -1;
  }

  index = (int)((recid - 1) * sizeof(pef_table_entry_t));

  if (fseek(fp, EFT_DATA_OFFSET + index, SEEK_SET)) {
    printf("set_etf_entry: fseek\n");
    fclose(fp);
    return -1;
  }

  if (fread(entry, sizeof(pef_table_entry_t), 1, fp) <= 0) {
    printf("get_etf_entry: fread\n");
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);
  return 0;
}

/*
 * set_eft_entry
 * Param : void *entry
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : Set Event Filter Table's entry
 */
int set_eft_entry(unsigned char recid, void *entry) {
  unsigned char size;
  void *entry_tmp;
  pef_table_entry_t temp_eft_data;
  if (etf_init() < 0) {
    return -1;
  }

  if (get_eft_entry_num_from_file(&size) < 0) {
    return -1;
  }

  if (recid > EVENT_FILTER_TABLE_INDEX_MAX) {
    printf("Inaccessible Index\n");
    return -1;
  }

  if (recid > size)
    size = recid;

  g_eft_data[recid].data1 = recid;
  entry_tmp = entry;
  g_eft_data[recid].config = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].action = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].policy_number = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].severity = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].generator_ID_addr = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].generator_ID_lun = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].sensor_type = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].sensor_number = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_trigger = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_1_offset_mask[0] = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_1_offset_mask[1] = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_1_AND_mask = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_1_compare_1 = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_1_compare_2 = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_2_AND_mask = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_2_compare_1 = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_2_compare_2 = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_3_AND_mask = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_3_compare_1 = *(unsigned char *)entry_tmp++;
  g_eft_data[recid].event_data_3_compare_2 = *(unsigned char *)entry_tmp++;

  if (set_eft_entry_to_file(&size, (unsigned char *)&g_eft_data[recid]) < 0) {
    return -1;
  }

  return 0;
}

int set_eft_entry_to_file(unsigned char *size, void *entry) {
  FILE *fp;
  int index = 0;

  pef_table_entry_t *tmp;
  tmp = (pef_table_entry_t *)entry;

  fp = fopen(EVENT_FILTER_TABLE_FILE, "r+");
  if (fp == NULL) {
    printf("get_eft_entry: fopen\n");
    return -1;
  }

  if (set_eft_entry_num_to_file(size) < 0) {
    fclose(fp);
    return -1;
  }

  index = (int)((tmp->data1 - 1) * sizeof(pef_table_entry_t));
  if (fseek(fp, EFT_DATA_OFFSET + index, SEEK_SET)) {
    printf("set_etf_entry: fseek\n");
    fclose(fp);
    return -1;
  }

  if (fwrite(entry, sizeof(pef_table_entry_t), 1, fp) <= 0) {
    *size--;
    set_eft_entry_num_to_file(size);
    fclose(fp);
    return -1;
  }

  // success
  fclose(fp);

  // char save[500] = {0,};
  // sprintf(save, "cp %s /backup_conf/ipmi/", EVENT_FILTER_TABLE_FILE);
  // system(save);

  return 0;
}

/*
 * check_pef_table_sensor
 * Param : uint8_t sType (sensor Type)
 *         uint8_t sNum (sensor Number)
 *         uint8_t severity (severity)
 * Return : positive interger (policy_num)
 *          0xff (Wrong)
 *
 * Description :
 */
uint8_t check_pef_table_sensor(uint8_t sType, uint8_t sNum, uint8_t severity) {
  unsigned char size;
  uint8_t rtn;
  unsigned char i;

  if (get_eft_entry_num_from_file(&size) < 0) {
    return 0xff;
  }

  for (i = 1; i <= size; i++) {
    if (g_eft_data[i].sensor_type == sType &&
        g_eft_data[i].sensor_number == sNum &&
        g_eft_data[i].severity == severity) {
      rtn = g_eft_data[i].policy_number;
      return rtn;
    }
  }

  return 0xff;
  ;
}

int find_msg_str(struct bit_desc_map sList, uint8_t num) {
  int i = 0, rtn = -1;
  while (sList.list[i].desc != NULL && i < 128) {
    if (sList.list[i].mask == num) {
      rtn = i;
      break;
    }
    i++;
  }

  return rtn;
}

/*
 * find_sensor_name
 * Param : uint8_t sType (sensor Type)
 *         uint8_t sNum (sensor Number)
 *         char *msg (msg, save and return the msg using this array)
 * Return : positive interger (Success, string length)
 *          -1 (Wrong)
 *
 * Description : find sensor name using sensor type and sensor number
 */

int find_sensor_name(uint8_t sType, uint8_t sNum, char *msg) {
  int sensor_index = -1, namelen = -1;
  char *tmp = msg;

  switch (sType) {
  case SENSOR_TYPE_TEMPERATURE: // temperature
    sensor_index = find_msg_str(temperature_sensor_name, sNum);
    if (sensor_index < 0) {
      printf("get_pef_alert_msg: can't find sensor.\n");
      return -1;
    }
    namelen = strlen(temperature_sensor_name.list[sensor_index].desc);
    //			strncpy(msg,
    // temperature_sensor_name.list[sensor_index].desc, namelen);
    strcpy(msg, temperature_sensor_name.list[sensor_index].desc);
    break;
  case SENSOR_TYPE_VOLTAGE: // voltage
    sensor_index = find_msg_str(voltage_sensor_name, sNum);
    if (sensor_index < 0) {
      printf("get_pef_alert_msg: can't find sensor.\n");
      return -1;
    }
    namelen = strlen(voltage_sensor_name.list[sensor_index].desc);
    //			strncpy(msg,
    // voltage_sensor_name.list[sensor_index].desc, namelen);
    strcpy(msg, voltage_sensor_name.list[sensor_index].desc);
    break;
  case SENSOR_TYPE_CURRENT: // current
    sensor_index = find_msg_str(current_sensor_name, sNum);
    if (sensor_index < 0) {
      printf("get_pef_alert_msg: can't find sensor.\n");
      return -1;
    }
    namelen = strlen(current_sensor_name.list[sensor_index].desc);
    //			strncpy(msg,
    // current_sensor_name.list[sensor_index].desc, namelen);
    strcpy(msg, current_sensor_name.list[sensor_index].desc);
    break;
  case SENSOR_TYPE_FAN: // Fan
    sensor_index = find_msg_str(fan_sensor_name, sNum);
    if (sensor_index < 0) {
      printf("get_pef_alert_msg: can't find sensor.\n");
      return -1;
    }
    namelen = strlen(fan_sensor_name.list[sensor_index].desc);
    //			strncpy(msg, fan_sensor_name.list[sensor_index].desc,
    // namelen);
    strcpy(msg, fan_sensor_name.list[sensor_index].desc);
    break;
    default:
      break;
    
  }

  return namelen;
}

/*
 * get_pef_alert_msg
 * Param : uint8_t sType (sensor Type)
 *         uint8_t sNum (sensor Number)
 *         uint8_t severity (severity)
 *         char *msg (msg, save and return the msg using this array)
 * Return : 0 (Success)
 *          -1 (Wrong)
 *
 * Description : generate pef alert msg
 *               before callign the fuction, define and initialize char array.
 */
int get_pef_alert_msg(uint8_t sType, uint8_t sNum, uint8_t severity,
                      char *msg) {
  int severity_index = -1;
  int namelen = -1, severitylen = -1;
  char *tmp = msg;

  namelen = find_sensor_name(sType, sNum, msg);
  if (namelen < 0) {
    return -1;
  }

  tmp += namelen;
  *tmp++ = ' ';
  *tmp++ = ':';
  *tmp++ = ' ';

  severity_index = find_msg_str(severity_name, severity);
  if (severity_index < 0) {
    printf("get_pef_alert_msg: can't find severity");
    return -1;
  }

  severitylen = strlen(severity_name.list[severity_index].desc);
  strncpy(tmp, severity_name.list[severity_index].desc, severitylen);
  tmp += severitylen;
  *tmp = '\0';

  return 0;
}

void pef_get_config_param(ipmi_req_t *request, ipmi_res_t *response,
                          unsigned char *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];

  unsigned char id = req->data[0];
  unsigned char set = req->data[1];
  unsigned char block = req->data[2];
  cout << "efte.cpp : " << (int)id << endl;
  res->cc = CC_INVALID_PARAM;
  switch (id) { // id = psel.id(ipmitool)

  case 1:           // PEF_CFGPARM_ID_PEF_CONTROL
    *data++ = 0x11; // lika: TODO
    if (get_pef_ctrl(data) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    data += 2;
    res->cc = CC_SUCCESS;
    break;
  case 2:           // PEF_CFGPARM_ID_PEF_CONTROL
    *data++ = 0x11; // lika: TODO
    if (get_pef_act_glb_ctrl(data) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    data += 2;
    res->cc = CC_SUCCESS;
    break;
  case 5:
    *data++ = 0x11;
    if (get_pef_supported_number(data) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    data++;
    res->cc = CC_SUCCESS;
    break;
  case 6:
    *data++ = 0x11; // lika: TODO
    if (get_eft_entry(set, data) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    data += 21;
    res->cc = CC_SUCCESS;
    break;
  case 8: // PEF_CFGPARM_ID_PEF_ALERT_POLICY_TABLE_SIZE
    // !!rjs : get from sel list size
    *data++ = 0x11;
    *data++ = get_policy_table_size();
    res->cc = CC_SUCCESS;
    break;
  case 9: // PEF_CFGPARM_ID_PEF_ALERT_POLICY_TABLE_SIZE
    *data++ = 0x11;
    *data++ = get_data1(set);
    *data++ = get_policy(set);
    *data++ = get_chan_dest(set);
    *data++ = get_alert_string_key(set);
    res->cc = CC_SUCCESS;
    break;
  case 10: // PEF_CFGPARM_ID_SYSTEM_GUID
    *data++ = 0x1;
    *data++ = 0x2;
    *data++ = 0x3;
    *data++ = 0x4;
    *data++ = 0x5;
    *data++ = 0x6;
    *data++ = 0x7;
    *data++ = 0x8;
    *data++ = 0x9;
    *data++ = 0xa;
    *data++ = 0xb;
    *data++ = 0xc;
    *data++ = 0xd;
    *data++ = 0xe;
    *data++ = 0xf;
    // memcpy(data, guid, strlen(guid));
    res->cc = CC_SUCCESS;
    break;
  default:
    res->cc = CC_INVALID_PARAM;
    break;
  }

  if (res->cc == CC_SUCCESS) {
    *res_len = data - &res->data[0];
  }
  return;
}

void pef_set_config_param(ipmi_req_t *request, ipmi_res_t *response,
                          unsigned char *res_len) {
  ipmi_req_t *req = (ipmi_req_t *)request;
  ipmi_res_t *res = (ipmi_res_t *)response;

  unsigned char param, id, block;
  unsigned char *entry;

  param = req->data[0];
  res->cc = CC_SUCCESS;
  switch (param) {
  case 1:
    entry = &req->data[1];
    if (set_pef_ctrl(entry) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    break;
  case 2:
    entry = &req->data[1];
    if (set_pef_act_glb_ctrl(entry) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    break;
  case 6:
    id = req->data[1];
    entry = &req->data[2];
    if (set_eft_entry(id, entry) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    break;
  case 7:
    id = req->data[1];
    entry = req->data[2];
    if (set_pef_enable(id, entry) < 0) {
      res->cc = CC_UNSPECIFIED_ERROR;
      return;
    }
    break;
  case 8: // PEF_CFGPARM_ID_PEF_ALERT_POLICY_TABLE_SIZE
    break;
  case 9: // PEF_CFGPARM_ID_PEF_ALERT_POLICY_TABLE_ENTRY
    id = req->data[1];
    entry = &req->data[2];
    set_policy_ipmi(id, (struct pef_policy_entry *)entry);
    break;
  case 10: // PEF_CFGPARM_ID_SYSTEM_GUID
    break;
  default:
    res->cc = CC_INVALID_PARAM;
    break;
  }
  return;
}

void pef_capabilities_info(ipmi_res_t *response, unsigned char *res_len) {
  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];
  unsigned char tmp;
  cout << "pef_capabilities_info" << endl;
  res->cc = CC_SUCCESS;
  if (get_eft_entry_num(&tmp) < 0) {
    res->cc = CC_UNSPECIFIED_ERROR;
    return;
  }

  // lika) TODO data[0], data[1]
  *data++ = 0x51; // pcap->version
  *data++ = 0x0F; // pcap->actions
  *data++ = tmp;  // pcap->event_filter_count'
  *res_len = data - &res->data[0];
  return;
}

#define IPMI_EVENT_LOG_VIEW 1
void pef_get_status(ipmi_res_t *response, unsigned char *res_len) {
  ipmi_res_t *res = (ipmi_res_t *)response;
  unsigned char *data = &res->data[0];
  time_t timer;
  time(&timer);
  // !!rjs: Need to read from sel

  res->cc = CC_SUCCESS;
  // time
  cout << "status 문제 확인" << endl;
#if IPMI_EVENT_LOG_VIEW
  plat_sel_ts_recent_add((time_stamp_t *)data);
#else
  *data++ = 0xFF;
  *data++ = 0xFF;
  *data++ = 0xFF;
  *data++ = 0xFF;
#endif
  data += 4;

  // record ID
#if IPMI_EVENT_LOG_VIEW
  plat_sel_last_rsv_id(data);
#else
  *data++ = 0xFF;
  *data++ = 0xFF;
#endif
  data += 2;
  // S/W ID
  *data++ = 0xFF;
  *data++ = 0xFF;
  // BMC ID
  *data++ = 0xFF;
  *data++ = 0xFF;
  *res_len = data - &res->data[0];
  return;
}

int rest_get_pef_config(char *res) {
  pef_table_entry_t eft;
  int i = 0, entrynum = 0;
  unsigned char tmp, num = 0;

  if (get_eft_entry_num(&num) < 0) {
    num = 0;
  }

  entrynum = (int)num;

  char buf[32];
  sprintf(buf, "{\n{\n{\n");
  strcat(res, buf);
  sprintf(buf, "   \"PEF\": [\n");
  strcat(res, buf);
  for (i = 1; i <= entrynum; i++) {
    get_eft_entry(i, (unsigned char *)&eft);
    sprintf(buf, "       {\n");
    strcat(res, buf);
    sprintf(buf, "          \"INDEX\": \"%d\",\n", i);
    strcat(res, buf);
    sprintf(buf, "          \"ENABLED\": ");
    strcat(res, buf);
    if (eft.config & PEF_CONFIG_ENABLED) {
      sprintf(buf, "\"enabled\",\n");
      strcat(res, buf);
    } else {
      sprintf(buf, "\"disabled\",\n");
      strcat(res, buf);
    }

    sprintf(buf, "          \"CONFIGURED\": ");
    strcat(res, buf);
    switch (eft.config & 0x60) {
    case 0x40:
      sprintf(buf, "\"pre-configured\"");
      strcat(res, buf);
      break;
    case 0x00:
      sprintf(buf, "\"configurable\"");
      strcat(res, buf);
      break;
    default:
      sprintf(buf, "\"reserved\"");
      strcat(res, buf);
      break;
    }

    if (eft.config & PEF_CONFIG_ENABLED) {
      sprintf(buf, ",\n");
      strcat(res, buf);
      sprintf(buf, "          \"SEVERITY\": ");
      strcat(res, buf);
      switch (eft.severity) {
      case PEF_SEVERITY_NON_RECOVERABLE:
        sprintf(buf, "\"non recoverable\",\n");
        strcat(res, buf);
        break;
      case PEF_SEVERITY_CRITICAL:
        sprintf(buf, "\"critical\",\n");
        strcat(res, buf);
        break;
      case PEF_SEVERITY_WARNING:
        sprintf(buf, "\"warning\",\n");
        strcat(res, buf);
        break;
      case PEF_SEVERITY_OK:
        sprintf(buf, "\"ok\",\n");
        strcat(res, buf);
        break;
      case PEF_SEVERITY_INFORMATION:
        sprintf(buf, "\"information\",\n");
        strcat(res, buf);
        break;
      case PEF_SEVERITY_MONITOR:
        sprintf(buf, "\"monitor\",\n");
        strcat(res, buf);
        break;
      default:
        sprintf(buf, "\"unkown\",\n");
        strcat(res, buf);
        break;
      }
      sprintf(buf, "         \"SENSOR_TYPE\": \"%x\",\n",
              eft.sensor_type); // lika) ipmitool pef_b2s_sensortypes
      strcat(res, buf);
      sprintf(buf, "         \"SENSOR_NUMBER\": \"%x\",\n", eft.sensor_number);
      strcat(res, buf);
      sprintf(buf, "         \"ACTION\": \"%x\",\n",
              eft.action); // lika) ipmitool pef_b2s_actions
      strcat(res, buf);
      sprintf(buf, "         \"POLICY_NUM\": \"%x\"\n", eft.policy_number);
      strcat(res, buf);
    } else {
      sprintf(buf, "\n");
      strcat(res, buf);
    }

    if (i == entrynum) {
      sprintf(buf, "       }\n");
      strcat(res, buf);
    } else {
      sprintf(buf, "       },\n");
      strcat(res, buf);
    }
  }
  sprintf(buf, "   ]\n");
  strcat(res, buf);
  sprintf(buf, "}\n");
  strcat(res, buf);
  //	printf("strlen: %d\n", strlen(res));
  return strlen(res);
}
