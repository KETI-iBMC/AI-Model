#include <condition_variable>
#include <mutex>
#include <redfish/resource.hpp>

extern unordered_map<string, Resource *> g_record;
extern ServiceRoot *g_service_root;
unordered_map<string, Event *> event_map;

// Resource start
json::value Resource::get_json(void) {
  json::value j;
  try {
    // log(info) << "oid : " << this->odata.id << ", otype : " <<
    // this->odata.type << " type : " << this->type << "name : " << this->name
    // << endl;
    j[U("@odata.type")] = json::value::string(U(this->odata.type));
    j[U("Name")] = json::value::string(U(this->name));
    j[U("@odata.id")] = json::value::string(U(this->odata.id));
    j["type"] = json::value::string(to_string(this->type)); // uint_8
  } catch (json::json_exception &e) {
    return j = json::value::null();
  }
  return j;
}

json::value Resource::get_json(int type) {
  json::value j;

  switch (type) {
  case NO_DATA_TYPE: {
    try {
      j[U("Name")] = json::value::string(U(this->name));
      j[U("@odata.id")] = json::value::string(U(this->odata.id));
      j["type"] = json::value::string(to_string(this->type)); // uint_8
    } catch (json::json_exception &e) {
      return j = json::value::null();
    }
    return j;
    break;
  }
  default:
    cout << "unknown type get json" << endl;
    return json::value::null();
    break;
  }
}

json::value Resource::get_odata_id_json(void) {
  json::value j;

  j[U("@odata.id")] = json::value::string(U(this->odata.id));

  return j;
}

bool Resource::save_json(void) {
  string json_content;
  json_content = record_get_json(this->odata.id).serialize();

  if (json_content == "null") {
    log(warning) << "Something Wrong in save json : " << this->odata.id << endl;
    return false;
  }

  // log(info) << "file " << this->odata.id << " : " << json_content;

  vector<string> tokens = string_split(this->odata.id, '/');
  string sub_path = "/";
  for (unsigned int i = 0; i < tokens.size() - 1; i++) {
    if (i == 0)
      sub_path = sub_path + tokens[i];
    else
      sub_path = sub_path + '/' + tokens[i];

    if (!access(sub_path.c_str(), F_OK))
      continue;
    mkdir(sub_path.c_str(), 0755);
  }

  // Save json file to path
  ofstream out(this->odata.id + ".json");
  out << json_content << endl;
  out.close();
  // log(info) << "save complete : " << this->odata.id + ".json" << endl <<
  // endl;

  return true;
}

bool Resource::load_json(json::value &j) {

  try {
    this->name = j.at("Name").as_string();
    this->type = atoi(j.at("type").as_string().c_str());
    this->odata.id = j.at("@odata.id").as_string();
    this->odata.type = j.at("@odata.type").as_string();
  } catch (json::json_exception &e) {
    return false;
  }
  return true;
}

bool Resource::load_json(json::value &j, int type) {
  try {
    switch (type) {
    case NO_DATA_TYPE: {
      this->name = j.at("Name").as_string();
      this->type = atoi(j.at("type").as_string().c_str());
      this->odata.id = j.at("@odata.id").as_string();
      break;
    }
    default: {
      cout << "unknown type load json" << endl;
      break;
    }
    }
  } catch (json::json_exception &e) {
    return false;
  }
  return true;
}

bool Resource::load_json_from_file(json::value &j) {
  try {
    ifstream target_file(this->odata.id + ".json");
    stringstream string_stream;

    string_stream << target_file.rdbuf();
    target_file.close();

    j = json::value::parse(string_stream);
    auto j_obj = j.as_object();

    if (j_obj.find("@odata.type") == j_obj.end()) {
      Resource::load_json(j, NO_DATA_TYPE);
    } else {
      Resource::load_json(j);
    }
  } catch (json::json_exception &e) {
    cout << "error : no Resource Data in " << this->odata.id << endl;
    return false;
  }

  return true;
}
// Resource end

// Collection start
json::value Collection::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Members")] = json::value::array();
  for (unsigned int i = 0; i < this->members.size(); i++) {
    j["Members"][i] =
        get_resource_odata_id_json(this->members[i], this->odata.id);
  }
  j[U("Members@odata.count")] = json::value::number(U(this->members.size()));
  return j;
}

void Collection::add_member(Resource *_resource) {
  this->members.push_back(_resource);
}

vector<Resource *> *Collection::get_member(void) { return &this->members; }

bool Collection::load_json(json::value &j) {
  try {
    Resource::load_json(j);
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Collection end

// List start
json::value List::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["MemberType"] = json::value::number(this->member_type);
  for (unsigned int i = 0; i < this->members.size(); i++)
    switch (this->member_type) {
    case TEMPERATURE_TYPE:
      j[U("Temperatures")][i] = ((Temperature *)this->members[i])->get_json();
      break;
    case FAN_TYPE:
      j[U("Fans")][i] = ((Fan *)this->members[i])->get_json();
      break;
    case STORAGE_CONTROLLER_TYPE:
      j[U("StorageControllers")][i] =
          ((StorageControllers *)this->members[i])->get_json();
      break;
    case POWER_CONTROL_TYPE:
      j[U("PowerControl")][i] = ((PowerControl *)this->members[i])->get_json();
      break;
    case VOLTAGE_TYPE:
      j[U("Voltages")][i] = ((Voltage *)this->members[i])->get_json();
      break;
    case POWER_SUPPLY_TYPE:
      j[U("PowerSupplies")][i] = ((PowerSupply *)this->members[i])->get_json();
      break;
    }
  // 위에 this->name으로 이름을 만들었기 때문에 name하고 이름 똑같아야 새로
  // 안만들고 잘 들어감ㅇㅇ
  return j;
}

void List::add_member(Resource *_resource) {
  this->members.push_back(_resource);
}

bool List::load_json(json::value &j) {
  json::value members;

  try {
    Resource::load_json(j);
    this->member_type = j.at("MemberType").as_integer();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// List end

// Account start
json::value Account::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value k;
  j[U("Id")] = json::value::string(U(this->id));
  j[U("Enabled")] = json::value::boolean(U(this->enabled));
  j[U("Password")] = json::value::string(U(this->password));
  j[U("UserName")] = json::value::string(U(this->user_name));
  j[U("Locked")] = json::value::boolean(U(this->locked));
  j["RoleOdataId"] = json::value::string(this->role_id);

  j["CALLIN"] = json::value::number(callin);
  j["LINKAUTH"] = json::value::number(link_auth);
  j["IPMIMSG"] = json::value::number(ipmi);
  j["PRIVILEGE"] = json::value::number(priv);

  if (this->role == nullptr) {
    j[U("RoleId")] = json::value::null();
    k = json::value::object();
  } else {
    j[U("RoleId")] = json::value::string(U(this->role->id));
    // k[U("Role")] = this->role->get_odata_id_json();
    k[U("Role")] = get_resource_odata_id_json(this->role, this->odata.id);
  }
  j[U("Links")] = k;

  j["Certificates"] =
      get_resource_odata_id_json(this->certificates, this->odata.id);

  return j;
}

bool Account::load_json(json::value &j) {
  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->enabled = j.at("Enabled").as_bool();
    this->password = j.at("Password").as_string();
    this->user_name = j.at("UserName").as_string();
    this->locked = j.at("Locked").as_bool();
    this->role_id = j.at("RoleOdataId").as_string();
    ipmi = j.at("IPMIMSG").as_integer();

    priv = j.at("PRIVILEGE").as_integer();
    link_auth = j.at("LINKAUTH").as_integer();
    callin = j.at("CALLIN").as_integer();

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Account end

// Role start
json::value Role::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("IsPredefined")] = json::value::boolean(U(this->is_predefined));
  for (unsigned int i = 0; i < this->assigned_privileges.size(); i++)
    j[U("AssignedPrivileges")][i] =
        json::value::string(U(this->assigned_privileges[i]));
  return j;
}

bool Role::load_json(json::value &j) {
  json::value role;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->is_predefined = j.at("IsPredefined").as_bool();

    role = j.at("AssignedPrivileges");
    for (auto str : role.as_array())
      this->assigned_privileges.push_back(str.as_string());
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Role end

// AccountService start
json::value AccountService::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value k;
  j[U("Id")] = json::value::string(U(this->id));
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;
  j[U("ServiceEnabled")] = json::value::boolean(U(this->service_enabled));
  j[U("AuthFailureLoggingThreshold")] =
      json::value::number(U(this->auth_failure_logging_threshold));
  j[U("MinPasswordLength")] = json::value::number(U(this->min_password_length));
  j[U("MaxPasswordLength")] = json::value::number(U(this->max_password_length));
  j[U("AccountLockoutThreshold")] =
      json::value::number(U(this->account_lockout_threshold));
  j[U("AccountLockoutDuration")] =
      json::value::number(U(this->account_lockout_duration));
  j[U("AccountLockoutCounterResetAfter")] =
      json::value::number(U(this->account_lockout_counter_reset_after));
  j[U("AccountLockoutCounterResetEnabled")] =
      json::value::boolean(U(this->account_lockout_counter_reset_enabled));

  j["Accounts"] =
      get_resource_odata_id_json(this->account_collection, this->odata.id);
  j["Roles"] =
      get_resource_odata_id_json(this->role_collection, this->odata.id);

  // LDAP
  json::value ldap;
  ldap["AccountProviderType"] =
      json::value::string(this->ldap.account_provider_type);
  ldap["PasswordSet"] = json::value::boolean(this->ldap.password_set);
  ldap["ServiceEnabled"] = json::value::boolean(this->ldap.service_enabled);
  ldap["Port"] = json::value::number(this->ldap.port);

  json::value service_addresses = json::value::array();
  for (int i = 0; i < this->ldap.service_addresses.size(); i++) {
    service_addresses[i] = json::value::string(this->ldap.service_addresses[i]);
  }
  ldap["ServiceAddresses"] = service_addresses;
  // ldap["ServiceAddresses"] =
  // json::value::array(this->ldap.service_addresses); // vector

  json::value auth;
  auth["AuthenticationType"] =
      json::value::string(this->ldap.authentication.authentication_type);
  auth["Username"] = json::value::string(this->ldap.authentication.username);
  auth["Password"] = json::value::string(this->ldap.authentication.password);
  ldap["Authentication"] = auth;

  json::value service, setting;
  setting["GroupsAttribute"] = json::value::string(
      this->ldap.ldap_service.search_settings.groups_attribute);
  setting["GroupNameAttribute"] = json::value::string(
      this->ldap.ldap_service.search_settings.group_name_attribute);
  setting["UsernameAttribute"] = json::value::string(
      this->ldap.ldap_service.search_settings.user_name_attribute);

  json::value distinguish = json::value::array();
  for (int i = 0;
       i <
       this->ldap.ldap_service.search_settings.base_distinguished_names.size();
       i++) {
    distinguish[i] = json::value::string(
        this->ldap.ldap_service.search_settings.base_distinguished_names[i]);
  }
  setting["BaseDistinguishedNames"] = distinguish;
  // setting["BaseDistinguishedNames"] =
  // json::value::array(this->ldap.ldap_service.search_settings.base_distinguished_names);
  service["SearchSettings"] = setting;
  ldap["LDAPService"] = service;

  j["LDAP"] = ldap;

  // Active Directory
  json::value ad;
  ad["AccountProviderType"] =
      json::value::string(this->active_directory.account_provider_type);
  ad["ServiceEnabled"] =
      json::value::boolean(this->active_directory.service_enabled);
  ad["Port"] = json::value::number(this->active_directory.port);

  json::value service_addresses2 = json::value::array();
  for (int i = 0; i < this->active_directory.service_addresses.size(); i++) {
    service_addresses2[i] =
        json::value::string(this->active_directory.service_addresses[i]);
  }
  ad["ServiceAddresses"] = service_addresses2;
  // ad["ServiceAddresses"] =
  // json::value::array(this->active_directory.service_addresses);

  json::value auth2;
  auth2["AuthenticationType"] = json::value::string(
      this->active_directory.authentication.authentication_type);
  auth2["Username"] =
      json::value::string(this->active_directory.authentication.username);
  auth2["Password"] =
      json::value::string(this->active_directory.authentication.password);
  ad["Authentication"] = auth2;

  j["ActiveDirectory"] = ad;

  return j;
}

bool AccountService::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->service_enabled = j.at("ServiceEnabled").as_bool();
    this->auth_failure_logging_threshold =
        j.at("AuthFailureLoggingThreshold").as_integer();
    this->min_password_length = j.at("MinPasswordLength").as_integer();
    this->max_password_length = j.at("MaxPasswordLength").as_integer();
    this->account_lockout_threshold =
        j.at("AccountLockoutThreshold").as_integer();
    this->account_lockout_duration =
        j.at("AccountLockoutDuration").as_integer();
    this->account_lockout_counter_reset_after =
        j.at("AccountLockoutCounterResetAfter").as_integer();
    this->account_lockout_counter_reset_enabled =
        j.at("AccountLockoutCounterResetEnabled").as_bool();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    // LDAP
    json::value ldap;
    get_value_from_json_key(j, "LDAP", ldap);

    get_value_from_json_key(ldap, "AccountProviderType",
                            this->ldap.account_provider_type);
    get_value_from_json_key(ldap, "PasswordSet", this->ldap.password_set);
    get_value_from_json_key(ldap, "ServiceEnabled", this->ldap.service_enabled);
    get_value_from_json_key(ldap, "Port", this->ldap.port);

    json::value service_address;
    get_value_from_json_key(ldap, "ServiceAddresses", service_address);
    for (auto obj : service_address.as_array()) {
      this->ldap.service_addresses.push_back(obj.as_string());
    }

    json::value auth;
    get_value_from_json_key(ldap, "Authentication", auth);
    get_value_from_json_key(auth, "AuthenticationType",
                            this->ldap.authentication.authentication_type);
    get_value_from_json_key(auth, "Username",
                            this->ldap.authentication.username);
    get_value_from_json_key(auth, "Password",
                            this->ldap.authentication.password);

    json::value service, setting;
    get_value_from_json_key(ldap, "LDAPService", service);
    get_value_from_json_key(service, "SearchSettings", setting);

    json::value distinguish;
    get_value_from_json_key(setting, "BaseDistinguishedNames", distinguish);
    for (auto obj : distinguish.as_array()) {
      this->ldap.ldap_service.search_settings.base_distinguished_names
          .push_back(obj.as_string());
    }
    get_value_from_json_key(
        setting, "GroupsAttribute",
        this->ldap.ldap_service.search_settings.groups_attribute);
    get_value_from_json_key(
        setting, "GroupNameAttribute",
        this->ldap.ldap_service.search_settings.group_name_attribute);
    get_value_from_json_key(
        setting, "UsernameAttribute",
        this->ldap.ldap_service.search_settings.user_name_attribute);

    // Active Directory
    json::value ad;
    get_value_from_json_key(j, "ActiveDirectory", ad);

    get_value_from_json_key(ad, "AccountProviderType",
                            this->active_directory.account_provider_type);
    get_value_from_json_key(ad, "ServiceEnabled",
                            this->active_directory.service_enabled);
    get_value_from_json_key(ad, "Port", this->active_directory.port);

    json::value service_address2;
    get_value_from_json_key(ad, "ServiceAddresses", service_address2);
    for (auto obj : service_address2.as_array()) {
      this->active_directory.service_addresses.push_back(obj.as_string());
    }

    json::value auth2;
    get_value_from_json_key(ad, "Authentication", auth2);
    get_value_from_json_key(
        auth2, "AuthenticationType",
        this->active_directory.authentication.authentication_type);
    get_value_from_json_key(auth2, "Username",
                            this->active_directory.authentication.username);
    get_value_from_json_key(auth2, "Password",
                            this->active_directory.authentication.password);

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

bool AccountService::wrtie_ladap_to_nclcd()
{
  char buf[128];
	char full_buf[4092];
	if (access(NSLCD_FILE, F_OK) == 0)
	    system("rm /etc/nslcd.conf");
  int fd_write = open(NSLCD_FILE, O_RDWR | O_CREAT , 0700);
  if(fd_write<0){
    log(error)<<"wrtie_ladap_to_nclcd error :fd="<<fd_write;
    return false;
  }
  if(this->ldap.service_addresses.size()>=1)
  {
    sprintf(buf, "uri ldap://%s\n", this->ldap.service_addresses[0].c_str());
    strcpy(full_buf, buf);
  }
	
  if(this->ldap.ldap_service.search_settings.base_distinguished_names.size()>=1)
  {
    sprintf(buf, "base %s\n", ldap.ldap_service.search_settings.base_distinguished_names[0].c_str());
	  strcat(full_buf, buf);
  }
  sprintf(buf, "binddn %s\n", ldap.authentication.username.c_str());
	strcat(full_buf, buf);
  if(ldap.authentication.password.size()==0)
    sprintf(buf, "bindpw %s\n", "null");
  else
  sprintf(buf, "bindpw %s\n", ldap.authentication.password.c_str());
	// strcat(full_buf, buf);
  // sprintf(buf, "timelimit %d\n", ldap_config.timelimit);	
	strcat(full_buf, buf);
	write(fd_write, full_buf, strlen(full_buf));
	close(fd_write);
  return true ;
  
}
bool AccountService::write_ad_to_file()
{
  char buf[128];
	char full_buf[4092];
  if (access(NSLCD_FILE, F_OK) == 0)
	  system("rm /etc/nslcd.conf");
  
  int fd_write = open(NSLCD_FILE, O_RDWR | O_CREAT , 0700);
  if(fd_write<0){
    log(error)<<"wrtie_ladap_to_nclcd error :fd="<<fd_write;
    return false;
  }
  if(this->active_directory.service_addresses.size()>=1)
  {
    sprintf(buf, "uri ldap://%s\ntls_reqcert allow\n", active_directory.service_addresses[0].c_str());
    strcpy(full_buf, buf);
  }
  sprintf(buf, "binddn %s\n", active_directory.authentication.username.c_str());
  strcat(full_buf, buf);
  if(active_directory.authentication.password.size()==0)
    sprintf(buf, "bindpw %s\n", "null");
  else
    sprintf(buf, "bindpw %s\n", active_directory.authentication.password.c_str());
  strcat(full_buf, buf);
  sprintf(buf, "filter     passwd (objectClass=*)\nmap     passwd  uid     sAMAccountName\n");
  strcat(full_buf, buf);
  sprintf(buf, "filter     shadow (objectClass=*)\nmap     shadow  uid     sAMAccountName\n");
  strcat(full_buf, buf);
  write(fd_write, full_buf, strlen(full_buf));
  close(fd_write);
}
// AccountService end

// SessionService start
json::value SessionService::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value k;
  j[U("Id")] = json::value::string(U(this->id));
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;
  j[U("ServiceEnabled")] = json::value::boolean(U(this->service_enabled));
  j[U("SessionTimeout")] = json::value::number(U(this->session_timeout));

  j["Sessions"] =
      get_resource_odata_id_json(this->session_collection, this->odata.id);

  return j;
}

bool SessionService::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->service_enabled = j.at("ServiceEnabled").as_bool();
    this->session_timeout = j.at("SessionTimeout").as_integer();
    this->id = j.at("Id").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// SessionService end

// Session start
json::value Session::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));

  // 연결된 account가 없는 Session이 있을수 있나?
  if (this->account == nullptr)
    j[U("UserName")] = json::value::string("");
  else
    j[U("UserName")] = json::value::string(U(this->account->user_name));

  j[U("AccountId")] = json::value::string(this->account_id);
  j[U("AuthToken")] = json::value::string(this->x_token);

  return j;
}

