#ifndef __DEMO_ECHO_CLIENT_H
#define __DEMO_ECHO_CLIENT_H

#include <dbus-c++/dbus.h>
#include "util/Sensor_Proxy.h"
#include <vector>

/**
 * @brief DBus_Sensor 서버측
 * @details rpc를 사용하기 위한 adaptor 상속 필수
 * 
 */

static const char *SERVER_NAME_1 = "org.freedesktop.keti.bmc.dbus";
static const char *SERVER_PATH_1 = "/org/freedesktop/keti/bmc/dbus";
class DBus_Sensor
  : public org::freedesktop::keti::bmc::edge_proxy,
  public org::freedesktop::keti::bmc::ADC_proxy,
  public org::freedesktop::keti::bmc::FAN_proxy,
  public org::freedesktop::keti::bmc::PSU_proxy,
  public DBus::IntrospectableProxy,
  public DBus::ObjectProxy
{
  public:
// private:
  DBus_Sensor(DBus::Connection &connection, const char *path, const char *name);
~DBus_Sensor(){};
  // static DBus_Sensor *ins_DBus_Sensor;
// public:
  // static DBus_Sensor *SingleInstance();  
  // DBus_Sensor_1(DBus::Connection &connection, const char *path, const char *name);
};




#endif//__DEMO_ECHO_CLIENT_H
