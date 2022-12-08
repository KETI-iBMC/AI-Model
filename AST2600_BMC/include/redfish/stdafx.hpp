#pragma once

#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/filestream.h>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/thread/thread.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

// default port define
#define DEFAULT_SNMP_PORT 161
#define DEFAULT_IPMI_PORT 623
#define DEFAULT_NTP_PORT 123
#define DEFAULT_KVMIP_PORT 5288
#define DEFAULT_HTTPS_PORT 443
#define DEFAULT_HTTP_PORT 80
#define DEFAULT_VIRTUAL_MEDIA_PORT 3900
#define DEFAULT_SSH_PORT 22
#define DEFAULT_SSDP_PORT 1900
#define DEFAULT_LDAP_PORT 636
#define DEFAULT_AD_PORT 389
#define DEFAULT_SYSLOG_PORT 514
#define DEFAULT_RADIUS_PORT 1812

// default conf file path define
#define DHCPV4_CONF "/etc/dhcp/dhcpd.conf"
#define DHCPV6_CONF "/etc/dhcp/dhcpd6.conf"
#define DNS_CONF "/etc/resolv.conf"
#define VLAN_CONF "/proc/net/vlan/config"

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace web::http::client;
using namespace utility;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
using namespace logging::trivial;

#ifndef __has_include
static_assert(false, "__has_include not supported");
#else
#if __cplusplus >= 201703L && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif __has_include(<boost/filesystem.hpp>)
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif
#endif

extern src::severity_logger<severity_level> g_logger;

/**
 * @brief Logging macro
 */
#define log(level) BOOST_LOG_SEV(g_logger, level)

/**
 * @brief Function of utilities
 */
vector<string> string_split(const string _string, char _delimiter);
string make_path(vector<string> tokens);
bool comp(const string &s1, const string &s2);
void timer(boost::asio::deadline_timer *_timer,
           unsigned int *_remain_expires_time);
string generate_token(const int len);

enum ALLOCATE_NUM {
  ALLOCATE_TASK_NUM,
  ALLOCATE_ACCOUNT_NUM,
  ALLOCATE_SESSION_NUM,
  ALLOCATE_VM_CD_NUM,
  ALLOCATE_VM_USB_NUM,
  ALLOCATE_SUBSCRIPTION_NUM,

  ALLOCATE_NUM_COUNT
};

void init_numset(void);
unsigned int allocate_numset_num(int _index);
void insert_numset_num(int _index, unsigned int num);
void delete_numset_num(int _index, unsigned int num);

string get_current_object_name(string _uri, string delimiter);
string get_parent_object_uri(string _uri, string delimiter);
bool isNumber(const string str);
bool isNumber(const char c);

bool validateIPv4(const string str);
bool validateDatetimeLocalOffset(const string str);
bool validateSeverity(const string str);
bool validateLogFacility(const string str);
bool validateDHCPv6OperatingMode(const string str);

bool validateMACAddress(const string str);

bool validateDate(const string str);
bool validateYear(const string str);
bool validateMonth(const string str);
bool validateDay(const string str);
bool validateTime(const string str);
bool validateHour(const string str);
bool validateMinute(const string str); // second는 같은걸 사용

string ltrim(string str);

char *get_popen_string(char *command);
string get_popen_string(string command);
string get_extracted_bmc_id_uri(string _uri);
bool check_role_privileges(string _pri);
string get_value_from_cmd_str(string cmd_str, string key);

bool get_value_from_json_key(json::value body, string key, int &value);
bool get_value_from_json_key(json::value body, string key, unsigned int &value);
bool get_value_from_json_key(json::value body, string key, string &value);
bool get_value_from_json_key(json::value body, string key, json::value &value);
bool get_value_from_json_key(json::value body, string key, double &value);
bool get_value_from_json_key(json::value body, string key, bool &value);

/**
 * @brief file read & write util func
 */
void read_file_line_by_line(string _filename, vector<string> &_lines);
void write_file_line_by_line(string _filename, vector<string> _lines);
void append_newline_and_push_back(string _str, vector<string> &_vec);

string generate_uuid(void);
void make_time_with_tm(tm _tm, char *_output);

/**
 * @brief Function of record
 */
bool record_is_exist(const string _uri);
bool event_is_exist(const string _uri);
json::value record_get_json(const string _uri);
bool record_load_json(void);
bool record_save_json(void);
void record_print(void);
void record_init_load(string _path);
void erase_blank(string &st);
void delete_resource(string odata_id);
void synchronize_dir_list();

void remove_if_exists(fs::path file);
float improved_stof(string str);
int improved_stoi(string str);

/**
 * @brief redfish 요청 구조체
 */
struct redfish_req_t {
  int netfn;
  int method;
  json::value data;
  string uri;
};
/**
 * @brief redfish 요청 응답 구조체
 */
struct redfish_res_t {
  int netfn;
  int c_code;
  json::value data;
};

enum { POST = 0, GET, PATCH, DELETE };

enum // Account Service에서 Request JSON Query의 Role ID를 구분하기 위한 Flag
     // Enum
{ R_NOACCESS = 1,
  R_READONLY,
  R_OPERATOR,
  R_ADMINISTRATOR,
};

enum // Response Changed Flag Enum
{ CF_COMPLETE = 0,
  CF_PROGRESS,
  CF_CHANGED,
};
enum { // IPC 통신 시 Sender, Receiver의 NetFn을 설정하기 위한 Enum
  NETFN_ACCOUNT_ENGINE_REQ = 0,
  NETFN_ACCOUNT_ENGINE_RES,
  NETFN_CHASSIS_ENGINE_REQ,
  NETFN_CHASSIS_ENGINE_RES,
  NETFN_SYSTEMS_ENGINE_REQ,
  NETFN_SYSTEMS_ENGINE_RES,
  NETFN_MANAGERS_ENGINE_REQ,
  NETFN_MANAGERS_ENGINE_RES,
  NETFN_TASKSERVICE_ENGINE_REQ,
  NETFN_TASKSERVICE_ENGINE_RES,
  NETFN_SESSIONSERVICE_ENGINE_REQ,
  NETFN_SESSIONSERVICE_ENGINE_RES,
  NETFN_EVENTSERVICE_ENGINE_REQ,
  NETFN_EVENTSERVICE_ENGINE_RES,
  NETFN_MONITOR_ENGINE_REQ,
  NETFN_MONITOR_ENGINE_RES,
  NETFN_HA_ENGINE_REQ,
  NETFN_HA_ENGINE_RES,
  NETFN_COMPOSITION_REQ,
  NETFN_COMPOSITION_RES,
};

enum { // Response Message Complete Code Enum
  RCC_SUCCESS = 0x0,
  RCC_FAILED = 0x1,
  RCC_UNKNOWN = 0x2,
};

#endif