bool Session::load_json(json::value &j) {
  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->account_id = j.at("AccountId").as_string();
    this->x_token = j.at("AuthToken").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

pplx::task<void> Session::start(void) {
  Session *session = this;
  unsigned int *remain_expires_time = &this->_remain_expires_time;
  /*
   * @ authors 강
   * @ TODO : 세션종료(로그아웃시) cancel하는 방식으로 해야함
   * 현재 세션만료시간 조정으로 종료함
   */
  // pplx::cancellation_token *c_token = &this->c_token;
  return pplx::create_task([remain_expires_time] {
           boost::asio::io_service io;
           boost::asio::deadline_timer session_timer(
               io, boost::posix_time::seconds(1));
           session_timer.async_wait(
               boost::bind(timer, &session_timer, remain_expires_time));
           io.run();

           // @@@@@
           //    if(pplx::is_task_cancellation_requested())
           //    {
           //     //    cout << session->id << "is Canceled" << endl;
           //        pplx::cancel_current_task();
           //        cout << "cancel??" << endl;
           //    }
         }) //, *c_token)
            // 여기 *c_token 추가했음
      .then([session] {
        // @@@@@
        log(info) << "[Session Expired] : " << session->id;
        // cout << "IN START : " << session->id << endl;

        string path = session->odata.id;

        cout << "지우기 전" << endl;
        cout << record_get_json(ODATA_SESSION_ID) << endl;

        bool exist = false;
        Collection *col = (Collection *)g_record[ODATA_SESSION_ID];
        std::vector<Resource *>::iterator iter;
        for (iter = col->members.begin(); iter != col->members.end(); iter++) {
          Session *s = (Session *)*iter;

          if (s->id == session->id) {
            // col->members.erase(iter);
            exist = true;
            break;
          }
        }
        // 컬렉션에서 해당 세션 찾아 iterator얻기

        if (exist) {
          unsigned int id_num;
          id_num = improved_stoi(session->id);
          delete_numset_num(ALLOCATE_SESSION_NUM, id_num);
          // delete_session_num(id_num);
          // numset에서 num id 삭제

          delete (*iter);
          col->members.erase(iter);
          // session자체 객체삭제, g_record에서 삭제, session collection에서
          // 삭제
          delete_resource(path);
          // session json삭제
          resource_save_json(col);
          // collection json갱신

          // record_save_json(); // 레코드 json파일 갱신
          // string json_path = path;
          // json_path = json_path + ".json";
          // if(remove(json_path.c_str()) < 0)
          // {
          //     cout << "delete error in session remove" << endl;
          // }
          // // session json파일 삭제
        }

        cout << record_get_json(ODATA_SESSION_ID) << endl;
      });
}
// Session end

// Log Service & Log Entry start
json::value LogService::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Description")] = json::value::string(U(this->description));
  j[U("MaxNumberOfRecords")] = json::value::number(U(this->max_number_of_records));
  j[U("OverWritePolicy")] = json::value::string(U(this->overwrite_policy));
  j[U("DateTime")] = json::value::string(U(this->datetime));
  j[U("DateTimeLocalOffset")] = json::value::string(U(this->datetime_local_offset));
  j[U("ServiceEnabled")] = json::value::boolean(U(this->service_enabled));
  j[U("LogEntryType")] = json::value::string(U(this->log_entry_type));

  j[U("SyslogFilters")] = json::value::array();
  for(int i=0; i<this->syslogFilters.size(); i++)
  {
      json::value filter = json::value::object();
      filter[U("LogFacilities")] = json::value::array();

      for (int j = 0; j < this->syslogFilters[i].logFacilities.size(); j++){
          filter[U("LogFacilities")][j] = json::value::string(this->syslogFilters[i].logFacilities[j]);
      }
      filter[U("LowestSeverity")] = json::value::string(U(this->syslogFilters[i].lowestSeverity));
      j[U("SyslogFilters")][i] = filter;
  }

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j["Entries"] = get_resource_odata_id_json(this->entry, this->odata.id);

  j["Actions"] = get_action_info(this->actions);

  return j;
}

bool LogService::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->description = j.at("Description").as_string();
    this->datetime = j.at("DateTime").as_string();
    this->datetime_local_offset = j.at("DateTimeLocalOffset").as_string();
    this->max_number_of_records = j.at("MaxNumberOfRecords").as_integer();
    this->log_entry_type = j.at("LogEntryType").as_string();
    this->overwrite_policy = j.at("OverWritePolicy").as_string();
    this->service_enabled = j.at("ServiceEnabled").as_bool();

    json::value syslog_filters;
    get_value_from_json_key(j, "SyslogFilters", syslog_filters);
    for(auto obj : syslog_filters.as_array()){
        SyslogFilter temp;
        json::value log_facilities;
        
        get_value_from_json_key(obj, "LogFacilities", log_facilities);
        for (auto facility : log_facilities.as_array())
            temp.logFacilities.push_back(facility.as_string());
          
        get_value_from_json_key(obj, "LowestSeverity", temp.lowestSeverity);
        this->syslogFilters.push_back(temp);
    }

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

bool LogService::ClearLog() {
  try
  {
    if (this->entry == nullptr) {
      log(info) << "Cannot find any logEntry in " << this->odata.id;
      return false;
    }

    //#1 all logentry delete in g_record and memory
    vector<Resource *> temp;

    for (int i = 0; i < this->entry->members.size(); i++){
        temp.push_back(this->entry->members[i]);
        
        // all logentry json file delete in LogService folder
        fs::path target_file(this->entry->members[i]->odata.id + ".json");
        log(debug) << target_file;
        if (fs::exists(target_file)){
            log(debug) << target_file;
            fs::remove(target_file);
        }
    }
    this->entry->members.clear();
    
    for(auto item : temp)
        delete(item);
    temp.clear();
    this->record_count = 0;
    
    resource_save_json(this->entry);

  } catch (const std::exception &e)
  {
    log(warning) << e.what();
    return false;
  }

  return true;
}

LogEntry *LogService::new_log_entry(IpmiLogEvent _ev) {
  if (this->entry == nullptr) {
    this->entry = new Collection(this->odata.id + "/Entries",
                                 ODATA_LOG_ENTRY_COLLECTION_TYPE);
    this->entry->name = "Log Entry Collection";
  }

  // unsigned int num = this->entry_count;
  unsigned int num = this->record_count;
  if (num > this->max_number_of_records) {
    return nullptr;
  }
  string entry_odata = this->entry->odata.id + "/" + to_string(num);
  LogEntry *log_entry = new LogEntry(entry_odata, to_string(num));
  log_entry->created = _ev.event_time;
  log_entry->message.message = _ev.msg;
  log_entry->message.message_args = _ev.message_args;
  // log_entry->entry_type = _ev.event_type;
  if (_ev.sensornumber == "")
    log_entry->sensor_number = 0;
  else
    log_entry->sensor_number = improved_stoi(_ev.sensornumber);
  log_entry->severity = _ev.severity;
  // entry_type은 내가 알아서 설정하는거고 event_type에따라서
  // 온도관련인지, 팬관련인지, 뭐 무엇에 관한 로그인지판별
  if (this->log_entry_type == "SEL")
    log_entry->entry_type = "SEL";
  else if (this->log_entry_type == "Event")
    log_entry->entry_type = "Event";
  // multiple일 경우 추가

  this->entry->add_member(log_entry);

  // init_log_entry(this->entry, to_string(num)); // entry_id 0번부터
  // this->entry_count++;
  this->record_count++;

  return log_entry;
}

void LogService::set_description(string _val) { this->description = _val; }
void LogService::set_datetime(string _val) { this->datetime = _val; }
void LogService::set_datetime_offset(string _val) {
  this->datetime_local_offset = _val;
}
void LogService::set_logentry_type(string _val) { this->log_entry_type = _val; }
void LogService::set_overwrite_policy(string _val) {
  this->overwrite_policy = _val;
}
void LogService::set_service_enabled(bool _val) {
  this->service_enabled = _val;
}
void LogService::set_max_record(unsigned int _val) {
  this->max_number_of_records = _val;
}

json::value LogEntry::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("EntryType")] = json::value::string(U(this->entry_type));
  j[U("Severity")] = json::value::string(U(this->severity));
  j[U("Created")] = json::value::string(U(this->created));

  this->message.get_specific_json(j);

  j[U("SensorNumber")] = json::value::number(U(this->sensor_number));
  j[U("SensorType")] = json::value::string(U(this->sensor_type));
  j[U("EntryCode")] = json::value::string(U(this->entry_code));

  j[U("EventId")] = json::value::string(U(this->event_id));
  j[U("EventTimeStamp")] = json::value::string(U(this->event_timestamp));
  j[U("EventType")] = json::value::string(U(this->event_type));

  return j;
}

bool LogEntry::load_json(json::value &j) {
  json::value message_args;

  try {
    Resource::load_json(j);

    get_value_from_json_key(j, "Id", this->id);
    get_value_from_json_key(j, "EntryType", this->entry_type);
    get_value_from_json_key(j, "Severity", this->severity);
    get_value_from_json_key(j, "Created", this->created);

    this->message.load_specific_json(j);

    get_value_from_json_key(j, "SensorNumber", this->sensor_number);
    get_value_from_json_key(j, "SensorType", this->sensor_type);
    get_value_from_json_key(j, "EntryCode", this->entry_code);

    get_value_from_json_key(j, "EventId", this->event_id);
    get_value_from_json_key(j, "EventTimeStamp", this->event_timestamp);
    get_value_from_json_key(j, "EventType", this->event_type);

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

void LogEntry::set_entry_type(string _val) { this->entry_type = _val; }
void LogEntry::set_severity(string _val) { this->severity = _val; }
void LogEntry::set_message(string _val) { this->message.message = _val; }
void LogEntry::set_message_id(string _val) { this->message.id = _val; }
void LogEntry::add_message_args(string _val) {
  this->message.message_args.push_back(_val);
}
void LogEntry::set_sensor_num(unsigned int _val) { this->sensor_number = _val; }
void LogEntry::set_created(string _val) { this->created = _val; }
// Log Service & Log Entry end

// Event Service & Event Destination start
json::value Event::get_json() {
  json::value j;

  j["Context"] = json::value::string(this->context);

  for (int i = 0; i < this->events.size(); i++){
      json::value ev = this->events[i].message.get_json();
      ev["EventId"] = json::value::string(this->events[i].event_id);
      ev["EventTimeStamp"] = json::value::string(this->events[i].event_timestamp);
      ev["EventType"] = json::value::string(this->events[i].event_type);
      ev["MemberId"] = json::value::string(this->events[i].member_id);

      j["Events"][i] = ev;
  }

  // j["Id"] = json::value::string(this->id);
  // j["@odata.type"] = json::value::string(this->type);
  // j["Name"] = json::value::string(this->name);
  // j["Context"] = json::value::string(this->context);
  // j["description"] = json::value::string(this->description);
  // j["Events"] = json::value::array();
  // for (int i = 0; i < this->events.size(); i++) {
  //   json::value events;
  //   events["EventGroupId"] =
  //       json::value::number(this->events[i].event_group_id);
  //   events["EventId"] = json::value::string(this->events[i].event_id);
  //   events["EventTimestamp"] =
  //       json::value::string(this->events[i].event_timestamp);
  //   events["MessageSeverity"] =
  //       json::value::string(this->events[i].message_severity);
  //   events["Message"] = json::value::string(this->events[i].message);
  //   events["MessageId"] = json::value::string(this->events[i].message_id);

  //   json::value ooc;
  //   ooc["@odata.id"] = json::value::string(this->events[i].origin_of_condition);
  //   events["OriginOfCondition"] = ooc;

  //   events["MessageArgs"] = json::value::array();
  //   for (int j = 0; j < this->events[i].message_args.size(); j++)
  //     events["MessageArgs"][j] =
  //         json::value::string(this->events[i].message_args[j]);
  //   j["Events"][i] = events;
  // }

  return j;
}

json::value SEL::get_json(void)
{
    json::value j;

    j = this->message.get_json();
    j["SensorNumber"] = json::value::number(this->sensor_number);
    j["SensorType"] = json::value::string(this->sensor_type);
    j["EntryCode"] = json::value::string(this->entry_code);
    j["EventTimestamp"] = json::value::string(this->event_timestamp);
    j["EventType"] = json::value::string(this->event_type);

    return j;
}

bool event_is_exist(const string _uri) {
  if (event_map.find(_uri) != event_map.end())
    return true;
  return false;
}

json::value EventDestination::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Destination")] = json::value::string(U(this->destination));
  j[U("SubscriptionType")] = json::value::string(U(this->subscription_type));
  j[U("DeliveryRetryPolicy")] =
      json::value::string(U(this->delivery_retry_policy));
  j[U("Context")] = json::value::string(U(this->context));
  j[U("Protocol")] = json::value::string(U(this->protocol));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("EventTypes")] = json::value::array();
  for(int i=0; i<this->event_types.size(); i++)
      j[U("EventTypes")][i] = json::value::string(U(this->event_types[i]));

  return j;
}

bool EventDestination::load_json(json::value &j) {
  json::value status;
  json::value event_types;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->destination = j.at("Destination").as_string();
    this->subscription_type = j.at("SubscriptionType").as_string();
    this->delivery_retry_policy = j.at("DeliveryRetryPolicy").as_string();
    this->context = j.at("Context").as_string();
    this->protocol = j.at("Protocol").as_string();

    event_types = j.at("EventTypes");
    for(auto str : event_types.as_array())
        this->event_types.push_back(str.as_string());

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value EventService::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("ServiceEnabled")] = json::value::boolean(U(this->service_enabled));
  j[U("DeliveryRetryAttempts")] =
      json::value::number(U(this->delivery_retry_attempts));
  j[U("DeliveryRetryIntervalSeconds")] =
      json::value::number(U(this->delivery_retry_interval_seconds));
  j[U("ServerSentEventUri")] =
      json::value::string(U(this->serversent_event_uri));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("EventTypesForSubscription")] = json::value::array();
  for (int i = 0; i < this->event_types_for_subscription.size(); i++)
    j[U("EventTypesForSubscription")][i] =
        json::value::string(U(this->event_types_for_subscription[i]));

  json::value j_sse;
  // j_sse[U("EventType")] = json::value::boolean(U(this->sse.event_type));
  j_sse[U("MetricReportDefinition")] =
      json::value::boolean(U(this->sse.metric_report_definition));
  j_sse[U("RegistryPrefix")] =
      json::value::boolean(U(this->sse.registry_prefix));
  j_sse[U("ResourceType")] = json::value::boolean(U(this->sse.resource_type));
  j_sse[U("EventFormatType")] =
      json::value::boolean(U(this->sse.event_format_type));
  j_sse[U("MessageId")] = json::value::boolean(U(this->sse.message_id));
  j_sse[U("OriginResource")] =
      json::value::boolean(U(this->sse.origin_resource));
  j_sse[U("SubordinateResources")] =
      json::value::boolean(U(this->sse.subordinate_resources));
  j[U("SSEFilterPropertiesSupported")] = j_sse;


  // smtp
  json::value j_smtp;
  j_smtp["MachineName"] = json::value::string(this->smtp.smtp_machineName);
  j_smtp["SmtpServer"] = json::value::string(this->smtp.smtp_server);
  j_smtp["SmtpUserName"] = json::value::string(this->smtp.smtp_username);
  j_smtp["SmtpPassword"] = json::value::string(this->smtp.smtp_password);
  j_smtp["SecondSmtpServer"] = json::value::string(this->smtp.smtp_second_server);
  j_smtp["SecondSmtpUserName"] = json::value::string(this->smtp.smtp_second_username);
  j_smtp["SecondSmtpPassword"] = json::value::string(this->smtp.smtp_second_username);
  j_smtp["SmtpSenderAddress"] =
      json::value::string(this->smtp.smtp_sender_address);
  j_smtp["SmtpPortNumber"] = json::value::number(this->smtp.smtp_port);
  j_smtp["SmtpSSLEnabled"] = json::value::boolean(this->smtp.smtp_smtp_enabled);
  j["SMTP"] = j_smtp;

  j["Subscriptions"] =
      get_resource_odata_id_json(this->subscriptions, this->odata.id);

  j["Actions"] = get_action_info(this->actions);

  return j;
}

bool EventService::load_json(json::value &j) {
  json::value status;
  json::value event_types_for_subscription, sse, smtp;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->service_enabled = j.at("ServiceEnabled").as_bool();
    this->delivery_retry_attempts = j.at("DeliveryRetryAttempts").as_integer();
    this->delivery_retry_interval_seconds =
        j.at("DeliveryRetryIntervalSeconds").as_integer();
    this->serversent_event_uri = j.at("ServerSentEventUri").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    event_types_for_subscription = j.at("EventTypesForSubscription");
    for (auto str : event_types_for_subscription.as_array())
      this->event_types_for_subscription.push_back(str.as_string());

    sse = j.at("SSEFilterPropertiesSupported");
    // this->sse.event_type = sse.at("EventType").as_bool();
    this->sse.metric_report_definition =
        sse.at("MetricReportDefinition").as_bool();
    this->sse.registry_prefix = sse.at("RegistryPrefix").as_bool();
    this->sse.resource_type = sse.at("ResourceType").as_bool();
    this->sse.event_format_type = sse.at("EventFormatType").as_bool();
    this->sse.message_id = sse.at("MessageId").as_bool();
    this->sse.origin_resource = sse.at("OriginResource").as_bool();
    this->sse.subordinate_resources = sse.at("SubordinateResources").as_bool();

    get_value_from_json_key(j, "SMTP", smtp);
    get_value_from_json_key(smtp, "MachineName", this->smtp.smtp_machineName);
    get_value_from_json_key(smtp, "SmtpSenderAddress", this->smtp.smtp_sender_address);
    get_value_from_json_key(smtp, "SmtpServer", this->smtp.smtp_server);    get_value_from_json_key(smtp, "SmtpUserName", this->smtp.smtp_username);
    get_value_from_json_key(smtp, "SmtpPassword", this->smtp.smtp_password);

    get_value_from_json_key(smtp, "SecondSmtpServer", this->smtp.smtp_second_server);
    get_value_from_json_key(smtp, "SecondSmtpUserName", this->smtp.smtp_second_username);
    get_value_from_json_key(smtp, "SecondSmtpPassword", this->smtp.smtp_second_password);
    get_value_from_json_key(smtp, "SmtpSSLEnabled",
                             this->smtp.smtp_smtp_enabled);
    // get_value_from_json_key(smtp, "SmtpServer", this->smtp.smtp_server);
    // get_value_from_json_key(smtp, "SmtpUserName", this->smtp.smtp_username);
    // get_value_from_json_key(smtp, "SmtpPassword", this->smtp.smtp_password);
    // get_value_from_json_key(smtp, "SmtpSenderAddress",
    //                         this->smtp.smtp_sender_address);
    this->smtp.smtp_port=smtp.at("SmtpPortNumber").as_integer();
    get_value_from_json_key(smtp, "SmtpPortNumber", this->smtp.smtp_port);
    get_value_from_json_key(smtp, "SmtpSSLEnabled",
                            this->smtp.smtp_smtp_enabled);
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value EventService::SubmitTestEvent(json::value body) {
  Event_Info e;
  json::value args;
  string egi;

  // #1 get json value
  if (!get_value_from_json_key(body, "MessageId", e.message.id))
    return json::value::null();

  // if (!get_value_from_json_key(body, "EventGroupId", e.event_group_id))
  //   e.event_group_id = 0;
  // egi = to_string(e.event_group_id);

  if (!get_value_from_json_key(body, "EventId", e.event_id))
    e.event_id = "Test Event";

  if (!get_value_from_json_key(body, "EventTimestamp", e.event_timestamp))
    e.event_timestamp = currentDateTime();

  if (!get_value_from_json_key(body, "EventType", e.event_type))
    e.event_type = "Alert";

  if (!get_value_from_json_key(body, "Message", e.message.message))
    e.message.message = "This is Test Message";

  if (get_value_from_json_key(body, "MessageArgs", args)) {
    for (auto arg : args.as_array())
      e.message.message_args.push_back(arg.as_string());
    // for(int i=0; i<args.size(); i++)
    // {
    //     e.message_args.push_back(args[i].as_string());
    // }
  }
  // if (!get_value_from_json_key(body, "OriginOfCondition", e.origin_of_condition))
  //   e.origin_of_condition = "SubmitTestEvent";

  if (!get_value_from_json_key(body, "Severity", e.message.severity))
    e.message.severity = "OK";

  Event *event = new Event();
  event->events.push_back(e);

  // test_send_event(*event); // 이거 실제동작

  // #2 make event and return
  // if (record_is_exist(egi))
  //     event_map[egi]->events.push_back(e);
  // else{
  //     Event *event = new Event(egi);
  //     event->events.push_back(e);
  //     event_map[egi] = event;
  // }

  return event->get_json();
  // return event_map[egi]->get_json();
}
// Event Service & Event Destination end

// Update Service & Software Inventory start
json::value UpdateService::get_json(void) {
  json::value j = this->Resource::get_json();
  j[U("Id")] = json::value::string(U(this->id));
  j[U("ServiceEnabled")] = json::value::boolean(U(this->service_enabled));
  j[U("HttpPushUri")] = json::value::string(U(this->http_push_uri));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j["FirmwareInventory"] = get_resource_odata_id_json(this->firmware_inventory, this->odata.id);
  j["SoftwareInventory"] = get_resource_odata_id_json(this->software_inventory, this->odata.id);


  j["Actions"] = get_action_info(this->actions);


  return j;
}

bool UpdateService::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->service_enabled = j.at("ServiceEnabled").as_bool();
    this->http_push_uri = j.at("HttpPushUri").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

bool UpdateService::SimpleUpdate(json::value body) {
  string image_uri; // <bmc ip>/<path and image file name>
  string transfer_protocol;
  json::value j_targets;
  vector<string> targets;

  // #1 get request body
  get_value_from_json_key(body, "ImageURI", image_uri);
  get_value_from_json_key(body, "TransferProtocol", transfer_protocol);
  get_value_from_json_key(body, "Targets", j_targets);
  for (auto target : j_targets.as_array()) {
    targets.push_back(target.as_string());
  }

  // #2 create task

  // #3 get uri to check the updating process

  // #4 process restart
  return true;
}

json::value SoftwareInventory::get_json(void) {
  json::value j = this->Resource::get_json();
  j[U("Id")] = json::value::string(U(this->id));
  j[U("Updatable")] = json::value::boolean(U(this->updatable));
  j[U("Manufacturer")] = json::value::string(U(this->manufacturer));
  j[U("ReleaseDate")] = json::value::string(U(this->release_date));
  j[U("Version")] = json::value::string(U(this->version));
  j[U("SoftwareId")] = json::value::string(U(this->software_id));
  j[U("LowestSupportedVersion")] = json::value::string(U(this->lowest_supported_version));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("UefiDevicePaths")] = json::value::array();
  for (int i = 0; i < this->uefi_device_paths.size(); i++)
    j[U("UefiDevicePaths")][i] =
        json::value::string(U(this->uefi_device_paths[i]));

  j["Actions"] = get_action_info(this->actions);

  return j;
}

