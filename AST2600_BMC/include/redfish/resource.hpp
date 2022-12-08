#pragma once

#ifndef RESOURCE_H
#define RESOURCE_H

#include <ipmi/apps.hpp>
#include <ipmi/chassis.hpp>
#include <ipmi/fru.hpp>
#include <ipmi/network.hpp>
#include <ipmi/setting_service.hpp>
#include <ipmi/user.hpp>
#include <redfish/hwcontrol.hpp>
#include <redfish/stdafx.hpp>
#include <redfish/ntp.hpp>
extern Ipmichassis ipmiChassis;
extern Ipmiapplication ipmiApplication;
extern std::map<uint8_t, std::map<uint8_t, Ipmifru>> fru_rec;
extern char uuid_hex[16];
extern char uuid_str[37];
extern Ipmiuser ipmiUser[10];

/**
 * @brief Redfish information
 */
#define REDFISH_VERSION "v1"

/**
 * @brief Open data protocol information
 */
#define ODATA_TYPE_VERSION REDFISH_VERSION "_0_0"

// Open data protocol path
#define ODATA_SERVICE_ROOT_ID "/redfish/" REDFISH_VERSION
#define ODATA_SYSTEM_ID ODATA_SERVICE_ROOT_ID "/Systems"
#define ODATA_CHASSIS_ID ODATA_SERVICE_ROOT_ID "/Chassis"
#define ODATA_MANAGER_ID ODATA_SERVICE_ROOT_ID "/Managers"
#define ODATA_RADIUS_ID ODATA_MANAGER_ID "/Radius"
#define ODATA_ETHERNET_INTERFACE_ID ODATA_MANAGER_ID "/EthernetInterfaces"
#define ODATA_TASK_SERVICE_ID ODATA_SERVICE_ROOT_ID "/TaskService"
#define ODATA_TASK_ID ODATA_TASK_SERVICE_ID "/Tasks"
#define ODATA_SESSION_SERVICE_ID ODATA_SERVICE_ROOT_ID "/SessionService"
#define ODATA_SESSION_ID ODATA_SESSION_SERVICE_ID "/Sessions"
#define ODATA_ACCOUNT_SERVICE_ID ODATA_SERVICE_ROOT_ID "/AccountService"
#define ODATA_ACCOUNT_ID ODATA_ACCOUNT_SERVICE_ID "/Accounts"
#define ODATA_ROLE_ID ODATA_ACCOUNT_SERVICE_ID "/Roles"
#define ODATA_EVENT_SERVICE_ID ODATA_SERVICE_ROOT_ID "/EventService"
#define ODATA_EVENT_DESTINATION_ID ODATA_EVENT_SERVICE_ID "/Subscriptions"
#define ODATA_UPDATE_SERVICE_ID ODATA_SERVICE_ROOT_ID "/UpdateService"

#define ODATA_MESSAGE_REGISTRY_ID "/redfish/" REDFISH_VERSION "/MessageRegistry"

// Open data protocol resource type
#define ODATA_RESOURCE_TYPE "#Resource." ODATA_TYPE_VERSION ".Resource"
#define ODATA_SERVICE_ROOT_TYPE                                                \
  "#ServiceRoot." ODATA_TYPE_VERSION ".ServiceRoot"
#define ODATA_COLLECTION_TYPE "#Collection.Collection"
#define ODATA_LIST_TYPE "#List.List"
#define ODATA_SYSTEM_COLLECTION_TYPE                                           \
  "#ComputerSystemCollection.ComputerSystemCollection"
#define ODATA_SYSTEM_TYPE                                                      \
  "#ComputerSystem." ODATA_TYPE_VERSION ".ComputerSystem"
#define ODATA_PROCESSOR_COLLECTION_TYPE                                        \
  "#ProcessorCollection.ProcessorCollection"
#define ODATA_PROCESSOR_TYPE "#Processor." ODATA_TYPE_VERSION ".Processor"
#define ODATA_MEMORY_COLLECTION_TYPE "#MemoryCollection.MemoryCollection"
#define ODATA_MEMORY_TYPE "#Memory." ODATA_TYPE_VERSION ".Memory"
#define ODATA_PROCESSOR_SUMMARY_TYPE                                           \
  "#ProcessorSummary." ODATA_TYPE_VERSION ".ProcessorSummary"
#define ODATA_NETWORK_INTERFACE_TYPE                                           \
  "#NetworkInterface." ODATA_TYPE_VERSION ".NetworkInterface"
#define ODATA_STORAGE_TYPE "#Storage." ODATA_TYPE_VERSION ".Storage"
#define ODATA_STORAGE_CONTROLLER_TYPE                                          \
  "#StorageController." ODATA_TYPE_VERSION ".StorageController"
#define ODATA_STORAGE_COLLECTION_TYPE                                          \
  "#StorageCollection.StorageCollection" // @@@@@ 추가
#define ODATA_BIOS_TYPE "#Bios." ODATA_TYPE_VERSION ".Bios"
#define ODATA_SIMPLE_STORAGE_COLLECTION_TYPE                                   \
  "#SimpleStorageCollection.SimpleStorageCollection"
#define ODATA_SIMPLE_STORAGE_TYPE                                              \
  "#SimpleStorage." ODATA_TYPE_VERSION ".SimpleStorage"
#define ODATA_CHASSIS_COLLECTION_TYPE "#ChassisCollection.ChassisCollection"
#define ODATA_CHASSIS_TYPE "#Chassis." ODATA_TYPE_VERSION ".Chassis"
#define ODATA_SENSOR_TYPE "#Sensor." ODATA_TYPE_VERSION ".Sensor"
#define ODATA_SENSOR_COLLECTION_TYPE "#SensorCollection.SensorCollection"
#define ODATA_THERMAL_TYPE "#Thermal." ODATA_TYPE_VERSION ".Thermal"
#define ODATA_POWER_TYPE "#Power." ODATA_TYPE_VERSION ".Power"
#define ODATA_MANAGER_COLLECTION_TYPE "#ManagerCollection.ManagerCollection"
#define ODATA_MANAGER_TYPE "#Manager." ODATA_TYPE_VERSION ".Manager"
#define ODATA_NETWORK_PROTOCOL_TYPE                                            \
  "#NetworkProtocol." ODATA_TYPE_VERSION ".NetworkProtocol"
#define ODATA_ETHERNET_INTERFACE_COLLECTION_TYPE                               \
  "#EthernetInterfaceCollection.EthernetInterfaceCollection"
#define ODATA_ETHERNET_INTERFACE_TYPE                                          \
  "#EthernetInterface." ODATA_TYPE_VERSION ".EthernetInterface"
#define ODATA_LOG_SERVICE_COLLECTION_TYPE                                      \
  "#LogServiceCollection.LogServiceCollection"
#define ODATA_LOG_SERVICE_TYPE "#LogService." ODATA_TYPE_VERSION ".LogService"
#define ODATA_LOG_ENTRY_COLLECTION_TYPE "#LogEntryCollection.LogEntryCollection"
#define ODATA_LOG_ENRTY_TYPE "#LogEntry." ODATA_TYPE_VERSION ".LogEntry"
#define ODATA_TASK_SERVICE_TYPE                                                \
  "#TaskService." ODATA_TYPE_VERSION ".TaskService"
#define ODATA_TASK_COLLECTION_TYPE "#TaskCollection.TaskCollection"
#define ODATA_TASK_TYPE "#Task." ODATA_TYPE_VERSION ".Task"
#define ODATA_SESSION_SERVICE_TYPE                                             \
  "#SessionService." ODATA_TYPE_VERSION ".SessionService"
#define ODATA_SESSION_COLLECTION_TYPE "#SessionCollection.SessionCollection"
#define ODATA_SESSION_TYPE "#Session." ODATA_TYPE_VERSION ".Session"
#define ODATA_ACCOUNT_SERVICE_TYPE                                             \
  "#AccountService." ODATA_TYPE_VERSION ".AccountService"
#define ODATA_ACCOUNT_COLLECTION_TYPE                                          \
  "#ManagerAccountCollection.ManagerAccountCollection"
#define ODATA_ACCOUNT_TYPE                                                     \
  "#ManagerAccount." ODATA_TYPE_VERSION ".ManagerAccount"
#define ODATA_ROLE_COLLECTION_TYPE "#RoleCollection.RoleCollection"
#define ODATA_ROLE_TYPE "#Role." ODATA_TYPE_VERSION ".Role"
#define ODATA_EVENT_SERVICE_TYPE                                               \
  "#EventService." ODATA_TYPE_VERSION ".EventService"
#define ODATA_EVENT_DESTINATION_TYPE                                           \
  "#EventDestination." ODATA_TYPE_VERSION ".EventDestination"
#define ODATA_EVENT_DESTINATION_COLLECTION_TYPE                                \
  "#EventDestinationCollection.EventDestinationCollection"
#define ODATA_EVENT_TYPE "#Event." ODATA_TYPE_VERSION ".Event"
// #define ODATA_DESTINATION_COLLECTION_TYPE
// "#EventDestinationCollection.EventDestinationCollection" #define
// ODATA_DESTINATION_TYPE "#EventDestination." ODATA_TYPE_VERSION
// ".EventDestination"
#define ODATA_UPDATE_SERVICE_TYPE                                              \
  "#UpdateService." ODATA_TYPE_VERSION ".UpdateService"
#define ODATA_UPDATE_SERVICE_COLLECTION_TYPE                                   \
  "#UpdateServiceCollection.UpdateServiceCollection"
#define ODATA_SOFTWARE_INVENTORY_TYPE                                          \
  "#SoftwareInventory." ODATA_TYPE_VERSION ".SoftwareInventory"
#define ODATA_SOFTWARE_INVENTORY_COLLECTION_TYPE                               \
  "#SoftwareInventoryCollection.SoftwareInventoryCollection"

// dy : certificate
#define ODATA_CERTIFICATE_SERVICE_ID ODATA_SERVICE_ROOT_ID "/CertificateService"
#define ODATA_CERTIFICATE_SERVICE_TYPE                                         \
  "#CertificateService." ODATA_TYPE_VERSION ".CertificateService"
#define ODATA_CERTIFICATE_LOCATION_ID                                          \
  ODATA_CERTIFICATE_SERVICE_ID "/CertificateLocations"
#define ODATA_CERTIFICATE_LOCATION_TYPE                                        \
  "#CertificateLocation." ODATA_TYPE_VERSION ".CertificateLocation"
#define ODATA_CERTIFICATE_COLLECTION_TYPE                                      \
  "#CertificateCollection.CertificateCollection"
