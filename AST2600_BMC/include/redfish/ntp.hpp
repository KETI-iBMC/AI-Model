#ifndef __NTP_HPP__
#define __NTP_HPP__

#include <redfish/stdafx.hpp>


typedef struct _NTP
{
    bool protocol_enabled;
    int port;
    string primary_ntp_server;
    string secondary_ntp_server;
    vector<string> ntp_servers;
    string date_str;
    string time_str;
    string timezone;
} NTP;

// date로 출력이랑 수동변경하고
// rdate로 서버변경
// 데몬이랑 conf관계 파악하고

// get time info
void get_current_fulltime_info(string& _date, string& _time, string& _utc);
void get_current_date_info(string& _date);
void get_current_time_info(string& _time);
void get_current_utc_info(string& _utc);


// set time by user
void set_time_by_userDate(string _date, string _time);
void set_time_by_userTimezone(string _new_tz, string _origin_tz="+09:00");
void calculate_diff_time(string _origin_tz, string _new_tz, string& _op, string& _hours);
// 이거 받는 date, time, utc같은거 validate검사 필요함 --> util에 만듬
// 

void set_localtime_by_userTimezone(string _time);

// set time using ntp server
int set_time_by_ntp_server(string _server);



#endif