bool SoftwareInventory::load_json(json::value &j) {
  json::value status;
  json::value uefi_device_paths;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->updatable = j.at("Updatable").as_bool();
    this->manufacturer = j.at("Manufacturer").as_string();
    this->release_date = j.at("ReleaseDate").as_string();
    this->version = j.at("Version").as_string();
    this->software_id = j.at("SoftwareId").as_string();
    this->lowest_supported_version = j.at("LowestSupportedVersion").as_string();

    uefi_device_paths = j.at("UefiDevicePaths");
    for (auto str : uefi_device_paths.as_array())
      this->uefi_device_paths.push_back(str.as_string());

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Update Service & Software Inventory end

// Temperature start
json::value Temperature::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value k;

  j[U("MemberId")] = json::value::string(U(this->member_id));
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("SensorNumber")] = json::value::number(U(this->sensor_num));
  j[U("ReadingCelsius")] = json::value::number(U(this->reading_celsius));
  j[U("UpperThresholdNonCritical")] =
      json::value::number(U(this->upper_threshold_non_critical));
  j[U("UpperThresholdCritical")] =
      json::value::number(U(this->upper_threshold_critical));
  j[U("UpperThresholdFatal")] =
      json::value::number(U(this->upper_threshold_fatal));
  j[U("LowerThresholdNonCritical")] =
      json::value::number(U(this->lower_threshold_non_critical));
  j[U("LowerThresholdCritical")] =
      json::value::number(U(this->lower_threshold_critical));
  j[U("LowerThresholdFatal")] =
      json::value::number(U(this->lower_threshold_fatal));

  j[U("MinReadingRangeTemp")] =
      json::value::number(U(this->min_reading_range_temp));
  j[U("MaxReadingRangeTemp")] =
      json::value::number(U(this->max_reading_range_temp));
  j[U("PhysicalContext")] = json::value::string(U(this->physical_context));

  return j;
}

bool Temperature::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->member_id = j.at("MemberId").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    this->sensor_num = j.at("SensorNumber").as_integer();
    this->reading_celsius = j.at("ReadingCelsius").as_double();
    this->upper_threshold_non_critical =
        j.at("UpperThresholdNonCritical").as_double();
    this->upper_threshold_critical = j.at("UpperThresholdCritical").as_double();
    this->upper_threshold_fatal = j.at("UpperThresholdFatal").as_double();
    this->lower_threshold_non_critical =
        j.at("LowerThresholdNonCritical").as_double();
    this->lower_threshold_critical = j.at("LowerThresholdCritical").as_double();
    this->lower_threshold_fatal = j.at("LowerThresholdFatal").as_double();

    this->min_reading_range_temp = j.at("MinReadingRangeTemp").as_double();
    this->max_reading_range_temp = j.at("MaxReadingRangeTemp").as_double();
    this->physical_context = j.at("PhysicalContext").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

// pplx::task<void> Temperature::read(uint8_t _sensor_index, uint8_t
// _sensor_context)
// {
//     double *reading_celsius = &this->reading_celsius;
//     bool *thread = &this->thread;
//     *thread = true;

//     switch (_sensor_context)
//     {
//     case INTAKE_CONTEXT:
//         return pplx::create_task([thread, reading_celsius, _sensor_index] {
//             while (*thread)
//             {
//                 *reading_celsius =
//                 round(get_intake_temperature(_sensor_index)); sleep(1);
//             }
//         });
//     case CPU_CONTEXT:
//         return pplx::create_task([thread, reading_celsius, _sensor_index] {
//             while (*thread)
//             {
//                 *reading_celsius = round(get_cpu_temperature(_sensor_index));
//                 sleep(1);
//             }
//         });
//     }
//     return pplx::create_task([]{});
// }
// Temperature end

// Fan start
json::value Fan::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value k;

  j[U("MemberId")] = json::value::string(U(this->member_id));

  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("SensorNumber")] = json::value::number(U(this->sensor_num));
  j[U("Reading")] = json::value::number(U(this->reading));
  j[U("ReadingUnits")] = json::value::string(U(this->reading_units));

  j[U("UpperThresholdNonCritical")] =
      json::value::number(U(this->upper_threshold_non_critical));
  j[U("UpperThresholdCritical")] =
      json::value::number(U(this->upper_threshold_critical));
  j[U("UpperThresholdFatal")] =
      json::value::number(U(this->upper_threshold_fatal));
  j[U("LowerThresholdNonCritical")] =
      json::value::number(U(this->lower_threshold_non_critical));
  j[U("LowerThresholdCritical")] =
      json::value::number(U(this->lower_threshold_critical));
  j[U("LowerThresholdFatal")] =
      json::value::number(U(this->lower_threshold_fatal));

  j[U("MinReadingRange")] = json::value::number(U(this->min_reading_range));
  j[U("MaxReadingRange")] = json::value::number(U(this->max_reading_range));
  j[U("PhysicalContext")] = json::value::string(U(this->physical_context));

  return j;
}

bool Fan::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->member_id = j.at("MemberId").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    this->sensor_num = j.at("SensorNumber").as_integer();
    this->reading = j.at("Reading").as_integer();
    this->reading_units = j.at("ReadingUnits").as_string();

    this->upper_threshold_non_critical =
        j.at("UpperThresholdNonCritical").as_integer();
    this->upper_threshold_critical =
        j.at("UpperThresholdCritical").as_integer();
    this->upper_threshold_fatal = j.at("UpperThresholdFatal").as_integer();
    this->lower_threshold_non_critical =
        j.at("LowerThresholdNonCritical").as_integer();
    this->lower_threshold_critical =
        j.at("LowerThresholdCritical").as_integer();
    this->lower_threshold_fatal = j.at("LowerThresholdFatal").as_integer();

    this->min_reading_range = j.at("MinReadingRange").as_integer();
    this->max_reading_range = j.at("MaxReadingRange").as_integer();
    this->physical_context = j.at("PhysicalContext").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Fan end

// Thermal start
json::value Thermal::get_json(void) {
  unsigned int i;
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Temperatures")] = json::value::array();
  for (i = 0; i < this->temperatures->members.size(); i++)
    j[U("Temperatures")][i] =
        ((Temperature *)this->temperatures->members[i])->get_json();
  j[U("Fans")] = json::value::array();
  for (i = 0; i < this->fans->members.size(); i++)
    j[U("Fans")][i] = ((Fan *)this->fans->members[i])->get_json();

  return j;
}

bool Thermal::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();

    // temperature list, fan list
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Thermal end

// Sensor start
json::value Sensor::get_json(void) {
  json::value j;
  j = this->Resource::get_json();
  j[U("Id")] = json::value::string(U(this->id));
  j[U("ReadingType")] = json::value::string(U(this->reading_type));
  j[U("ReadingTime")] = json::value::string(U(this->reading_time));

  // ostringstream o;
  // o << this->reading;
  // j[U("Reading")] = json::value::string(U(o.str()));
  // 이렇게 하면 소수점 다 안나오게 할 수 있네 double형인거? 나중에 참고

  j[U("Reading")] = json::value::number(U(this->reading));
  j[U("ReadingUnits")] = json::value::string(U(this->reading_units));
  j[U("ReadingRangeMin")] = json::value::number(U(this->reading_range_min));
  j[U("ReadingRangeMax")] = json::value::number(U(this->reading_range_max));
  j[U("Accuracy")] = json::value::number(U(this->accuracy));
  j[U("Precision")] = json::value::number(U(this->precision));
  j[U("SensingInterval")] = json::value::string(U(this->sensing_interval));
  j[U("PhysicalContext")] = json::value::string(U(this->physical_context));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  json::value thresh;
  json::value i;
  i[U("Reading")] = json::value::number(U(this->thresh.upper_critical.reading));
  i[U("Activation")] =
      json::value::string(U(this->thresh.upper_critical.activation));
  thresh[U("UpperCritical")] = i;

  i[U("Reading")] = json::value::number(U(this->thresh.upper_caution.reading));
  i[U("Activation")] =
      json::value::string(U(this->thresh.upper_caution.activation));
  thresh[U("UpperCaution")] = i;

  i[U("Reading")] = json::value::number(U(this->thresh.upper_fatal.reading));
  i[U("Activation")] =
      json::value::string(U(this->thresh.upper_fatal.activation));
  thresh[U("UpperFatal")] = i;

  i[U("Reading")] = json::value::number(U(this->thresh.lower_critical.reading));
  i[U("Activation")] =
      json::value::string(U(this->thresh.lower_critical.activation));
  thresh[U("LowerCritical")] = i;

  i[U("Reading")] = json::value::number(U(this->thresh.lower_caution.reading));
  i[U("Activation")] =
      json::value::string(U(this->thresh.lower_caution.activation));
  thresh[U("LowerCaution")] = i;

  i[U("Reading")] = json::value::number(U(this->thresh.lower_fatal.reading));
  i[U("Activation")] =
      json::value::string(U(this->thresh.lower_fatal.activation));
  thresh[U("LowerFatal")] = i;

  j[U("Thresholds")] = thresh;

  return j;
}

bool Sensor::load_json(json::value &j) {
  json::value status, threshold;
  json::value upper_critical, upper_caution, upper_fatal, lower_caution,
      lower_critical, lower_fatal;
  int step = 0;

  try {
    Resource::load_json(j);

    this->id = j.at("Id").as_string();
    this->reading_type = j.at("ReadingType").as_string();
    this->reading_time = j.at("ReadingTime").as_string();
    this->reading = j.at("Reading").as_double();
    this->reading_units = j.at("ReadingUnits").as_string();
    this->reading_range_max = j.at("ReadingRangeMax").as_double();
    this->reading_range_min = j.at("ReadingRangeMin").as_double();
    this->accuracy = j.at("Accuracy").as_double();
    this->precision = j.at("Precision").as_double();
    this->physical_context = j.at("PhysicalContext").as_string();
    this->sensing_interval = j.at("SensingInterval").as_string();

    step = 1;
    threshold = j.at("Thresholds");
    upper_critical = threshold.at("UpperCritical");
    this->thresh.upper_critical.reading =
        upper_critical.at("Reading").as_double();
    this->thresh.upper_critical.activation =
        upper_critical.at("Activation").as_string();

    step = 2;
    upper_caution = threshold.at("UpperCaution");
    this->thresh.upper_caution.reading =
        upper_caution.at("Reading").as_double();
    this->thresh.upper_caution.activation =
        upper_caution.at("Activation").as_string();

    step = 3;
    upper_fatal = threshold.at("UpperFatal");

    this->thresh.upper_fatal.reading = upper_fatal.at("Reading").as_double();
    this->thresh.upper_fatal.activation =
        upper_fatal.at("Activation").as_string();

    step = 4;
    lower_critical = threshold.at("LowerCritical");
    this->thresh.lower_critical.reading =
        lower_critical.at("Reading").as_double();
    this->thresh.lower_critical.activation =
        lower_critical.at("Activation").as_string();

    step = 5;
    lower_caution = threshold.at("LowerCaution");
    this->thresh.lower_caution.reading =
        lower_caution.at("Reading").as_double();
    this->thresh.lower_caution.activation =
        lower_caution.at("Activation").as_string();

    step = 6;
    lower_fatal = threshold.at("LowerFatal");
    this->thresh.lower_fatal.reading = lower_fatal.at("Reading").as_double();
    this->thresh.lower_fatal.activation =
        lower_fatal.at("Activation").as_string();

    step = 7;
    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

  } catch (json::json_exception &e) {
    log(warning) << "read something failed in sensor : " << this->odata.id
                 << " at step " << step;

    return true;
  }

  return true;
}
// Sensor end

// Power &  PowerControl, Voltage, PowerSupply start
json::value PowerSupply::get_json(void) {
  auto j = this->Resource::get_json(NO_DATA_TYPE);
  if (j.is_null())
    return j;

  json::value k;

  j[U("MemberId")] = json::value::string(U(this->member_id));

  j[U("PowerSupplyType")] = json::value::string(U(this->power_supply_type));
  j[U("LineInputVoltageType")] =
      json::value::string(U(this->line_input_voltage_type));
  j[U("LineInputVoltage")] = json::value::number(U(this->line_input_voltage));
  j[U("PowerCapacityWatts")] =
      json::value::number(U(this->power_capacity_watts));
  j[U("LastPowerOutputWatts")] =
      json::value::number(U(this->last_power_output_watts));

  j[U("Model")] = json::value::string(U(this->model));
  j[U("Manufacturer")] = json::value::string(U(this->manufacturer));
  j[U("FirmwareVersion")] = json::value::string(U(this->firmware_version));
  j[U("SerialNumber")] = json::value::string(U(this->serial_number));
  j[U("PartNumber")] = json::value::string(U(this->part_number));
  j[U("SparePartNumber")] = json::value::string(U(this->spare_part_number));

  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("InputRanges")] = json::value::array();
  for (int i = 0; i < this->input_ranges.size(); i++) {
    json::value l = json::value::object();
    l[U("InputType")] =
        json::value::string(U(this->input_ranges[i].input_type));
    l[U("MinimumVoltage")] =
        json::value::number(U(this->input_ranges[i].minimum_voltage));
    l[U("MaximumVoltage")] =
        json::value::number(U(this->input_ranges[i].maximum_voltage));
    l[U("OutputWattage")] =
        json::value::number(U(this->input_ranges[i].output_wattage));
    j[U("InputRanges")][i] = l;
  }

  return j;
}

bool PowerSupply::load_json(json::value &j) {
  json::value status, input_ranges;

  try {
    Resource::load_json(j, NO_DATA_TYPE);
    this->member_id = j.at("MemberId").as_string();

    this->power_supply_type = j.at("PowerSupplyType").as_string();
    this->line_input_voltage_type = j.at("LineInputVoltageType").as_string();
    this->line_input_voltage = j.at("LineInputVoltage").as_double();
    this->power_capacity_watts = j.at("PowerCapacityWatts").as_double();
    this->last_power_output_watts = j.at("LastPowerOutputWatts").as_double();

    this->model = j.at("Model").as_string();
    this->manufacturer = j.at("Manufacturer").as_string();
    this->firmware_version = j.at("FirmwareVersion").as_string();
    this->serial_number = j.at("SerialNumber").as_string();
    this->part_number = j.at("PartNumber").as_string();
    this->spare_part_number = j.at("SparePartNumber").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    input_ranges = j.at("InputRanges");
    for (auto item : input_ranges.as_array()) {
      InputRange temp;

      temp.input_type = item.at("InputType").as_string();
      temp.minimum_voltage = item.at("MinimumVoltage").as_double();
      temp.maximum_voltage = item.at("MaximumVoltage").as_double();
      temp.output_wattage = item.at("OutputWattage").as_double();

      this->input_ranges.push_back(temp);
    }
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value Voltage::get_json(void) {
  auto j = this->Resource::get_json(NO_DATA_TYPE);
  if (j.is_null())
    return j;

  json::value k;

  j[U("MemberId")] = json::value::string(U(this->member_id));
  j[U("SensorNumber")] = json::value::number(U(this->sensor_num));
  j[U("ReadingVolts")] = json::value::number(U(this->reading_volts));
  j[U("UpperThresholdNonCritical")] =
      json::value::number(U(this->upper_threshold_non_critical));
  j[U("UpperThresholdCritical")] =
      json::value::number(U(this->upper_threshold_critical));
  j[U("UpperThresholdFatal")] =
      json::value::number(U(this->upper_threshold_fatal));
  j[U("LowerThresholdNonCritical")] =
      json::value::number(U(this->lower_threshold_non_critical));
  j[U("LowerThresholdCritical")] =
      json::value::number(U(this->lower_threshold_critical));
  j[U("LowerThresholdFatal")] =
      json::value::number(U(this->lower_threshold_fatal));
  j[U("MinReadingRange")] = json::value::number(U(this->min_reading_range));
  j[U("MaxReadingRange")] = json::value::number(U(this->max_reading_range));
  j[U("PhysicalContext")] = json::value::string(U(this->physical_context));

  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  return j;
}

bool Voltage::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j, NO_DATA_TYPE);
    this->member_id = j.at("MemberId").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    this->sensor_num = j.at("SensorNumber").as_integer();
    this->reading_volts = j.at("ReadingVolts").as_double();

    this->upper_threshold_non_critical =
        j.at("UpperThresholdNonCritical").as_double();
    this->upper_threshold_critical = j.at("UpperThresholdCritical").as_double();
    this->upper_threshold_fatal = j.at("UpperThresholdFatal").as_double();
    this->lower_threshold_non_critical =
        j.at("LowerThresholdNonCritical").as_double();
    this->lower_threshold_critical = j.at("LowerThresholdCritical").as_double();
    this->lower_threshold_fatal = j.at("LowerThresholdFatal").as_double();

    this->min_reading_range = j.at("MinReadingRange").as_double();
    this->max_reading_range = j.at("MaxReadingRange").as_double();
    this->physical_context = j.at("PhysicalContext").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value PowerControl::get_json(void) {
  auto j = this->Resource::get_json(NO_DATA_TYPE);
  if (j.is_null())
    return j;

  json::value k;

  j[U("MemberId")] = json::value::string(U(this->member_id));
  j[U("PowerConsumedWatts")] =
      json::value::number(U(this->power_consumed_watts));
  j[U("PowerRequestedWatts")] =
      json::value::number(U(this->power_requested_watts));
  j[U("PowerAvailableWatts")] =
      json::value::number(U(this->power_available_watts));
  j[U("PowerCapacityWatts")] =
      json::value::number(U(this->power_capacity_watts));
  j[U("PowerAllocatedWatts")] =
      json::value::number(U(this->power_allocated_watts));

  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  json::value metric, limit;
  metric[U("IntervalInMin")] =
      json::value::number(U(this->power_metrics.interval_in_min));
  metric[U("MinConsumedWatts")] =
      json::value::number(U(this->power_metrics.min_consumed_watts));
  metric[U("MaxConsumedWatts")] =
      json::value::number(U(this->power_metrics.max_consumed_watts));
  metric[U("AverageConsumedWatts")] =
      json::value::number(U(this->power_metrics.avg_consumed_watts));
  j[U("PowerMetrics")] = metric;

  limit[U("LimitInWatts")] =
      json::value::number(U(this->power_limit.limit_in_watts));
  limit[U("LimitException")] =
      json::value::string(U(this->power_limit.limit_exception));
  limit[U("CorrectionInMs")] =
      json::value::number(U(this->power_limit.correction_in_ms));
  j[U("PowerLimit")] = limit;

  return j;
}

bool PowerControl::load_json(json::value &j) {
  json::value status, power_limit, power_metrics;

  try {
    Resource::load_json(j, NO_DATA_TYPE);
    this->member_id = j.at("MemberId").as_string();
    this->power_consumed_watts = j.at("PowerConsumedWatts").as_double();
    this->power_requested_watts = j.at("PowerRequestedWatts").as_double();
    this->power_available_watts = j.at("PowerAvailableWatts").as_double();
    this->power_capacity_watts = j.at("PowerCapacityWatts").as_double();
    this->power_allocated_watts = j.at("PowerAvailableWatts").as_double();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    power_metrics = j.at("PowerMetrics");
    this->power_metrics.interval_in_min =
        power_metrics.at("IntervalInMin").as_integer();
    this->power_metrics.min_consumed_watts =
        power_metrics.at("MinConsumedWatts").as_double();
    this->power_metrics.max_consumed_watts =
        power_metrics.at("MaxConsumedWatts").as_double();
    this->power_metrics.avg_consumed_watts =
        power_metrics.at("AverageConsumedWatts").as_double();

    power_limit = j.at("PowerLimit");
    this->power_limit.limit_in_watts =
        power_limit.at("LimitInWatts").as_double();
    this->power_limit.limit_exception =
        power_limit.at("LimitException").as_string();
    this->power_limit.correction_in_ms =
        power_limit.at("CorrectionInMs").as_integer();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value Power::get_json(void) {
  unsigned int i;
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));

  j[U("PowerControl")] = json::value::array();
  for (i = 0; i < this->power_control->members.size(); i++)
    j[U("PowerControl")][i] =
        ((PowerControl *)this->power_control->members[i])->get_json();
  j[U("Voltages")] = json::value::array();
  for (i = 0; i < this->voltages->members.size(); i++)
    j[U("Voltages")][i] = ((Voltage *)this->voltages->members[i])->get_json();
  j[U("PowerSupplies")] = json::value::array();
  for (i = 0; i < this->power_supplies->members.size(); i++)
    j[U("PowerSupplies")][i] =
        ((PowerSupply *)this->power_supplies->members[i])->get_json();

  return j;
}

bool Power::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Power &  PowerControl, Voltage, PowerSupply end

// Chassis start
json::value Chassis::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value k;
  json::value l;
  j[U("Id")] = json::value::string(U(this->id));
  j[U("ChassisType")] = json::value::string(U(this->chassis_type));
  j[U("Manufacturer")] = json::value::string(U(this->manufacturer));
  j[U("Model")] = json::value::string(U(this->model));
  j[U("SerialNumber")] = json::value::string(U(this->serial_number));
  j[U("PartNumber")] = json::value::string(U(this->part_number));
  j[U("AssetTag")] = json::value::string(U(this->asset_tag));
  j[U("PowerState")] = json::value::string(U(this->power_state));

  switch (this->indicator_led) {
  case LED_OFF:
    j[U("IndicatorLED")] = json::value::string(U("Off"));
    break;
  case LED_BLINKING:
    j[U("IndicatorLED")] = json::value::string(U("Blinking"));
    break;
  case LED_LIT:
    j[U("IndicatorLED")] = json::value::string(U("Lit"));
    break;
  }

  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  k = json::value::object();
  l[U("Country")] =
      json::value::string(U(this->location.postal_address.country));
  l[U("Territory")] =
      json::value::string(U(this->location.postal_address.territory));
  l[U("City")] = json::value::string(U(this->location.postal_address.city));
  l[U("Street")] = json::value::string(U(this->location.postal_address.street));
  l[U("HouseNumber")] =
      json::value::string(U(this->location.postal_address.house_number));
  l[U("Name")] = json::value::string(U(this->location.postal_address.name));
  l[U("PostalCode")] =
      json::value::string(U(this->location.postal_address.postal_code));
  k[U("PostalAddress")] = l;

  l = json::value::object();
  l[U("Row")] = json::value::string(U(this->location.placement.row));
  l[U("Rack")] = json::value::string(U(this->location.placement.rack));
  l[U("RackOffsetUnits")] =
      json::value::string(U(this->location.placement.rack_offset_units));
  l[U("RackOffset")] =
      json::value::number(U(this->location.placement.rack_offset));
  k[U("Placement")] = l;
  j[U("Location")] = k;

  j["Thermal"] = get_resource_odata_id_json(this->thermal, this->odata.id);
  j["Power"] = get_resource_odata_id_json(this->power, this->odata.id);
  j["Storage"] = get_resource_odata_id_json(this->storage, this->odata.id);
  j["Sensors"] = get_resource_odata_id_json(this->sensors, this->odata.id);
  j["LogServices"] =
      get_resource_odata_id_json(this->log_service, this->odata.id);

  return j;
}

