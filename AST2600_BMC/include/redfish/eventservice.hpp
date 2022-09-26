#ifndef EVENTSERVICE_H
#define EVENTSERVICE_H

#include <redfish/stdafx.hpp>
#include <redfish/resource.hpp>


// subscription value check
bool check_protocol(string _protocol);
bool check_policy(string _policy);
bool check_subscription_type(string _type);
bool check_event_type(string _type);

// Generate Event
Event_Info generate_event_info(string _event_id, string _event_type, string _msg_id, vector<string> _args);
// SEL generate_sel(unsigned int _sensor_num, string _code, string _sensor_type, string _msg, string _severity);
SEL generate_sel(unsigned int _sensor_num, string _code, string _sensor_type, string _msg, string _severity, string _time, string _event_type);

// Send Event to Subscribers
void send_event_to_subscriber(Event_Info _ev);
void send_event_to_subscriber(SEL _sel);

bool event_type_exist(vector<string> _vector, string _type);
void shoot_redfish(string _destination, json::value _form);
void shoot_smtp(string _destination, json::value _form);

#endif