#define ODATA_CERTIFICATE_ID "/Certificates"
#define ODATA_CERTIFICATE_TYPE "#Certificate." ODATA_TYPE_VERSION ".Certificate"

// dy : virtual media
#define ODATA_VIRTUAL_MEDIA_TYPE                                               \
  "#VirtualMedia." ODATA_TYPE_VERSION ".VirtualMedia"
#define ODATA_VIRTUAL_MEDIA_COLLECTION_TYPE                                    \
  "#VirtualMediaCollection.VirtualMediaCollection"

// dy : storage (drive, volume)
#define ODATA_DRIVE_TYPE "#Drive." ODATA_TYPE_VERSION ".Drive"
#define ODATA_DRIVE_COLLECTION_TYPE "#DriveCollection.DriveCollection"
#define ODATA_VOLUME_TYPE "#Volume." ODATA_TYPE_VERSION ".Volume"
#define ODATA_VOLUME_COLLECTION_TYPE "#VolumeCollection.VolumeCollection"

#define ODATA_MESSAGE_REGISTRY_TYPE                                            \
  "#MessageRegistry" ODATA_TYPE_VERSION ".MessageRegistry"

#define ODATA_RADIUS_TYPE "#Radius." ODATA_TYPE_VERSION ".Radius"

#define NO_DATA_TYPE 0

// systems
#define BF1_VALID_SHIFT 7
#define BF1_INVALID 0
#define BF1_VALID (1 << BF1_VALID_SHIFT)
#define BF1_VALID_MASK BF1_VALID

#define BF1_PERSIST_SHIFT 6
#define BF1_ONCE 0
#define BF1_PERSIST (1 << BF1_PERSIST_SHIFT)
#define BF1_PERSIST_MASK BF1_PERSIST

#define BF1_BOOT_TYPE_SHIFT 5
#define BF1_BOOT_TYPE_LEGACY 0
#define BF1_BOOT_TYPE_EFI (1 << BF1_BOOT_TYPE_SHIFT)
#define BF1_BOOT_TYPE_MASK BF1_BOOT_TYPE_EFI

#define BF2_BOOTDEV_SHIFT 2
#define BF2_BOOTDEV_DEFAULT (0 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_PXE (1 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_HDD (2 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_HDD_SAFE (3 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_DIAG_PART (4 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_CDROM (5 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_SETUP (6 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_REMOTE_FDD (7 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_REMOTE_CDROM (8 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_REMOTE_PRIMARY_MEDIA (9 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_REMOTE_HDD (11 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_FDD (15 << BF2_BOOTDEV_SHIFT)
#define BF2_BOOTDEV_MASK (0xF << BF2_BOOTDEV_SHIFT)

/**
 * @brief Redfish resource status element
 */
#define STATUS_STATE_ENABLED "Enabled"
#define STATUS_STATE_DISABLED "Disabled"
#define STATUS_STATE_STAND_BY_OFFLINE "StandbyOffline"
#define STATUS_STATE_STARTING "Starting"
#define STATUS_HEALTH_OK "OK"
#define STATUS_HEALTH_CRITICAL "Critical"
#define STATUS_HEALTH_WARNING "Warning"

/**
 * @brief Redfish chassis resource indicator led state element
 */
#define LED_OFF 0
#define LED_BLINKING 1
#define LED_LIT 2

/**
 * @brief Redfish chassis resource power state element
 */
#define POWER_STATE_OFF "Off"
#define POWER_STATE_ON "On"
#define POWER_STATE_POWERING_OFF "PoweringOff"
#define POWER_STATE_POWERING_ON "PoweringOn"

/**
 * @brief Redfish Task State & Task Status element
 */
#define TASK_STATE_NEW "New"
#define TASK_STATE_RUNNING "Running"
#define TASK_STATE_COMPLETED "Completed"
#define TASK_STATE_CANCELLED "Cancelled"
#define TASK_STATE_SUSPENDED "Suspended"
#define TASK_STATE_KILLED "Killed"

#define TASK_STATUS_CRITICAL "Critical"
#define TASK_STATUS_OK "OK"
#define TASK_STATUS_WARNING "Warning"

/**
 * @brief Redfish Certificate Key Usage
 */
#define KEY_USAGE_CLIENT_AUTHENTICATION "ClientAuthentication"
#define KEY_USAGE_CODE_SIGNING "CodeSigning"
#define KEY_USAGE_CRL_SIGNING "CRLSigning"
#define KEY_USAGE_DATA_ENCIPHERMENT "DataEncipherment"
#define KEY_USAGE_DECIPHER_ONLY "DecipherOnly"
#define KEY_USAGE_DIGITAL_SIGNATURE "DigitalSignature"
#define KEY_USAGE_EMAIL_PROTECTION "EmailProtection"
#define KEY_USAGE_ENCIPHER_ONLY "EncipherOnly"
#define KEY_USAGE_KEY_AGREEMENT "KeyAgreement"
#define KEY_USAGE_KEY_CERT_SIGN "KeyCertSign"
#define KEY_USAGE_KEY_ENCIPHERMENT "KeyEncipherment"
#define KEY_USAGE_NON_REPUDIATION "NonRepudiation"
#define KEY_USAGE_OCSP_SIGNING "OCSPSigning"
#define KEY_USAGE_SERVER_AUTHENTICATION "ServerAuthentication"
#define KEY_USAGE_TIMESTAMPING "Timestamping"

/**
 * @brief 도메인 이름 저장된곳
 *
 */
#define DOMAINNAME_FILE "/etc/hosts"
#define GET_WEB_PORT                                                           \
  "head -n 35 /etc/nginx/nginx.conf | tail -1 | awk \'{print $2}\' | awk -F "  \
  "\';\' \'{print $1}\'"
/**
 * @brief 현재시간 반환
 * @details yyyy-mm-dd.hh:mm:ss
 * @return string
 *
 */
extern Ipminetwork ipmiNetwork[4];
const std::string currentDateTime();

extern alert_servce_t g_alert_service;
extern ssh_service_t g_ssh_service;
extern ss_t g_setting;
extern web_service_t g_web_service;
extern kvm_service_t g_kvm_service;

char *get_popen_string(char *command);

/**
 * @brief Resource class type
 */
enum RESOURCE_TYPE {
  SUPER_TYPE,
  SERVICE_ROOT_TYPE,
  COLLECTION_TYPE,
  LIST_TYPE,
  ACTIONS_TYPE,
  SYSTEM_TYPE,
  STORAGE_TYPE,
  STORAGE_CONTROLLER_TYPE,
  SIMPLE_STORAGE_TYPE,
  BIOS_TYPE,
  PROCESSOR_TYPE,
  MEMORY_TYPE,
  PROCESSOR_SUMMARY_TYPE,
  NETWORK_INTERFACE_TYPE,
  CHASSIS_TYPE,
  THERMAL_TYPE,
  TEMPERATURE_TYPE,
  FAN_TYPE,
  SENSOR_TYPE,
  POWER_TYPE,
  POWER_CONTROL_TYPE,
  VOLTAGE_TYPE,
  POWER_SUPPLY_TYPE,
  MANAGER_TYPE,
  NETWORK_PROTOCOL_TYPE,
  ETHERNET_INTERFACE_TYPE,
  LOG_SERVICE_TYPE,
  LOG_ENTRY_TYPE,
  TASK_SERVICE_TYPE,
  TASK_TYPE,
  SESSION_SERVICE_TYPE,
  SESSION_TYPE,
  ACCOUNT_SERVICE_TYPE,
  ACCOUNT_TYPE,
  ROLE_TYPE,
  EVENT_SERVICE_TYPE,
  EVENT_DESTINATION_TYPE,
  UPDATE_SERVICE_TYPE,
  SOFTWARE_INVENTORY_TYPE,
  DESTINATION_TYPE,
  CERTIFICATE_TYPE,
  CERTIFICATE_LOCATION_TYPE,
  CERTIFICATE_SERVICE_TYPE,
  VIRTUAL_MEDIA_TYPE,
  DRIVE_TYPE,
  VOLUME_TYPE,
  MESSAGE_REGISTRY_TYPE,
  SYSLOG_SERVICE_TYPE,
  RADIUS_TYPE
};

enum ACTION_NAME {
  RESET_BIOS,
  CHANGE_PASSWORD,
  CLEAR_LOG,
  RESET_SYSTEM,
  RESET_MANAGER,
  RE_KEY,
  RE_NEW,
  GENERATE_CSR,
  REPLACE_CERTIFICATE,
  SUBMIT_TEST_EVENT,
  SIMPLE_UPDATE,
  FIRMWARE_UPDATE,
  SOFTWARE_UPDATE,
  RESOURCE_BACKUP,
  RESOURCE_RESTORE,
  INSERT_MEDIA,
  EJECT_MEDIA
};

enum NETWORK_PROTOCOL_IPTABLE_INDEX {
  HTTP_INDEX = 1,
  HTTPS_INDEX,
  SNMP_INDEX,
  IPMI_INDEX,
  KVMIP_INDEX,
  SSH_INDEX,
  VM_INDEX,
  NTP_INDEX
};

/**
 * @brief Sensor context
 */
enum SENSOR_CONTEXT { INTAKE_CONTEXT, CPU_CONTEXT };

/**
 * @brief Open data protocol information
 *        @odata.id
 *        @odata.type
 */
typedef struct _OData {
  string id;
  string type;
} OData;

/**
 * @brief Status of redfish resource
 */
typedef struct _Status {
  string state;
  string health;
} Status;

/**
 * @brief PostalAddress information of machine
 */
typedef struct _PostalAddress {
  string country;
  string territory;
  string city;
  string street;
  string house_number;
  string name;
  string postal_code;
} PostalAddress;

/**
 * @brief Placement information of machine
 */
typedef struct _Placement {
  string row;
  string rack;
  string rack_offset_units;
  unsigned int rack_offset;
} Placement;

/**
 * @brief Location information of machine
 */
typedef struct _Location {
  PostalAddress postal_address;
  Placement placement;
} Location;

typedef struct _Actions_Parameter {
  string name;
  std::vector<string> allowable_values;
} Parameter;

typedef struct _Actions {
  int type;
  string name;
  string target;
  std::vector<Parameter> parameters;
} Actions;

typedef struct _Boot {
  // BootSOE(Source Override Enabled)
  string boot_source_override_enabled;
  string boot_source_override_target;
  string boot_source_override_mode;
  string uefi_target_boot_source_override;

  // 여기에 enabled, target, mode 3가지 들어갈수있는 Allowable value를
  // 넣어놓아야하나?

  // string BootSourceOverrideEnabled;
  // string BootSourceOverrideTarget;
} Boot;