bool Chassis::load_json(json::value &j) {
  json::value status, loc, postal_address, placement;
  string s_led;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->chassis_type = j.at("ChassisType").as_string();
    this->manufacturer = j.at("Manufacturer").as_string();
    this->model = j.at("Model").as_string();
    this->serial_number = j.at("SerialNumber").as_string();
    this->part_number = j.at("PartNumber").as_string();
    this->asset_tag = j.at("AssetTag").as_string();
    this->power_state = j.at("PowerState").as_string();

    s_led = j.at("IndicatorLED").as_string();
    if (s_led == "Off")
      this->indicator_led = LED_OFF;
    else if (s_led == "Lit")
      this->indicator_led = LED_LIT;
    else if (s_led == "Blinking")
      this->indicator_led = LED_BLINKING;
    else
      log(warning) << "led value is strange..";

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    loc = j.at("Location");
    postal_address = loc.at("PostalAddress");
    this->location.postal_address.country =
        postal_address.at("Country").as_string();
    this->location.postal_address.territory =
        postal_address.at("Territory").as_string();
    this->location.postal_address.city = postal_address.at("City").as_string();
    this->location.postal_address.street =
        postal_address.at("Street").as_string();
    this->location.postal_address.house_number =
        postal_address.at("HouseNumber").as_string();
    this->location.postal_address.name = postal_address.at("Name").as_string();
    this->location.postal_address.postal_code =
        postal_address.at("PostalCode").as_string();

    placement = loc.at("Placement");
    this->location.placement.row = placement.at("Row").as_string();
    this->location.placement.rack = placement.at("Rack").as_string();
    this->location.placement.rack_offset_units =
        placement.at("RackOffsetUnits").as_string();
    this->location.placement.rack_offset =
        placement.at("RackOffset").as_integer();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

// bool Chassis::ResetChassis(string _reset_type)
// {
//     if (_reset_type == "ForceOff"){

//     }
//     if (_reset_type == "ForceOn"){

//     }
//     if (_reset_type == "ForceRestart"){

//     }
//     if (_reset_type == "GracefulRestart"){

//     }
//     if (_reset_type == "GracefulShutdown"){

//     }
//     if (_reset_type == "Nmi"){

//     }
//     if (_reset_type == "On"){
//         this->power_state = POWER_STATE_ON;
//     }
//     if (_reset_type == "PowerCycle"){

//     }
//     if (_reset_type == "PushPowerButton"){

//     }

// }

void Chassis::new_log_service(string _service_id) {
  if (this->log_service == nullptr) {
    this->log_service = new Collection(this->odata.id + "/LogServices",
                                       ODATA_LOG_SERVICE_COLLECTION_TYPE);
    this->log_service->name = "Log Service Collection";
  }

  init_log_service(this->log_service, _service_id);
}

// pplx::task<void> Chassis::led_off(uint8_t _led_index)
// {
//     uint8_t *indicator_led = &this->indicator_led;
//     *indicator_led = LED_OFF;
//     log(info) << this->name << ": LED off";
//     return pplx::create_task([indicator_led, _led_index] {
//         GPIO_CLR = 1 << _led_index;
//     });
// }

// pplx::task<void> Chassis::led_lit(uint8_t _led_index)
// {
//     uint8_t *indicator_led = &this->indicator_led;
//     *indicator_led = LED_LIT;
//     log(info) << this->name << ": LED lit";
//     return pplx::create_task([indicator_led, _led_index] {
//         GPIO_SET = 1 << _led_index;
//     });
// }

// pplx::task<void> Chassis::led_blinking(uint8_t _led_index)
// {

//     uint8_t *indicator_led = &this->indicator_led;
//     *indicator_led = LED_BLINKING;
//     log(info) << this->name << ": LED blinking";
//     return pplx::create_task([indicator_led, _led_index] {
//         while (*indicator_led == LED_BLINKING)
//         {
//             GPIO_CLR = 1 << _led_index;
//             usleep(30000);
//             GPIO_SET = 1 << _led_index;
//             usleep(30000);
//         }
//     });
// }
// Chassis end

// Manager start
json::value Manager::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("ManagerType")] = json::value::string(U(this->manager_type));
  j[U("Description")] = json::value::string(U(this->description));
  j[U("FirmwareVersion")] = json::value::string(U(this->firmware_version));
  j[U("DateTime")] = json::value::string(U(this->datetime));
  j[U("DateTimeLocalOffset")] = json::value::string(U(this->datetime_offset));
  j[U("Model")] = json::value::string(U(this->model));
  j[U("UUID")] = json::value::string(U(this->uuid));
  j[U("PowerState")] = json::value::string(U(this->power_state));

  j["NetworkProtocol"] =
      get_resource_odata_id_json(this->network, this->odata.id);
  j["EthernetInterfaces"] =
      get_resource_odata_id_json(this->ethernet, this->odata.id);
  j["LogServices"] =
      get_resource_odata_id_json(this->log_service, this->odata.id);
  j["VirtualMedia"] =
      get_resource_odata_id_json(this->virtual_media, this->odata.id);

  j["Radius"] = get_resource_odata_id_json(this->radius, this->odata.id);

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;
  j["GlobaleEnable"] = json::value::number(KETI_define::global_enabler);
  j["Actions"] = get_action_info(this->actions);

  return j;
}

bool Manager::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->manager_type = j.at("ManagerType").as_string();
    this->description = j.at("Description").as_string();
    this->uuid = j.at("UUID").as_string();
    this->model = j.at("Model").as_string();
    this->firmware_version = j.at("FirmwareVersion").as_string();
    this->datetime = j.at("DateTime").as_string();
    this->datetime_offset = j.at("DateTimeLocalOffset").as_string();
    this->power_state = j.at("PowerState").as_string();
    KETI_define::global_enabler = status.at("GlobaleEnable").as_integer();
    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

void Manager::new_log_service(string _service_id) {
  if (this->log_service == nullptr) {
    this->log_service = new Collection(this->odata.id + "/LogServices",
                                       ODATA_LOG_SERVICE_COLLECTION_TYPE);
    this->log_service->name = "Log Service Collection";
  }

  init_log_service(this->log_service, _service_id);
}
// Manager end

// Radius start
json::value Radius::get_json(void) {
  json::value j;
  j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["Id"] = json::value::string(this->id);
  j["RadiusEnabled"] = json::value::boolean(this->radius_enabled);
  j["RadiusServer"] = json::value::string(this->radius_server);
  j["RadiusPortNumber"] = json::value::number(this->radius_port);
  j["RadiusSecret"] = json::value::string(this->radius_secret);

  return j;
}

bool Radius::load_json(json::value &j) {
  try {
    Resource::load_json(j);
    get_value_from_json_key(j, "Id", this->id);
    get_value_from_json_key(j, "RadiusEnabled", this->radius_enabled);
    get_value_from_json_key(j, "RadiusServer", this->radius_server);
    get_value_from_json_key(j, "RadiusPortNumber", this->radius_port);
    get_value_from_json_key(j, "RadiusSecret", this->radius_secret);

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Radius end

// EthernetInterface start
json::value EthernetInterfaces::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Description")] = json::value::string(U(this->description));
  j[U("LinkStatus")] = json::value::string(U(this->link_status));
  j[U("PermanentMACAddress")] =
      json::value::string(U(this->permanent_mac_address));
  j[U("MACAddress")] = json::value::string(U(this->mac_address));
  j[U("SpeedMbps")] = json::value::number(U(this->speed_Mbps));
  j[U("AutoNeg")] = json::value::boolean(U(this->autoneg));
  j[U("FullDuplex")] = json::value::boolean(U(this->full_duplex));
  j[U("MTUSize")] = json::value::number(U(this->mtu_size));
  j[U("HostName")] = json::value::string(U(this->hostname));
  j[U("FQDN")] = json::value::string(U(this->fqdn));
  j[U("IPv6DefaultGateway")] =
      json::value::string(U(this->ipv6_default_gateway));
  j[U("InterfaceEnabled")] = json::value::boolean(U(this->interfaceEnabled));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("NameServers")] = json::value::array(); // 네임서버
  for (int i = 0; i < this->name_servers.size(); i++)
    j[U("NameServers")][i] = json::value::string(U((this->name_servers)[i]));

  json::value dh_v4, dh_v6;
  json::value v_ip4, v_ip6;
  json::value o_ip4, o_ip6;
  json::value vlan;

  dh_v4[U("DHCPEnabled")] = json::value::boolean(U(this->dhcp_v4.dhcp_enabled));
  dh_v4[U("UseDNSServers")] =
      json::value::boolean(U(this->dhcp_v4.use_dns_servers));
  dh_v4[U("UseGateway")] = json::value::boolean(U(this->dhcp_v4.use_gateway));
  dh_v4[U("UseNTPServers")] =
      json::value::boolean(U(this->dhcp_v4.use_ntp_servers));
  dh_v4[U("UseStaticRoutes")] =
      json::value::boolean(U(this->dhcp_v4.use_static_routes));
  dh_v4[U("UseDomainName")] =
      json::value::boolean(U(this->dhcp_v4.use_domain_name));
  j[U("DHCPv4")] = dh_v4;

  dh_v6[U("OperatingMode")] =
      json::value::string(U(this->dhcp_v6.operating_mode));
  dh_v6[U("UseDNSServers")] =
      json::value::boolean(U(this->dhcp_v6.use_dns_servers));
  dh_v6[U("UseDomainName")] =
      json::value::boolean(U(this->dhcp_v6.use_domain_name));
  dh_v6[U("UseNTPServers")] =
      json::value::boolean(U(this->dhcp_v6.use_ntp_servers));
  dh_v6[U("UseRapidCommit")] =
      json::value::boolean(U(this->dhcp_v6.use_rapid_commit));
  j[U("DHCPv6")] = dh_v6;

  v_ip4 = json::value::array();
  for (int i = 0; i < this->v_ipv4.size(); i++) {
    o_ip4 = json::value::object();
    o_ip4[U("Address")] = json::value::string(U(this->v_ipv4[i].address));
    o_ip4[U("SubnetMask")] =
        json::value::string(U(this->v_ipv4[i].subnet_mask));
    o_ip4[U("AddressOrigin")] =
        json::value::string(U(this->v_ipv4[i].address_origin));
    o_ip4[U("Gateway")] = json::value::string(U(this->v_ipv4[i].gateway));
    v_ip4[i] = o_ip4;
  }
  j[U("IPv4Addresses")] = v_ip4;

  v_ip6 = json::value::array();
  for (int i = 0; i < this->v_ipv6.size(); i++) {
    o_ip6 = json::value::object();
    o_ip6[U("Address")] = json::value::string(U(this->v_ipv6[i].address));
    o_ip6[U("AddressState")] =
        json::value::string(U(this->v_ipv6[i].address_state));
    o_ip6[U("AddressOrigin")] =
        json::value::string(U(this->v_ipv6[i].address_origin));
    o_ip6[U("PrefixLength")] =
        json::value::number(U(this->v_ipv6[i].prefix_length));
    v_ip6[i] = o_ip6;
  }
  j[U("IPv6Addresses")] = v_ip6;

  vlan["VLANEnable"] = json::value::boolean(this->vlan.vlan_enable);
  vlan["VLANId"] = json::value::number(this->vlan.vlan_id);
  j["VLAN"] = vlan;

  return j;
}

bool EthernetInterfaces::load_json(json::value &j) {
  json::value status;
  json::value name_servers, dhcp_v4, dhcp_v6, v_ipv4, v_ipv6, vlan;
  int step = 0;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->description = j.at("Description").as_string();
    this->link_status = j.at("LinkStatus").as_string();
    this->permanent_mac_address = j.at("PermanentMACAddress").as_string();
    this->mac_address = j.at("MACAddress").as_string();
    this->speed_Mbps = j.at("SpeedMbps").as_integer();
    this->autoneg = j.at("AutoNeg").as_bool();
    this->full_duplex = j.at("FullDuplex").as_bool();
    this->mtu_size = j.at("MTUSize").as_integer();
    this->hostname = j.at("HostName").as_string();
    this->fqdn = j.at("FQDN").as_string();
    this->interfaceEnabled = j.at("InterfaceEnabled").as_bool();

    step = 1;
    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    step = 2;
    name_servers = j.at("NameServers");
    for (auto item : name_servers.as_array())
      this->name_servers.push_back(item.as_string());

    step = 3;
    dhcp_v4 = j.at("DHCPv4");
    this->dhcp_v4.dhcp_enabled = dhcp_v4.at("DHCPEnabled").as_bool();
    this->dhcp_v4.use_dns_servers = dhcp_v4.at("UseDNSServers").as_bool();
    this->dhcp_v4.use_domain_name = dhcp_v4.at("UseDomainName").as_bool();
    this->dhcp_v4.use_gateway = dhcp_v4.at("UseGateway").as_bool();
    this->dhcp_v4.use_ntp_servers = dhcp_v4.at("UseNTPServers").as_bool();
    this->dhcp_v4.use_static_routes = dhcp_v4.at("UseStaticRoutes").as_bool();

    step = 4;
    dhcp_v6 = j.at("DHCPv6");
    this->dhcp_v6.operating_mode = dhcp_v6.at("OperatingMode").as_string();
    this->dhcp_v6.use_dns_servers = dhcp_v6.at("UseDNSServers").as_bool();
    this->dhcp_v6.use_domain_name = dhcp_v6.at("UseDomainName").as_bool();
    this->dhcp_v6.use_ntp_servers = dhcp_v6.at("UseNTPServers").as_bool();
    this->dhcp_v6.use_rapid_commit = dhcp_v6.at("UseRapidCommit").as_bool();

    step = 5;
    v_ipv4 = j.at("IPv4Addresses");
    for (auto item : v_ipv4.as_array()) {
      IPv4_Address temp;
      temp.address = item.at("Address").as_string();
      temp.address_origin = item.at("AddressOrigin").as_string();
      temp.subnet_mask = item.at("SubnetMask").as_string();
      temp.gateway = item.at("Gateway").as_string();
      this->v_ipv4.push_back(temp);
    }

    step = 6;
    v_ipv6 = j.at("IPv6Addresses");
    for (auto item : v_ipv6.as_array()) {
      IPv6_Address temp;
      temp.address = item.at("Address").as_string();
      temp.address_origin = item.at("AddressOrigin").as_string();
      temp.address_state = item.at("AddressState").as_string();
      temp.prefix_length = item.at("PrefixLength").as_integer();
      this->v_ipv6.push_back(temp);
    }
    this->ipv6_default_gateway = j.at("IPv6DefaultGateway").as_string();

    step = 7;
    vlan = j.at("VLAN");
    this->vlan.vlan_enable = vlan.at("VLANEnable").as_bool();
    this->vlan.vlan_id = vlan.at("VLANId").as_integer();
  } catch (json::json_exception &e) {
    log(warning) << "read something failed in ethernet_interfaces : "
                 << this->odata.id << " at step " << step;

    return true;
  }

  return true;
}
// EthernetInterface end

