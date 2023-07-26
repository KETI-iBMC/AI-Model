
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void pmbus_write(int addr, int command, int length, unsigned char *data);
void pmbus_read(int addr, int command, int length, unsigned char *data);
void set_page(int addr, unsigned char page);
void get_phase_info(int addr, unsigned char *data);
void print_dpwms(unsigned char phase_info);
void clear_faults(int addr);

double linear2voltage(unsigned short word);
double linear2nonvoltage(unsigned short word);

enum { PM_VOLTAGE, PM_NONVOLTAGE, PM_STATUS } pm_types ;

struct pm_list {
  unsigned char command;
  int size;
  char *name;
  char *units;
  int type;
};

static struct pm_list pm_common[] = {
  { 0x19, 1, "CAPABILITY", "", PM_STATUS },
  { 0x79, 2, "STATUS_WORD", "", PM_STATUS },
  { 0x7e, 1, "STATUS_CML", "", PM_STATUS },
  { 0x8d, 2, "READ_TEMPERATURE_1", "Celsius", PM_NONVOLTAGE },
  { 0x88, 2, "READ_VIN", "V", PM_NONVOLTAGE },
  { 0x89, 2, "READ_IIN", "A", PM_NONVOLTAGE },
  { 0xd3, 2, "VIN_SCALE_MONITOR", "V/V", PM_NONVOLTAGE },
  { }
};

static struct pm_list pm_paged[] = {
  { 0x01, 1, "OPERATION", "", PM_STATUS },
  { 0x02, 1, "ON_OFF_CONFIG", "", PM_STATUS },
  { 0x20, 1, "VOUT_MODE", "", PM_STATUS },
  { 0x7a, 1, "STATUS_VOUT", "", PM_STATUS },
  { 0x7b, 1, "STATUS_IOUT", "", PM_STATUS },
  { 0x7c, 1, "STATUS_INPUT", "", PM_STATUS },
  { 0x7d, 1, "STATUS_TEMPERATURE", "", PM_STATUS },
  { 0x21, 2, "VOUT_COMMAND", "V", PM_VOLTAGE },
  { 0x5e, 2, "POWER_GOOD_ON", "V", PM_VOLTAGE },
  { 0x5f, 2, "POWER_GOOD_OFF", "V", PM_VOLTAGE },
  { 0x24, 2, "VOUT_MAX", "V", PM_VOLTAGE },
  { 0x23, 2, "VOUT_CAL_OFFSET", "V, signed", PM_VOLTAGE },
  { 0x40, 2, "VOUT_OV_FAULT_LIMIT", "V", PM_VOLTAGE },
  { 0x42, 2, "VOUT_OV_WARN_LIMIT", "V", PM_VOLTAGE },
  { 0x44, 2, "VOUT_UV_FAULT_LIMIT", "V", PM_VOLTAGE },
  { 0x43, 2, "VOUT_UV_WARN_LIMIT", "V", PM_VOLTAGE },
  { 0x46, 2, "IOUT_OC_FAULT_LIMIT", "A", PM_NONVOLTAGE },
  { 0x4a, 2, "IOUT_OC_WARN_LIMIT", "A", PM_NONVOLTAGE },
  { 0x4b, 2, "IOUT_UC_FAULT_LIMIT", "A", PM_NONVOLTAGE },  

  { 0x25, 2, "VOUT_MARGIN_HIGH", "V", PM_VOLTAGE },
  { 0x26, 2, "VOUT_MARGIN_LOW", "V", PM_VOLTAGE },
  { 0x27, 2, "VOUT_TRANSITION_RATE", "V/ms", PM_NONVOLTAGE },
  { 0x29, 2, "VOUT_SCALE_LOOP", "V/V", PM_NONVOLTAGE },
  { 0x2a, 2, "VOUT_SCALE_MONITOR", "V/V", PM_NONVOLTAGE },
  { 0x8b, 2, "READ_VOUT", "V", PM_VOLTAGE },
  { 0x8c, 2, "READ_IOUT", "A", PM_NONVOLTAGE },
 { }
};