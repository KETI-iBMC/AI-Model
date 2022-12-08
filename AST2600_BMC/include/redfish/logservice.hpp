#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include <redfish/stdafx.hpp>
#include <redfish/resource.hpp>
#include <redfish/eventservice.hpp>
#include "../KETI-Edge/handler.hpp"
#include <sqlite3.h>

// #define LOG_DB "/conf/database/log.db"
#define LOG_DB "/redfish/log.db"

typedef struct _Hour_Reading_Info
{
    int max_count; // items에 들어갈 로그데이터 개수(max) 그리고 그거의 cur_count는 items.size()로 파악해
    string cur_time; // item에 들어간 최신 reading 데이터의 Time정보
    string compare_time; // cur_time 으로 부터 1시간 이전 시간으로 비교기준이 되는 시간
    vector<json::value> items;
} Hour_Reading_Info;


// Database
void make_test_db();

int check_reading_db_callback(void *NotUsed, int argc, char **argv, char **azColName);
int result_callback(void *NotUsed, int argc, char **argv, char **azColName);
int check_same_time_callback(void *NotUsed, int argc, char **argv, char **azColName);

void insert_reading_table(string _sensor_id, string _module, string _type, string _detail, int _value, string _time);
void insert_reading_table(string _sensor_id, string _module, string _type, string _detail, double _value, string _time);
// void insert_reading_table(string _sensor_id, string _type, double _value, string _time, string _module);
// void insert_reading_table(string _sensor_id, string _type, int _value, tm _tm, string _location);
// void insert_reading_table(string _sensor_id, string _type, double _value, tm _tm, string _location);
// 처음 insert함수 리딩테이블에니까  센서id, 센서타입, 센서측정값(), 측정시간(tm구조체), 센서위치(str)
// 이걸 호출하는 로그가 발생하는 곳에서 tm에 localtime 그거만 해서 담아주면됨 
// tm 넘겨주는 거에서 tm을 string으로 바꾼담에 그냥 string 값을 넘겨주는 것으로 변경


int extract_column_callback(void *_vector, int argc, char **argv, char **azColName);
int make_json_min_reading_callback(void *_vector, int argc, char **argv, char **azColName);
int make_json_hour_reading_callback(void *_struct, int argc, char **argv, char **azColName);


json::value select_min_reading(string _module, string _type, string _detail, int _count);
json::value select_hour_reading(string _module, string _type, string _detail, int _count);


// json::value select_all_time_reading(string _module, string _type);
// json::value select_one_hour_reading(string _module, string _type);
// json::value select_half_hour_reading(string _module, string _type);

// json::value select_all_reading(string _type); // type은 uri에 따라서 조절됨
// json::value select_hour_reading(string _type);
/*개수, 시간간격(이건 로그를 시간정해놓고 측정하는 방식에서의 select임*/

// BMC PowerUsage Function
#define PSU1_SENSOR_ID "P12V_PSU1"
#define PSU2_SENSOR_ID "P12V_PSU2"
#define PEAK_TIME_ONE_DAY 1
#define PEAK_TIME_LAST_TEN_MINUTES 2
#define PEAK_TIME_LAST_ONE_HOUR 3
#define PEAK_TIME_LAST_TEN_HOURS 4

json::value getTodayTotal(void);
double getLast60Min(string sensorId);
double getLast24Hour(string sensorId);

json::value getPeakData(void);
json::value getHighestPeak(int timeOption);
json::value getLowestPeak(int timeOption);

// graph
json::value getGraphData(void);
json::value getMinGraphData(string sensorId, int count);
json::value getHourGraphData(string sensorId, int count);
json::value getDayGraphData(string sensorId, int count);

string getChangedTime(string currentTime, int seconds);
int get_last_time_callback(void *_string, int argc, char **argv, char **azColName);
int get_total_power_value_callback(void *_double, int argc, char **argv, char **azColName);

int get_peak_power_value_callback(void *_json, int argc, char **argv, char **azColName);

int get_power_value_graph_callback(void *_vector, int argc, char **argv, char **azColName);
// int get_hour_power_value_graph_callback(void *_vector, int argc, char **argv, char **azColName);
// int get_day_power_value_graph_callback(void *_vector, int argc, char **argv, char **azColName);




// 디비넣어놓고 테스트

// Redfish

// Event_Info generate_event_info(string _event_id, string _event_type, string _msg_id, vector<string> _args);
// SEL generate_sel(unsigned int _sensor_num, string _code, string _sensor_type, string _msg, string _severity);
// EventService 쪽으로 이동

void generate_log(string _position, string _facility, Event_Info _ev);
void generate_log(string _position, string _facility, SEL _sel);
// event_info에서 member_id??? 스키마에 required로...
LogEntry* make_log(string odata_id, string id, Event_Info ev);
LogEntry* make_log(string odata_id, string id, SEL sel);


// Reading Operation
void log_operation(http_request _request);
// reading 관련처리함수랑 event 관련처리함수 2가지로 분기
void reading_operation(http_request _request, http_response &_response);

void event_operation(http_request _request, http_response &_response);
const std::string DB_currentDateTime();
bool check_reading_type(string _types);
bool check_reading_detail(string _type, string _detail);
bool check_reading_time_option(string _option);

#endif