// NetworkProtocol start
json::value NetworkProtocol::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value snmp, ipmi, ntp, kvmip, https, http, virtual_media, ssh;
  j[U("FQDN")] = json::value::string(U(this->fqdn));
  j[U("Id")] = json::value::string(U(this->id));
  j[U("HostName")] = json::value::string(U(this->hostname));
  j[U("Description")] = json::value::string(U(this->description));

  json::value k = json::value::object();
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  // snmp[U("ProtocolEnabled")] = json::value::boolean(this->snmp_enabled);
  // snmp[U("Port")] = json::value::number(U(this->snmp_port));
  // j[U("SNMP")] = snmp;

  ipmi[U("ProtocolEnabled")] = json::value::boolean(this->ipmi_enabled);
  ipmi[U("Port")] = json::value::number(U(this->ipmi_port));
  j[U("IPMI")] = ipmi;

  virtual_media[U("ProtocolEnabled")] =
      json::value::boolean(this->virtual_media_enabled);
  virtual_media[U("Port")] = json::value::number(U(this->virtual_media_port));
  j[U("VirtualMedia")] = virtual_media;

  ssh[U("ProtocolEnabled")] = json::value::boolean(this->ssh_enabled);
  ssh[U("Port")] = json::value::number(U(this->ssh_port));
  j[U("SSH")] = ssh;

  kvmip[U("ProtocolEnabled")] = json::value::boolean(this->kvmip_enabled);
  kvmip[U("Port")] = json::value::number(U(this->kvmip_port));
  j[U("KVMIP")] = kvmip;

  http[U("ProtocolEnabled")] = json::value::boolean(this->http_enabled);
  http[U("Port")] = json::value::number(U(this->http_port));
  j[U("HTTP")] = http;

  https[U("ProtocolEnabled")] = json::value::boolean(this->https_enabled);
  https[U("Port")] = json::value::number(U(this->https_port));
  j[U("HTTPS")] = https;

  // ntp[U("ProtocolEnabled")] = json::value::boolean(this->ntp_enabled);
  // ntp["NTPServers"] = json::value::array();
  // for (unsigned int i = 0; i < this->v_netservers.size(); i++)
  //   ntp[U("NTPServers")][i] = json::value::string(this->v_netservers[i]);
  // j[U("NTP")] = ntp;

  ntp[U("ProtocolEnabled")] = json::value::boolean(this->ntp.protocol_enabled);
  ntp["Port"] = json::value::number(this->ntp.port);
  ntp["PrimaryNTPServer"] = json::value::string(this->ntp.primary_ntp_server);
  ntp["SecondaryNTPServer"] = json::value::string(this->ntp.secondary_ntp_server);
  ntp["Date"] = json::value::string(this->ntp.date_str);
  ntp["Time"] = json::value::string(this->ntp.time_str);
  ntp["TimeZone"] = json::value::string(this->ntp.timezone);
  ntp["NTPServers"] = json::value::array();
  for (unsigned int i = 0; i < this->ntp.ntp_servers.size(); i++)
      ntp[U("NTPServers")][i] = json::value::string(this->ntp.ntp_servers[i]);
  j[U("NTP")]=ntp;

  snmp["AuthenticationProtocol"] = json::value::string(this->snmp.authentication_protocol);
  snmp["CommunityAccessMode"] = json::value::string(this->snmp.community_access_mode);
  snmp["EnableSNMPv1"] = json::value::boolean(this->snmp.enable_SNMPv1);
  snmp["EnableSNMPv2c"] = json::value::boolean(this->snmp.enable_SNMPv2c);
  snmp["EnableSNMPv3"] = json::value::boolean(this->snmp.enable_SNMPv3);
  snmp["EncryptionProtocol"] = json::value::string(this->snmp.encryption_protocol);
  snmp["HideCommunityStrings"] = json::value::boolean(this->snmp.hide_community_strings);
  snmp["Port"] = json::value::number(this->snmp.port);
  snmp["ProtocolEnabled"] = json::value::boolean(this->snmp.protocol_enabled);
  
  json::value engine_id = json::value::object();
  engine_id["ArchitectureId"] = json::value::string(this->snmp.engine_id.architectureId);
  engine_id["EnterpriseSpecificMethod"] = json::value::string(this->snmp.engine_id.enterpriseSpecificMethod);
  engine_id["PrivateEnterpriseId"] = json::value::string(this->snmp.engine_id.privateEnterpriseId);
  snmp["EngineId"] = engine_id;

  snmp["CommunityStrings"] = json::value::array();
  for(int i=0; i<this->snmp.community_strings.size(); i++){
      json::value community_string = json::value::object();
      community_string["AccessMode"] = json::value::string(this->snmp.community_strings[i].access_mode);
      community_string["CommunityString"] = json::value::string(this->snmp.community_strings[i].community_string);
      community_string["Name"] = json::value::string(this->snmp.community_strings[i].name);
      snmp["CommunityStrings"][i] = community_string;
  }

  j["SNMP"] = snmp;

  j["Certificates"] =
      get_resource_odata_id_json(this->certificates, this->odata.id);
  return j;
}

bool NetworkProtocol::load_json(json::value &j) {
  json::value status;
  json::value obj, v_netservers;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->fqdn = j.at("FQDN").as_string();
    this->hostname = j.at("HostName").as_string();
    this->description = j.at("Description").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    // test
    // obj = j.at("SNMP");
    // get_value_from_json_key(obj, "ProtocolEnabled", this->snmp_enabled);
    // get_value_from_json_key(obj, "Port", this->snmp_port);

    obj = j.at("IPMI");
    get_value_from_json_key(obj, "ProtocolEnabled", this->ipmi_enabled);
    get_value_from_json_key(obj, "Port", this->ipmi_port);

    obj = j.at("KVMIP");
    get_value_from_json_key(obj, "ProtocolEnabled", this->kvmip_enabled);
    get_value_from_json_key(obj, "Port", this->kvmip_port);

    obj = j.at("HTTP");
    get_value_from_json_key(obj, "ProtocolEnabled", this->http_enabled);
    get_value_from_json_key(obj, "Port", this->http_port);

    obj = j.at("HTTPS");
    get_value_from_json_key(obj, "ProtocolEnabled", this->https_enabled);
    get_value_from_json_key(obj, "Port", this->https_port);

    obj = j.at("SSH");
    get_value_from_json_key(obj, "ProtocolEnabled", this->ssh_enabled);
    get_value_from_json_key(obj, "Port", this->ssh_port);


    obj = j.at("NTP");
    get_value_from_json_key(obj, "ProtocolEnabled", this->ntp.protocol_enabled);
    get_value_from_json_key(obj, "Port", this->ntp.port);
    get_value_from_json_key(obj, "PrimaryNTPServer", this->ntp.primary_ntp_server);
    get_value_from_json_key(obj, "SecondaryNTPServer", this->ntp.secondary_ntp_server);
    get_value_from_json_key(obj, "Date", this->ntp.date_str);
    get_value_from_json_key(obj, "Time", this->ntp.time_str);
    get_value_from_json_key(obj, "TimeZone", this->ntp.timezone);
    get_value_from_json_key(obj, "NTPServers", v_netservers);
    if (v_netservers != json::value::null()){
        for (auto str : v_netservers.as_array())
            this->ntp.ntp_servers.push_back(str.as_string());
    }

    obj = j.at("SNMP");
    get_value_from_json_key(obj, "AuthenticationProtocol", this->snmp.authentication_protocol);
    get_value_from_json_key(obj, "CommunityAccessMode", this->snmp.community_access_mode);
    get_value_from_json_key(obj, "EnableSNMPv1", this->snmp.enable_SNMPv1);
    get_value_from_json_key(obj, "EnableSNMPv2c", this->snmp.enable_SNMPv2c);
    get_value_from_json_key(obj, "EnableSNMPv3", this->snmp.enable_SNMPv3);
    get_value_from_json_key(obj, "EncryptionProtocol", this->snmp.encryption_protocol);
    get_value_from_json_key(obj, "HideCommunityStrings", this->snmp.hide_community_strings);
    get_value_from_json_key(obj, "Port", this->snmp.port);
    get_value_from_json_key(obj, "ProtocolEnabled", this->snmp.protocol_enabled);

    json::value community_strings, engine_id;

    get_value_from_json_key(obj, "EngineId", engine_id);
    get_value_from_json_key(engine_id, "ArchitectureId", this->snmp.engine_id.architectureId);
    get_value_from_json_key(engine_id, "EnterpriseSpecificMethod", this->snmp.engine_id.enterpriseSpecificMethod);
    get_value_from_json_key(engine_id, "PrivateEnterpriseId", this->snmp.engine_id.privateEnterpriseId);

    get_value_from_json_key(obj, "CommunityStrings", community_strings);
    if (community_strings != json::value::null()){
        for (auto cs_obj : community_strings.as_array()){
            Community_String temp;
            get_value_from_json_key(cs_obj, "AccessMode", temp.access_mode);
            get_value_from_json_key(cs_obj, "CommunityString", temp.community_string);
            get_value_from_json_key(cs_obj, "Name", temp.name);
            this->snmp.community_strings.push_back(temp);
        }
    }        


  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// NerworkProtocol end

// Task Service start
json::value TaskService::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  json::value k;
  j[U("Id")] = json::value::string(U(this->id));
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;
  j[U("ServiceEnabled")] = json::value::boolean(U(this->service_enabled));
  j[U("DateTime")] = json::value::string(U(this->datetime));

  j["Tasks"] =
      get_resource_odata_id_json(this->task_collection, this->odata.id);

  return j;
}

bool TaskService::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->datetime = j.at("DateTime").as_string();
    this->service_enabled = j.at("ServiceEnabled").as_bool();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Task Service end

// Task start
json::value Task::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("StartTime")] = json::value::string(U(this->start_time));
  j[U("EndTime")] = json::value::string(U(this->end_time));
  j[U("TaskState")] = json::value::string(U(this->task_state));
  j[U("TaskStatus")] = json::value::string(U(this->task_status));

  json::value k;
  json::value l;
  std::map<string, string>::iterator iter;
  for (iter = this->payload.http_headers.begin();
       iter != this->payload.http_headers.end(); iter++) {
    l[U(iter->first)] = json::value::string(U(iter->second));
  }

  k[U("HttpHeaders")] = l;
  k[U("HttpOperation")] = json::value::string(U(this->payload.http_operation));
  k[U("JsonBody")] = this->payload.json_body;
  k[U("TargetUri")] = json::value::string(U(this->payload.target_uri));
  j[U("Payload")] = k;

  return j;
}

bool Task::load_json(json::value &j) {
  json::value payload, http_headers;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->start_time = j.at("StartTime").as_string();
    this->end_time = j.at("EndTime").as_string();
    this->task_state = j.at("TaskState").as_string();
    this->task_status = j.at("TaskStatus").as_string();

    payload = j.at("Payload");
    this->payload.http_operation = payload.at("HttpOperation").as_string();
    this->payload.json_body = payload.at("JsonBody");
    this->payload.target_uri = payload.at("TargetUri").as_string();

    http_headers = payload.at("HttpHeaders");
    if (!http_headers.is_null()) {
      for (auto iter = http_headers.as_object().begin();
           iter != http_headers.as_object().end(); ++iter) {
        this->payload.http_headers[iter->first] = iter->second.as_string();
      }
    }
  } catch (json::json_exception &e) {
    log(warning) << "read something failed in Task";
    return true;
  }

  return true;
}

void Task::set_payload(http_headers _header, string _operation,
                       json::value _json, string _target_uri) {
  web::http::http_headers::iterator iter;
  for (iter = _header.begin(); iter != _header.end(); iter++) {
    this->payload.http_headers[iter->first] =
        iter->second; //.insert(make_pair(iter->first,iter->second));
  }

  this->payload.http_operation = _operation;
  this->payload.json_body = _json;
  this->payload.target_uri = _target_uri;
}
// Task end

// System start
json::value Systems::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(this->id);
  j[U("SystemType")] = json::value::string(this->system_type);
  j[U("AssetTag")] = json::value::string(this->asset_tag);
  j[U("Manufacturer")] = json::value::string(this->manufacturer);
  j[U("Model")] = json::value::string(this->model);
  j[U("SerialNumber")] = json::value::string(this->serial_number);
  j[U("PartNumber")] = json::value::string(this->part_number);
  j[U("Description")] = json::value::string(this->description);
  j[U("UUID")] = json::value::string(this->uuid);
  j[U("HostName")] = json::value::string(this->hostname);

  json::value k = json::value::object();
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  switch (this->indicator_led) {
  case LED_OFF:
    j[U("IndicatorLED")] = json::value::string(U("Off"));
    break;
  case LED_BLINKING:
    j[U("IndicatorLED")] = json::value::string(U("Blinking"));
    break;
  case LED_LIT:
    j[U("IndicatorLED")] = json::value::string(U("Lit"));
    break;
  }

  j[U("PowerState")] = json::value::string(this->power_state);
  j[U("BiosVersion")] = json::value::string(this->bios_version);

  json::value j_boot;
  j_boot[U("BootSourceOverrideEnabled")] =
      json::value::string(U(this->boot.boot_source_override_enabled));
  j_boot[U("BootSourceOverrideTarget")] =
      json::value::string(U(this->boot.boot_source_override_target));
  j_boot[U("BootSourceOverrideMode")] =
      json::value::string(U(this->boot.boot_source_override_mode));
  j_boot[U("UefiTargetBootSourceOverride")] =
      json::value::string(U(this->boot.uefi_target_boot_source_override));
  j[U("Boot")] = j_boot;

  j["Bios"] = get_resource_odata_id_json(this->bios, this->odata.id);
  j["Storage"] = get_resource_odata_id_json(this->storage, this->odata.id);
  j["Processors"] = get_resource_odata_id_json(this->processor, this->odata.id);
  j["Memory"] = get_resource_odata_id_json(this->memory, this->odata.id);
  j["EthernetInterfaces"] =
      get_resource_odata_id_json(this->ethernet, this->odata.id);
  j["SimpleStorage"] =
      get_resource_odata_id_json(this->simple_storage, this->odata.id);
  j["LogServices"] =
      get_resource_odata_id_json(this->log_service, this->odata.id);

  j["Actions"] = get_action_info(this->actions);

  return j;
}

bool Systems::load_json(json::value &j) {
  json::value status, boot; // 구조체
  json::value storage, processor, memory, ethernet, log_service,
      simple_storage; // 컬렉션

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->system_type = j.at("SystemType").as_string();
    this->manufacturer = j.at("Manufacturer").as_string();
    this->model = j.at("Model").as_string();
    this->serial_number = j.at("SerialNumber").as_string();
    this->part_number = j.at("PartNumber").as_string();
    this->description = j.at("Description").as_string();
    this->hostname = j.at("HostName").as_string();
    this->asset_tag = j.at("AssetTag").as_string();
    this->power_state = j.at("PowerState").as_string();
    this->uuid = j.at("UUID").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    string led_status = j.at("IndicatorLED").as_string();
    if (led_status == "Off")
      this->indicator_led = LED_OFF;
    else if (led_status == "Lit")
      this->indicator_led = LED_LIT;
    else if (led_status == "Blinking")
      this->indicator_led = LED_BLINKING;
    else
      log(warning) << "led value is strange..";

    boot = j.at("Boot");
    this->boot.boot_source_override_enabled =
        boot.at("BootSourceOverrideEnabled").as_string();
    this->boot.boot_source_override_mode =
        boot.at("BootSourceOverrideMode").as_string();
    this->boot.boot_source_override_target =
        boot.at("BootSourceOverrideTarget").as_string();
    this->boot.uefi_target_boot_source_override =
        boot.at("UefiTargetBootSourceOverride").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

bool Systems::Reset(json::value body) {
  string reset_type;
  char cmds[1024] = {
      0,
  };
  int c_status=0;
  // // #1 get systems pid && systems bin file
  // int pid = getpid();
  //     int c_status, res_len = 0;
  // sprintf(cmds, "cat /proc/%d/cmdline", pid);
  // log(info) << cmds;
  // string this_proc_name(
  //     get_popen_string(cmds)); // todo : not this process, target system
  // log(info) << pid << " : " << this_proc_name;

  get_value_from_json_key(body, "ResetType", reset_type);


  if (reset_type == "On") {
    cout << "[!@#$ RESET TYPE !@#$] : On" << endl;
    c_status = 0x1;

  }
  if (reset_type == "ForceOff") {
    cout << "[!@#$ RESET TYPE !@#$] : ForceOff" << endl;
     c_status = 0x0;
  }
  if (reset_type == "GracefulShutdown") {
    cout << "[!@#$ RESET TYPE !@#$] : GracefulShutdown" << endl;
    c_status = 0x6;

  }
  if (reset_type == "GracefulRestart") {
    cout << "[!@#$ RESET TYPE !@#$] : GracefulRestart" << endl;
    c_status = 0x2;

  }
  if (reset_type == "ForceRestart") {
    cout << "[!@#$ RESET TYPE !@#$] : ForceRestart" << endl;
    c_status = 0x3;

  }
  if (reset_type == "Nmi") {
    
    // /proc/sys/kernel/nmi_watchdog flag를 1로 체인지.. 현재 해당 파일 없음.
    // buildroot 환경설정?
  }
  if (reset_type == "ForceOn") {
    cout << "[!@#$ RESET TYPE !@#$] : ForceOn" << endl;
    c_status = 0x3;
  }
  
      ipmi_req_t *req;
      req->netfn_lun = 0;
      req->cmd = CMD_SET_POWER_STATUS;
      req->data[0] = c_status;
       uint8_t response[50];
       int res_len=0;
      ipmiChassis.chassis_control((ipmi_req_t *)req, (ipmi_res_t *)response,
                                (uint8_t *)res_len);

  return true;
}

void Systems::new_log_service(string _service_id) {
  if (this->log_service == nullptr) {
    this->log_service = new Collection(this->odata.id + "/LogServices",
                                       ODATA_LOG_SERVICE_COLLECTION_TYPE);
    this->log_service->name = "Log Service Collection";
  }

  init_log_service(this->log_service, _service_id);
}
// System end

// Processor start
json::value Processors::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Socket")] = json::value::string(U(this->socket));
  j[U("ProcessorType")] = json::value::string(U(this->processor_type));
  j[U("ProcessorArchitecture")] =
      json::value::string(U(this->processor_architecture));
  j[U("InstructionSet")] = json::value::string(U(this->instruction_set));
  j[U("Manufacturer")] = json::value::string(U(this->manufacturer));
  j[U("Model")] = json::value::string(U(this->model));
  j[U("MaxSpeedMHz")] = json::value::number(U(this->max_speed_mhz));
  j[U("TotalCores")] = json::value::number(U(this->total_cores));
  j[U("TotalThreads")] = json::value::number(U(this->total_threads));

  json::value k = json::value::object();
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  json::value l;
  l[U("VendorId")] = json::value::string(U(this->p_id.vendor_id));
  l[U("IdentificationRegisters")] =
      json::value::string(U(this->p_id.identification_registers));
  l[U("EffectiveFamily")] = json::value::string(U(this->p_id.effective_family));
  l[U("EffectiveModel")] = json::value::string(U(this->p_id.effective_model));
  l[U("Step")] = json::value::string(U(this->p_id.step));
  l[U("MicrocodeInfo")] = json::value::string(U(this->p_id.microcode_info));
  j[U("ProcessorId")] = l;

  return j;
}

bool Processors::load_json(json::value &j) {
  json::value status, p_id;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->socket = j.at("Socket").as_string();
    this->processor_type = j.at("ProcessorType").as_string();
    this->processor_architecture = j.at("ProcessorArchitecture").as_string();
    this->instruction_set = j.at("InstructionSet").as_string();
    this->manufacturer = j.at("Manufacturer").as_string();
    this->model = j.at("Model").as_string();
    this->max_speed_mhz = j.at("MaxSpeedMHz").as_integer();
    this->total_cores = j.at("TotalCores").as_integer();
    this->total_threads = j.at("TotalThreads").as_integer();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    p_id = j.at("ProcessorId");
    this->p_id.vendor_id = p_id.at("VendorId").as_string();
    this->p_id.identification_registers =
        p_id.at("IdentificationRegisters").as_string();
    this->p_id.effective_family = p_id.at("EffectiveFamily").as_string();
    this->p_id.effective_model = p_id.at("EffectiveModel").as_string();
    this->p_id.step = p_id.at("Step").as_string();
    this->p_id.microcode_info = p_id.at("MicrocodeInfo").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// json::value ProcessorSummary::get_json(void)
// {
//     auto j = this->Resource::get_json();
// if (j.is_null())
//     return j;

//     j[U("Id")] = json::value::string(U(this->id));
//     j[U("Count")] = json::value::number(U(this->count));
//     j[U("LogicalProcessorCount")] =
//     json::value::number(U(this->logical_processor_count)); j[U("Model")] =
//     json::value::string(U(this->model));

//     json::value k;
//     k[U("State")] = json::value::string(U(this->status.state));
//     k[U("Health")] = json::value::string(U(this->status.health));
//     j[U("Status")] = k;

//     return j;
// }

// storage start
json::value Storage::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Description")] = json::value::string(U(this->description));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("StorageControllers")] = json::value::array();
  for (int i = 0; i < this->controller->members.size(); i++)
    j[U("StorageControllers")][i] =
        ((StorageControllers *)this->controller->members[i])->get_json();

  j["Drives"] = get_resource_odata_id_json(this->drives, this->odata.id);
  j["Volumes"] = get_resource_odata_id_json(this->volumes, this->odata.id);

  return j;
}

