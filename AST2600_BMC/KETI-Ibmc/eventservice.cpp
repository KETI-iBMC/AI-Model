#include <redfish/eventservice.hpp>

bool check_protocol(string _protocol)
{
    if(_protocol == "Redfish" || _protocol == "SMTP")
        return true;
    
    return false;
    // schema value : OEM, Redfish, SMTP, SNMPv1, SNMPv2c, SNMPv3, SyslogRELP, SyslogTCP, SyslogTLS, SyslogUDP
}

bool check_policy(string _policy)
{
    if(_policy == "RetryForever" || _policy == "SuspendRetries" || _policy == "TerminateAfterRetries")
        return true;

    return false;
}

bool check_subscription_type(string _type)
{
    if(_type == "RedfishEvent" || _type == "OEM") // OEM 을 SMTP로 지원
        return true;

    return false;
    // schema value : OEM , RedfishEvent, SNMPInform, SNMPTrap, SSE, Syslog
}

bool check_event_type(string _type)
{
    if(_type == "Alert" || _type == "ResourceAdded" || _type == "ResourceRemoved" 
    || _type == "ResourceUpdated" || _type == "StatusChange")
        return true;

    return false;
    // schema value : Alert, MetricReport, Other, ResourceAdded, ResourceRemoved, ResourceUpdated, StatusChange
}

Event_Info generate_event_info(string _event_id, string _event_type, string _msg_id, vector<string> _args)
{
  Event_Info ev;
  ev.event_id = _event_id;
  ev.event_timestamp = currentDateTime();
  
  if(!check_event_type(_event_type))
    log(warning) << "Invalid Event Type";

  ev.event_type = _event_type;
  ev.message.id = _msg_id;

  string registry_odata = ODATA_MESSAGE_REGISTRY_ID;
  MessageRegistry *registry = (MessageRegistry *)g_record[registry_odata];
  int flag=0;

  for(int i=0; i<registry->messages.v_msg.size(); i++)
  {
    if(registry->messages.v_msg[i].pattern == _msg_id)
    {
      ev.message.message = registry->messages.v_msg[i].message;
      ev.message.severity = registry->messages.v_msg[i].severity;
      // ev.message.message_args = registry->messages.v_msg[i].message_args;
      ev.message.resolution = registry->messages.v_msg[i].resolution;

      if(registry->messages.v_msg[i].number_of_args != _args.size())
      {
        flag = 1;
        break;
      }
      else
        ev.message.message_args = _args;
      flag = 2;
      break;
    }
  }

  if(flag == 0)
  {
    log(warning) << "In MessageRegistry, No Information about Message ID : " << _msg_id;
  }
  else if(flag == 1)
  {
    log(warning) << "Message Args Count Incorrect";
  }
  // 이렇게 하고 message에서 message 내용 severity, message_args, resolution정도 있는데
  // 이게 messageregistry에 지정된거 가지고 다 사용하는거면 id로 레지스트리 검색해서 message, severity, args
  // resolution 가져오면 될듯?

  return ev;
}

SEL generate_sel(unsigned int _sensor_num, string _code, string _sensor_type, string _msg, string _severity, string _time, string _event_type)
{
  SEL sel;
  sel.sensor_number = _sensor_num;
  sel.entry_code = _code;
  sel.sensor_type = _sensor_type;
  // sel.message.id = _msg_id;
  sel.message.message = _msg;
  sel.message.severity = _severity;
  sel.event_timestamp = _time;
  sel.event_type = _event_type;

  // sel event 정보를 따로받아야할거같은데? 메세지도 들어있고 severity라든가

  return sel;
}


void send_event_to_subscriber(Event_Info _ev)
{
    string col_odata = ODATA_EVENT_DESTINATION_ID;
    Collection *col = (Collection *)g_record[col_odata];
    cout << "Get In !" << endl;

    // 구독자 목록 순회
    for(int i=0; i<col->members.size(); i++)
    {
        EventDestination *ed = (EventDestination *)(col->members[i]);
        cout << " Current Subscriber Context : " << ed->context << endl;

        // #1 구독 state 유효한지 검사
        if(ed->status.state != "Enabled")
            continue;
        
        // #2 구독자의 이벤트 타입목록에 발생한 이벤트 타입이 있는지 검사
        if(!event_type_exist(ed->event_types, _ev.event_type))
            continue;

        Event event;
        event.context = ed->context;
        event.events.push_back(_ev);
        if(ed->protocol == "Redfish")
        {
            // HTTP로 전송
            // 폼을 만들고 전송할 json폼 그다음에 해당 uri에 보내보내
            shoot_redfish(ed->destination, event.get_json());
        }
        else if(ed->protocol == "SMTP")
        {
            // SMTP로 전송
            shoot_smtp(ed->destination, event.get_json());
        }

    }
    cout << "Get Out !" << endl;

}

