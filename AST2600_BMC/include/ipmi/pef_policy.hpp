#ifndef __PEF_POLICY_H__
#define __PEF_POLICY_H__
#include "ipmi/ipmi.hpp"
#include "ipmi/common.hpp"
#include "ipmi/sel.hpp"
#define EVENT_FILTER_TABLE_FILE "/conf/ipmi/eft.bin"
#define EFT_PEF_CTRL_OFFSET 0x10
#define EFT_PEF_ACT_GLB_CTRL_OFFSET 0x11
#define EFT_PEF_CTRL_OFFSET 0x10
#define EFT_PEF_ACT_GLB_CTRL_OFFSET 0x11
#define EFT_DATA_OFFSET 0x20

int set_policy_ipmi(uint8_t i, struct pef_policy_entry* entry);
int set_policy(uint8_t index, uint8_t policy_set, uint8_t rule, uint8_t alert_num);
int get_entry_by_policy_set(uint8_t policy_set, struct pef_policy_entry* matching_entries);
uint8_t delete_policy_entry(uint8_t i);
uint8_t get_policy_table_size();
void init_policy_table();
void save_policy_table();
uint8_t get_data1(uint8_t i);
uint8_t get_policy(uint8_t i);
uint8_t get_chan_dest(uint8_t i);
uint8_t get_alert_string_key(uint8_t i);



#endif