bool Storage::load_json(json::value &j) {
  json::value status;

  try {
    Resource::load_json(j);

    this->id = j.at("Id").as_string();
    this->description = j.at("Description").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value StorageControllers::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Manufacturer")] = json::value::string(U(this->manufacturer));
  j[U("Model")] = json::value::string(U(this->model));
  j[U("SerialNumber")] = json::value::string(U(this->serial_number));
  j[U("PartNumber")] = json::value::string(U(this->part_number));
  j[U("SpeedGbps")] = json::value::number(U(this->speed_gbps));
  j[U("FirmwareVersion")] = json::value::string(U(this->firmware_version));

  json::value k;
  k[U("DurableName")] = json::value::string(U(this->identifier.durable_name));
  k[U("DurableNameFormat")] =
      json::value::string(U(this->identifier.durable_name_format));
  j[U("Identifiers")] = k;

  json::value l;
  l[U("State")] = json::value::string(U(this->status.state));
  l[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = l;

  j[U("SupportedControllerProtocols")] = json::value::array();
  for (int i = 0; i < this->support_controller_protocols.size(); i++)
    j[U("SupportedControllerProtocols")][i] =
        json::value::string(U((this->support_controller_protocols)[i]));

  j[U("SupportedDeviceProtocols")] = json::value::array();
  for (int i = 0; i < this->support_device_protocols.size(); i++)
    j[U("SupportedDeviceProtocols")][i] =
        json::value::string(U((this->support_device_protocols)[i]));

  return j;
}

bool StorageControllers::load_json(json::value &j) {
  json::value status, identifier;
  json::value support_controller_protocols, support_device_protocols;
  int step = 0;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->manufacturer = j.at("Manufacturer").as_string();
    this->model = j.at("Model").as_string();
    this->serial_number = j.at("SerialNumber").as_string();
    this->part_number = j.at("PartNumber").as_string();
    this->speed_gbps = j.at("SpeedGbps").as_double();
    this->firmware_version = j.at("FirmwareVersion").as_string();

    step = 1;

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    step = 2;

    identifier = j.at("Identifiers");
    this->identifier.durable_name = identifier.at("DurableName").as_string();
    this->identifier.durable_name_format =
        identifier.at("DurableNameFormat").as_string();

    step = 3;

    support_controller_protocols = j.at("SupportedControllerProtocols");
    for (auto str : support_controller_protocols.as_array())
      this->support_controller_protocols.push_back(str.as_string());

    step = 4;
    support_device_protocols = j.at("SupportedDeviceProtocols");
    for (auto str : support_device_protocols.as_array())
      this->support_device_protocols.push_back(str.as_string());
  } catch (json::json_exception &e) {
    log(warning) << "read something failed in storage controllers "
                 << this->odata.id << " at step " << step;
    return true;
  }

  return true;
}

json::value SimpleStorage::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("Description")] = json::value::string(U(this->description));
  j[U("UefiDevicePath")] = json::value::string(U(this->uefi_device_path));
  // j[U("FileSystem")] = json::value::string(U(this->file_system));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("Devices")] = json::value::array();
  for (int i = 0; i < this->devices.size(); i++) {
    json::value o = json::value::object();
    o[U("Name")] = json::value::string(U(this->devices[i].name));
    o[U("Manufacturer")] =
        json::value::string(U(this->devices[i].manufacturer));
    o[U("Model")] = json::value::string(U(this->devices[i].model));
    o[U("CapacityKBytes")] =
        json::value::number(U(this->devices[i].capacity_KBytes));
    o[U("FileSystem")] = json::value::string(U(this->devices[i].file_system));

    json::value ok;
    ok[U("State")] = json::value::string(U(this->devices[i].status.state));
    ok[U("Health")] = json::value::string(U(this->devices[i].status.health));
    o[U("Status")] = ok;
    j[U("Devices")][i] = o;
  }

  return j;
}

