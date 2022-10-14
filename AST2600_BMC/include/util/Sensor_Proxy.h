
/*
 *	This file was automatically generated by dbusxx-xml2cpp; DO NOT EDIT!
 */

#ifndef __dbusxx__Sensor_Proxy_h__PROXY_MARSHAL_H
#define __dbusxx__Sensor_Proxy_h__PROXY_MARSHAL_H
#include <dbus-c++-1/dbus-c++/dbus.h>
#include <cassert>

namespace org {
namespace freedesktop {
namespace keti {
namespace bmc {

class ADC_proxy
: public ::DBus::InterfaceProxy
{
public:

    ADC_proxy()
    : ::DBus::InterfaceProxy("org.freedesktop.keti.bmc.ADC")
    {
    }

public:

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    int32_t lightning_sensor_read(const uint8_t& fru, const uint8_t& sensor_num, const int32_t& value)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << fru;
        wi << sensor_num;
        wi << value;
        call.member("lightning_sensor_read");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }


public:

    /* signal handlers for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
};

} } } } 
namespace org {
namespace freedesktop {
namespace keti {
namespace bmc {

class edge_proxy
: public ::DBus::InterfaceProxy
{
public:

    edge_proxy()
    : ::DBus::InterfaceProxy("org.freedesktop.keti.bmc.edge")
    {
    }

public:

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    std::string rpc_test(const std::string& msg)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << msg;
        call.member("rpc_test");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }


public:

    /* signal handlers for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
};

} } } } 
namespace org {
namespace freedesktop {
namespace keti {
namespace bmc {

class FAN_proxy
: public ::DBus::InterfaceProxy
{
public:

    FAN_proxy()
    : ::DBus::InterfaceProxy("org.freedesktop.keti.bmc.FAN")
    {
    }

public:

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    int32_t read_fan_value(const int32_t& fanno, const int32_t& value)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << fanno;
        wi << value;
        call.member("read_fan_value");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }


public:

    /* signal handlers for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
};

} } } } 
namespace org {
namespace freedesktop {
namespace keti {
namespace bmc {

class PSU_proxy
: public ::DBus::InterfaceProxy
{
public:

    PSU_proxy()
    : ::DBus::InterfaceProxy("org.freedesktop.keti.bmc.PSU")
    {
    }

public:

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    int32_t read_psu_value(const & device, const uint8_t& addr, const uint8_t& type, const int32_t& value)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << device;
        wi << addr;
        wi << type;
        wi << value;
        call.member("read_psu_value");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }


public:

    /* signal handlers for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
};

} } } } 
#endif //__dbusxx__Sensor_h__PROXY_MARSHAL_H