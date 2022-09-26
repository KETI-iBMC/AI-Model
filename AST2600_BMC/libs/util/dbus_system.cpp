
#include "redfish/stdafx.hpp"
#include <dbus-c++-1/dbus-c++/connection.h>
#include <dbus-c++-1/dbus-c++/message.h>
#include <dbus-c++-1/dbus-c++/object.h>

#include <util/dbus_system.hpp>

static const char *SERVER_NAME = "org.freedesktop.keti.bmc.dbus";
static const char *SERVER_PATH = "/org/freedesktop/keti/bmc/dbus";
using namespace std;

// DBus singleinstance
// DBus_Sensor *DBus_Sensor::ins_DBus_Sensor=nullptr;
// DBus_Sensor *DBus_Sensor::SingleInstance(){
//     if(DBus_Sensor::ins_DBus_Sensor==nullptr){
//         DBus::BusDispatcher dispatcher;
//         DBus::default_dispatcher = &dispatcher;
//         DBus::Connection conn = DBus::Connection::SystemBus();
//         DBus_Sensor::ins_DBus_Sensor = new  DBus_Sensor(conn,SERVER_PATH,SERVER_NAME);
//         // cout <<rpc_test("fsdf")<<endl;
//         cout<<ins_DBus_Sensor->rpc_test("asda")<<endl;
        
//         cout <<"===================conect dbus==============="<<endl;
//     }
//     cout<<ins_DBus_Sensor<<endl;
//     return DBus_Sensor::ins_DBus_Sensor;
// }

DBus_Sensor::DBus_Sensor(DBus::Connection &connection, const char *path,
                         const char *name)
    : DBus::ObjectProxy(connection, path, name) {}

// DBus_Sensor::DBus_Sensor_1(DBus::Connection &connection, const char *path,
//                         const char *name)
// : DBus::ObjectProxy(connection, path, name) {}
static bool spin = true;
// DBus::BusDispatcher dispatcher;
DBus::DefaultTimeout *timeout;
// static const char *SERVER_NAME_1 = "org.freedesktop.keti.bmc.dbus";
// static const char *SERVER_PATH_1 = "/org/freedesktop/keti/bmc/dbus";
//   DBus::BusDispatcher dispatcher;
//   DBus::default_dispatcher = &dispatcher;
//   DBus::Connection conn = DBus::Connection::SystemBus();
//   DBus_Sensor dbus_sensor_time = DBus_Sensor(conn,SERVER_PATH_1,SERVER_NAME_1);
  
// void DBus_Sensor::conn(void){
//   DBus::default_dispatcher = &dispatcher;

//   dispatcher.enter();
// }

// void niam(int sig) {
//   spin = false;

//   dispatcher.leave();
// }