bool SimpleStorage::load_json(json::value &j) {
  json::value status, devices, dev_status;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->description = j.at("Description").as_string();
    this->uefi_device_path = j.at("UefiDevicePath").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    devices = j.at("Devices");
    for (auto item : devices.as_array()) {
      Device_Info temp;
      temp.name = item.at("Name").as_string();
      temp.manufacturer = item.at("Manufacturer").as_string();
      temp.model = item.at("Model").as_string();
      temp.capacity_KBytes = item.at("CapacityKBytes").as_integer();

      dev_status = item.at("Status");
      temp.status.state = dev_status.at("State").as_string();
      temp.status.health = dev_status.at("Health").as_string();

      this->devices.push_back(temp);
    }
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value Drive::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["Id"] = json::value::string(this->id);
  j["AssetTag"] = json::value::string(this->asset_tag);
  j["Description"] = json::value::string(this->description);
  j["EncryptionAbility"] = json::value::string(this->encryption_ability);
  j["EncryptionStatus"] = json::value::string(this->encryption_status);
  j["HotspareType"] = json::value::string(this->hotspare_type);
  j["Manufacturer"] = json::value::string(this->manufacturer);
  j["MediaType"] = json::value::string(this->media_type);
  j["Model"] = json::value::string(this->model);
  j["Name"] = json::value::string(this->name);
  j["SKU"] = json::value::string(this->sku);
  j["StatusIndicator"] = json::value::string(this->status_indicator);
  j["PartNumber"] = json::value::string(this->part_number);
  j["Protocol"] = json::value::string(this->protocol);
  j["Revision"] = json::value::string(this->revision);
  j["SerialNumber"] = json::value::string(this->serial_number);
  j["BlockSizeBytes"] = json::value::number(this->block_size_bytes);
  j["CapableSpeedGbs"] = json::value::number(this->capable_speed_Gbs);
  j["NegotiatedSpeedGbs"] = json::value::number(this->negotiated_speed_Gbs);
  j["PredictedMediaLifeLeftPercent"] =
      json::value::number(this->predicted_media_life_left_percent);
  j["RotationSpeedRPM"] = json::value::number(this->rotation_speed_RPM);
  j["FailurePredicted"] = json::value::boolean(this->failure_predicted);

  vector<json::value> vec;
  for (auto idnt : this->identifier) {
    json::value k = json::value::object();
    j["DurableName"] = json::value::string(idnt.durable_name);
    j["DurableNameFormat"] = json::value::string(idnt.durable_name_format);
    vec.push_back(k);
  }
  j["Identifiers"] = json::value::array(vec);

  json::value PhysicalLocation, PartLocation;
  PartLocation["LocationType"] =
      json::value::string(this->physical_location.part_location.location_type);
  PartLocation["ServiceLabel"] =
      json::value::string(this->physical_location.part_location.service_label);
  PartLocation["LocationOrdinalValue"] = json::value::number(
      this->physical_location.part_location.location_ordinal_value);
  PhysicalLocation["PartLocation"] = PartLocation;
  PhysicalLocation["InfoFormat"] =
      json::value::string(this->physical_location.info_format);
  PhysicalLocation["Info"] = json::value::string(this->physical_location.info);
  j["PhysicalLocation"] = PhysicalLocation;

  json::value l;
  l[U("State")] = json::value::string(U(this->status.state));
  l[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = l;

  return j;
}

bool Drive::load_json(json::value &j) {
  try {
    Resource::load_json(j);
    get_value_from_json_key(j, "Id", this->id);
    get_value_from_json_key(j, "AssetTag", this->asset_tag);
    get_value_from_json_key(j, "Description", this->description);
    get_value_from_json_key(j, "EncryptionAbility", this->encryption_ability);
    get_value_from_json_key(j, "EncryptionStatus", this->encryption_status);
    get_value_from_json_key(j, "HotspareType", this->hotspare_type);
    get_value_from_json_key(j, "Manufacturer", this->manufacturer);
    get_value_from_json_key(j, "Model", this->model);
    get_value_from_json_key(j, "Name", this->name);
    get_value_from_json_key(j, "SKU", this->sku);
    get_value_from_json_key(j, "StatusIndicator", this->status_indicator);
    get_value_from_json_key(j, "PartNumber", this->part_number);
    get_value_from_json_key(j, "Protocol", this->protocol);
    get_value_from_json_key(j, "Revision", this->revision);
    get_value_from_json_key(j, "SerialNumber", this->serial_number);
    get_value_from_json_key(j, "BlockSizeBytes", this->block_size_bytes);
    get_value_from_json_key(j, "CapableSpeedGbs", this->capable_speed_Gbs);
    get_value_from_json_key(j, "NegotiatedSpeedGbs",
                            this->negotiated_speed_Gbs);
    get_value_from_json_key(j, "PredictedMediaLifeLeftPercent",
                            this->predicted_media_life_left_percent);
    get_value_from_json_key(j, "RotationSpeedRPM", this->rotation_speed_RPM);
    get_value_from_json_key(j, "FailurePredicted", this->failure_predicted);

    json::value PhysicalLocation, PartLocation;
    get_value_from_json_key(j, "PhysicalLocation", PhysicalLocation);
    get_value_from_json_key(PhysicalLocation, "PartLocation", PartLocation);
    get_value_from_json_key(PhysicalLocation, "Info",
                            this->physical_location.info);
    get_value_from_json_key(PhysicalLocation, "InfoFormat",
                            this->physical_location.info_format);
    get_value_from_json_key(
        PartLocation, "LocationOrdinalValue",
        this->physical_location.part_location.location_ordinal_value);
    get_value_from_json_key(
        PartLocation, "ServiceLabel",
        this->physical_location.part_location.service_label);
    get_value_from_json_key(
        PartLocation, "LocationType",
        this->physical_location.part_location.location_type);

    json::value status;
    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value Volume::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["Id"] = json::value::string(this->id);
  j["Description"] = json::value::string(this->description);
  j["RAIDType"] = json::value::string(this->RAID_type);
  j["Name"] = json::value::string(this->name);
  j["ReadCachePolicy"] = json::value::string(this->read_cache_policy);
  j["WriteCachePolicy"] = json::value::string(this->write_cache_policy);
  j["StripSizeBytes"] = json::value::string(this->strip_size_bytes);
  j["DisplayName"] = json::value::string(this->display_name);
  j["BlockSizeBytes"] = json::value::number(this->block_size_bytes);
  j["CapacityBytes"] = json::value::number(this->capacity_bytes);

  vector<json::value> vec;
  for (auto k : this->access_capabilities)
    vec.push_back(json::value::string(k));
  j["AccessCapabilities"] = json::value::array(vec);

  json::value l;
  l[U("State")] = json::value::string(U(this->status.state));
  l[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = l;

  return j;
}

bool Volume::load_json(json::value &j) {
  try {
    Resource::load_json(j);

    get_value_from_json_key(j, "Id", this->id);
    get_value_from_json_key(j, "Description", this->description);
    get_value_from_json_key(j, "RAIDType", this->RAID_type);
    get_value_from_json_key(j, "Name", this->name);
    get_value_from_json_key(j, "ReadCachePolicy", this->read_cache_policy);
    get_value_from_json_key(j, "WriteCachePolicy", this->write_cache_policy);
    get_value_from_json_key(j, "StripSizeBytes", this->strip_size_bytes);
    get_value_from_json_key(j, "DisplayName", this->display_name);
    get_value_from_json_key(j, "BlockSizeBytes", this->block_size_bytes);
    get_value_from_json_key(j, "CapacityBytes", this->capacity_bytes);

    json::value objVec;
    get_value_from_json_key(j, "AccessCapabilities", objVec);
    if (objVec != json::value::null()) {
      for (auto str : objVec.as_array())
        this->access_capabilities.push_back(str.as_string());
    }

    json::value status;
    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

// Storage end

// Bios start
json::value Bios::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null()) {
    cout << "j is null!" << endl;
    return j;
  }
  j[U("Id")] = json::value::string(U(this->id));
  j[U("AttributeRegistry")] = json::value::string(U(this->attribute_registry));

  json::value k;
  k[U("BootMode")] = json::value::string(U(this->attribute.boot_mode));
  k[U("EmbeddedSata")] = json::value::string(U(this->attribute.embedded_sata));
  k[U("NicBoot1")] = json::value::string(U(this->attribute.nic_boot1));
  k[U("NicBoot2")] = json::value::string(U(this->attribute.nic_boot2));
  k[U("PowerProfile")] = json::value::string(U(this->attribute.power_profile));
  k[U("ProcCoreDisable")] =
      json::value::number(U(this->attribute.proc_core_disable));
  k[U("ProcHyperthreading")] =
      json::value::string(U(this->attribute.proc_hyper_threading));
  k[U("ProcTurboMode")] =
      json::value::string(U(this->attribute.proc_turbo_mode));
  k[U("UsbControl")] = json::value::string(U(this->attribute.usb_control));
  j[U("Attributes")] = k;

  j["Actions"] = get_action_info(this->actions);

  return j;
}

bool Bios::load_json(json::value &j) {
  json::value attribute;
  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->attribute_registry = j.at("AttributeRegistry").as_string();

    attribute = j.at("Attributes");
    this->attribute.boot_mode = attribute.at("BootMode").as_string();
    this->attribute.embedded_sata = attribute.at("EmbeddedSata").as_string();
    this->attribute.nic_boot1 = attribute.at("NicBoot1").as_string();
    this->attribute.nic_boot2 = attribute.at("NicBoot2").as_string();
    this->attribute.power_profile = attribute.at("PowerProfile").as_string();
    this->attribute.proc_core_disable =
        attribute.at("ProcCoreDisable").as_integer();
    this->attribute.proc_hyper_threading =
        attribute.at("ProcHyperthreading").as_string();
    this->attribute.proc_turbo_mode = attribute.at("ProcTurboMode").as_string();
    this->attribute.usb_control = attribute.at("UsbControl").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

bool Bios::ResetBios() {
  try {
    init_bios(this);
  } catch (exception &e) {
    log(warning) << "Reset Bios Failed..";
    ;
    return false;
  }
  return true;
}

bool Bios::ChangePassword(string new_password, string old_password,
                          string password_name) {
  // BMC password change..
  // # request = [module_id]/acccountService/accounts/[account_num]
  // #1 get module id.. ex) 1, 500..
  string module_id;
  module_id =
      ((Systems *)g_record[get_parent_object_uri(this->odata.id, "/")])->id;

  // #2 get parameter
  // #3 change password
}
// Bios end

// Memory start
json::value Memory::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("RankCount")] = json::value::number(U(this->rank_count));
  j[U("CapacityKiB")] = json::value::number(U(this->capacity_kib));
  j[U("DataWidthBits")] = json::value::number(U(this->data_width_bits));
  j[U("BusWidthBits")] = json::value::number(U(this->bus_width_bits));
  j[U("ErrorCorrection")] = json::value::string(U(this->error_correction));
  j[U("MemoryType")] = json::value::string(U(this->memory_type));
  j[U("MemoryDeviceType")] = json::value::string(U(this->memory_device_type));
  j[U("BaseModuleType")] = json::value::string(U(this->base_module_type));

  json::value k;
  k[U("State")] = json::value::string(U(this->status.state));
  k[U("Health")] = json::value::string(U(this->status.health));
  j[U("Status")] = k;

  j[U("MaxTDPMilliWatts")] = json::value::array();
  for (int i = 0; i < this->max_TDP_milliwatts.size(); i++)
    j[U("MaxTDPMilliWatts")][i] =
        json::value::number(U(this->max_TDP_milliwatts[i]));

  json::value loc = json::value::object();
  loc[U("Socket")] = json::value::number(U(this->m_location.socket));
  loc[U("MemoryController")] =
      json::value::number(U(this->m_location.memory_controller));
  loc[U("Channel")] = json::value::number(U(this->m_location.channel));
  loc[U("Slot")] = json::value::number(U(this->m_location.slot));
  j[U("MemoryLocation")] = loc;

  j[U("MemoryMedia")] = json::value::array();
  for (int i = 0; i < this->memory_media.size(); i++)
    j[U("MemoryMedia")][i] = json::value::string(U(this->memory_media[i]));

  return j;
}

bool Memory::load_json(json::value &j) {
  json::value status, m_location, max_TDP_milliwatts, memory_media;

  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
    this->rank_count = j.at("RankCount").as_integer();
    this->capacity_kib = j.at("CapacityKiB").as_integer();
    this->data_width_bits = j.at("DataWidthBits").as_integer();
    this->bus_width_bits = j.at("BusWidthBits").as_integer();
    this->error_correction = j.at("ErrorCorrection").as_string();
    this->memory_type = j.at("MemoryType").as_string();
    this->memory_device_type = j.at("MemoryDeviceType").as_string();
    this->base_module_type = j.at("BaseModuleType").as_string();

    status = j.at("Status");
    this->status.state = status.at("State").as_string();
    this->status.health = status.at("Health").as_string();

    m_location = j.at("MemoryLocation");
    this->m_location.socket = m_location.at("Socket").as_integer();
    this->m_location.memory_controller =
        m_location.at("MemoryController").as_integer();
    this->m_location.channel = m_location.at("Channel").as_integer();
    this->m_location.slot = m_location.at("Slot").as_integer();

    max_TDP_milliwatts = j.at("MaxTDPMilliWatts");
    for (auto item : max_TDP_milliwatts.as_array())
      this->max_TDP_milliwatts.push_back(item.as_integer());

    memory_media = j.at("MemoryMedia");
    for (auto item : memory_media.as_array())
      this->memory_media.push_back(item.as_string());
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}
// Memory end

// dy : certificate start
json::value Certificate::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["Id"] = json::value::string(this->id);
  j["CertificateString"] = json::value::string(this->certificateString);
  j["CertificateType"] = json::value::string(this->certificateType);

  json::value Issuer = json::value::object();
  Issuer["Country"] = json::value::string(this->issuer.country);
  Issuer["State"] = json::value::string(this->issuer.state);
  Issuer["City"] = json::value::string(this->issuer.city);
  Issuer["Organization"] = json::value::string(this->issuer.organization);
  Issuer["OrganizationUnit"] =
      json::value::string(this->issuer.organizationUnit);
  Issuer["CommonName"] = json::value::string(this->issuer.commonName);
  j["Issuer"] = Issuer;

  json::value Subject = json::value::object();
  Subject["Country"] = json::value::string(this->subject.country);
  Subject["State"] = json::value::string(this->subject.state);
  Subject["City"] = json::value::string(this->subject.city);
  Subject["Organization"] = json::value::string(this->subject.organization);
  Subject["OrganizationUnit"] =
      json::value::string(this->subject.organizationUnit);
  Subject["CommonName"] = json::value::string(this->subject.commonName);
  j["Subject"] = Subject;

  j["ValidNotBefore"] = json::value::string(this->validNotBefore);
  j["ValidNotAfter"] = json::value::string(this->validNotAfter);

  j["Email"] = json::value::string(this->email);
  j["KeyBitLength"] = json::value::number(this->key_bit_length);

  j["KeyUsage"] = json::value::array();
  for (int i = 0; i < this->keyUsage.size(); i++)
    j["KeyUsage"][i] = json::value::string(this->keyUsage[i]);

  j["Actions"] = get_action_info(this->actions);
  return j;
}

bool Certificate::load_json(json::value &j) {
  json::value issuer, subject;
  json::value key_usage;

  try {
    this->certificateString = j.at("CertificateString").as_string();
    this->certificateType = j.at("CertificateType").as_string();
    this->id = j.at("Id").as_string();
    this->validNotAfter = j.at("ValidNotAfter").as_string();
    this->validNotBefore = j.at("ValidNotBefore").as_string();

    key_usage = j.at("KeyUsage");
    for (json::value item : key_usage.as_array())
      this->keyUsage.push_back(item.at("@odata.id").as_string());

    issuer = j.at("Issuer");
    this->issuer.city = issuer.at("City").as_string();
    this->issuer.commonName = issuer.at("CommonName").as_string();
    this->issuer.country = issuer.at("Country").as_string();
    this->issuer.organization = issuer.at("Organization").as_string();
    this->issuer.organizationUnit = issuer.at("OrganizationUnit").as_string();
    this->issuer.state = issuer.at("State").as_string();

    subject = j.at("Subject");
    this->subject.city = subject.at("City").as_string();
    this->subject.commonName = subject.at("CommonName").as_string();
    this->subject.country = subject.at("Country").as_string();
    this->subject.organization = subject.at("Organization").as_string();
    this->subject.organizationUnit = subject.at("OrganizationUnit").as_string();
    this->subject.state = subject.at("State").as_string();

    get_value_from_json_key(j, "Email", this->email);
    get_value_from_json_key(j, "KeyBitLength", this->key_bit_length);
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

// Rekey와 Renew.. CSR갱신만 하고 Certificate replace는 하지 않는
// 것인가..?!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
json::value Certificate::Rekey(json::value body) {
  // #1 generateCSR with new key and return
  json::value rsp;
  int key_bit_length;
  fs::path key("/conf/ssl/cert.key");
  fs::path conf("/conf/ssl/cert.cnf");
  fs::path csr("/conf/ssl/cert.csr");

  // not implemented..
  string key_curve_id;
  string key_pair_algorithm;
  string challenge_password;

  if (!(get_value_from_json_key(body, "KeyBitLength", key_bit_length))) {
    rsp = json::value::null();
    return rsp;
  }

  if (fs::exists(key)) {
    fs::remove(key);
    log(info) << "[...] remove old ssl key";
  }

  generate_ssl_private_key(key, to_string(key_bit_length));

  rsp = generate_CSR_return_result(conf, key, csr, this->odata.id);
  resource_save_json(this); // ??

  return rsp;
}

json::value Certificate::Renew(void) {
  // #1 generateCSR with exists information
  json::value rsp;
  fs::path key("/conf/ssl/cert.key");
  fs::path conf("/conf/ssl/cert.cnf");
  fs::path csr("/conf/ssl/cert.csr");

  if (!(fs::exists(conf))) {
    log(warning) << "[...] ssl config file doesn't exists.. failed to renew";
    return json::value::null();
  }

  if (!(fs::exists(key))) {
    log(warning) << "[...] ssl key file doesn't exists.. failed to renew";
    return json::value::null();
  }

  rsp = generate_CSR_return_result(conf, key, csr, this->odata.id);
  resource_save_json(this); // ??

  return rsp;
}

json::value CertificateLocation::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["Id"] = json::value::string(this->id);
  j["Certificates"] = json::value::array();
  for (int i = 0; i < this->certificates.size(); i++)
    j["Certificates"][i] =
        get_resource_odata_id_json(this->certificates[i], this->odata.id);

  return j;
}

bool CertificateLocation::load_json(json::value &j) {
  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value CertificateService::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["Id"] = json::value::string(this->id);
  j["CertificateLocations"] =
      get_resource_odata_id_json(this->certificate_location, this->odata.id);
  j["Actions"] = get_action_info(this->actions);

  return j;
}

bool CertificateService::load_json(json::value &j) {
  try {
    Resource::load_json(j);
    this->id = j.at("Id").as_string();
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

json::value CertificateService::GenerateCSR(json::value body) {
  string cmds;
  json::value rsp;
  // required
  string certificate_odata_id;
  string country;
  string city;
  string common_name;
  string state;
  string organization;
  string organization_unit;

  // optional

  // optional (not implemented..)
  vector<string> alternative_names;
  vector<string> key_usage;
  string challenge_password; // pass phrase
  string contact_person;
  string email;
  string given_name;
  string initials;
  string key_curve_id;       // ahffk
  string key_pair_algorithm; // rsa only!
  string surname;
  string unstructured_name;

  log(info) << "[...]generateCSR start";
  certificate_odata_id =
      body.at("CertificateCollection").at("@odata.id").as_string();
  get_value_from_json_key(body, "Country", country);
  get_value_from_json_key(body, "City", city);
  get_value_from_json_key(body, "CommonName", common_name);
  get_value_from_json_key(body, "State", state);
  get_value_from_json_key(body, "Organization", organization);
  get_value_from_json_key(body, "Organization", organization_unit);

  // #1 key, conf(inform), csr path declare
  fs::path key("/conf/ssl/cert.key");
  fs::path conf("/conf/ssl/cert.cnf");
  fs::path csr("/conf/ssl/cert.csr");
  string key_length = "1024";

  // #2 이미 존재한다면, 삭제
  remove_if_exists(conf);
  remove_if_exists(key);

  // #3 ssl key (개인키) 생성
  generate_ssl_private_key(key, key_length);

  // #4 ssl config file 생성
  log(info) << "[...]ssl config save";
  char cert_text[14][200];

  sprintf(cert_text[0], "[ req ]\n");
  sprintf(cert_text[1], "default_bits\t= %s\n", key_length.c_str());
  sprintf(cert_text[2], "default_md\t= sha256\n");
  sprintf(cert_text[3], "default_keyfile\t=%s\n", key.c_str());
  sprintf(cert_text[4], "prompt\t = no\n");
  sprintf(cert_text[5], "encrypt_key\t= no\n\n");
  sprintf(cert_text[6],
          "# base request\ndistinguished_name = req_distinguished_name\n");
  sprintf(cert_text[7], "\n# distinguished_name\n[ req_distinguished_name ]\n");
  sprintf(cert_text[8], "countryName\t= \"%s\"\n", country.c_str());
  sprintf(cert_text[9], "stateOrProvinceName\t= \"%s\"\n", state.c_str());
  sprintf(cert_text[10], "localityName\t=\"%s\"\n", city.c_str());
  sprintf(cert_text[11], "organizationName\t=\"%s\"\n", organization.c_str());
  sprintf(cert_text[12], "organizationalUnitName\t=\"%s\"\n",
          organization_unit.c_str());
  sprintf(cert_text[13], "commonName\t=\"%s\"\n", common_name.c_str());

  ofstream cert_conf(conf);
  for (int i = 0; i < 14; i++)
    cert_conf << cert_text[i];
  cert_conf.close();

  // #5 CSR 생성 && return request
  rsp = generate_CSR_return_result(conf, key, csr, certificate_odata_id);
  resource_save_json(this); // ??

  return rsp;
}

/**
 * @brief generate new ssl private key
 * @author dyk
 */
void generate_ssl_private_key(fs::path key, string key_length) {
  string cmds = "openssl genrsa -out " + key.string() + " " + key_length;
  system(cmds.c_str());
  log(info) << "[###]generate private key";
}

/**
 * @brief generate Certificate Signing Request with absolute path of conf, key,
 * csr
 * @author dyk
 * @return json information about this request // can be changed
 */
json::value generate_CSR_return_result(fs::path conf, fs::path key,
                                       fs::path csr, string target_id) {
  json::value rsp;
  string cmds = "openssl req -config " + conf.string() + " -new -key " +
                key.string() + " -out " + csr.string() + " -verbose";

  if (!std::system(cmds.c_str())) {
    if (fs::exists(csr) && fs::exists(key) && fs::exists(conf)) {
      log(info) << "[###]CSR is generated";

      json::value odata_id;

      odata_id["@odata.id"] = json::value::string(target_id);
      rsp["CertificateCollection"] = odata_id;

      rsp["CSRString"] = json::value::string(file2str(csr.string()));
    }
  } else {
    log(warning) << "[Error] Failed to Generate CSR";
    json::value msg;
    msg["Message"] = json::value::string("Request Failed.");
    rsp["Failed"] = msg;
  }

  return rsp;
}

/**
 * @brief read .csr, .pem file and get CSRString
 * @author dyk
 * @return CSRString
 */
string file2str(string file_path) {
  auto csr_str = ostringstream{};
  ifstream read_csr(file_path);
  if (!read_csr.is_open()) {
    log(warning) << "Could not open the file : " << file_path;
    return "";
  }
  csr_str << read_csr.rdbuf();
  read_csr.close();

  return csr_str.str();
}

bool CertificateService::ReplaceCertificate(json::value body) {
  // #1 body로부터 CSR string 및 CSR Type 입력받기.
  log(info) << "[...]Replace Certificate Start";
  string target_odata_id =
      body.at("CertificateUri").at("@odata.id").as_string();
  Certificate *replacedCert = (Certificate *)g_record[target_odata_id];

  get_value_from_json_key(body, "CertificateString",
                          replacedCert->certificateString);
  get_value_from_json_key(body, "CertificateType",
                          replacedCert->certificateType);

  // #2 pem 파일 입력받은 certificate로 교체
  // 존재하면 교체. 존재하지 않는다면 생성.
  fs::path cert_file(target_odata_id + ".pem");
  if (fs::exists(cert_file)) {
    // backup 필요??
    fs::remove(cert_file);
    log(info) << "[...]Original Certificate is removed..";
  }

  ofstream cert(cert_file.string());
  cert << replacedCert->certificateString;
  cert.close();

  // #3 수정된 certificate 정보 읽어서 g_record 수정
  update_cert_with_pem(cert_file, replacedCert);
  log(info) << "[...]Certificate is replaced..";
  resource_save_json(replacedCert);

  return true;
}

// 다른 field도 읽어야함
void update_cert_with_pem(fs::path cert, Certificate *certificate) {
  string cmds;
  // get dates
  // log(info) << cert.string();
  cmds = "openssl x509 -in " + cert.string() + " -noout -startdate";
  string notbefore(get_popen_string(const_cast<char *>(cmds.c_str())));
  notbefore.erase(0, 10);
  // log(info) << "startdate : " << notbefore;
  certificate->validNotBefore = notbefore;

  cmds = "openssl x509 -in " + cert.string() + " -noout -enddate";
  string notafter(get_popen_string(const_cast<char *>(cmds.c_str())));
  notafter.erase(0, 9);
  // log(info) << "enddate : " << notafter;
  certificate->validNotAfter = notafter;

  // get issuer
  vector<string> issuerV;
  cmds = "openssl x509 -in " + cert.string() + " -noout -issuer";
  string issuer(get_popen_string(const_cast<char *>(cmds.c_str())));
  issuer.erase(0, 7);
  issuerV = string_split(issuer, ',');
  for (auto item : issuerV) {
    vector<string> itemV;
    itemV = string_split(item, ' ');
    if (itemV[0] == "C")
      certificate->issuer.country = itemV[2];
    if (itemV[0] == "ST")
      certificate->issuer.state = itemV[2];
    if (itemV[0] == "L")
      certificate->issuer.city = itemV[2];
    if (itemV[0] == "O")
      certificate->issuer.organization = itemV[2];
    if (itemV[0] == "OU")
      certificate->issuer.organizationUnit = itemV[2];
    if (itemV[0] == "CN")
      certificate->issuer.commonName = itemV[2];
    if (itemV[0] == "emailAddress")
      certificate->issuer.email = itemV[2];
  }

  // get subject
  vector<string> subjectV;
  cmds = "openssl x509 -in " + cert.string() + " -noout -subject";
  string subject(get_popen_string(const_cast<char *>(cmds.c_str())));
  subject.erase(0, 8);
  subjectV = string_split(subject, ',');
  for (auto item : subjectV) {
    vector<string> itemV;
    itemV = string_split(item, ' ');
    if (itemV[0] == "C")
      certificate->subject.country = itemV[2];
    if (itemV[0] == "ST")
      certificate->subject.state = itemV[2];
    if (itemV[0] == "L")
      certificate->subject.city = itemV[2];
    if (itemV[0] == "O")
      certificate->subject.organization = itemV[2];
    if (itemV[0] == "OU")
      certificate->subject.organizationUnit = itemV[2];
    if (itemV[0] == "CN")
      certificate->subject.commonName = itemV[2];
    if (itemV[0] == "emailAddress")
      certificate->subject.email = itemV[2];
  }
  return;
}
// dy : certificate end

// dy : virtual media start
json::value VirtualMedia::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j["ConnectedVia"] = json::value::string(this->connected_via);
  j["Id"] = json::value::string(this->id);
  j["Image"] = json::value::string(this->image);
  j["ImageName"] = json::value::string(this->image_name);
  j["UserName"] = json::value::string(this->user_name);
  j["Password"] = json::value::string(this->password);
  j["Inserted"] = json::value::boolean(this->inserted);
  j["WriteProtected"] = json::value::boolean(this->write_protected);
  j["MediaTypes"] = json::value::array();
  j["Size"] = json::value::string(this->size);
  j["CreateTime"] = json::value::string(this->create_time);
  for (int i = 0; i < this->media_type.size(); i++)
    j["MediaTypes"][i] = json::value::string(this->media_type[i]);

  j["Actions"] = get_action_info(this->actions);
  return j;
}

bool VirtualMedia::load_json(json::value &j) {
  json::value media_type;
  try {
    Resource::load_json(j);
    get_value_from_json_key(j, "ConnectedVia", this->connected_via);
    get_value_from_json_key(j, "Id", this->id);
    get_value_from_json_key(j, "Image", this->image);
    get_value_from_json_key(j, "ImageName", this->image_name);
    get_value_from_json_key(j, "UserName", this->user_name);
    get_value_from_json_key(j, "Password", this->password);
    get_value_from_json_key(j, "Inserted", this->inserted);
    get_value_from_json_key(j, "WriteProtected", this->write_protected);
    get_value_from_json_key(j, "MediaTypes", media_type);
    get_value_from_json_key(j, "Size", this->size);
    get_value_from_json_key(j, "CreateTime", this->create_time);
    for (auto types : media_type.as_array())
      this->media_type.push_back(types.as_string());
  } catch (json::json_exception &e) {
    return false;
  }

  return true;
}

/**
 * @brief virtual media insert via nfs
 * @author 김
 * @return if insert failed, return json::value::null(). else return virtual
 * media resource info
 */
json::value VirtualMedia::InsertMedia(json::value body) {
  // have to : odata.id 설정, media type를 어떻게 얻어올 지..

  int ret = 0;

  string image, user_name, password;
  bool write_protected;

  // get_value_from_json_key(body, "Image", image);
  // get_value_from_json_key(body, "UserName", user_name);
  // get_value_from_json_key(body, "Password", password);
  // get_value_from_json_key(body, "WriteProtected", write_protected);

  if (!(get_value_from_json_key(body, "Image", image)))
    return json::value::null();

  // #0 mount check
  ret = umount();
  if (ret == -1) {
    log(warning) << "umount error!";
    return json::value::null();
  }

  // #1 mount
  // #1-1 set mount point
  if (!fs::exists("/etc/nfs"))
    mkdir("/etc/nfs", 777);

  // #1-2 using nfs,
  // string cmd = "mount -vt nfs " + image + " /etc/nfs";
  string cmd = "mount -vt nfs " + image + " /etc/nfs -o vers=3";
  ret = system(cmd.c_str());
  if (ret == -1) {
    log(warning) << "mount error!";
    return json::value::null();
  }

  // #2 make virtual media, insert

  // this->name = "VirtualMedia";

  // this->inserted = true;
  this->image = image;
  this->image_name = get_current_object_name(image, "/");
  this->connected_via = "URI";
  this->size =
      get_popen_string("df -h | grep \"" + image + "\" | awk {\'print $2\'}");
  this->create_time = currentDateTime();

  if (get_value_from_json_key(body, "UserName", user_name))
    this->user_name = user_name;
  if (get_value_from_json_key(body, "Password", password))
    this->password = password;

  if (get_value_from_json_key(body, "Inserted", inserted))
    this->inserted = inserted;
  else
    this->inserted = true;
  if (get_value_from_json_key(body, "WriteProtected", write_protected))
    this->write_protected = write_protected;
  else
    this->write_protected = true;

  // this->id = get_current_object_name(this->odata.id, "/");
  // this->write_protected = write_protected;
  // this->media_type.push_back("CD");
  // this->media_type.push_back("DVD");

  // ((Collection *)g_record[get_parent_object_uri(this->odata.id,
  // "/")])->add_member(this);

  // #3 return resource
  return this->get_json();
}

json::value VirtualMedia::EjectMedia(void) {
  int ret;
  json::value target = this->get_json();
  string odata_id = this->odata.id;

  // #1 unmount
  ret = umount();
  if (ret == -1)
    return json::value::null();

  // InsertMedia가 리소스를 그때그때 생성하므로 Eject는 리소스 삭제하겠음
  string col_uri = get_parent_object_uri(this->odata.id, "/");
  Collection *col = (Collection *)g_record[col_uri];
  std::vector<Resource *>::iterator iter;
  for (iter = col->members.begin(); iter != col->members.end(); iter++) {
    VirtualMedia *del = (VirtualMedia *)*iter;
    if (del->id == this->id) {
      break;
    }
  }

  cout << "지우기전!! " << endl;
  cout << record_get_json(col->odata.id) << endl;
  cout << " $$$$$$$ " << endl;

  // numset지우기
  unsigned int id_num;
  if (this->media_type[0] == "CD") {
    string id = this->id;
    string extract = id.substr(2);
    id_num = stoi(extract);
    delete_numset_num(ALLOCATE_VM_CD_NUM, id_num);
  }
  else if (this->media_type[0] == "USBStick") {
    string id = this->id;
    string extract = id.substr(3);
    id_num = stoi(extract);
    delete_numset_num(ALLOCATE_VM_USB_NUM, id_num);
  }

  delete (*iter);
  col->members.erase(iter);
  resource_save_json(col);
  delete_resource(odata_id);

  cout << "지운후~~ " << endl;
  cout << "지워진놈 : " << odata_id << endl;
  cout << target << endl;

  // #2 connectedVia => notconnected, inserted => false, image => null  <?
  // data는 보존한다는 얘기??> this->connected_via = "NotConnected";
  // this->inserted = false;
  // this->image = "";

  return target;
  // #3 return resource
  // return this->get_json();
}

static int umount() {
  if (improved_stoi(get_popen_string("mount | grep /etc/nfs | wc -l")) > 0) {
    string cmd = "umount -l /etc/nfs > /dev/null 2>&1";
    int ret = system(cmd.c_str());
    if (ret == 0)
      return 0;
    else
      return -1;
  }
  return 0;
}
// dy : virtual media end


// message & message registry start
json::value Message::get_json(void)
{
    json::value j;

    j["Message"] = json::value::string(this->message);

    j["MessageArgs"] = json::value::array();
    for (int i = 0; i < this->message_args.size(); i++){
        j["MessageArgs"][i] = json::value::string(this->message_args[i]);
    }
    
    j["MessageId"] = json::value::string(this->id);
    j["MessageSeverity"] = json::value::string(this->severity);
    j["Resolution"] = json::value::string(this->resolution);

    return j;
}

// Message구조체에서 Message, MessageArgs, MessageId만 취급하는 get_json
// logentry에서 사용
void Message::get_specific_json(json::value &j)
{
    j["Message"] = json::value::string(this->message);

    j["MessageArgs"] = json::value::array();
    for (int i = 0; i < this->message_args.size(); i++){
        j["MessageArgs"][i] = json::value::string(this->message_args[i]);
    }
    
    j["MessageId"] = json::value::string(this->id);

    return;
}

void Message::load_json(json::value &j)
{
    get_value_from_json_key(j, "Message", this->message);
    get_value_from_json_key(j, "MessageId", this->id);
    get_value_from_json_key(j, "MessageSeverity", this->severity);
    get_value_from_json_key(j, "Resolution", this->resolution);
    
    json::value args;
    get_value_from_json_key(j, "MessageArgs", args);
    this->message_args.clear();
    for (auto val : args.as_array())
        this->message_args.push_back(val.as_string());
    return;
}

void Message::load_specific_json(json::value &j)
{
    get_value_from_json_key(j, "Message", this->message);
    get_value_from_json_key(j, "MessageId", this->id);

    json::value args;
    get_value_from_json_key(j, "MessageArgs", args);
    this->message_args.clear();
    for (auto val : args.as_array())
        this->message_args.push_back(val.as_string());
    return;
}

json::value Message_For_Registry::get_json(void)
{
    // json::value k;

    json::value k;
    k[("Description")] = json::value::string(this->description);
    k[("Message")] = json::value::string(this->message);
    k[("Severity")] = json::value::string(this->severity);
    k[("Resolution")] = json::value::string(this->resolution);
    k[("NumberOfArgs")] = json::value::number(this->number_of_args);
    k[("ParamTypes")] = json::value::array();
    for(int i=0; i<this->param_types.size(); i++)
    {
        k[("ParamTypes")][i] = json::value::string(this->param_types[i]);
    }
    
    // j[this->pattern] = k;
    return k;
}

json::value MessageRegistry::get_json(void)
{
    auto j = this->Resource::get_json();
    if (j.is_null())
        return j;

    j[U("Id")] = json::value::string(U(this->id));
    j[U("Language")] = json::value::string(U(this->language));
    j[U("RegistryPrefix")] = json::value::string(U(this->registry_prefix));
    j[U("RegistryVersion")] = json::value::string(U(this->registry_version));

    json::value messages;
    for(int i=0; i<this->messages.v_msg.size(); i++)
    {
        Message_For_Registry msg = this->messages.v_msg[i];
        messages[U(msg.pattern)] = msg.get_json();
        // messages[U(msg.id)] = msg.get_json();
    }

    j[U("Messages")] = messages;

    return j;
}

bool MessageRegistry::load_json(json::value &j)
{
    try
    {
        Resource::load_json(j);
        this->id = j.at("Id").as_string();
        this->language = j.at("Language").as_string();
        this->registry_prefix = j.at("RegistryPrefix").as_string();
        this->registry_version = j.at("RegistryVersion").as_string();

        // 이 외에 this->messages에 들어갈 Message vector요소들은 키워드(pattern)을 일일이
        // hasfield같은걸로 찾아서 읽은다음 거기서 Message 구조체에 담고 그걸 messages.v_msg에 push_back해주는식
        // 으로 구현해야함 아직 들어갈 키워드(패턴)들이 지정되지 않아서 자리만 만들어둠
        // for(int i = 0; i < this->messages.v_msg.size(); i++){
        //     this->messages.v_msg[i].load_json(j);
        // }
    }
    catch(json::json_exception &e)
    {
        return false;
    }
    
    return true;
}
// message registry end

// // message registry start
// json::value MessageRegistry::get_json(void) {
//   auto j = this->Resource::get_json();
//   if (j.is_null())
//     return j;

//   j[U("Id")] = json::value::string(U(this->id));
//   j[U("Language")] = json::value::string(U(this->language));
//   j[U("RegistryPrefix")] = json::value::string(U(this->registry_prefix));
//   j[U("RegistryVersion")] = json::value::string(U(this->registry_version));

//   json::value messages;
//   for (int i = 0; i < this->messages.v_msg.size(); i++) {
//     Message msg = this->messages.v_msg[i];
//     json::value tmp;
//     tmp[U("Description")] = json::value::string(U(msg.description));
//     tmp[U("Message")] = json::value::string(U(msg.message));
//     tmp[U("Severity")] = json::value::string(U(msg.severity));
//     tmp[U("Resolution")] = json::value::string(U(msg.resolution));
//     tmp[U("NumberOfArgs")] = json::value::number(U(msg.number_of_args));

//     if (!msg.param_types.empty()) {
//       tmp[U("ParamTypes")] = json::value::array();
//       for (int j = 0; j < msg.param_types.size(); j++) {
//         tmp[U("ParamTypes")][j] = json::value::string(U(msg.param_types[j]));
//       }
//     }

//     messages[U(msg.pattern)] = tmp;
//   }

//   j[U("Messages")] = messages;

//   return j;
// }

// bool MessageRegistry::load_json(json::value &j) {
//   try {
//     Resource::load_json(j);
//     this->id = j.at("Id").as_string();
//     this->language = j.at("Language").as_string();
//     this->registry_prefix = j.at("RegistryPrefix").as_string();
//     this->registry_version = j.at("RegistryVersion").as_string();

//     // 이 외에 this->messages에 들어갈 Message vector요소들은 키워드(pattern)을
//     // 일일이 hasfield같은걸로 찾아서 읽은다음 거기서 Message 구조체에 담고 그걸
//     // messages.v_msg에 push_back해주는식 으로 구현해야함 아직 들어갈
//     // 키워드(패턴)들이 지정되지 않아서 자리만 만들어둠

//   } catch (json::json_exception &e) {
//     return false;
//   }

//   return true;
// }
// // message registry end

// ServiceRoot start
json::value ServiceRoot::get_json(void) {
  auto j = this->Resource::get_json();
  if (j.is_null())
    return j;

  j[U("Id")] = json::value::string(U(this->id));
  j[U("RedfishVersion")] = json::value::string(U(this->redfish_version));
  j[U("UUID")] = json::value::string(U(this->uuid));

  j["Systems"] = get_resource_odata_id_json(this->system, this->odata.id);
  j["Chassis"] = get_resource_odata_id_json(this->chassis, this->odata.id);
  j["Managers"] = get_resource_odata_id_json(this->manager, this->odata.id);

  j["UpdateService"] =
      get_resource_odata_id_json(this->update_service, this->odata.id);
  j["AccountService"] =
      get_resource_odata_id_json(this->account_service, this->odata.id);
  j["SessionService"] =
      get_resource_odata_id_json(this->session_service, this->odata.id);
  j["TaskService"] =
      get_resource_odata_id_json(this->task_service, this->odata.id);
  j["EventService"] =
      get_resource_odata_id_json(this->event_service, this->odata.id);
  j["CertificateService"] =
      get_resource_odata_id_json(this->certificate_service, this->odata.id);

  return j;
}

// bool ServiceRoot::load_json(void)
// {
//     json::value j; // JSON read from input file

//     try
//     {
//         ifstream target_file(this->odata.id + ".json");
//         stringstream string_stream;

//         string_stream << target_file.rdbuf();
//         target_file.close();

//         j = json::value::parse(string_stream);
//     }
//     catch (json::json_exception &e)
//     {
//         throw json::json_exception("Error Parsing JSON file " +
//         this->odata.id); return false;
//     }

//     this->name = j.at("Name").as_string();
//     this->id = j.at("Id").as_string();
//     this->redfish_version = j.at("RedfishVersion").as_string();
//     this->uuid = j.at("UUID").as_string();
//     return true;
// }

// ServiceRoot end

bool is_session_valid(const string _token) {
  Session *session;
  // string odata_id = ODATA_SESSION_ID;
  Collection *col = (Collection *)g_record[ODATA_SESSION_ID];
  // odata_id = odata_id + '/' + _token_id;

  bool exist = false;
  std::vector<Resource *>::iterator iter;
  for (iter = col->members.begin(); iter != col->members.end(); iter++) {
    if (((Session *)(*iter))->x_token == _token) {
      session = (Session *)(*iter);
      exist = true;
      break;
    }
  }

  if (!exist)
    return false;

  if (!record_is_exist(session->odata.id))
    return false;
  // session = (Session *)g_record[odata_id];
  if (session->_remain_expires_time <= 0)
    return false;

  return true;
}

string get_session_odata_id_by_token(string _token) {
  Collection *session_col = (Collection *)g_record[ODATA_SESSION_ID];
  string session_odata = "";

  std::vector<Resource *>::iterator iter;
  for (iter = session_col->members.begin(); iter != session_col->members.end();
       iter++) {
    if (((Session *)(*iter))->x_token == _token) {
      session_odata = ((Session *)(*iter))->odata.id;
      break;
    }
  }
  return session_odata;
}

const std::string currentDateTime() {
  time_t now = time(0); //현재 시간을 time_t 타입으로 저장
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X",
           &tstruct); // YYYY-MM-DD.HH:mm:ss 형태의 스트링

  return buf;
}

/**
 * @brief Resource null pointer check, return odata id json
 * @author 김
 * @return if Resource is null, return json::value::null(). else return odata id
 * json
 */
json::value get_resource_odata_id_json(Resource *res, string loc) {
  if (res == nullptr) {
    log(warning) << "null pointer warning at " << loc;
    return json::value::null();
  } else
    return res->get_odata_id_json();
}

json::value get_action_info(unordered_map<string, Actions> act) {
  json::value actions;
  for (auto item : act) {
    json::value act_obj;
    act_obj["target"] = json::value::string(item.second.target);

    if (!item.second.parameters.empty()) {
      for (auto parameter : item.second.parameters) {
        if (!parameter.allowable_values.empty()) {
          act_obj[parameter.name + "@Redfish.AllowableValues"] =
              json::value::array();
          for (int i = 0; i < parameter.allowable_values.size(); i++) {
            act_obj[parameter.name + "@Redfish.AllowableValues"][i] =
                json::value::string(parameter.allowable_values[i]);
          }
        }
      }
    }
    actions[item.second.name] = act_obj;
  }

  return actions;
}

extern std::mutex redfish_m_mutex[5];
extern std::condition_variable redfish_m_cond[5];

void chassis_monitor(void *data) {
  std::unique_lock<std::mutex> lock(redfish_m_mutex[MU_CHASSIS]);
  while (true) {
    log(info) << "befor wait";
    redfish_m_cond[MU_CHASSIS].wait(lock);
    log(info) << "chassis_monitor";
  };
}
void systems_monitor(void *data) {
  std::unique_lock<std::mutex> lock(redfish_m_mutex[MU_SYSTEM]);
  while (true) {
    log(info) << "befor wait";
    redfish_m_cond[MU_SYSTEM].wait(lock);
    log(info) << "systems_monitor";
  };
  cout << "t" << endl;
}
void account_monitor(void *data) {
  std::unique_lock<std::mutex> lock(redfish_m_mutex[MU_ACCOUNT]);
  while (true) {
    log(info) << "befor wait";
    redfish_m_cond[MU_ACCOUNT].wait(lock);
    log(info) << "account_monitor";
  };
  cout << "t" << endl;
}
void manager_monitor(void *data) {
  std::unique_lock<std::mutex> lock(redfish_m_mutex[MU_MANAGER]);
  while (true) {
    log(info) << "befor wait";
    redfish_m_cond[MU_MANAGER].wait(lock);
    log(info) << "manager_monitor";
  };
  cout << "t" << endl;
}
void sessions_monitor(void *data) {
  std::unique_lock<std::mutex> lock(redfish_m_mutex[MU_SESSION]);
  while (true) {
    log(info) << "befor wait";
    redfish_m_cond[MU_SESSION].wait(lock);
    log(info) << "sessions_monitor";
  }
  cout << "t" << endl;
}

// void generate_logservice(string _res_odata, string _service_id)
// {
//     // _res_odata는 system, chassis, manager에 해당
//     uint8_t type = g_record[_res_odata]->type;

//     switch(type)
//     {
//         case SYSTEM_TYPE:{
//             Systems *sys = (Systems *)g_record[_res_odata];
//             sys->new_log_service(_service_id);
//             break;
//         }
//         case CHASSIS_TYPE:{
//             Chassis *cha = (Chassis *)g_record[_res_odata];
//             cha->new_log_service(_service_id);
//             break;
//         }
//         case MANAGER_TYPE:{
//             Manager *man = (Manager *)g_record[_res_odata];
//             man->new_log_service(_service_id);
//             break;
//         }
//         default:
//             log(error) << "wrong resource in generate logservice";
//             break;
//     }
// }

LogEntry *generate_logentry(IpmiLogEvent _ev) {
  // _ev :: event_path에 로그서비스가 있는 리소스가 들어있고
  // event_type에 로그서비스 id 되시겠습니다
  string owner = _ev.event_path;
  string service_odata;
  if (owner == "Chassis") {
    Chassis *cha = (Chassis *)g_record[ODATA_CHASSIS_ID];
    service_odata = ODATA_CHASSIS_ID;
    service_odata = service_odata + "/LogServices/" + _ev.event_type;
    if (!(record_is_exist(service_odata)))
      cha->new_log_service(_ev.event_type);
  } else if (owner == "System") {
    Systems *sys = (Systems *)g_record[ODATA_SYSTEM_ID];
    service_odata = ODATA_SYSTEM_ID;
    service_odata = service_odata + "/LogServices/" + _ev.event_type;
    if (!(record_is_exist(service_odata)))
      sys->new_log_service(_ev.event_type);
  } else if (owner == "Manager") {
    Manager *man = (Manager *)g_record[ODATA_MANAGER_ID];
    service_odata = ODATA_MANAGER_ID;
    service_odata = service_odata + "/LogServices/" + _ev.event_type;
    if (!(record_is_exist(service_odata)))
      man->new_log_service(_ev.event_type);
  } else {
    // error..?
  }
  // 로그서비스 생성(없는경우만)

  LogService *service = (LogService *)g_record[service_odata];
  LogEntry *entry;

  entry = service->new_log_entry(_ev);

  return entry;
}

// iptable
string make_iptable_cmd(string _op, string _pos, int _index, int _port,
                        int _able) {
  //  -I/-R, INPUT/OUTPUT, _index, _port, _able
  string cmd;
  switch (_able) {
  case 0:
    // ACCEPT
    if (_pos == "INPUT") {
      cmd = "iptables -" + _op;
      cmd = cmd + " INPUT " + to_string(_index) + " -p tcp --dport ";
      cmd = cmd + to_string(_port) + " -j ACCEPT";
    } else if (_pos == "OUTPUT") {
      cmd = "iptables -" + _op;
      cmd = cmd + " OUTPUT " + to_string(_index) + " -p tcp --sport ";
      cmd = cmd + to_string(_port) + " -j ACCEPT";
    }
    break;
  case 1:
    // REJECT
    if (_pos == "INPUT") {
      cmd = "iptables -" + _op;
      cmd = cmd + " INPUT " + to_string(_index) + " -p tcp --dport ";
      cmd = cmd + to_string(_port) + " -j REJECT";
    } else if (_pos == "OUTPUT") {
      cmd = "iptables -" + _op;
      cmd = cmd + " OUTPUT " + to_string(_index) + " -p tcp --sport ";
      cmd = cmd + to_string(_port) + " -j REJECT";
    }
    break;
  default:
    break;
  }

  return cmd;
}

void execute_iptables(NetworkProtocol *_net, int _index, string _op) {
  string cmd_input, cmd_output;
  switch (_index) {
  case HTTP_INDEX:
    if (_net->http_enabled == true) {
      // ACCEPT
      cmd_input =
          make_iptable_cmd(_op, "INPUT", HTTP_INDEX, _net->http_port, 0);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", HTTP_INDEX, _net->http_port, 0);
    } else {
      // REJECT
      cmd_input =
          make_iptable_cmd(_op, "INPUT", HTTP_INDEX, _net->http_port, 1);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", HTTP_INDEX, _net->http_port, 1);
    }
    break;

  case HTTPS_INDEX:
    if (_net->https_enabled == true) {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", HTTPS_INDEX, _net->https_port, 0);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", HTTPS_INDEX, _net->https_port, 0);

    } else {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", HTTPS_INDEX, _net->https_port, 1);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", HTTPS_INDEX, _net->https_port, 1);
    }
    break;

  case SNMP_INDEX:
    if (_net->snmp.protocol_enabled == true) {
      cmd_input = make_iptable_cmd(_op, "INPUT", SNMP_INDEX, _net->snmp.port, 0);
      cmd_output = make_iptable_cmd(_op, "OUTPUT", SNMP_INDEX, _net->snmp.port, 0);
      // cmd_input = make_iptable_cmd(_op, "INPUT", SNMP_INDEX, _net->snmp_port, 0);
      // cmd_output = make_iptable_cmd(_op, "OUTPUT", SNMP_INDEX, _net->snmp_port, 0);
    } else {
      cmd_input = make_iptable_cmd(_op, "INPUT", SNMP_INDEX, _net->snmp.port, 1);
      cmd_output = make_iptable_cmd(_op, "OUTPUT", SNMP_INDEX, _net->snmp.port, 1);
      // cmd_input = make_iptable_cmd(_op, "INPUT", SNMP_INDEX, _net->snmp_port, 1);
      // cmd_output = make_iptable_cmd(_op, "OUTPUT", SNMP_INDEX, _net->snmp_port, 1);
    }
    break;

  case IPMI_INDEX:
    if (_net->ipmi_enabled == true) {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", IPMI_INDEX, _net->ipmi_port, 0);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", IPMI_INDEX, _net->ipmi_port, 0);
    } else {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", IPMI_INDEX, _net->ipmi_port, 1);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", IPMI_INDEX, _net->ipmi_port, 1);
    }
    break;

  case KVMIP_INDEX:
    if (_net->kvmip_enabled == true) {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", KVMIP_INDEX, _net->kvmip_port, 0);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", KVMIP_INDEX, _net->kvmip_port, 0);
    } else {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", KVMIP_INDEX, _net->kvmip_port, 1);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", KVMIP_INDEX, _net->kvmip_port, 1);
    }
    break;

  case SSH_INDEX:
    if (_net->ssh_enabled == true) {
      cmd_input = make_iptable_cmd(_op, "INPUT", SSH_INDEX, _net->ssh_port, 0);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", SSH_INDEX, _net->ssh_port, 0);
    } else {
      cmd_input = make_iptable_cmd(_op, "INPUT", SSH_INDEX, _net->ssh_port, 1);
      cmd_output =
          make_iptable_cmd(_op, "OUTPUT", SSH_INDEX, _net->ssh_port, 1);
    }
    break;

  case VM_INDEX:
    if (_net->virtual_media_enabled == true) {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", VM_INDEX, _net->virtual_media_port, 0);
      cmd_output = make_iptable_cmd(_op, "OUTPUT", VM_INDEX,
                                    _net->virtual_media_port, 0);
    } else {
      cmd_input =
          make_iptable_cmd(_op, "INPUT", VM_INDEX, _net->virtual_media_port, 1);
      cmd_output = make_iptable_cmd(_op, "OUTPUT", VM_INDEX,
                                    _net->virtual_media_port, 1);
    }
    break;

  case NTP_INDEX:
    if (_net->ntp.protocol_enabled == true) {
      cmd_input = make_iptable_cmd(_op, "INPUT", NTP_INDEX, _net->ntp.port, 0);
      cmd_output = make_iptable_cmd(_op, "OUTPUT", NTP_INDEX, _net->ntp.port, 0);
      // cmd_input = make_iptable_cmd(_op, "INPUT", NTP_INDEX, _net->ntp_port, 0);
      // cmd_output = make_iptable_cmd(_op, "OUTPUT", NTP_INDEX, _net->ntp_port, 0);
    } else {
      cmd_input = make_iptable_cmd(_op, "INPUT", NTP_INDEX, _net->ntp.port, 1);
      cmd_output = make_iptable_cmd(_op, "OUTPUT", NTP_INDEX, _net->ntp.port, 1);
      // cmd_input = make_iptable_cmd(_op, "INPUT", NTP_INDEX, _net->ntp_port, 1);
      // cmd_output = make_iptable_cmd(_op, "OUTPUT", NTP_INDEX, _net->ntp_port, 1);
    }
    break;
  default:
    break;
  }

  system(cmd_input.c_str());
  system(cmd_output.c_str());
}

