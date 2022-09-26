#ifndef __DEMO_ECHO_SERVER_H
#define __DEMO_ECHO_SERVER_H

#include <dbus-c++-1/dbus-c++/dbus.h>
#include "keti_dbus_adaptor.h"
#include "common.hpp"
#include "smbus.hpp"
#include <sys/ipc.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/msg.h>

class KETI_Dbus
  : public org::freedesktop::keti::bmc::ADC_adaptor, 
   public org::freedesktop::keti::bmc::edge_adaptor,
   public org::freedesktop::keti::bmc::FAN_adaptor,
   public org::freedesktop::keti::bmc::PSU_adaptor,
  public DBus::IntrospectableAdaptor,
  public DBus::ObjectAdaptor
  
{
public:

  KETI_Dbus(DBus::Connection &connection);

public: 

  std::string rpc_test(const std::string& msg);
  int lightning_sensor_read(uint8_t fru, uint8_t sensor_num,  int *value);
  static int read_device_int(const char *device, int *value);
  int read_adc_value_KTNF(const int pin, const char *device, int *value);  
  int read_fan_value(int fanno, int *value);
  int read_psu_value(const std::string device, const uint8_t addr, const uint8_t type, int *value);  
};


static int i2c_readw(int i2c_dev, int addr);
static unsigned int i2c_readw_r(int i2c_dev, int addr);
int read_tmp75_temp_value(const char *device, int *value);
static int read_i2c_value(char *device, uint8_t addr, uint8_t type, uint32_t *value);
int read_temp_value(char *device, uint8_t addr, uint8_t type, uint32_t *value);
static int read_tacho_value(const int pin, int *value);


#endif//__DEMO_ECHO_SERVER_H