typedef struct _MemoryLocation {
  unsigned int channel;
  unsigned int memory_controller;
  unsigned int slot;
  unsigned int socket;

} MemoryLocation;

// typedef struct _MemorySummary
// {
//     double total_system_memory_GiB;
//     double total_system_persistent_memory_GiB;
//     string memory_mirroring;
//     Status status;

// } MemorySummary; // 뺄듯

typedef struct _DHCP_v4 {
  bool dhcp_enabled;
  bool use_dns_servers;
  bool use_gateway;
  bool use_ntp_servers;
  bool use_static_routes;
  bool use_domain_name;

} DHCP_v4;

typedef struct _DHCP_v6 {
  string operating_mode;
  bool use_dns_servers;
  bool use_ntp_servers;
  bool use_domain_name;
  bool use_rapid_commit;

} DHCP_v6;

typedef struct _IPv4_Address {
  string address;
  string address_origin;
  string subnet_mask;
  string gateway;

} IPv4_Address;

typedef struct _IPv6_Address {
  string address;
  string address_origin;
  string address_state;
  int prefix_length;

} IPv6_Address;

typedef struct _VLAN {
  bool vlan_enable;
  int vlan_id;
} Vlan;

typedef struct _Device_Info {
  unsigned int capacity_KBytes;
  string file_system;
  string manufacturer;
  string model;
  string name;
  Status status;

} Device_Info;

typedef struct _CertContent {
  string city;
  string commonName;
  string country;
  string email;
  string organization;
  string organizationUnit;
  string state;
} CertContent; // dy : in certificate

/**
 * @brief Task Resource에 들어가는 Payload
 * @author 강
 * @details http_hearders에 들어온 요청의 헤더, http_operation에 요청의
 * 종류(get,post..) json_body에 json값, target_uri에 api list값
 */
typedef struct _Payload {
  map<string, string> http_headers;
  string http_operation;
  json::value json_body;
  string target_uri;

} Payload;

typedef struct _Identifier {
  string durable_name;
  string durable_name_format;
} Identifier;

typedef struct _Attribute {
  string boot_mode;
  string embedded_sata;
  string nic_boot1;
  string nic_boot2;
  string power_profile;
  int proc_core_disable;
  string proc_hyper_threading;
  string proc_turbo_mode;
  string usb_control;
} Attribute;

typedef struct _ProcessorId {
  string identification_registers;
  string effective_family;
  string effective_model;
  string step;
  string microcode_info;
  string vendor_id;
} ProcessorId;

// typedef struct _ProcessorSummary
// {
//     int count;
//     int logical_processor_count;
//     // Metrics *metrics; // class 만들어야함
//     string model;
//     Status status;

// } ProcessorSummary;

typedef struct _SSE_Filter_Properties_Supported {
  // bool event_type;
  bool metric_report_definition;
  bool registry_prefix;
  bool resource_type;
  bool event_format_type;
  bool message_id;
  bool origin_resource;
  bool subordinate_resources;

} SSE_filter;

typedef struct _PowerLimit {
  int correction_in_ms;
  string limit_exception;
  double limit_in_watts;

} PowerLimit; // in PowerControl

typedef struct _PowerMetrics {
  int interval_in_min;
  double min_consumed_watts;
  double max_consumed_watts;
  double avg_consumed_watts;

} PowerMetrics; // in PowerControl

typedef struct _InputRange {
  string input_type;
  double minimum_voltage;
  double maximum_voltage;
  double output_wattage;

} InputRange; // in PowerSupply
/**
 *
 */
typedef struct _Info_Threshold {
  string activation;
  double reading;
  // string dwelltime;

} Info_Threshold; // in struct Thresholds (for Sensor)

/**
 * @brief LNC
 * @param lower_caution Lower Non-Critical (LNC)
 * @param lower_critical Lower Critical (LC)
 * @param lower_fatal Lower Non-Recoverable (LNR)
 * @param upper_caution Upper Non-Critical (UNC)
 * @param upper_critical Upper Critical (UC)
 * @param upper_fatal Upper Non-Recoverable (UNR)
 */
typedef struct _Thresholds {
  Info_Threshold lower_caution;
  Info_Threshold lower_critical;

  Info_Threshold lower_fatal;
  Info_Threshold upper_caution;
  Info_Threshold upper_critical;
  Info_Threshold upper_fatal;

} Thresholds; // in Sensor

typedef struct _Value_About_HA {
  string peer_primary_address;
  int primary_port;
  string peer_secondary_address;
  int second_port;
  bool origin;
  bool enabled;
} Value_About_HA;

typedef struct _Message
{
    string id;
    string message;
    string severity;
    vector<string> message_args;
    string resolution; // Used to provide suggestions on how to resolve the situation that caused the error.

    json::value get_json(void); // message 전체 정보를 json으로 리턴
    void load_json(json::value &j);

    void get_specific_json(json::value &j); // 자주 사용되는 message args를 기존 json에 추가
    void load_specific_json(json::value &j);
} Message;

typedef struct _Event_Info{
    string event_id;
    string event_timestamp;
    string event_type;
    string member_id;
    Message message;
} Event_Info;

// typedef struct _Event_Info {
//   int event_group_id;
//   string event_id;
//   string event_timestamp;
//   string event_type;
//   string message_severity;
//   string message;
//   string message_id;
//   string origin_of_condition;
//   vector<string> message_args;
// } Event_Info;

// typedef struct _Message {
//   string pattern; // id
//   string description;
//   string message;
//   string severity;
//   int number_of_args;
//   vector<string> param_types;
//   string resolution;
// } Message;

// typedef struct _Messages {
//   vector<Message> v_msg;
// } Messages;

typedef struct _Message_For_Registry
{
    string pattern; // id
    string description;
    string message;
    string severity;
    int number_of_args;
    vector<string> param_types;
    string resolution;

    json::value get_json(void);
} Message_For_Registry;

typedef struct _Messages_For_Registry
{
    vector<Message_For_Registry> v_msg;
} Messages_For_Registry;

typedef struct _Part_Location {
  int location_ordinal_value;
  string location_type; // Bay
  string service_label; // the service label of this drive
} Part_Location;

typedef struct _Physical_Location {
  Part_Location part_location;
  string info; // slot number of the drive. if storage is host bus adaptoer or
               // Raid, this property will be displayed
  string info_format; // Slot Number. if storage is host bus adaptoer or Raid,
                      // this property will be displayed
} Physical_Location;

typedef struct _Community_String
{
    string access_mode;
    string community_string;
    string name;
} Community_String;

typedef struct _EngineId
{
    string architectureId;
    string enterpriseSpecificMethod;
    string privateEnterpriseId;
} EngineId;

typedef struct _Snmp
{
    string authentication_protocol;
    string community_access_mode;
    vector<Community_String> community_strings;
    bool enable_SNMPv1;
    bool enable_SNMPv2c;
    bool enable_SNMPv3;
    string encryption_protocol;
    EngineId engine_id;
    bool hide_community_strings;
    int port;
    bool protocol_enabled;
} Snmp;

typedef struct _SyslogFilter
{
    vector<string> logFacilities;
    string lowestSeverity;
} SyslogFilter;

// typedef struct _NTP
// {
//     bool protocol_enabled;
//     int port;
//     string primary_ntp_server;
//     string secondary_ntp_server;
//     vector<string> ntp_servers;
//     string date_str;
//     string time_str;
//     string timezone;
// } NTP;
// ntp.hpp에 들어갈건데 일단 ntp생성전 임시로 생성

typedef struct _SensorMake {
  string id;
  string reading_type;
  string reading_time;
  double reading;
  // 필수값

  string reading_units;
  double reading_range_max;
  double reading_range_min;
  double accuracy;
  double precision;
  string physical_context;
  string sensing_interval;

  Thresholds thresh;
  Status status;
} SensorMake;

typedef struct _Authentication {
  string authentication_type;
  string username;
  string password;
} Authentication;

typedef struct _SearchSetting {
  vector<string> base_distinguished_names;
  string groups_attribute;
  string group_name_attribute;
  string user_name_attribute;

} SearchSetting;

typedef struct _LDAP_Service {
  SearchSetting search_settings;
} LDAP_Service;

typedef struct _LDAP {
  string account_provider_type;
  bool password_set;
  bool service_enabled;
  int port;
  vector<string> service_addresses;

  Authentication authentication;
  LDAP_Service ldap_service;
} LDAP;

typedef struct _ActiveDirectory {
  string account_provider_type;
  bool service_enabled;
  int port;
  vector<string> service_addresses;

  Authentication authentication;
} ActiveDirectory;

typedef struct _SMTP {
  string smtp_machineName;
  string smtp_sender_address;
  string smtp_server;
  string smtp_username;
  string smtp_password;
  
  string smtp_second_server;
  string smtp_second_username;
  string smtp_second_password;

  int smtp_port;
  bool smtp_smtp_enabled;
} SMTP;

/**
 * @brief Resource of redfish schema
 */
class Resource {
public:
  OData odata;
  string name;
  uint8_t type;

  // Class constructor, destructor oveloading
  Resource(const string _odata_id) {
    this->name = "";
    this->type = -1;
    this->odata.id = _odata_id;
    this->odata.type = "";
  };
  Resource(const uint8_t _type, const string _odata_id) {
    this->name = "";
    this->type = _type;
    this->odata.id = _odata_id;
    this->odata.type = "";
  };
  Resource(const uint8_t _type, const string _odata_id,
           const string _odata_type)
      : Resource(_type, _odata_id) {
    this->odata.type = _odata_type;
  };
  virtual ~Resource(){
      // cout << odata.id << ": Resource Destructed.." << endl;
  };

  json::value get_json(void);
  json::value get_json(int type);
  json::value get_odata_id_json(void);
  bool save_json(void);
  bool load_json(json::value &j);
  bool load_json(json::value &j, int type);
  bool load_json_from_file(json::value &j);
};

/**
 * @brief Resource map of resource
 */
extern unordered_map<string, Resource *> g_record;

/**
 * @brief Root of resource
 *        /redfish/v1/Systems
 *        /redfish/v1/Chassis
 *        /redfish/v1/Managers
 *        /redfish/v1/AccountService/Accounts
 *        /redfish/v1/SessionService/Sessions
 */
class Collection : public Resource {
public:
  vector<Resource *> members;

  // Class constructor, destructor oveloading
  Collection(const string _odata_id) : Resource(COLLECTION_TYPE, _odata_id) {
    g_record[_odata_id] = this;
  }
  Collection(const string _odata_id, const string _odata_type)
      : Collection(_odata_id) {
    this->odata.type = _odata_type;
  };
  ~Collection() { g_record.erase(this->odata.id); };