void send_event_to_subscriber(SEL _sel)
{
    string col_odata = ODATA_EVENT_DESTINATION_ID;
    Collection *col = (Collection *)g_record[col_odata];
    cout << "Get In !" << endl;

    // 구독자 목록 순회
    for(int i=0; i<col->members.size(); i++)
    {
        EventDestination *ed = (EventDestination *)(col->members[i]);
        cout << " Current Subscriber Context : " << ed->context << endl;

        // #1 구독 state 유효한지 검사
        if(ed->status.state != "Enabled")
            continue;
        
        // #2 구독자의 이벤트 타입목록에 발생한 이벤트 타입이 있는지 검사
        if(!event_type_exist(ed->event_types, _sel.event_type))
            continue;

        if(ed->protocol == "Redfish")
        {
            // HTTP로 전송
            // 폼을 만들고 전송할 json폼 그다음에 해당 uri에 보내보내
            shoot_redfish(ed->destination, _sel.get_json());
        }
        else if(ed->protocol == "SMTP")
        {
            // SMTP로 전송
            shoot_smtp(ed->destination, _sel.get_json());
        }

    }
    cout << "Get Out !" << endl;

}

bool event_type_exist(vector<string> _vector, string _type)
{
    for(int i=0; i<_vector.size(); i++)
    {
        if(_type == _vector[i])
            return true;
    }

    return false;
}

void shoot_redfish(string _destination, json::value _form)
{
    http_client client(_destination);
    http_request req(methods::POST);
    req.set_body(_form);
    http_response res;

    try{
        pplx::task<http_response> responseTask = client.request(req);
        res = responseTask.get();
        log(info) << "[Response Redfish Event From " + _destination + "] : " << res.status_code();
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void shoot_smtp(string _destination, json::value _form)
{

    EventService *eventService = (EventService *)g_record[ODATA_EVENT_SERVICE_ID];
    if(!eventService->smtp.smtp_smtp_enabled)
    {
        return;
    }
    if(fs::exists("/redfish/v1/EventService/message.txt"))
    {
        fs::path msg_text("/redfish/v1/EventService/message.txt");
        fs::remove(msg_text);
    }

    ofstream msg_text("/redfish/v1/EventService/message.txt");
    msg_text << "To: " << _destination << endl;
    msg_text << "Subject: Event Occur!" << endl;
    msg_text << endl;
    msg_text << _form.serialize() << endl;
    
//     EventService *eventService = (EventService *)g_record[ODATA_EVENT_SERVICE_ID];
//   if(eventService==nullptr)
//   {
//     printf("Get_Smtp_Info null error\n");
//     return ;
//   }
    // system("msmtp -a default -t < /redfish/v1/EventService/message.txt");
    system("msmtp -a PRIMARY --file=/etc/msmtprc -t < /redfish/v1/EventService/message.txt");
   // log(info) << "[SMTP] Send Mail To " << _destination << " Successfully";
}
int set_ssh_port(string port) {
    string network_odata = ODATA_MANAGER_ID;
    network_odata.append("/NetworkProtocol");
    NetworkProtocol *net = (NetworkProtocol *)g_record[network_odata];
    if(improved_stoi(port)==net->ssh_port)
    {   
        return;
    }
    cout<<"net->ssh_port"<<net->ssh_port<<endl;
    net->ssh_port=improved_stoi(port);
    cout<<"net->ssh_port"<<net->ssh_port<<endl;
    char buf[128];
    resource_save_json(net);
    sprintf(buf, "sed -i 'q/Port.*/Port %d' /etc/ssh/sshd_config", net->ssh_port);
	system(buf);
	system("/etc/init.d/S50sshd restart");

}
int set_smtp_port(string port) {
    EventService *eventService = (EventService *)g_record[ODATA_EVENT_SERVICE_ID];
    
     if(improved_stoi(port)==eventService->smtp.smtp_port)
    {
        return;
    }
    
    eventService->smtp.smtp_port=improved_stoi(port);
    resource_save_json(eventService);
	if (access("/etc/msmtprc", F_OK) != 0) {
		perror("No file");
		return -1;
	}
	char buf[128];
	sprintf(buf, "sed -i '5s/port.*/port %s/' /etc/msmtprc", port.c_str());
	if (system(buf) == 0)
		return 0;
	else
		return -1;
}
void send_smtp(json::value _form)
{
    
   EventService *eventService = (EventService *)g_record[ODATA_EVENT_SERVICE_ID];
    if(!eventService->smtp.smtp_smtp_enabled)
    {
        // cout<<"send_smtp disabled"<<endl;
        return;
    }
  if(eventService==nullptr)
  {
    printf("Get_Smtp_Info null error\n");
    return ;
  }
    if(fs::exists("/redfish/v1/EventService/message.txt"))
    {
        fs::path msg_text("/redfish/v1/EventService/message.txt");
        fs::remove(msg_text);
    }

    ofstream msg_text("/redfish/v1/EventService/message.txt");
    msg_text << "To: " << eventService->smtp.smtp_sender_address << endl;
    msg_text << "Subject: Subject: [Alert from KETI BMC] The abnormal sensors are detected." << endl;
    msg_text <<"Event Message: The abnormal sensors are detected."<< endl;
    msg_text << _form.serialize() << endl;
  if(eventService==nullptr)
  {
    printf("Get_Smtp_Info null error\n");
    return ;
  }
    // system("msmtp -a default -t < /redfish/v1/EventService/message.txt");
    system("msmtp -a PRIMARY --file=/etc/msmtprc -t < /redfish/v1/EventService/message.txt");
    //log(info) << "[SMTP] Send Mail To " << eventService->smtp.smtp_sender_address << " Successfully";
}