void init_iptable(NetworkProtocol *_net) {
  execute_iptables(_net, HTTP_INDEX, "I");
  execute_iptables(_net, HTTPS_INDEX, "I");
  execute_iptables(_net, SNMP_INDEX, "I");
  execute_iptables(_net, IPMI_INDEX, "I");
  execute_iptables(_net, KVMIP_INDEX, "I");
  execute_iptables(_net, SSH_INDEX, "I");
  execute_iptables(_net, VM_INDEX, "I");
  execute_iptables(_net, NTP_INDEX, "I");

  system("iptables-save > /etc/iptables.rules");
}

void patch_iptable(NetworkProtocol *_net) {
  execute_iptables(_net, HTTP_INDEX, "R");
  execute_iptables(_net, HTTPS_INDEX, "R");
  execute_iptables(_net, SNMP_INDEX, "R");
  execute_iptables(_net, IPMI_INDEX, "R");
  execute_iptables(_net, KVMIP_INDEX, "R");
  execute_iptables(_net, SSH_INDEX, "R");
  execute_iptables(_net, VM_INDEX, "R");
  execute_iptables(_net, NTP_INDEX, "R");

  system("iptables-save > /etc/iptables.rules");
}

/**
 * @brief 센서 생성&업데이트 (BMC용)
 * @param _sm --> 센서에 들어갈 값을 가지는 구조체
 * @param _flag --> _sm 구조체에서 값을 반영할 멤버변수를 가리키는 플래그정보
 *
 * @details _flag는 16bit에서 15비트 사용한다. SensorMake 구조체에서 필수값(id,
 * reading_type, reading_time, reading)을 제외한 나머지 멤버변수들을 순차적으로
 * 우측에서부터 비트값을 준다. reading_units -> bit 0x1 (0000 0000 0000 0001)
 * reading_range_max -> bit 0x2 (0000 0000 0000 0010)
 * ...
 * thresh.lower_caution -> bit 0x80 (0000 0000 1000 0000)
 * ...
 * status.health -> bit 0x4000 (0100 0000 0000 0000)
 */
void make_sensor(SensorMake _sm, uint16_t _flag) {
  string odata = ODATA_CHASSIS_ID;
  odata = odata + "/Sensors/";
  odata = odata + _sm.id;

  Sensor *sensor;
  if (record_is_exist(odata)) {
    sensor = (Sensor *)g_record[odata];
  } else {
    sensor = new Sensor(odata, _sm.id);
    string col_odata = get_parent_object_uri(odata, "/");
    Collection *col = nullptr;
    col = (Collection *)g_record[col_odata];
    if (col == nullptr) {
      cout << "Collection Not Init .. deley" << endl;
      if (sensor != nullptr)
        delete (sensor);
      delay(100);
      return;
    }
    col->add_member(sensor);
    resource_save_json(col);
  }

  sensor->reading_type = _sm.reading_type;
  sensor->reading_time = _sm.reading_time;
  sensor->reading = _sm.reading;
  if (_flag & 0x1) {
    // cout << "0000 0000 0000 0001" << endl;
    sensor->reading_units = _sm.reading_units;
  }

  if (_flag & 0x2) {
    // cout << "0000 0000 0000 0010" << endl;
    sensor->reading_range_max = _sm.reading_range_max;
  }

  if (_flag & 0x4) {
    // cout << "0000 0000 0000 0100" << endl;
    sensor->reading_range_min = _sm.reading_range_min;
  }

  if (_flag & 0x8) {
    // cout << "0000 0000 0000 1000" << endl;
    sensor->accuracy = _sm.accuracy;
  }

  if (_flag & 0x10) {
    // cout << "0000 0000 0001 0000" << endl;
    sensor->precision = _sm.precision;
  }

  if (_flag & 0x20) {
    // cout << "0000 0000 0010 0000" << endl;
    sensor->physical_context = _sm.physical_context;
  }

  if (_flag & 0x40) {
    // cout << "0000 0000 0100 0000" << endl;
    sensor->sensing_interval = _sm.sensing_interval;
  }

  if (_flag & 0x80) {
    // cout << "0000 0000 1000 0000" << endl;
    sensor->thresh.lower_caution = _sm.thresh.lower_caution;
  }

  if (_flag & 0x100) {
    // cout << "0000 0001 0000 0000" << endl;
    sensor->thresh.lower_critical = _sm.thresh.lower_critical;
  }

  if (_flag & 0x200) {
    // cout << "0000 0010 0000 0000" << endl;
    sensor->thresh.lower_fatal = _sm.thresh.lower_fatal;
  }

  if (_flag & 0x400) {
    // cout << "0000 0100 0000 0000" << endl;
    sensor->thresh.upper_caution = _sm.thresh.upper_caution;
  }

  if (_flag & 0x800) {
    // cout << "0000 1000 0000 0000" << endl;
    sensor->thresh.upper_critical = _sm.thresh.upper_critical;
  }

  if (_flag & 0x1000) {
    // cout << "0001 0000 0000 0000" << endl;
    sensor->thresh.upper_fatal = _sm.thresh.upper_fatal;
  }

  if (_flag & 0x2000) {
    // cout << "0010 0000 0000 0000" << endl;
    sensor->status.state = _sm.status.state;
  }

  if (_flag & 0x4000) {
    // cout << "0100 0000 0000 0000" << endl;
    sensor->status.health = _sm.status.health;
  }
  resource_save_json(sensor);
}