  json::value find_member(string key);
  void add_member(Resource *);
  json::value get_json(void);
  vector<Resource *> *get_member(void);
  bool load_json(json::value &j);
};

/**
 * @brief List of resource
 *        /redfish/v1/Chassis/#/Thermal/Temperatures
 *        /redfish/v1/Chassis/#/Thermal/Fans
 */
class List : public Resource {
public:
  vector<Resource *> members;
  uint8_t member_type;

  // Class constructor, destructor oveloading
  List(const string _odata_id)
      : Resource(LIST_TYPE, _odata_id, ODATA_LIST_TYPE) {
    this->member_type = SUPER_TYPE;
    g_record[_odata_id] = this;
  }
  List(const string _odata_id, const uint8_t _member_type) : List(_odata_id) {
    this->member_type = _member_type;
  };

  ~List() { g_record.erase(this->odata.id); };

  void add_member(Resource *);
  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Resource of MessageRegistry
 *
 */

class MessageRegistry : public Resource {
public:
  string id;       // MessageId는 여기 이 id + message pattern
  string language; // required
  string registry_prefix;  // required
  string registry_version; // required

  Messages_For_Registry messages;

  MessageRegistry(const string _odata_id)
      : Resource(MESSAGE_REGISTRY_TYPE, _odata_id,
                 ODATA_MESSAGE_REGISTRY_TYPE) {
    g_record[_odata_id] = this;
  }
  MessageRegistry(const string _odata_id, const string _registry_id)
      : MessageRegistry(_odata_id) {
    this->id = _registry_id;
  }
  ~MessageRegistry() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of account
 */
class Role : public Resource {
public:
  string id;
  bool is_predefined;
  vector<string> assigned_privileges;

  Role(const string _odata_id)
      : Resource(ROLE_TYPE, _odata_id, ODATA_ROLE_TYPE) {
    g_record[_odata_id] = this;
  }
  Role(const string _odata_id, const string _role_id) : Role(_odata_id) {
    this->id = _role_id;
  }
  ~Role() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of account
 */
class Account : public Resource {
public:
  string id;
  bool enabled;
  string password;
  string user_name;
  bool locked;
  Role *role;
  string role_id;

  int ipmi;
  int priv;
  int link_auth;
  int callin;

  Collection *certificates;

  Account(const string _odata_id)
      : Resource(ACCOUNT_TYPE, _odata_id, ODATA_ACCOUNT_TYPE) {
    this->enabled = true;
    this->locked = false;

    this->role = nullptr;
    this->certificates = nullptr;

    g_record[_odata_id] = this;
  }
  Account(const string _odata_id, const string _account_id)
      : Account(_odata_id) {
    this->id = _account_id;
  }
  Account(const string _odata_id, const string _account_id,
          const string _role_id)
      : Account(_odata_id, _account_id) {
    string odata_id;
    odata_id = ODATA_ROLE_ID;
    odata_id = odata_id + '/' + _role_id;

    if (record_is_exist(odata_id)) {
      this->role = (Role *)g_record[odata_id];
      this->role_id = this->role->odata.id;
    } else {
      this->role = nullptr;
    }
  };
  ~Account() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of account service
 **/

class AccountService : public Resource {
public:
  string id;
  Status status;
  bool service_enabled;
  unsigned int auth_failure_logging_threshold;
  unsigned int min_password_length;
  unsigned int max_password_length;
  unsigned int account_lockout_threshold;
  unsigned int account_lockout_duration;
  unsigned int account_lockout_counter_reset_after;
  bool account_lockout_counter_reset_enabled;
  // unsigned int account_lockout_counter_reset_enabled;
  // threshold(횟수) 로그인실패하면 계정잠금됨
  // reset_enabled가 false면 로그인실패횟수 리셋안되고 계정잠금은 관리자가
  // 풀어줘야함 reset_enabled가 true면 reset_after(초)가 지나면 로그인실패횟수
  // 리셋됨 reset_after는 duration이 최대치인가봄(duration보다 작거나
  // 같은값으로만 설정가능 logging threshold(횟수)는 계정당 인증실패하면 관리자
  // 로그에 기록됨
  Collection *account_collection;
  Collection *role_collection;

  LDAP ldap;
  ActiveDirectory active_directory;

  // Class constructor, destructor oveloading
  AccountService()
      : Resource(ACCOUNT_SERVICE_TYPE, ODATA_ACCOUNT_SERVICE_ID,
                 ODATA_ACCOUNT_SERVICE_TYPE) {
    this->account_collection = nullptr;
    this->role_collection = nullptr;

    g_record[ODATA_ACCOUNT_SERVICE_ID] = this;
  }
  AccountService(const string _odata_id)
      : Resource(ACCOUNT_SERVICE_TYPE, _odata_id, ODATA_ACCOUNT_SERVICE_TYPE) {
    this->account_collection = nullptr;
    this->role_collection = nullptr;

    g_record[_odata_id] = this;
    // 매니저에 들어가는 리모트어카운트서비스의 생성자에 해당하게됨
  };
  ~AccountService() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  bool wrtie_ladap_to_nclcd();
  bool write_ad_to_file();
};

class LogEntry : public Resource {
public:
  string id;
  string entry_type;
  string severity;
  string created;

  Message message; // if EVENT, it contains Event.Message. if SEL, it contains SEL-Specific Message

  // IPMI SEL Variable
  unsigned int sensor_number;
  string sensor_type;
  string entry_code;

  // Redfish EVENT Variable
  string event_id;
  string event_timestamp;    
  string event_type;

  LogEntry(const string _odata_id) : Resource(LOG_ENTRY_TYPE, _odata_id, ODATA_LOG_ENRTY_TYPE) {
    this->created = currentDateTime();

    g_record[_odata_id] = this;
  }
  LogEntry(const string _odata_id, const string _entry_id) : LogEntry(_odata_id) {
    this->id = _entry_id;
  };
  ~LogEntry()
  {
    g_record.erase(this->odata.id);
  };

  json::value get_json(void);
  bool load_json(json::value &j);
  void set_entry_type(string _val);
  void set_severity(string _val);
  void set_message(string _val);
  void set_message_id(string _val);
  void add_message_args(string _val);
  void set_sensor_num(unsigned int _val);
  void set_created(string _val);
};

class LogService : public Resource {
public:
  string id;
  string description;
  string datetime;
  string datetime_local_offset;
  unsigned int max_number_of_records;
  string log_entry_type;
  string overwrite_policy;
  bool service_enabled;
  Status status;
  // unsigned int entry_count;
  vector<SyslogFilter> syslogFilters;

  Collection *entry;
  unsigned int record_count;

  unordered_map<string, Actions> actions;

  LogService(const string _odata_id) : Resource(LOG_SERVICE_TYPE, _odata_id, ODATA_LOG_SERVICE_TYPE) {
    this->entry = nullptr;

    Actions clearlog;
    clearlog.type = CLEAR_LOG;
    clearlog.name = "#LogService.ClearLog";
    clearlog.target = this->odata.id + "/Actions/LogService.ClearLog";

    this->actions["ClearLog"] = clearlog;

    g_record[_odata_id] = this;
  }
  LogService(const string _odata_id, const string _log_service_id)
      : LogService(_odata_id) {
    this->id = _log_service_id;
  };
  ~LogService() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  bool ClearLog();
  // 로그 엔트리생성함수
  LogEntry *new_log_entry(IpmiLogEvent _ev);
  void set_description(string _val);
  void set_datetime(string _val);
  void set_datetime_offset(string _val);
  void set_logentry_type(string _val);
  void set_overwrite_policy(string _val);
  void set_service_enabled(bool _val);
  void set_max_record(unsigned int _val);
};

/**
 * @brief Redfish resource of Event
 * @authors 김
 */
class Event
{
    public:
    string context; // A context can be supplied at subscription time. This property is the context value supplied by the subscriber
    vector<Event_Info> events;

    Event()
    {
        context = "";
        events.clear();
    };
    ~Event(){};

    json::value get_json(void);
};

class SEL
{
    public:
    unsigned int sensor_number;
    string entry_code;
    string sensor_type;
    string event_timestamp;
    string event_type;
  
    Message message;

    json::value get_json(void);
};

class EventDestination : public Resource {
public:
  string id;
  string destination;
  string subscription_type;
  string delivery_retry_policy;
  /*
      available values..
      RetryForever : 구독이 중지되거나 종료되지 않는다. 재전송 최대 횟수를
     넘어도 계속 시도한다. SuspendRetries : 재전송 최대 횟수를 넘으면 구독이
     중지된다. TerminateAfterRetries : 재전송 최대 횟수를 넘으면 구독이
     종료된다.
  */
  vector<string> event_types;
  string protocol;
  string context;

  Status status;

  EventDestination(const string _odata_id)
      : Resource(EVENT_DESTINATION_TYPE, _odata_id,
                 ODATA_EVENT_DESTINATION_TYPE) {
    g_record[_odata_id] = this;
  }
  EventDestination(const string _odata_id, const string _dest_id)
      : EventDestination(_odata_id) {
    this->id = _dest_id;
  };
  ~EventDestination() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class EventService : public Resource {
public:
  string id;
  bool service_enabled;
  int delivery_retry_attempts;
  unsigned int delivery_retry_interval_seconds;
  vector<string> event_types_for_subscription;
  string serversent_event_uri;

  SSE_filter sse;
  Status status;
  Collection *subscriptions;

  SMTP smtp;

  unordered_map<string, Actions> actions;

  EventService()
      : Resource(EVENT_SERVICE_TYPE, ODATA_EVENT_SERVICE_ID,
                 ODATA_EVENT_SERVICE_TYPE) {
    this->subscriptions = nullptr;

    Actions submit_test_event;
    submit_test_event.type = SUBMIT_TEST_EVENT;
    submit_test_event.name = "#EventService.SubmitTestEvent";
    submit_test_event.target =
        this->odata.id + "Actions/EventService.SubmitTestEvent";

    this->actions["SubmitTestEvent"] = submit_test_event;

    g_record[ODATA_EVENT_SERVICE_ID] = this;
  };
  ~EventService() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  json::value SubmitTestEvent(json::value body);
};

/**
 * @brief Redfish resource of Update Service
 * @authors 강
 */
class SoftwareInventory : public Resource {
public:
  string id;
  bool updatable;
  string manufacturer;
  string release_date;
  string version;
  string software_id;
  string lowest_supported_version;
  vector<string> uefi_device_paths;

