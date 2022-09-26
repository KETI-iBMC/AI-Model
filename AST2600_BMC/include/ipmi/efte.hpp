
#ifndef __EFTE_H__
#define __EFTE_H__
#include "ipmi/pef_policy.hpp"
#include "ipmi/ipmi.hpp"
#include "ipmi/sel.hpp"
/* PEF Control*/
#define CONTROL_PEF_ENABLE 0x01
#define CONTROL_PEF_EVENT_MSG 0x02
#define CONTROL_PEF_STARTUP_DELAY 0x04
#define CONRTOL_PEF_ALERT_STARTUP_DELAY 0x08

/* Event Filter Action */

#define EVENT_FILTER_ACTION_ALERT 0x01
#define EVENT_FILTER_ACTION_POWER_OFF 0x02
#define EVENT_FILTER_ACTION_RESET 0x04
#define EVENT_FILTER_ACTION_POWER_CYCLE 0x08
#define EVENT_FILTER_ACTION_OEM_ACTION 0x10
#define EVENT_FILTER_ACTION_DIAGNOSTIC_INTERRUPT 0x20
#define EVENT_FILTER_ACTION_GROUP_CONTROL_OPERATION 0x40
#define EVENT_FILTER_ACTION_RESERVED 0x7




/* Event Severity */
#define EVENT_SEVERITY_UNSPECIFIED 0x00
#define EVENT_SEVERITY_MONITOR 0x01
#define EVENT_SEVERITY_INFORMATION 0x02
#define EVENT_SEVERITY_OK 0x04
#define EVENT_SEVERITY_NON_CRITICAL 0x08	// a.k.a Warning
#define EVENT_SEVERITY_CRITICAL 0x10
#define EVENT_SEVERITY_NON_RECOVERABLE 0x20

#define MATCH_ANY = 0xFF

// msg

#define SENSOR_TYPE_TEMPERATURE 0x01
#define SENSOR_TYPE_VOLTAGE 0x02
#define SENSOR_TYPE_CURRENT 0x03
#define SENSOR_TYPE_FAN 0x04


//CONFIGUREATION
#define PEF_FILTER_DISABLED 0x7F
#define PEF_FILTER_ENABLED 0x80
#define PEF_FILTER_TABLE_ID_MASK 0x7F
#define PEF_CONFIG_ENABLED 0x80
#define PEF_CONFIG_PRECONFIGURED 0x40
//ACTION
#define PEF_ACTION_DIAGNOSTIC_INTERRUPT 0x20
#define PEF_ACTION_ALL 0xff
#define PEF_ACTION_OEM 0x10
#define PEF_ACTION_NOT_OEM 0x0f
#define PEF_ACTION_NOT_ALERT_OEM 0x0e
#define PEF_ACTION_ALERT_AND_RESET_POWER_CYCLE 0x0d
#define PEF_ACTION_RESET_AND_POWER_CYCLE 0x0c
#define PEF_ACTION_ALERT_AND_POWER_DOWN_AND_POWER_CYCLE 0x0b
#define PEF_ACTION_POWER_DOWN_AND_POWER_CYCLE 0x0a
#define PEF_ACTION_ALERT_AND_POWER_CYCLE 0x09
#define PEF_ACTION_POWER_CYCLE 0x08
#define PEF_ACTION_ALERT_AND_POWER_DOWN_AND_RESET 0x07
#define PEF_ACTION_POWER_DOWN_AND_RESET 0x06
#define PEF_ACTION_ALERT_AND_RESET 0x05
#define PEF_ACTION_RESET 0x04
#define PEF_ACTION_ALERT_AND_POWER_DOWN 0x03
#define PEF_ACTION_POWER_DOWN 0x02
#define PEF_ACTION_ALERT 0x01
//POLICY NUMBER
#define PEF_POLICY_NUMBER_MASK 0x0f
#define PEF_POLICY_NUMBER_DONT_CARE 0x00
//EVENT SEVERITY
#define PEF_SEVERITY_NON_RECOVERABLE 0x20
#define PEF_SEVERITY_CRITICAL 0x10
#define PEF_SEVERITY_WARNING 0x08
#define PEF_SEVERITY_OK 0x04
#define PEF_SEVERITY_INFORMATION 0x02
#define PEF_SEVERITY_MONITOR 0x01
#define PEF_SEVERITY_UNSPECIFIED 0x00

