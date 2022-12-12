#pragma once
#include <redfish/stdafx.hpp>
#include <redfish/resource.hpp>
#include <ipmi/rest_get.hpp>
#include <ipmi/rest_post.hpp>
#include <ipmi/rest_put.hpp>
#include <ipmi/rest_delete.hpp>
#include <ipmi/ipmi.hpp>
#include <ipmi/fru.hpp>
#include <ipmi/user.hpp>
#include <redfish/stdafx.hpp>
#include <fstream>
#include <redfish/ethernetinterface.hpp>
#include <redfish/eventservice.hpp>
#include <redfish/logservice.hpp>

#define SERVER_CERTIFICATE_CHAIN_PATH "/conf/ssl/rootca.crt"
#define SERVER_PRIVATE_KEY_PATH "/conf/ssl/rootca.key"
#define SERVER_TMP_DH_PATH "/conf/ssl/dh2048.pem"

#define SERVER_REQUEST_TIMEOUT 10
// #define SERVER_ENTRY_PORT ":443"
#define SERVER_ENTRY_PORT ":8000"
#define SERVER_ENTRY_POINT "http://0.0.0.0" SERVER_ENTRY_PORT


#define MODULE_NAME "CM1"
// #define MODULE_NAME "CM2"
#define MODULE_TYPE "CM"
#define CMM_ADDRESS "http://10.0.6.106:8000"
#define HA_ADDRESS "http://10.0.6.107:644"

// UpdateService
// #define UPDATE_FIRMWARE_FILE
#define UPDATE_SOFTWARE_Ibmc_FILE "/tmp/new_version_Ibmc"
#define UPDATE_SOFTWARE_Ibmc_SH_FILE "/tmp/update_Ibmc.sh"
#define UPDATE_SOFTWARE_DB_FILE "/tmp/new_version_db"
#define UPDATE_SOFTWARE_DB_SH_FILE "/tmp/update_log_db.sh"
// #define UPDATE_SOFTWARE_REST_FILE
// #define UPDATE_SOFTWARE_IPMI_FILE
#define RESOURCE_BACKUP_FILE "/redfish/redfish_backup.tar"
#define RESOURCE_BACKUP_FILE_OLD "/redfish/redfish_backup_old.tar"
#define RESOURCE_RESTORE_FILE "/redfish/resource_restore.tar"
#define RESOURCE_RESTORE_FILE_OLD "/redfish/resource_restore_old.tar"



class Handler
{
public:
    // Constructor
    Handler(){};
    Handler(utility::string_t _url, http_listener_config _config);

    // Destructor
    ~Handler(){};

    // Server operation
    pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }

private:
    http_listener m_listener;
    vector<string> list;

    // Request handler
    void handle_get(http_request _request);
    void handle_put(http_request _request);
    void handle_post(http_request _request);
    void handle_patch(http_request _request);
    void handle_delete(http_request _request);
    void handle_options(http_request _request);

    int ipmi_handle_redfish(redfish_req_t *request, redfish_res_t *response);
    void all_handle();
};

unsigned int generate_task_resource(string _method, string _uri, json::value _jv, http_headers _header);
void complete_task_resource(unsigned int _num);

// void treat_get(http_request _request, http_response& _response);
void treat_post(http_request _request, json::value _jv, http_response& _response);
void do_actions(http_request _request, json::value _jv, http_response& _response);
void act_certificate(json::value _jv, string _resource, string _what, http_response& _response);
void act_certificate_service(json::value _jv, string _resource, string _what, http_response& _response);
void act_system(json::value _jv, string _resource, string _what, http_response& _response);
void act_eventservice(json::value _jv, string _resource, string _what, http_response& _response);
void act_logservice(json::value _jv, string _resource, string _what, http_response& _response);

void act_update_service(http_request _request, json::value _jv, string _resource, string _what, http_response& _response);
void update_firmware(http_request _request, string _firm_id, http_response& _response);
void update_software(http_request _request, string _firm_id, http_response& _response);
void update_resource_backup(http_response& _response);
void update_resource_restore(http_request& _request, http_response& _response);
void get_software_category(string _str, string& _front, string& _end);
void save_file_from_request(http_request _request, string _path);
// bool pass_request_to_bmc(http_request _request, string _module);

void act_virtualmedia(json::value _jv, string _resource, string _what, http_response& _response);

void make_account(json::value _jv, http_response& _response);
void make_session(json::value _jv, http_response& _response);
void make_subscription(json::value _jv, http_response& _response);

void treat_patch(http_request _request, json::value _jv, http_response& _response);
void modify_account(http_request _request, json::value _jv, string _uri, http_response& _response);
void modify_role(http_request _request, json::value _jv, string _uri, http_response& _response);

bool patch_account_service(json::value _jv, string _record_uri);
bool patch_session_service(json::value _jv);
bool patch_manager(json::value _jv, string _record_uri);
bool patch_network_protocol(json::value _jv, string _record_uri);
void patch_fan_mode(string _mode, string _record_uri);
bool patch_ethernet_interface(json::value _jv, string _record_uri, int _flag);
void apply_ethernet_patch(Ethernet_Patch_Info _info, EthernetInterfaces *_eth, int _flag);


bool patch_system(json::value _jv, string _record_uri);
bool patch_chassis(json::value _jv, string _record_uri);
bool patch_power_control(json::value _jv, string _record_uri);
bool patch_event_service(json::value _jv, string _record_uri);
bool patch_subscription(json::value _jv, string _record_uri);
bool patch_logservice(json::value _jv, string _record_uri);
bool patch_drive(json::value _jv, string _record_uri);

void treat_delete(http_request _request, json::value _jv, http_response& _response);
void remove_account(json::value _jv, string _uri, string _service_uri, http_response& _response);
void remove_session(http_request _request, http_response& _response);
void remove_subscription(string _uri, string _service_uri, http_response& _response);

void error_reply(json::value _jv, status_code _status, http_response& _response);
json::value get_error_json(string _message);
void success_reply(json::value _jv, status_code _status, http_response& _response);

void report_last_command(string _uri);

void test_send_event(Event _event);

void send_kvm_image(http_request _request);


// /log/reading~
// void log_operation(http_request _request);
// // reading 관련처리함수랑 event 관련처리함수 2가지로 분기
// void reading_operation(http_request _request, http_response &_response);
// void event_operation(http_request _request, http_response &_response);

// bool check_reading_type(string _types);
// bool check_reading_detail(string _type, string _detail);
// bool check_reading_time_option(string _option);