  Status status;
  unordered_map<string, Actions> actions;

  SoftwareInventory(const string _odata_id) : Resource(SOFTWARE_INVENTORY_TYPE, _odata_id, ODATA_SOFTWARE_INVENTORY_TYPE) {

    Actions firmware_update;
    firmware_update.type = FIRMWARE_UPDATE;
    firmware_update.name = "#UpdateService.FirmwareUpdate";
    firmware_update.target = this->odata.id + "/Actions/UpdateService.FirmwareUpdate";

    this->actions["FirmwareUpdate"] = firmware_update;

    Actions software_update;
    software_update.type = SOFTWARE_UPDATE;
    software_update.name = "#UpdateService.SoftwareUpdate";
    software_update.target = this->odata.id + "/Actions/UpdateService.SoftwareUpdate";

    this->actions["SoftwareUpdate"] = software_update;

    g_record[_odata_id] = this;
  }
  SoftwareInventory(const string _odata_id, const string _soft_id) : SoftwareInventory(_odata_id) {
    this->id = _soft_id;
  };
  ~SoftwareInventory() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class UpdateService : public Resource {
public:
  string id;
  bool service_enabled;
  string http_push_uri;

  Status status;

  Collection *firmware_inventory;
  Collection *software_inventory;

  unordered_map<string, Actions> actions;

  UpdateService(const string _odata_id) : Resource(UPDATE_SERVICE_TYPE, _odata_id, ODATA_UPDATE_SERVICE_TYPE) {
    this->firmware_inventory = nullptr;
    this->software_inventory = nullptr;

    // Actions simple_update;
    // simple_update.type = SIMPLE_UPDATE;
    // simple_update.name = "#UpdateService.SimpleUpdate";
    // simple_update.target =
    //     this->odata.id + "/Actions/UpdateService.SimpleUpdate";

    // Parameter transfer_protocol;
    // transfer_protocol.name = "TransferProtocol";
    // transfer_protocol.allowable_values.push_back("TFTP");
    // transfer_protocol.allowable_values.push_back("SFTP");

    // // target 변경 가능
    // Parameter targets;
    // targets.name = "Targets";
    // targets.allowable_values.push_back(
    //     "/redfish/v1/UpdateService/FirmwareInventory/BMC-Backup");

    // simple_update.parameters.push_back(transfer_protocol);
    // simple_update.parameters.push_back(targets);

    // this->actions["SimpleUpdate"] = simple_update;

    Actions resource_backup, resource_restore;

    resource_backup.type = RESOURCE_BACKUP;
    resource_backup.name = "#UpdateService.ResourceBackUp";
    resource_backup.target = this->odata.id + "/Actions/UpdateService.ResourceBackUp";

    resource_restore.type = RESOURCE_RESTORE;
    resource_restore.name = "#UpdateService.ResourceRestore";
    resource_restore.target = this->odata.id + "/Actions/UpdateService.ResourceRestore";

    this->actions["ResourceBackUp"] = resource_backup;
    this->actions["ResourceRestore"] = resource_restore;

    g_record[_odata_id] = this;
  };
  ~UpdateService() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  bool SimpleUpdate(json::value body);
};

class Certificate : public Resource {
public:
  string id;
  string certificateString;
  /* available certificate type
      0. PEM
      1. PKCS7                    */
  string certificateType;
  string validNotBefore; // The date when the certificate becomes valid
  string validNotAfter;  // The date when the certificate is no longer valid
  CertContent issuer;
  CertContent subject;
  vector<string> keyUsage;

  // 기철need
  string email;
  int key_bit_length;

  unordered_map<string, Actions> actions;

  Certificate(const string _odata_id)
      : Resource(CERTIFICATE_TYPE, _odata_id, ODATA_CERTIFICATE_TYPE) {
    Actions rekey;
    rekey.type = RE_KEY;
    rekey.name = "#Certificate.Rekey";
    rekey.target = this->odata.id + "Actions/Certificate.ReKey";

    // required, allowable_values 바뀔 수 있음
    Parameter key_curve_id;
    key_curve_id.name = "KeyCurveId";
    key_curve_id.allowable_values.push_back("TPM_ECC_NIST_P384");

    Parameter key_pair_algorithm;
    key_pair_algorithm.name = "KeyPairAlgorithm";
    key_pair_algorithm.allowable_values.push_back("TPM_ALG_ECDH");

    rekey.parameters.push_back(key_curve_id);
    rekey.parameters.push_back(key_pair_algorithm);

    this->actions["ReKey"] = rekey;

    Actions renew;
    renew.type = RE_NEW;
    renew.name = "#Certificate.ReNew";
    renew.target = this->odata.id + "Actions/Certificate.ReNew";

    this->actions["ReNew"] = renew;

    g_record[_odata_id] = this;
  }
  Certificate(const string _odata_id, const string _certificateString,
              const string _certificateType)
      : Certificate(_odata_id) {
    this->certificateString = _certificateString;
    this->certificateType = _certificateType;
  };
  ~Certificate() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  json::value Rekey(json::value body);
  json::value Renew(void);
};

class CertificateLocation : public Resource {
public:
  string id;
  vector<Certificate *> certificates;

  CertificateLocation()
      : Resource(CERTIFICATE_LOCATION_TYPE, ODATA_CERTIFICATE_LOCATION_ID,
                 ODATA_CERTIFICATE_LOCATION_TYPE) {
    this->id = "CertificateLocations";
    this->name = "certificate Locations";

    g_record[ODATA_CERTIFICATE_LOCATION_ID] = this;
  };
  ~CertificateLocation() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class CertificateService : public Resource {
public:
  string id;

  CertificateLocation *certificate_location;

  unordered_map<string, Actions> actions;

  CertificateService()
      : Resource(CERTIFICATE_SERVICE_TYPE, ODATA_CERTIFICATE_SERVICE_ID,
                 ODATA_CERTIFICATE_SERVICE_TYPE) {
    this->id = "CertificateService";
    this->name = "Certificate Service";

    this->certificate_location = nullptr;

    Actions generate_CSR;
    generate_CSR.type = GENERATE_CSR;
    generate_CSR.name = "#CertificateService.GenerateCSR";
    generate_CSR.target =
        this->odata.id + "/Actions/CertificateService.GenerateCSR";

    Parameter key_usage;
    key_usage.name = "KeyUsage";
    key_usage.allowable_values.push_back("DigitalSignature");
    key_usage.allowable_values.push_back("NonRepudiation");
    key_usage.allowable_values.push_back("KeyEncipherment");

    Parameter key_curve_id;
    key_curve_id.name = "KeyCurveId";
    key_curve_id.allowable_values.push_back("TPM_ECC_NIST_P384");

    Parameter key_pair_altorithm;
    key_pair_altorithm.name = "KeyPairAlgorithm";
    key_pair_altorithm.allowable_values.push_back("TPM_ALG_ECDH");

    generate_CSR.parameters.push_back(key_usage);
    generate_CSR.parameters.push_back(key_curve_id);
    generate_CSR.parameters.push_back(key_pair_altorithm);

    this->actions["GenerateCSR"] = generate_CSR;

    Actions replace_certificate;
    replace_certificate.type = REPLACE_CERTIFICATE;
    replace_certificate.name = "#CertificateService.ReplaceCertificate";
    replace_certificate.target =
        this->odata.id + "/Actions/CertificateService.ReplaceCertificate";

    Parameter certificate_type;
    certificate_type.name = "CertificateType";
    certificate_type.allowable_values.push_back("PEM");

    replace_certificate.parameters.push_back(certificate_type);

    this->actions["ReplaceCertificate"] = replace_certificate;

    g_record[this->odata.id] = this;
  }
  ~CertificateService() { g_record.erase(this->odata.id); }

  json::value get_json(void);
  bool load_json(json::value &j);
  json::value GenerateCSR(json::value body);
  bool ReplaceCertificate(json::value body);
};

/**
 * @brief Redfish resource of managers
 * @authors 강
 * @details
 */
class NetworkProtocol : public Resource {
public:
  string id;
  string hostname;
  string description;
  string fqdn;
  string name;
  // bool snmp_enabled;
  bool ipmi_enabled;
  // bool ntp_enabled;
  bool kvmip_enabled;
  bool https_enabled;
  bool http_enabled;
  bool virtual_media_enabled;
  bool ssh_enabled;
  // int snmp_port;
  int ipmi_port;
  // int ntp_port;
  int kvmip_port;
  int https_port;
  int http_port;
  int virtual_media_port;
  int ssh_port;

  Snmp snmp;
  NTP ntp;

  // telnet, ssdp

  vector<string> v_netservers;

  Status status;

  Collection *certificates;