#define PEF_SENSOR_NUMBER_MATCH_ANY 0xff
//TRIGGER
#define PEF_EVENT_TRIGGER_UNSPECIFIED 0x0
#define PEF_EVENT_TRIGGER_THRESHOLD 0x1
#define PEF_EVENT_TRIGGER_SENSOR_SPECIFIC 0x6f
#define PEF_EVENT_TRIGGER_MATCH_ANY 0xff


#define PEF_POLICY_TABLE_ID_MASK 0x7f
#define PEF_POLICY_ID_MASK 0xf0
#define PEF_POLICY_ID_SHIFT 4
#define PEF_POLICY_DISABLED 0xF7
#define PEF_POLICY_ENABLED 0x08
#define PEF_POLICY_FLAGS_MASK 0x07
#define PEF_POLICY_FLAGS_MATCH_ALWAYS 0
#define PEF_POLICY_FLAGS_PREV_OK_SKIP 1
#define PEF_POLICY_FLAGS_PREV_OK_NEXT_POLICY_SET 2
#define PEF_POLICY_FLAGS_PREV_OK_NEXT_CHANNEL_IN_SET 3
#define PEF_POLICY_FLAGS_PREV_OK_NEXT_DESTINATION_IN_SET 4
#define PEF_POLICY_CHANNEL_MASK 0xf0
#define PEF_POLICY_CHANNEL_SHIFT 4
#define PEF_POLICY_DESTINATION_MASK 0x0f
#define PEF_POLICY_EVENT_SPECIFIC 0x80
#define PEF_SYSTEM_GUID_USED_IN_PET 0x01
#define PEB_SENSOR_NUM_BASE 0x24 //PEB_SENSOR_ADC_P12V



struct desc_map {						/* maps a description to a value/mask */
	const char *desc;
	uint8_t mask;
};

struct bit_desc_map{
	struct desc_map list[128];
};

int set_pef_enable(unsigned char recid, unsigned char enable);
int set_pef_cfg(unsigned char recid, void *entry);
int get_pef_supported_number(unsigned char *data);
int get_pef_ctrl(unsigned char *data);
int get_pef_ctrl_from_file(unsigned char *data);
int set_pef_ctrl(unsigned char *data);
int set_pef_ctrl_to_file(unsigned char *data);
int get_pef_act_glb_ctrl(unsigned char *data);
int get_pef_act_glb_ctrl_from_file(unsigned char *data);
int set_pef_act_glb_ctrl(unsigned char *data);
int set_pef_act_glb_ctrl_to_file(unsigned char *data);
int get_eft_entry_num(unsigned char *size);
int get_eft_entry_num_from_file(unsigned char *size);
int set_eft_entry_num(unsigned char *size);
int set_eft_entry_num_to_file(unsigned char *size);
int set_eft_init(void);
int etf_init(void);
int get_eft_entry(unsigned char recid, void *entry);
int get_eft_entry_from_file(unsigned char recid, void *entry);
int set_eft_entry(unsigned char recid, void *entry);
//int set_eft_entry_to_file(void *entry, unsigned char *size);
int set_eft_entry_to_file(unsigned char *size, void *entry);
uint8_t check_pef_table_sensor(uint8_t sType, uint8_t sNum, uint8_t severity);
int find_sensor_name(uint8_t sType, uint8_t sNum, char *msg);
int find_msg_str(struct bit_desc_map sList, uint8_t num);
int get_pef_alert_msg(uint8_t sType, uint8_t sNum, uint8_t severity, char *msg);
void pef_capabilities_info (ipmi_res_t *response, unsigned char *res_len);
void pef_set_config_param (ipmi_req_t *request, ipmi_res_t *response, unsigned char *res_len);
void pef_get_config_param (ipmi_req_t *request, ipmi_res_t *response, unsigned char *res_len);
void pef_get_status (ipmi_res_t *response, unsigned char *res_len);
#endif