  NetworkProtocol(const string _odata_id)
      : Resource(NETWORK_PROTOCOL_TYPE, _odata_id,
                 ODATA_NETWORK_PROTOCOL_TYPE) {
    g_record[_odata_id] = this;
    this->certificates = nullptr;
  }
  NetworkProtocol(const string _odata_id, const string _network_id)
      : NetworkProtocol(_odata_id) {
    this->id = "NIC";
    if (_network_id != "0")
      this->id += _network_id;
  };
  ~NetworkProtocol() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class EthernetInterfaces : public Resource {
public:
  string id;
  string name;
  string description;
  string link_status;
  string permanent_mac_address;
  string mac_address;
  unsigned int speed_Mbps;
  bool autoneg;
  bool full_duplex;
  unsigned int mtu_size;
  string hostname;
  string fqdn;
  string ipv6_default_gateway;
  bool interfaceEnabled;

  vector<string> name_servers;
  DHCP_v4 dhcp_v4;
  DHCP_v6 dhcp_v6;
  vector<IPv4_Address> v_ipv4;
  vector<IPv6_Address> v_ipv6;
  Vlan vlan;

  Status status;

  EthernetInterfaces(const string _odata_id) : Resource(ETHERNET_INTERFACE_TYPE, _odata_id,
                 ODATA_ETHERNET_INTERFACE_TYPE) {
    g_record[_odata_id] = this;
  }
  EthernetInterfaces(const string _odata_id, const string _ether_id)
      : EthernetInterfaces(_odata_id) {
    this->id = _ether_id;
  };
  ~EthernetInterfaces() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish Resource of Radius
 *
 */

class Radius : public Resource {
public:
  string id;
  string radius_server;
  string radius_secret;
  int radius_port;
  bool radius_enabled;

  Radius(const string _odata_id)
      : Resource(RADIUS_TYPE, _odata_id, ODATA_RADIUS_TYPE) {
    this->id = "Radius";
    g_record[_odata_id] = this;
  };
  ~Radius() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of Manager
 * @todo 구현안된 ㅇinit요소 가있음 redfish 참고할것
 * @author 기철
 */
class Manager : public Resource {
public:
  string id;
  string name;
  string manager_type;
  string description;
  string uuid;
  string model;
  string firmware_version;
  string datetime;
  string datetime_offset;
  string power_state;

  Status status;

  Collection *ethernet;
  Collection *log_service;
  Collection *virtual_media;

  AccountService
      *remote_account_service; // BMC의 계정정보를 관리하는 AccountService
  NetworkProtocol *network;

  Radius *radius;

  unordered_map<string, Actions> actions;

  Ipmifru *fru_this;

  int ethmember;

  /**
   * @brief Construct a new Manager object
   *
   * @param _odata_id
   * @param _Manager_id
   * @bug asset tag 가 dcmiㅎ를 통해얻어오지못함 버그해결함
   * @author 기철
   */
  Manager(const string _odata_id)
      : Resource(MANAGER_TYPE, _odata_id, ODATA_MANAGER_TYPE) {
    this->ethernet = nullptr;
    this->log_service = nullptr;
    this->remote_account_service = nullptr;
    this->network = nullptr;
    this->virtual_media = nullptr;

    this->radius = nullptr;

    Actions reset;
    reset.type = RESET_MANAGER;
    reset.name = "#Manager.Reset";
    reset.target = this->odata.id + "/Actions/Manager.Reset";

    Parameter reset_type;
    reset_type.name = "ResetType";
    reset_type.allowable_values.push_back("GracefulRestart");
    reset_type.allowable_values.push_back("ForceRestart");

    reset.parameters.push_back(reset_type);

    this->actions["Reset"] = reset;

    g_record[_odata_id] = this;
  };
  Manager(const string _odata_id, const string _manager_id)
      : Manager(_odata_id) {
    this->id = _manager_id;
  };
  ~Manager() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  bool Reset(json::value body);
  void new_log_service(string _service_id);
};

/**
 * @brief Redfish Resource of Task
 *
 */
class Task : public Resource {
public:
  string id;
  string start_time;
  string end_time;
  string task_state;
  string task_status;
  Payload payload;

  Task(const string _odata_id)
      : Resource(TASK_TYPE, _odata_id, ODATA_TASK_TYPE) {
    this->name = "Task";
    this->id = "";
    this->start_time = currentDateTime();
    this->end_time = "";
    this->task_state = TASK_STATE_NEW;
    this->task_status = TASK_STATUS_OK;
    // starttime, state, status는 임의로 넣어놓음

    g_record[_odata_id] = this;
  }
  Task(const string _odata_id, const string _task_id) : Task(_odata_id) {
    // ((Collection *)g_record[ODATA_TASK_ID])->add_member(this);
    this->id = _task_id;
  };
  ~Task() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  void set_payload(http_headers _header, string _operation, json::value _json,
                   string _target_uri);
  // pplx가 필요한구조인가..?
};

/**
 * @brief Redfish Resource of Task Service
 * @author 강
 *
 */
class TaskService : public Resource {
public:
  string id;
  string datetime;
  bool service_enabled;
  Status status;
  Collection *task_collection;
  // string overwrite_policy; // enum , 완료된 task overwrite
  // bool event_on_change; // task state바뀌었을 때 이벤트 전송여부

  TaskService()
      : Resource(TASK_SERVICE_TYPE, ODATA_TASK_SERVICE_ID,
                 ODATA_TASK_SERVICE_TYPE) {
    this->task_collection = nullptr;

    g_record[ODATA_TASK_SERVICE_ID] = this;
  };
  ~TaskService() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of session service
 */
class SessionService : public Resource {
public:
  string id;
  Status status;
  bool service_enabled;
  unsigned int session_timeout;
  Collection *session_collection;

  // Class constructor, destructor oveloading
  SessionService()
      : Resource(SESSION_SERVICE_TYPE, ODATA_SESSION_SERVICE_ID,
                 ODATA_SESSION_SERVICE_TYPE) {
    this->session_collection = nullptr;

    g_record[ODATA_SESSION_SERVICE_ID] = this;
  };
  ~SessionService() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of session
 */
class Session : public Resource {
public:
  string id;
  string account_id;
  Account *account;
  unsigned int _remain_expires_time;
  // @@@@@@@@ authors 강
  pplx::cancellation_token_source cts;
  pplx::cancellation_token c_token = cts.get_token();

  string x_token;

  // Class constructor, destructor oveloading
  Session(const string _odata_id)
      : Resource(SESSION_TYPE, _odata_id, ODATA_SESSION_TYPE) {
    this->name = "User Session";
    this->id = "";
    this->account = nullptr;

    g_record[_odata_id] = this;
  }
  Session(const string _odata_id, const string _session_id, Account *_account)
      : Session(_odata_id) {
    this->id = _session_id;
    this->account = _account;
    this->account_id = _account->odata.id;
    this->_remain_expires_time =
        ((SessionService *)g_record[ODATA_SESSION_SERVICE_ID])->session_timeout;
    // cout << "\t\t\t\t\t\t\\t" << _account->odata.id << "kk" << endl;
    // ((Collection *)g_record[ODATA_SESSION_ID])->add_member(this);
  };
  ~Session() { g_record.erase(this->odata.id); };
  json::value get_json(void);
  bool load_json(json::value &j);
  pplx::task<void> start(void);
};

/**
 * @brief Redfish resource of temperature
 */
class Temperature : public Resource {
public:
  string member_id;
  Status status;
  int sensor_num;
  double reading_celsius;
  double upper_threshold_non_critical;
  double upper_threshold_critical;
  double upper_threshold_fatal;
  double lower_threshold_non_critical;
  double lower_threshold_critical;
  double lower_threshold_fatal;
  double min_reading_range_temp;
  double max_reading_range_temp;
  string physical_context;

  // Class constructor, destructor oveloading
  Temperature(const string _odata_id)
      : Resource(TEMPERATURE_TYPE, _odata_id, ODATA_THERMAL_TYPE) {
    g_record[_odata_id] = this;
  }
  Temperature(const string _odata_id, const string _member_id)
      : Temperature(_odata_id) {
    this->member_id = _member_id;
  };
  ~Temperature() {
    this->thread = false;
    g_record.erase(this->odata.id);
  }

  json::value get_json(void);
  bool load_json(json::value &j);

private:
  bool thread;
};

/**
 * @brief Redfish resource of temperature
 */
class Sensor : public Resource {
public:
  string id;
  string reading_type; // 센서타입
  string reading_time;

  double reading;
  string reading_units; // 측정단위
  double reading_range_max;
  double reading_range_min;
  double accuracy;  // 오차율(%)
  double precision; // reading값의 유효숫자인듯
  string physical_context;
  string sensing_interval; // 센서값의 시간간격이라는데 왜 스트링?

  Thresholds thresh;
  Status status;

  Sensor(const string _odata_id)
      : Resource(SENSOR_TYPE, _odata_id, ODATA_SENSOR_TYPE) {
    g_record[_odata_id] = this;
    id = "";
    reading_type = "";
    reading_units = "";
    physical_context = "";
    sensing_interval = "";
    reading = 0.0;
    reading_range_max = 0.0;
    reading_range_min = 0.0;
    accuracy = 0.0;
    precision = 0.0;
    thresh.lower_caution.reading = 0.0;
    thresh.lower_critical.reading = 0.0;
    thresh.lower_fatal.reading = 0.0;
    thresh.upper_caution.reading = 0.0;
    thresh.upper_critical.reading = 0.0;
    thresh.upper_fatal.reading = 0.0;
  }
  Sensor(const string _odata_id, const string _sensor_id) : Sensor(_odata_id) {
    this->id = _sensor_id;
  };
  ~Sensor() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of temperature
 */
class Fan : public Resource {
public:
  string member_id;
  Status status;
  int sensor_num;
  int reading;
  string reading_units;

  int upper_threshold_non_critical;
  int upper_threshold_critical;
  int upper_threshold_fatal;
  int lower_threshold_non_critical;
  int lower_threshold_critical;
  int lower_threshold_fatal;
  int min_reading_range;
  int max_reading_range;
  string physical_context;

  // Class constructor, destructor oveloading
  Fan(const string _odata_id) : Resource(FAN_TYPE, _odata_id) {
    g_record[_odata_id] = this;
  }
  Fan(const string _odata_id, const string _member_id) : Fan(_odata_id) {
    this->member_id = _member_id;
  };
  ~Fan() { g_record.erase(this->odata.id); }

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of thermal
 *
 */
class Thermal : public Resource {
public:
  string id;
  List *temperatures;
  List *fans;

  // Class constructor, destructor oveloading
  Thermal(const string _odata_id)
      : Resource(THERMAL_TYPE, _odata_id, ODATA_THERMAL_TYPE) {
    this->id = "Thermal";
    this->temperatures = nullptr;
    this->fans = nullptr;

    g_record[_odata_id] = this;
  };
  ~Thermal() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Power 관련 resource
 * @authors 강
 */
class Voltage : public Resource {
public:
  string member_id;
  int sensor_num;
  double reading_volts;
  double upper_threshold_non_critical;
  double upper_threshold_critical;
  double upper_threshold_fatal;
  double lower_threshold_non_critical;
  double lower_threshold_critical;
  double lower_threshold_fatal;
  double min_reading_range;
  double max_reading_range;
  string physical_context;

  Status status;

  Voltage(const string _odata_id)
      : Resource(VOLTAGE_TYPE, _odata_id, ODATA_POWER_TYPE) {
    g_record[_odata_id] = this;
  }
  Voltage(const string _odata_id, const string _member_id)
      : Voltage(_odata_id) {
    this->member_id = _member_id;
  };
  ~Voltage() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class PowerSupply : public Resource {
public:
  string member_id;
  string power_supply_type;
  string line_input_voltage_type;
  double line_input_voltage;
  double power_capacity_watts;
  double last_power_output_watts;

  string model;
  string manufacturer;
  string firmware_version;
  string serial_number;
  string part_number;
  string spare_part_number;

  vector<InputRange> input_ranges;

  Status status;

  PowerSupply(const string _odata_id)
      : Resource(POWER_SUPPLY_TYPE, _odata_id, ODATA_POWER_TYPE) {
    g_record[_odata_id] = this;
  }
  PowerSupply(const string _odata_id, const string _member_id)
      : PowerSupply(_odata_id) {
    this->member_id = _member_id;
  };
  ~PowerSupply() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class PowerControl : public Resource {
public:
  string member_id;
  double power_consumed_watts;
  double power_requested_watts;
  double power_available_watts;
  double power_capacity_watts;
  double power_allocated_watts;

  PowerLimit power_limit;
  PowerMetrics power_metrics;
  Status status;

  PowerControl(const string _odata_id)
      : Resource(POWER_CONTROL_TYPE, _odata_id, ODATA_POWER_TYPE) {
    g_record[_odata_id] = this;
  }
  PowerControl(const string _odata_id, const string _member_id)
      : PowerControl(_odata_id) {
    this->member_id = _member_id;
  };
  ~PowerControl() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class Power : public Resource {
public:
  string id;
  List *power_control;
  List *voltages;
  List *power_supplies;

  Power(const string _odata_id)
      : Resource(POWER_TYPE, _odata_id, ODATA_POWER_TYPE) {
    this->id = "Power";
    this->power_control = nullptr;
    this->voltages = nullptr;
    this->power_supplies = nullptr;

    g_record[_odata_id] = this;
  };
  ~Power() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief 시스템 리소스에 필요한 storage, bios, simplestorage, processors..
 * processorsummary,networkinterface는뺌
 * @authors 강
 *
 */
class Bios : public Resource {
public:
  string id;
  string attribute_registry;
  Attribute attribute;
  unordered_map<string, Actions> actions;

  Bios(const string _odata_id)
      : Resource(BIOS_TYPE, _odata_id, ODATA_BIOS_TYPE) {

    this->id = "";
    this->attribute_registry = "";
    this->attribute_registry = "";
    this->attribute.boot_mode = "";
    this->attribute.embedded_sata = "";
    this->attribute.nic_boot1 = "";
    this->attribute.nic_boot2 = "";
    this->attribute.power_profile = "";
    this->attribute.proc_core_disable = 0;
    this->attribute.proc_hyper_threading = "";
    this->attribute.proc_turbo_mode = "";
    this->attribute.usb_control = "";

    Actions reset_bios;
    reset_bios.type = RESET_BIOS;
    reset_bios.name = "#Bios.ResetBios";
    reset_bios.target = this->odata.id + "/Actions/Bios.ResetBios";

    Actions change_password;
    change_password.type = CHANGE_PASSWORD;
    change_password.name = "#Bios.ChangePassword";
    change_password.target = this->odata.id + "/Actions/Bios.ChangePassword";

    Parameter password_name;
    password_name.name = "PasswordName";
    password_name.allowable_values.push_back("UefiAdminPassword");
    password_name.allowable_values.push_back("UefiPowerOnPassword");

    change_password.parameters.push_back(password_name);

    this->actions["ResetBios"] = reset_bios;
    this->actions["ChangePassword"] = change_password;

    g_record[_odata_id] = this;
  }
  Bios(const string _odata_id, const string _bios_id) : Bios(_odata_id) {
    this->id = _bios_id;
  };
  ~Bios() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  bool ChangePassword(string new_password, string old_password,
                      string password_name);
  bool ResetBios();
};

class SimpleStorage : public Resource {
public:
  string id;
  string description;
  string uefi_device_path;
  Status status;

  vector<Device_Info> devices;

  SimpleStorage(const string _odata_id)
      : Resource(SIMPLE_STORAGE_TYPE, _odata_id, ODATA_SIMPLE_STORAGE_TYPE) {
    g_record[_odata_id] = this;
  }
  SimpleStorage(const string _odata_id, const string _simple_id)
      : SimpleStorage(_odata_id) {
    this->id = _simple_id;
  };
  ~SimpleStorage() { g_record.erase(this->odata.id); }

  json::value get_json(void);
  bool load_json(json::value &j);
};

class StorageControllers : public Resource {
public:
  string id;
  string manufacturer;
  string model;
  string serial_number;
  string part_number;
  double speed_gbps;
  string firmware_version;
  Identifier identifier;
  vector<string> support_controller_protocols;
  vector<string> support_device_protocols;
  Status status;

  StorageControllers(const string _odata_id)
      : Resource(STORAGE_CONTROLLER_TYPE, _odata_id,
                 ODATA_STORAGE_CONTROLLER_TYPE) {
    g_record[_odata_id] = this;
  }
  StorageControllers(const string _odata_id, const string _controller_id)
      : StorageControllers(_odata_id) {
    this->id = _controller_id;
  };
  ~StorageControllers() { g_record.erase(this->odata.id); }

  json::value get_json(void);
  bool load_json(json::value &j);
};

class Storage : public Resource {
public:
  string id;
  string description;
  Status status;
  List *controller;
  Collection *drives;  // physical
  Collection *volumes; // logical

  Storage(const string _odata_id)
      : Resource(STORAGE_TYPE, _odata_id, ODATA_STORAGE_TYPE) {
    this->id = "";
    this->description = "";
    this->status.health = STATUS_HEALTH_OK;
    this->status.state = STATUS_STATE_ENABLED;

    this->controller = nullptr;
    this->drives = nullptr;
    this->volumes = nullptr;

    g_record[_odata_id] = this;
  };
  Storage(const string _odata_id, const string _storage_id)
      : Storage(_odata_id) {
    this->id = _storage_id;
  };
  ~Storage() { g_record.erase(this->odata.id); };

  json::value get_json(
      void); // 이거 할때 컨트롤러 리스트기때문에 Thermal-temperature 참고할것
  bool load_json(json::value &j);
};

class Drive : public Resource {
public:
  string id;
  string asset_tag;
  string description;
  string encryption_ability; // One of ("None", "SelfEncryptingDrive")
  string encryption_status;  // One of ("Unlocked", "Locked", "Unencrypted")
  string hotspare_type;      // One of ("None", "Global")
  string manufacturer;
  string media_type;
  string model;
  string name;
  string sku;
  string status_indicator;
  string part_number;
  string protocol;
  string revision;
  string serial_number;

  int block_size_bytes;
  int capable_speed_Gbs;
  int negotiated_speed_Gbs;
  int predicted_media_life_left_percent;
  int rotation_speed_RPM;

  bool failure_predicted;

  vector<Identifier> identifier;
  Physical_Location physical_location;

  Status status;

  Drive(const string _odata_id)
      : Resource(DRIVE_TYPE, _odata_id, ODATA_DRIVE_TYPE) {
    g_record[_odata_id] = this;
  };
  Drive(const string _odata_id, string _id) : Drive(_odata_id) {
    this->id = _id;
  };
  ~Drive() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class Volume : public Resource {
public:
  string id;
  string description;
  string RAID_type;
  string name;
  string read_cache_policy;
  string write_cache_policy;
  string strip_size_bytes;
  string display_name;
  int block_size_bytes;
  int capacity_bytes;

  vector<string> access_capabilities;

  Status status;

  Volume(const string _odata_id)
      : Resource(VOLUME_TYPE, _odata_id, ODATA_VOLUME_TYPE) {
    g_record[_odata_id] = this;
  };
  Volume(const string _odata_id, string _id) : Volume(_odata_id) {
    this->id = _id;
  };
  ~Volume() { g_record.erase(this->odata.id); };
  json::value get_json(void);
  bool load_json(json::value &j);
};

class Processors : public Resource {
public:
  string id;
  string socket;
  string processor_type;
  string processor_architecture;
  string instruction_set;
  string manufacturer;
  string model;
  int max_speed_mhz;
  int total_cores;
  int total_threads;
  Status status;
  // string UUID?
  ProcessorId p_id;

  Processors(const string _odata_id)
      : Resource(PROCESSOR_TYPE, _odata_id, ODATA_PROCESSOR_TYPE) {
    g_record[_odata_id] = this;
  }
  Processors(const string _odata_id, const string _processor_id)
      : Processors(_odata_id) {
    this->id = _processor_id;
  };
  ~Processors() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

class Memory : public Resource {
public:
  string id;
  unsigned int rank_count;
  unsigned int capacity_kib;
  unsigned int data_width_bits;
  unsigned int bus_width_bits;
  string error_correction;
  MemoryLocation m_location;
  string memory_type;
  string memory_device_type;
  string base_module_type;
  vector<string> memory_media;
  vector<unsigned int> max_TDP_milliwatts;
  Status status;

  Memory(const string _odata_id)
      : Resource(MEMORY_TYPE, _odata_id, ODATA_MEMORY_TYPE) {
    g_record[_odata_id] = this;
  }
  Memory(const string _odata_id, const string _memory_id) : Memory(_odata_id) {
    this->id = _memory_id;
  };
  ~Memory() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
};

/**
 * @brief Redfish resource of chassis
 */
class Systems : public Resource {
public:
  string id;
  string name;
  // string sku;
  string system_type;
  string manufacturer;
  string model;
  string serial_number;
  string part_number;
  string description;
  string hostname;
  // vector<string> hosting_roles;
  // string submodel;
  string asset_tag;
  string power_state;
  uint8_t indicator_led;
  string bios_version;
  string uuid;
  // Location location;
  // Thermal *thermal;
  // Resource *power;
  // ProcessorSummary *ps; // 구조체로 바꿔야할듯 현재리소슨데
  // MemorySummary ms;
  Bios *bios; // resource Bios
  Status status;
  Boot boot;

  // Collection *network; // resource NetworkInterfaces // 일단 없음
  Collection *storage;        // resource Storages
  Collection *processor;      // resource Processors
  Collection *memory;         // resource Memory
  Collection *ethernet;       // resource EthernetInterfaces
  Collection *log_service;    // resource LogService
  Collection *simple_storage; // resource SimpleStorage

  unordered_map<string, Actions> actions;

  Ipmifru *fru_this;

  Systems(const string _odata_id)
      : Resource(SYSTEM_TYPE, _odata_id, ODATA_SYSTEM_TYPE) {
    this->bios = nullptr;
    this->storage = nullptr;
    this->processor = nullptr;
    this->memory = nullptr;
    this->ethernet = nullptr;
    this->log_service = nullptr;
    this->simple_storage = nullptr;

    Actions reset;
    reset.type = RESET_SYSTEM;
    reset.name = "#ComputerSystem.Reset";
    reset.target = this->odata.id + "/Actions/ComputerSystem.Reset";

    Parameter reset_type;
    reset_type.name = "ResetType";
    reset_type.allowable_values.push_back("On");
    reset_type.allowable_values.push_back("ForceOff");
    reset_type.allowable_values.push_back("GracefulShutdown");
    reset_type.allowable_values.push_back("GracefulRestart");

    reset.parameters.push_back(reset_type);

    this->actions["Reset"] = reset;

    g_record[_odata_id] = this;
  }
  Systems(const string _odata_id, const string _systems_id)
      : Systems(_odata_id) {
    this->id = _systems_id;
  };
  ~Systems() { g_record.erase(this->odata.id); };

  bool load_json(json::value &j);
  json::value get_json(void);
  bool Reset(json::value body);
  void new_log_service(string _service_id);
};

/**
 * @brief Redfish resource of chassis
 */
class Chassis : public Resource {
public:
  string id;
  string chassis_type;
  string manufacturer;
  string model;
  // string sku;
  string serial_number;
  string part_number;
  string asset_tag;
  string power_state;
  uint8_t indicator_led;
  Status status;
  Location location;

  // TODO 리소스 변경 필요
  Thermal *thermal;
  Power *power;
  // Storage *storage;
  Collection *storage;

  Collection *sensors;
  Collection *log_service;

  Ipmifru *fru_this;

  // TODO Contains, ManagedBy 추가 필요
  Chassis(const string _odata_id)
      : Resource(CHASSIS_TYPE, _odata_id, ODATA_CHASSIS_TYPE) {
    this->thermal = nullptr;
    this->power = nullptr;
    this->storage = nullptr;
    this->sensors = nullptr;
    this->log_service = nullptr;

    g_record[_odata_id] = this;
  }
  Chassis(const string _odata_id, const string _chassis_id)
      : Chassis(_odata_id) {
    this->id = _chassis_id;
  };
  ~Chassis() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);

  pplx::task<void> led_off(uint8_t _led_index);
  pplx::task<void> led_lit(uint8_t _led_index);
  pplx::task<void> led_blinking(uint8_t _led_index);
  void new_log_service(string _service_id);
};

class VirtualMedia : public Resource {
public:
  string id;
  string image;
  string image_name;
  vector<string> media_type;
  string connected_via;
  bool inserted;
  bool write_protected;
  string user_name;
  string password;
  string size;
  string create_time;

  unordered_map<string, Actions> actions;

  VirtualMedia(const string _odata_id)
      : Resource(VIRTUAL_MEDIA_TYPE, _odata_id, ODATA_VIRTUAL_MEDIA_TYPE) {
    // Actions insert_media;
    // insert_media.type = INSERT_MEDIA;
    // insert_media.name = "#VirtualMedia.InsertMedia";
    // insert_media.target = this->odata.id +
    // "/Actions/VirtualMedia.InsertMedia";

    Actions insert_media_cd;
    insert_media_cd.type = INSERT_MEDIA;
    insert_media_cd.name = "#VirtualMedia.InsertMediaCD";
    insert_media_cd.target =
        this->odata.id + "/Actions/VirtualMedia.InsertMediaCD";

    // Actions insert_media_dvd;
    // insert_media_dvd.type = INSERT_MEDIA;
    // insert_media_dvd.name = "#VirtualMedia.InsertMediaDVD";
    // insert_media_dvd.target = this->odata.id +
    // "/Actions/VirtualMedia.InsertMediaDVD";

    // Actions insert_media_floppy;
    // insert_media_floppy.type = INSERT_MEDIA;
    // insert_media_floppy.name = "#VirtualMedia.InsertMediaFloppy";
    // insert_media_floppy.target = this->odata.id +
    // "/Actions/VirtualMedia.InsertMediaFloppy";

    Actions insert_media_usb;
    insert_media_usb.type = INSERT_MEDIA;
    insert_media_usb.name = "#VirtualMedia.InsertMediaUSB";
    insert_media_usb.target =
        this->odata.id + "/Actions/VirtualMedia.InsertMediaUSB";

    Actions eject_media;
    eject_media.type = EJECT_MEDIA;
    eject_media.name = "#VirtualMedia.EjectMedia";
    eject_media.target = this->odata.id + "/Actions/VirtualMedia.EjectMedia";

    this->actions["InsertMediaCD"] = insert_media_cd;
    this->actions["InsertMediaUSB"] = insert_media_usb;
    // this->actions["InsertMedia"] = insert_media;
    this->actions["EjectMedia"] = eject_media;

    g_record[_odata_id] = this;
  }
  ~VirtualMedia() { g_record.erase(this->odata.id); };

  json::value get_json(void);
  bool load_json(json::value &j);
  json::value InsertMedia(json::value body);
  json::value EjectMedia(void);
};

void init_system(Systems *system_collection, string _id);
void init_storage_collection(Collection *storage_collection, string _id);
void init_storage(Storage *storage);
void init_storage_controller(List *storage_controllers_list, string _id);
void init_processor(Collection *processor_collection, string _id);
void init_memory(Collection *memory_collection, string _id);
void init_ethernet(Collection *ethernet_collection, string _id);
void init_network_protocol(NetworkProtocol *network);
void init_log_service(Collection *log_service_collection, string _id);
void init_log_entry(Collection *log_entry_collection, string _id);
void init_simple_storage(Collection *simple_storage_collection, string _id);
void init_drive(Collection *drive_collection, string _id);
void init_volume(Collection *volume_collection, string _id);
void init_bios(Bios *bios);
void init_chassis(Chassis *chassis_collection, string _id);
void init_sensor(Collection *sensor_collection, string _id);
void init_thermal(Thermal *thermal);
void init_power(Power *power);
void init_temperature(List *temperatures_list, string _id);
void init_fan(List *fans_list, string _id);
void init_power_control(List *power_control_list, string _id);
void init_voltage(List *voltages_list, string _id);
void init_power_supply(List *power_supplies_list, string _id);
void snmp_config_init(Snmp *snmp);
void ntp_config_init(NTP *ntp);
void init_manager(Manager *manager_collection, string _id);
void init_update_service(UpdateService *update_service);
SoftwareInventory *
init_software_inventory(Collection *software_inventory_collection, string _id);
void init_task_service(TaskService *task_service);
void init_event_service(EventService *event_service);
void init_event_destination(Collection *event_destination_collection,
                            string _id);
void init_account_service(AccountService *account_service);
void init_session_service(SessionService *session_service);
void insert_virtual_media(Collection *virtual_media_collection, string _id);
void init_radius(Radius *radius);
void init_dbus_check(void);

/**
 * @brief Root of redfish
 *        This resource create only once
 *        /redfish/v1
 */
class ServiceRoot : public Resource {
public:
  string id;
  string redfish_version;
  string uuid;

  Systems *system;
  Chassis *chassis;
  Manager *manager;

  UpdateService *update_service;
  AccountService *account_service;
  SessionService *session_service;
  TaskService *task_service;
  CertificateService *certificate_service;
  EventService *event_service;

  // Class constructor, destructor oveloading
  ServiceRoot()
      : Resource(SERVICE_ROOT_TYPE, ODATA_SERVICE_ROOT_ID,
                 ODATA_SERVICE_ROOT_TYPE) {
    string odata_id;

    this->id = "RootService";
    this->name = "Root Service";
    this->redfish_version = "1.0.0";
    this->uuid = string(uuid_str);

    // Collection Generate in ServiceRoot
    if (!record_is_exist(ODATA_SYSTEM_ID)) {
      log(info) << "[...]System init";
      system = new Systems(ODATA_SYSTEM_ID, "Systems");
      system->name = "BMC Computer System";

      init_system(system, "Systems");
    }

    if (!record_is_exist(ODATA_CHASSIS_ID)) {
      log(info) << "[...]Chassis init";
      chassis = new Chassis(ODATA_CHASSIS_ID, "Chassis");
      chassis->name = "BMC Chassis";

      init_chassis(chassis, "Chassis");
    }

    if (!record_is_exist(ODATA_MANAGER_ID)) {
      log(info) << "[...]Manager init";
      manager = new Manager(ODATA_MANAGER_ID, "Manager");

      init_manager(manager, "Manager");
    }

    // UpdateService configuration
    if (!record_is_exist(ODATA_UPDATE_SERVICE_ID)) {
      log(info) << "[...]Update Service init";
      update_service = new UpdateService(ODATA_UPDATE_SERVICE_ID);

      init_update_service(update_service);
    }
    // TaskService configuration
    if (!record_is_exist(ODATA_TASK_SERVICE_ID)) {
      log(info) << "[...]Task Service init";
      task_service = new TaskService();

      init_task_service(task_service);
    }

    // EventService configuration
    if (!record_is_exist(ODATA_EVENT_SERVICE_ID)) {
      log(info) << "[...]Event Service init";
      event_service = new EventService();

      init_event_service(event_service);
    }
    if (!record_is_exist(ODATA_CERTIFICATE_SERVICE_ID)) {
      log(info) << "[...]Certificate Service init";
      certificate_service = new CertificateService();
      certificate_service->certificate_location = new CertificateLocation();
    }

    // AccountService configuration
    if (!record_is_exist(ODATA_ACCOUNT_SERVICE_ID)) {
      log(info) << "[...]Account Service init";
      account_service = new AccountService();

      init_account_service(account_service);
    }
    // SessionService configuration
    if (!record_is_exist(ODATA_SESSION_SERVICE_ID)) {
      log(info) << "[...]Session Service init";
      session_service = new SessionService();

      init_session_service(session_service);
    }

    g_record[ODATA_SERVICE_ROOT_ID] = this;
  };
  ~ServiceRoot() { g_record.erase(this->odata.id); };

  json::value get_json(void);
};

bool init_record(void);
bool init_resource(void);

void init_message_registry(void);

bool is_session_valid(const string _token);
string get_session_odata_id_by_token(string _token);
void clear_gc();
void dependency_injection(Resource *res);
void resource_save_json(Resource *Rsrc);
json::value get_resource_odata_id_json(Resource *res, string loc);
json::value get_action_info(unordered_map<string, Actions> act);

// certificate action
void generate_ssl_private_key(fs::path key, string key_length);
json::value generate_CSR_return_result(fs::path conf, fs::path key,
                                       fs::path csr, string target_id);
string file2str(string file_path);
void update_cert_with_pem(fs::path cert, Certificate *certificate);

// virtual media
static int umount();

// networkprotocol
string make_iptable_cmd(string _op, string _pos, int _index, int _port,
                        int _able);
void execute_iptables(NetworkProtocol *_net, int _index, string _op);
void init_iptable(NetworkProtocol *_net);
void patch_iptable(NetworkProtocol *_net);

// logservice 생성 함수 & resource.cpp에 구현부

LogEntry *generate_logentry(IpmiLogEvent _ev);

// Sensor Make
void make_sensor(SensorMake _sm, uint16_t _flag);

void chassis_monitor(void *data);
void systems_monitor(void *data);
void account_monitor(void *data);
void manager_monitor(void *data);

void sessions_monitor(void *data);

#endif