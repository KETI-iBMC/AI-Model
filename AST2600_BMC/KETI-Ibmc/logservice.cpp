#include <redfish/logservice.hpp>



// Database
void make_test_db()
{
    sqlite3 *db;
    char *err_msg=0;
    char query[500] = {0};
    int rc = sqlite3_open(LOG_DB, &db);

    sprintf(query, "CREATE TABLE Reading(ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, SensorID CHAR (50) NOT NULL, Type CHAR(50) NOT NULL, Value REAL, Time CHAR(60), Location CHAR(50));");
    sqlite3_exec(db, query, 0, 0, &err_msg);

    sprintf(query, "INSERT INTO Reading (SensorID, Type, Value, Time, Location) VALUES(\"Sensornumber123\", \"Temperature\", 36.5, \"2021-10-29 16:00:05\", \"Chassis\");");
    sqlite3_exec(db, query, 0, 0, &err_msg);

    sprintf(query, "INSERT INTO Reading (SensorID, Type, Value, Time, Location) VALUES(\"Sensornumber123456\", \"Temperature\", 40.8, \"2021-10-29 16:11:43\", \"Chassis\");");
    sqlite3_exec(db, query, 0, 0, &err_msg);

    sprintf(query, "INSERT INTO Reading (SensorID, Type, Value, Time, Location) VALUES(\"Sensornumber00700\", \"Fan\", 5000, \"2021-10-29 17:01:25\", \"Chassis\");");
    sqlite3_exec(db, query, 0, 0, &err_msg);

    sqlite3_close(db);
    sqlite3_free(err_msg);

    cout << " FIN!! " << endl;

}

int check_reading_db_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
  if(argv[0] != NULL){
    *((int *)NotUsed) = atoi(argv[0]);
  }
  
  return 0;
}

int result_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    NotUsed = 0;

    for(int i=0; i<argc; i++)
    {
        // cout << "현재 i : " << i << endl;
        cout << azColName[i] << " = " << argv[i] << endl;
    }

    return 0;
}

int check_same_time_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    if(argv[0] == NULL)
    {
        *((int *)NotUsed) = 0;
    }
    else
    {
        *((int *)NotUsed) = 1;
    }

    return 0;
}


/**
 * @brief 센서측정값 Reading Table에 insert하는 함수
 * @param _sensor_id - 센서id
 * @param _module - 센서가 위치한 모듈 ex> CMM1, CM1..
 * @param _type - 센서의 위치,타입 정보 ex> power, thermal..
 * @param _detail - 센서의 상세타입 ex> powercontrol, powersupply, temperature, fan...
 * @param _value - 센서 측정값 @param _time - 측정된 시각(yyyy-mm-dd hh::mm)
 */
void insert_reading_table(string _sensor_id, string _module, string _type, string _detail, int _value, string _time)
{
    sqlite3 *db;
    char *err_msg=0;
    char query[500] = {0};
    int rc = sqlite3_open(LOG_DB, &db);
    int callback_value = 0;

    // Table Check
    // sprintf(query, "SELECT * FROM Reading");
    sprintf(query, "SELECT COUNT(*) FROM sqlite_master where name=\"Reading\";");
    rc = sqlite3_exec(db, query, check_reading_db_callback, &callback_value, &err_msg);
    // cout << " CALLBACK RETURN " << endl;
    // cout << callback_value << endl;

    if(callback_value == 0)
    {
      // create table
      sprintf(query, "CREATE TABLE Reading(ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, SensorID TEXT NOT NULL, Module TEXT NOT NULL, Type TEXT NOT NULL, Detail TEXT NOT NULL, Value REAL, Time TEXT);");
      sqlite3_exec(db, query, 0, 0, &err_msg);
      log(info) << "TABLE [Reading] is Created in " << LOG_DB;
    }

    // Make Time Format
    // cout << " TIME TIME " << endl;
    // char outout[100] = {0};
    // make_time_with_tm(_tm, outout);
    // printf("%s\n", outout);

     // 추가 로직
    string inputYear = _time.substr(0,4);
    // cout << "[INPUT YEAR CHECK] : " << inputYear << endl;

    if(inputYear < "2000")
        return ;

    // 여기는 이제 같은 _sensor_id 값의 동일 시간 _time 의 데이터가 DB에 있는지 서치
    int existSameData;
    sprintf(query, "SELECT * FROM Reading WHERE Module=\"%s\" and SensorID=\"%s\" and Time=\"%s\";"
    , _module.c_str(), _sensor_id.c_str(), _time.c_str());
    sqlite3_exec(db, query, check_same_time_callback, &existSameData, &err_msg);

    if(existSameData == 1)
        return ;

    // Insert Query
    sprintf(query, "INSERT INTO Reading (SensorID, Module, Type, Detail, Value, Time) VALUES(\"%s\", \"%s\", \"%s\", \"%s\", %d, \"%s\");"
    , _sensor_id.c_str(), _module.c_str(), _type.c_str(), _detail.c_str(), _value, _time.c_str());
    sqlite3_exec(db, query, 0, 0, &err_msg);

    sqlite3_free(err_msg);
    sqlite3_close(db);
}

const std::string DB_currentDateTime() {
  time_t now = time(0); //현재 시간을 time_t 타입으로 저장
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  // strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M",&tstruct); // YYYY-MM-DD.HH:mm:ss 형태의 스트링
  strftime(buf, sizeof(buf), "%Y-%m-%d %X",&tstruct); // YYYY-MM-DD HH:mm:ss 형태의 스트링

  return buf;
}
// void insert_reading_table(string _sensor_id, string _type, double _value, tm
// _tm, string _module) void insert_reading_table(string _sensor_id, string
// _type, double _value, string _time, string _module)/**/
/**
 * @brief db 저장하는 함수
 * 
 * @param _sensor_id 센서 이름 SEL 센서이름,PSU1 PSU2
 * @param _module 예시 / CM1/CM2
 * @param _type power, thermal
 * @param _detail power: powercontrol, powersupply powervoltage /thermal: fan, temperature smartheater
 * @param _value value 
 * @param _time 
 * @author 현준
 */
void insert_reading_table(string _sensor_id, string _module, string _type, string _detail, double _value, string _time)
{
    sqlite3 *db;
    char *err_msg=0;
    char query[500] = {0};
    int rc = sqlite3_open(LOG_DB, &db);
    int callback_value = 0;
    // Table Check
    // sprintf(query, "SELECT * FROM Reading");
    sprintf(query, "SELECT COUNT(*) FROM sqlite_master where name=\"Reading\";");
    rc = sqlite3_exec(db, query, check_reading_db_callback, &callback_value, &err_msg);
    // cout << " CALLBACK RETURN " << endl;
    // cout << callback_value << endl;

    if(callback_value == 0)
    {
      // create table
      sprintf(query, "CREATE TABLE Reading(ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, SensorID TEXT NOT NULL, Module TEXT NOT NULL, Type TEXT NOT NULL, Detail TEXT NOT NULL, Value REAL, Time TEXT);");
      sqlite3_exec(db, query, 0, 0, &err_msg);
      log(info) << "TABLE [Reading] is Created in " << LOG_DB;
    }

    // Make Time Format
    // cout << " TIME TIME " << endl;
    // char outout[100] = {0};
    // make_time_with_tm(_tm, outout);
    // printf("%s\n", outout);

     // 추가 로직
    string inputYear = _time.substr(0,4);
    // cout << "[INPUT YEAR CHECK] : " << inputYear << endl;

    if(inputYear < "2000")
        return ;

    // 여기는 이제 같은 _sensor_id 값의 동일 시간 _time 의 데이터가 DB에 있는지 서치
    int existSameData;
    sprintf(query, "SELECT * FROM Reading WHERE Module=\"%s\" and SensorID=\"%s\" and Time=\"%s\";"
    , _module.c_str(), _sensor_id.c_str(), _time.c_str());
    sqlite3_exec(db, query, check_same_time_callback, &existSameData, &err_msg);

    if(existSameData == 1)
        return ;

    // Insert Query
    sprintf(query, "INSERT INTO Reading (SensorID, Module, Type, Detail, Value, Time) VALUES(\"%s\", \"%s\", \"%s\", \"%s\", %lf, \"%s\");"
    , _sensor_id.c_str(), _module.c_str(), _type.c_str(), _detail.c_str(), _value, _time.c_str());
    sqlite3_exec(db, query, 0, 0, &err_msg);

    sqlite3_free(err_msg);
    sqlite3_close(db);
}

int extract_column_callback(void *_vector, int argc, char **argv, char **azColName)
{
    ((vector<string> *)_vector)->push_back(argv[0]);

    return 0;
}


int make_json_min_reading_callback(void *_vector, int argc, char **argv, char **azColName)
{

    json::value jv;

    for(int i=0; i<argc; i++)
    {
        if(argv[i] == NULL)
          continue;

        string col = azColName[i];
        string val = argv[i];
        if(col == "Value")
        {
            jv[col] = json::value::number(stof(val));
        }
        else if(col == "Time")
        {
            jv[col] = json::value::string(val);
        }
    }
    // 일단 value랑 time값만

    ((vector<json::value> *)_vector)->push_back(jv);

    return 0;
}

int make_json_hour_reading_callback(void *_struct, int argc, char **argv, char **azColName)
{
    // ((Hour_Reading_Info *)_struct)  복사용
    if(((Hour_Reading_Info *)_struct)->items.size() >= ((Hour_Reading_Info *)_struct)->max_count)
      return 0;

    json::value jv;
    string time_val;
    
    for(int i=0; i<argc; i++)
    {
        if(argv[i] == NULL)
          continue;
        
        string col = azColName[i];
        string val = argv[i];
        if(col == "Value")
        {
            jv[col] = json::value::number(stof(val));
        }
        else if(col == "Time")
        {
            if(((Hour_Reading_Info *)_struct)->cur_time == "")
            {
              time_val = val;
            }
            else
            {
              // 조건안맞으면 바로 리턴 , 조건 맞으면 아래 추가하고 시간 다시 구하고 ㅇㅇ
              if(val > ((Hour_Reading_Info *)_struct)->compare_time)
              {
                return 0;
              }
              else
                time_val = val;
            }
            jv[col] = json::value::string(val);
        }

    }

    ((Hour_Reading_Info *)_struct)->items.push_back(jv);
    ((Hour_Reading_Info *)_struct)->cur_time = time_val; // item에 푸시된 녀석의 Time
    // cur_time(time_val) 가지고 1시간 이전의 시간을 구할거임
    
    time_t init_time;
    struct tm *tm_cur;
    time(&init_time);
    tm_cur = localtime(&init_time);

    // 현재시간 string -> tm
    strptime(time_val.c_str(), "%F %H:%M", tm_cur);
    // strptime(time_val.c_str(), "%F %T", tm_cur);

    // 현재시간 tm -> time_t + 1시간 이전시간 계산
    time_t t1 = mktime(tm_cur);
    // cout << "t1 : " << t1 << endl;
    t1 -= 3600;
    // cout << "t1 re : " << t1 << endl;

    // 1시간 이전시간 time_t -> tm
    struct tm *tm_pre;
    tm_pre = localtime(&t1);
    tm_pre->tm_sec = 0;

    // 1시간 이전시간 tm -> string
    char outout[30];
    make_time_with_tm(*tm_pre, outout);
    string str_out(outout);

    cout << "CUR : " << time_val << endl;
    cout << "Hour Before : " << str_out << endl;

    ((Hour_Reading_Info *)_struct)->compare_time = str_out;


    return 0;
}

// 바뀐구조로 새로 만든 select 함수
json::value select_min_reading(string _module, string _type, string _detail, int _count)
{
    sqlite3 *db;
    char *err_msg=0;
    char query[500] = {0};
    int rc = sqlite3_open(LOG_DB, &db);

    // #1 아예 type detail module 다 필수로 받기로했으니깐 첨에 sensorid얻어올때
    // module, type, detail 로 필터링해서 얻어오기
    // #2 그런다음에 json만들어주고 어쨋든 items 배열에는 value time값만 들어가니깐 얻어오는 콜백함수에서
    // 2개만 가져오면 될듯하다

    vector<string> v_sensor;
    // sprintf(query, "SELECT SensorID FROM Reading WHERE Module=\"%s\" and Type=\"%s\" and Detail=\"%s\" Group by SensorID;"
    // , _module.c_str(), _type.c_str(), _detail.c_str());

    // 쿼리에서 module_id 제거 
    sprintf(query, "SELECT SensorID FROM Reading WHERE Type=\"%s\" and Detail=\"%s\" Group by SensorID;"
    , _type.c_str(), _detail.c_str());
    sqlite3_exec(db, query, extract_column_callback, &v_sensor, &err_msg);


    json::value jv_result;
    jv_result["ModuleID"] = json::value::string(_module);
    jv_result["Readings"] = json::value::array();
    // cout << "EXTRACT ~~~~~~~" << endl;
    for(int i=0; i<v_sensor.size(); i++)
    {
        // cout << v_sensor[i] << endl;
        json::value jv_reading;
        jv_reading["SensorID"] = json::value::string(v_sensor[i]);
        jv_reading["Type"] = json::value::string(_type);
        jv_reading["Detail"] = json::value::string(_detail);
        
        vector<json::value> v_item;
       // 쿼리에서 module_id 제거 
        sprintf(query, "SELECT * FROM Reading WHERE SensorID=\"%s\" order by Time DESC limit %d;"
        , v_sensor[i].c_str(), _count);

        sqlite3_exec(db, query, make_json_min_reading_callback, &v_item, &err_msg);

        // item 을 jv_result에 담아주기
        jv_reading["Items"] = json::value::array(v_item);
        jv_result["Readings"][i] = jv_reading;
    }

    // cout << " RESULT !! " << endl;
    // cout << jv_result << endl;
    // cout << " RESULT !! " << endl;

    sqlite3_free(err_msg);
    sqlite3_close(db);

    return jv_result;
}

json::value select_hour_reading(string _module, string _type, string _detail, int _count)
{
    sqlite3 *db;
    char *err_msg=0;
    char query[500] = {0};
    int rc = sqlite3_open(LOG_DB, &db);

    // #1 아예 type detail module 다 필수로 받기로했으니깐 첨에 sensorid얻어올때
    // module, type, detail 로 필터링해서 얻어오기
    // #2 센서id 얻어오는것 까지는 똑같은데 센서id로 item뽑을때 첫 item으로부터 1시간 이전 시간인거까지 
    // continue로 지나쳐야함 그리고 조건에 맞는 녀석이 나오면 그때 item에넣고 시간갱신하고
    // min하고 콜백함수가 다른걸 써야겠네

    vector<string> v_sensor;
    // 쿼리에서 module_id 제거 
    sprintf(query, "SELECT SensorID FROM Reading WHERE Type=\"%s\" and Detail=\"%s\" Group by SensorID;"
    , _type.c_str(), _detail.c_str());
    sqlite3_exec(db, query, extract_column_callback, &v_sensor, &err_msg);


    json::value jv_result;
    jv_result["ModuleID"] = json::value::string(_module);
    jv_result["Readings"] = json::value::array();
    // cout << "EXTRACT ~~~~~~~" << endl;
    for(int i=0; i<v_sensor.size(); i++)
    {
        // cout << v_sensor[i] << endl;
        json::value jv_reading;
        jv_reading["SensorID"] = json::value::string(v_sensor[i]);
        jv_reading["Type"] = json::value::string(_type);
        jv_reading["Detail"] = json::value::string(_detail);
        

        Hour_Reading_Info reading_info;
        reading_info.max_count = _count;
        reading_info.cur_time = "";
        // vector<json::value> v_item;
        // 쿼리에서 module_id 제거 
        sprintf(query, "SELECT * FROM Reading WHERE SensorID=\"%s\" order by Time DESC;"
        , v_sensor[i].c_str());
        sqlite3_exec(db, query, make_json_hour_reading_callback, &reading_info, &err_msg);

        // item 을 jv_result에 담아주기
        jv_reading["Items"] = json::value::array(reading_info.items);
        jv_result["Readings"][i] = jv_reading;
    }

    // cout << " RESULT !! " << endl;
    // cout << jv_result << endl;
    // cout << " RESULT !! " << endl;

    sqlite3_free(err_msg);
    sqlite3_close(db);

    return jv_result;
  
}

// json::value select_all_time_reading(string _module, string _type)
// {
//     sqlite3 *db;
//     char *err_msg=0;
//     char query[500] = {0};
//     int rc = sqlite3_open(LOG_DB, &db);

//     // #1 Reading Table에서 SensorID부터 추출,파악
//     // #2 SensorID별로 최신 10개의 로그내용 추출 근데 일단 3개만 하는걸로 할게 테스트단계니까
//     // #3 Json Format으로 만들기

//     // NEW!!!!
//     // 파라미터로는 type하고 module만 받을게 그리고 시간은 쿼리로하는게 아니라 시간계산을 하고 쿼리를 해야돼서
//     // 함수자체를 여러개 만들거임
//     // INTERSECT로 SensorID뽑은거 교집합해서 구하고
//     // SELECT * 할때 센서id만 하면 다른모듈 같은 센서id나올수있으니까 조건에 module까지는 걸어줘야할듯

//     // Module Query
//     char query_module[500] = {0}, query_type[500] = {0};
//     if(_module == "all")
//     {
//       sprintf(query_module, "SELECT SensorID FROM Reading Group by SensorID");
//     }
//     else
//     {
//       sprintf(query_module, "SELECT SensorID FROM Reading WHERE Module=\"%s\" Group by SensorID", _module.c_str());
//     }

//     if(_type == "all")
//     {
//       sprintf(query_type, "SELECT SensorID FROM Reading Group by SensorID");
//     }
//     else
//     {
//       sprintf(query_type, "SELECT SensorID FROM Reading WHERE Type=\"%s\" Group by SensorID", _type.c_str());
//     }

//     vector<string> v_sensor;
//     sprintf(query, "%s INTERSECT %s;", query_module, query_type);
//     // sprintf(query, "SELECT SensorID from Reading where Type=\"%s\" Group by SensorID;", _type.c_str());
//     sqlite3_exec(db, query, extract_column_callback, &v_sensor, &err_msg);

//     json::value jv_result;
//     // cout << "EXTRACT ~~~~~~~" << endl;
//     for(int i=0; i<v_sensor.size(); i++)
//     {
//         // cout << v_sensor[i] << endl;
//         vector<json::value> v_item;
//         sprintf(query, "SELECT * from Reading where SensorID=\"%s\" order by Time DESC limit 3;", v_sensor[i].c_str());
//         sqlite3_exec(db, query, make_json_reading_callback, &v_item, &err_msg);

//         // item 을 jv_result에 담아주기
//         jv_result[v_sensor[i]] = json::value::array(v_item);
//     }

//     // cout << " RESULT !! " << endl;
//     // cout << jv_result << endl;
//     // cout << " RESULT !! " << endl;

//     sqlite3_free(err_msg);
//     sqlite3_close(db);

//     return jv_result;
// }



// json::value select_one_hour_reading(string _module, string _type)
// {
//     sqlite3 *db;
//     char *err_msg=0;
//     char query[500] = {0};
//     int rc = sqlite3_open(LOG_DB, &db);


//     vector<string> v_sensor;
//     sprintf(query, "SELECT SensorID from Reading where Type=\"%s\" Group by SensorID;", _type.c_str());
//     sqlite3_exec(db, query, extract_column_callback, &v_sensor, &err_msg);

//     time_t cur_time = time(NULL);
//     struct tm cur_tm;
//     char filter_time[30] = {0};
//     char origin_time[30] = {0};

//     // cur_tm = *localtime(&cur_time);
//     // make_time_with_tm(cur_tm, origin_time);
//     // cout << " CUR TIME !!!" << endl;
//     // cout << origin_time << endl;

//     cur_time -= 3600;
//     cur_tm = *localtime(&cur_time);
//     make_time_with_tm(cur_tm, filter_time);
//     // cout << " FILTER TIME !!!" << endl;
//     // cout << filter_time << endl;
    


//     json::value jv_result;
//     // cout << "EXTRACT ~~~~~~~" << endl;
//     for(int i=0; i<v_sensor.size(); i++)
//     {
//         // cout << v_sensor[i] << endl;
//         vector<json::value> v_item;
//         sprintf(query, "SELECT * from Reading where SensorID=\"%s\" and Time > \"%s\" order by Time DESC;"
//         , v_sensor[i].c_str(), filter_time);
//         sqlite3_exec(db, query, make_json_reading_callback, &v_item, &err_msg);

//         // item 을 jv_result에 담아주기
//         jv_result[v_sensor[i]] = json::value::array(v_item);
//     }

//     // cout << " RESULT !! " << endl;
//     // cout << jv_result << endl;
//     // cout << " RESULT !! " << endl;

//     sqlite3_free(err_msg);
//     sqlite3_close(db);
    
//     return jv_result;
// }

// 쿼리 하나당이니까 쿼리 하나에 psu1 꺼 마지막 데이터 읽어서 그때부터 60분 이전까지 데이터 읽기
json::value getTodayTotal()
{
  json::value jv_result;
  json::value jv_psu1, jv_psu2;

  jv_psu1["LAST60MIN"] = json::value::number(getLast60Min(PSU1_SENSOR_ID));
  jv_psu1["LAST24HOUR"] = json::value::number(getLast24Hour(PSU1_SENSOR_ID));
  jv_psu2["LAST60MIN"] = json::value::number(getLast60Min(PSU2_SENSOR_ID));
  jv_psu2["LAST24HOUR"] = json::value::number(getLast24Hour(PSU2_SENSOR_ID));
  
  jv_result["PSU1"] = jv_psu1;
  jv_result["PSU2"] = jv_psu2;

  // TODAYTOTAL 연결은 밖에서~~
  return jv_result;
}

double getLast60Min(string sensorId)
{
  sqlite3 *db;
  char *err_msg=0;
  char query[500] = {0};
  int rc = sqlite3_open(LOG_DB, &db);

  string lastTime;
  sprintf(query, "SELECT Time FROM Reading WHERE SensorID=\"%s\" order by Time DESC limit 1;", sensorId.c_str());
  sqlite3_exec(db, query, get_last_time_callback, &lastTime, &err_msg);

  string resultTime;
  resultTime = getChangedTime(lastTime, -3600);

  double totalValue = 0;
  sprintf(query, "SELECT Value FROM Reading WHERE SensorID=\"%s\" and Time > \"%s\" order by Time DESC", sensorId.c_str(), resultTime.c_str());
  sqlite3_exec(db, query, get_total_power_value_callback, &totalValue, &err_msg);

  // log(warning) << "[TOTAL VALUE in Last 60 Minutes] : " << totalValue;

  sqlite3_free(err_msg);
  sqlite3_close(db);

  return totalValue;
}

double getLast24Hour(string sensorId)
{
  sqlite3 *db;
  char *err_msg=0;
  char query[500] = {0};
  int rc = sqlite3_open(LOG_DB, &db);

  string lastTime;
  sprintf(query, "SELECT Time FROM Reading WHERE SensorID=\"%s\" order by Time DESC limit 1;", sensorId.c_str());
  sqlite3_exec(db, query, get_last_time_callback, &lastTime, &err_msg);

  string resultTime;
  resultTime = getChangedTime(lastTime, -86400);

  double totalValue = 0;
  sprintf(query, "SELECT Value FROM Reading WHERE SensorID=\"%s\" and Time > \"%s\" order by Time DESC", sensorId.c_str(), resultTime.c_str());
  sqlite3_exec(db, query, get_total_power_value_callback, &totalValue, &err_msg);

  // log(warning) << "[TOTAL VALUE in Last 60 Minutes] : " << totalValue;

  sqlite3_free(err_msg);
  sqlite3_close(db);

  return totalValue;
}

json::value getPeakData(void)
{
  json::value jv_result;
  json::value jv_type1, jv_type2, jv_type3, jv_type4;
  // type 1/2/3/4 : day/last 10m/last 1h/last 10h


  jv_type1["HIGHEST_PEAK"] = getHighestPeak(PEAK_TIME_ONE_DAY);
  jv_type1["LOWEST_PEAK"] = getLowestPeak(PEAK_TIME_ONE_DAY);
  jv_type2["HIGHEST_PEAK"] = getHighestPeak(PEAK_TIME_LAST_TEN_MINUTES);
  jv_type2["LOWEST_PEAK"] = getLowestPeak(PEAK_TIME_LAST_TEN_MINUTES);
  jv_type3["HIGHEST_PEAK"] = getHighestPeak(PEAK_TIME_LAST_ONE_HOUR);
  jv_type3["LOWEST_PEAK"] = getLowestPeak(PEAK_TIME_LAST_ONE_HOUR);
  jv_type4["HIGHEST_PEAK"] = getHighestPeak(PEAK_TIME_LAST_TEN_HOURS);
  jv_type4["LOWEST_PEAK"] = getLowestPeak(PEAK_TIME_LAST_TEN_HOURS);


  jv_result["DAY"] = jv_type1;
  jv_result["LAST_10_MIN"] = jv_type2;
  jv_result["LAST_1_HOUR"] = jv_type3;
  jv_result["LAST_10_HOUR"] = jv_type4;

  // PEAKDATA 연결은 밖에서~~
  return jv_result;
}

json::value getHighestPeak(int timeOption)
{
  sqlite3 *db;
  char *err_msg=0;
  char query[500] = {0};
  int rc = sqlite3_open(LOG_DB, &db);

  string lastTime;
  sprintf(query, "SELECT Time FROM Reading WHERE SensorID=\"%s\" or SensorID=\"%s\" order by Time DESC limit 1;", PSU1_SENSOR_ID, PSU2_SENSOR_ID);
  sqlite3_exec(db, query, get_last_time_callback, &lastTime, &err_msg);

  // log(error) << " [In getHighestPeak ] -----------------------------------------";
  // log(error) << " lastTime Query : " << query;
  // log(error) << " lastTime : " << lastTime;

  // Time Option 처리 부분
  string start_point, end_point;
  switch(timeOption)
  {
      case PEAK_TIME_ONE_DAY:{
        string date = lastTime.substr(0, 10);
        start_point = date + " 00:00:00";
        end_point = date + " 23:59:59";
        break;
      }
      case PEAK_TIME_LAST_TEN_MINUTES:{
        end_point = lastTime;
        start_point = getChangedTime(end_point, -600);
        break;
      }
      case PEAK_TIME_LAST_ONE_HOUR:{
        end_point = lastTime;
        start_point = getChangedTime(end_point, -3600);
        break;
      }
      case PEAK_TIME_LAST_TEN_HOURS:{
        end_point = lastTime;
        start_point = getChangedTime(end_point, -36000);
        break;
      }
      default:
        break;
  }

  // log(error) << " START END RANGE [ " << timeOption << " ] : " << start_point << " ~ " << end_point;

  json::value jv_result;
  sprintf(query, "SELECT SensorID, Value, Time FROM Reading WHERE (SensorID=\"%s\" or SensorID=\"%s\") and \
  (Time >= \"%s\" and Time <= \"%s\") order by Value DESC limit 1;", PSU1_SENSOR_ID, PSU2_SENSOR_ID, start_point.c_str(), end_point.c_str());
  sqlite3_exec(db, query, get_peak_power_value_callback, &jv_result, &err_msg);

  // log(error) << " QUERY : " << query;
  // log(error) << " AFTER JV_RESULT HIGH >>>> ";
  // log(error) << jv_result;

  sqlite3_free(err_msg);
  sqlite3_close(db);

  return jv_result;
}

json::value getLowestPeak(int timeOption)
{
  sqlite3 *db;
  char *err_msg=0;
  char query[500] = {0};
  int rc = sqlite3_open(LOG_DB, &db);

  string lastTime;
  sprintf(query, "SELECT Time FROM Reading WHERE SensorID=\"%s\" or SensorID=\"%s\" order by Time DESC limit 1;", PSU1_SENSOR_ID, PSU2_SENSOR_ID);
  sqlite3_exec(db, query, get_last_time_callback, &lastTime, &err_msg);

  // Time Option 처리 부분
  string start_point, end_point;
  switch(timeOption)
  {
      case PEAK_TIME_ONE_DAY:{
        string date = lastTime.substr(0, 10);
        start_point = date + " 00:00:00";
        end_point = date + " 23:59:59";
        break;
      }
      case PEAK_TIME_LAST_TEN_MINUTES:{
        end_point = lastTime;
        start_point = getChangedTime(end_point, -600);
        break;
      }
      case PEAK_TIME_LAST_ONE_HOUR:{
        end_point = lastTime;
        start_point = getChangedTime(end_point, -3600);
        break;
      }
      case PEAK_TIME_LAST_TEN_HOURS:{
        end_point = lastTime;
        start_point = getChangedTime(end_point, -36000);
        break;
      }
      default:
        break;
  }

  json::value jv_result;
  sprintf(query, "SELECT SensorID, Value, Time FROM Reading WHERE (SensorID=\"%s\" or SensorID=\"%s\") and \
  (Time >= \"%s\" and Time <= \"%s\") order by Value ASC limit 1;", PSU1_SENSOR_ID, PSU2_SENSOR_ID, start_point.c_str(), end_point.c_str());
  sqlite3_exec(db, query, get_peak_power_value_callback, &jv_result, &err_msg);

  // log(error) << " QUERY : " << query;
  // log(error) << " AFTER JV_RESULT LOW >>>> ";
  // log(error) << jv_result;

  sqlite3_free(err_msg);
  sqlite3_close(db);

  // 1 day / 10 min / 1 hour / 6 hour? or delete

  return jv_result;
}

json::value getGraphData(void)
{
  json::value jv_result;
  json::value jv_psu1, jv_psu2;

  jv_psu1["MIN_GRAPH"] = getMinGraphData(PSU1_SENSOR_ID, 20);
  jv_psu1["HOUR_GRAPH"] = getHourGraphData(PSU1_SENSOR_ID, 10);
  jv_psu1["DAY_GRAPH"] = getDayGraphData(PSU1_SENSOR_ID, 7);

  jv_psu2["MIN_GRAPH"] = getMinGraphData(PSU2_SENSOR_ID, 20);
  jv_psu2["HOUR_GRAPH"] = getHourGraphData(PSU2_SENSOR_ID, 10);
  jv_psu2["DAY_GRAPH"] = getDayGraphData(PSU2_SENSOR_ID, 7);

  jv_result["PSU1"] = jv_psu1;
  jv_result["PSU2"] = jv_psu2;

  // 밖에서 GRAPTH_DATA로 연결
  return jv_result;
}

json::value getMinGraphData(string sensorId, int count)
{
  sqlite3 *db;
  char *err_msg=0;
  char query[500] = {0};
  int rc = sqlite3_open(LOG_DB, &db);

  json::value jv_result;
  vector<json::value> v_item;
  sprintf(query, "SELECT * FROM Reading WHERE SensorID=\"%s\" order by Time DESC limit %d;", sensorId.c_str(), count);
  sqlite3_exec(db, query, get_power_value_graph_callback, &v_item, &err_msg);


  jv_result = json::value::array(v_item);

  // log(error) << " [GET MIN GRAPH JSON] >>>>>>>>>>>>>>>>>>>>>";
  // log(error) << jv_result;

  return jv_result;

}

json::value getHourGraphData(string sensorId, int count)
{
  sqlite3 *db;
  char *err_msg=0;
  char query[500] = {0};
  int rc = sqlite3_open(LOG_DB, &db);

  json::value jv_result;
  vector<json::value> v_item;

  string compareTime = DB_currentDateTime();
  for(int i=0; i<count; i++)
  {
    sprintf(query, "SELECT * FROM Reading WHERE SensorID=\"%s\" and Time <= \"%s\" order by Time DESC limit 1;", sensorId.c_str(), compareTime.c_str());
    sqlite3_exec(db, query, get_power_value_graph_callback, &v_item, &err_msg);
    // 처음껄 DB에 넣었고 v_item에 들어가겠지

    // 여기서 시간만 뽑아서 그거로부터 1시간 이후! 해서 쿼리 진행
    if(v_item.size() == 0)
      continue;

    string currentTime = v_item[v_item.size()-1].at("DATETIME").as_string();
    compareTime = getChangedTime(currentTime, -3600);

    // log(error) << " [FOR PHRASE] >>>>>> [ " << i << " ]";
    // log(error) << " v_item size : " << v_item.size();
    // log(error) << " currentTime : " << currentTime;
    // log(error) << " compareTime : " << compareTime;

  }


  jv_result = json::value::array(v_item);

  // log(error) << " [GET HOUR GRAPH JSON] >>>>>>>>>>>>>>>>>>>>>";
  // log(error) << jv_result;

  return jv_result;
}

json::value getDayGraphData(string sensorId, int count)
{
  sqlite3 *db;
  char *err_msg=0;
  char query[500] = {0};
  int rc = sqlite3_open(LOG_DB, &db);

  json::value jv_result;
  vector<json::value> v_item;

  string compareTime = DB_currentDateTime();
  for(int i=0; i<count; i++)
  {
    sprintf(query, "SELECT * FROM Reading WHERE SensorID=\"%s\" and Time <= \"%s\" order by Time DESC limit 1;", sensorId.c_str(), compareTime.c_str());
    sqlite3_exec(db, query, get_power_value_graph_callback, &v_item, &err_msg);
    // 처음껄 DB에 넣었고 v_item에 들어가겠지

    // 여기서 시간만 뽑아서 그거로부터 1시간 이후! 해서 쿼리 진행
    if(v_item.size() == 0)
      continue;

    string currentTime = v_item[v_item.size()-1].at("DATETIME").as_string();
    compareTime = getChangedTime(currentTime, -86400);

    // log(error) << " [FOR PHRASE] >>>>>> [ " << i << " ]";
    // log(error) << " v_item size : " << v_item.size();
    // log(error) << " currentTime : " << currentTime;
    // log(error) << " compareTime : " << compareTime;

  }


  jv_result = json::value::array(v_item);

  // log(error) << " [GET HOUR GRAPH JSON] >>>>>>>>>>>>>>>>>>>>>";
  // log(error) << jv_result;

  return jv_result;



}


string getChangedTime(string currentTime, int seconds)
{
  time_t init_time;
  struct tm *tm_cur;
  time(&init_time);
  tm_cur = localtime(&init_time);

  strptime(currentTime.c_str(), "%F %T", tm_cur);

  // 시간 tm_cur-> 멤버에 계산하는걸로 테스트하고 되면 enum flag랑 switch로 할것
  
  time_t t1 = mktime(tm_cur);
  // t1 -= 3600;
  t1 = t1 + seconds;

  struct tm *tm_pre;
  tm_pre = localtime(&t1);

  char outout[30];
  make_time_with_tm(*tm_pre, outout);
  string str_out(outout);

  return str_out;
}

int get_last_time_callback(void *_string, int argc, char **argv, char **azColName)
{
  for(int i=0; i<argc; i++)
  {
    if(argv[i] == NULL)
      continue;

    string val = argv[i];
    string col = azColName[i];

    if(col == "Time")
      *((string *)_string) = val;
  }
  
  return 0;
}

int get_total_power_value_callback(void *_double, int argc, char **argv, char **azColName)
{
  for(int i=0; i<argc; i++)
  {
    if(argv[i] == NULL)
      continue;

    string val = argv[i];
    string col = azColName[i];

    if(col == "Value")
      *((double *)_double) += improved_stof(val);
  }
  
  return 0;
  
}

int get_peak_power_value_callback(void *_json, int argc, char **argv, char **azColName)
{
  json::value jv_tmp;

  for(int i=0; i<argc; i++)
  {
    if(argv[i] == NULL)
      continue;

    string val = argv[i];
    string col = azColName[i];

    if(col == "SensorID")
      jv_tmp["SENSOR"] = json::value::string(val);
    else if(col == "Value")
      jv_tmp["VALUE"] = json::value::number(improved_stof(val));
    else if(col == "Time")
      jv_tmp["DATETIME"] = json::value::string(val);
  }

  *((json::value *)_json) = jv_tmp;
  
  return 0;
}

int get_power_value_graph_callback(void *_vector, int argc, char **argv, char **azColName)
{
    json::value jv;

    for(int i=0; i<argc; i++)
    {
      if(argv[i] == NULL)
        continue;

      string col = azColName[i];
      string val = argv[i];
      if(col == "Value")
      {
          jv["WATT"] = json::value::number(stof(val));
      }
      else if(col == "Time")
      {
          jv["DATETIME"] = json::value::string(val);
      }
    }

    jv["ID"] = json::value::string(to_string(((vector<json::value> *)_vector)->size()));
    ((vector<json::value> *)_vector)->push_back(jv);
    // 시간 역순으로 데이터가 들어가니깐 웹에선 ID 역순으로 정렬하거나 시간 정순으로 다시 정렬해야함

    return 0;
}


void generate_log(string _position, string _facility, Event_Info _ev)
{
  // 파라미터 필요한게
  // #1 로그서비스 위에 chassis , system, manager 어디에 들어갈지
  // #2 /-/LogServices/[HERE] 에 어느 facility로 들어갈지
  // #3 그다음에는 Event_Info 나 SEL 필요함
  string odata;
  if(_position == "Chassis")
    odata = ODATA_CHASSIS_ID;
  else if(_position == "System")
    odata = ODATA_SYSTEM_ID;
  else if(_position == "Manager")
    odata = ODATA_MANAGER_ID;
  
//   odata = odata + "/" + CMM_ID;
  odata = odata + "/LogServices/";
  odata = odata + _facility;

  if(!record_is_exist(odata))
  {
    // 로그서비스가 없다면 생성
    LogService *service = new LogService(odata, _facility);

    // TODO 로그서비스 멤버변수 값 채워넣는 정보를 어디서 얻는지 처리 필요
    // 현재는 임시값으로 넣음 // 이거 facility에 따라서 디폴트값 다르게 설정한다든가 하는방식도 있겠네
    service->max_number_of_records = 1000;
    service->log_entry_type = "Multiple";
    // service->overwrite_policy = "WrapsWhenFull";
    service->overwrite_policy = "NeverOverWrites";
    service->service_enabled = true;

    string col_odata = get_parent_object_uri(odata, "/");
    Collection *col = (Collection *)g_record[col_odata];

    col->add_member(service);

    resource_save_json(service); // logservice
    resource_save_json(col); // logservice collection
  }

  LogService *service = (LogService *)g_record[odata];
  string entry_odata = service->odata.id + "/Entries";

  if(!record_is_exist(entry_odata))
  {
    service->entry = new Collection(service->odata.id + "/Entries", ODATA_LOG_ENTRY_COLLECTION_TYPE);
    service->entry->name = "Log Entry Collection";
    resource_save_json(service->entry); // entry collection
  }
  
  Collection *en_col = (Collection *)g_record[entry_odata];

  // max record검사
  if(service->record_count >= service->max_number_of_records)
  {
    if(service->overwrite_policy == "WrapsWhenFull")
    {
      // 덮어쓰기
      service->record_count = 0;
    }
    else if(service->overwrite_policy == "NeverOverWrites")
    {
      // 로그 엔트리 소각
      log(warning) << "LogService Overwrite Policy is NeverOverWrites : Discard Log Entry";
      return ;
    }
    else
    {
      // Unknown이 있는데 안쓰일듯
      return ;
    }
  }
  unsigned int num = service->record_count + 1;
  entry_odata = entry_odata + "/" + to_string(num);
  service->record_count++;

  LogEntry *entry;
  entry = make_log(entry_odata, to_string(num), _ev);
  en_col->add_member(entry);

  resource_save_json(entry);
  resource_save_json(en_col);


  // BMC의 로그엔트리 CMM반영 >> GET요청 보내면 반영?

}

void generate_log(string _position, string _facility, SEL _sel)
{
  string odata;
  if(_position == "Chassis")
    odata = ODATA_CHASSIS_ID;
  else if(_position == "System")
    odata = ODATA_SYSTEM_ID;
  else if(_position == "Manager")
    odata = ODATA_MANAGER_ID;
  
//   odata = odata + "/" + CMM_ID;
  odata = odata + "/LogServices/";
  odata = odata + _facility;

  if(!record_is_exist(odata))
  {
    // 로그서비스가 없다면 생성
    LogService *service = new LogService(odata, _facility);

    // TODO 로그서비스 멤버변수 값 채워넣는 정보를 어디서 얻는지 처리 필요
    // 현재는 임시값으로 넣음
    service->max_number_of_records = 1000;
    service->log_entry_type = "Multiple";
    service->overwrite_policy = "WrapsWhenFull";
    service->service_enabled = true;

    string col_odata = get_parent_object_uri(odata, "/");
    Collection *col = (Collection *)g_record[col_odata];

    col->add_member(service);

    resource_save_json(service); // logservice
    resource_save_json(col); // logservice collection
  }

  LogService *service = (LogService *)g_record[odata];
  string entry_odata = service->odata.id + "/Entries";

  if(!record_is_exist(entry_odata))
  {
    service->entry = new Collection(service->odata.id + "/Entries", ODATA_LOG_ENTRY_COLLECTION_TYPE);
    service->entry->name = "Log Entry Collection";
    resource_save_json(service->entry); // entry collection
  }


  Collection *en_col = (Collection *)g_record[entry_odata];
  
  // max record검사
  if(service->record_count >= service->max_number_of_records)
  {
    if(service->overwrite_policy == "WrapsWhenFull")
    {
      // 덮어쓰기
      service->record_count = 0;
    }
    else if(service->overwrite_policy == "NeverOverWrites")
    {
      // 로그 엔트리 소각
      log(warning) << "LogService Overwrite Policy is NeverOverWrites : Discard Log Entry";
      return ;
    }
    else
    {
      // Unknown이 있는데 안쓰일듯
      return ;
    }
  }

  unsigned int num = service->record_count + 1;
  entry_odata = entry_odata + "/" + to_string(num);
  service->record_count++;

  LogEntry *entry;
  entry = make_log(entry_odata, to_string(num), _sel);
  en_col->add_member(entry);

  resource_save_json(entry);
  resource_save_json(en_col);

}


// 위에 event_info랑 sel 별도 호출해서 ev or sel 만들어놓고 로그서비스연결하는 함수호출
// cha, sys, man 어딘지 받고 퍼실리티 받고 해서 해당 uri의 로그서비스 생성 하고 이제 로그엔트리를 만드는데
// 그 안에서 만들지 그러면 odata같은건 바로들어가지만
// 로그서비스만드는건 어떻게 되는거죠?

// 로직이 어떻게 되냐면 이벤트&로그 발생하는곳에서
// #1 특정 값들 받아서 event_info, SEL 로 만들어주는거 함수하나 필요함 그거 호출해서 ev랑 sel만들고
// #2 로그서비스 만드는 함수 호출해서 여기에서 서비스랑 연결or생성
// #3 ev, sel 가지고 로그엔트리 생성하는함수를 호출 그러면 비로소 로그생김
// ##1사실 이건 로그만이고 별개로 이벤트 구독자에게 보내는 작업 필요한데 일단 로그부터하겟음
// ##2그리고 db에도 저장해야함



LogEntry* make_log(string odata_id, string id, Event_Info ev)
{
    LogEntry *le = new LogEntry(odata_id, id);
    
    le->entry_type = "Event";
    le->severity = ev.message.severity;
    le->message = ev.message;

    le->event_id = ev.event_id;
    le->event_timestamp = ev.event_timestamp;
    le->event_type = ev.event_type;

    return le;
}

LogEntry* make_log(string odata_id, string id, SEL sel)
{
    LogEntry *le = new LogEntry(odata_id, id);
    
    le->entry_type = "SEL";
    le->severity = sel.message.severity;
    le->message = sel.message;

    le->sensor_number = sel.sensor_number;
    le->sensor_type = sel.sensor_type;
    le->entry_code = sel.entry_code;

    le->event_timestamp = sel.event_timestamp;
    le->event_type = sel.event_type;

    return le;
}

void log_operation(http_request _request)
{
    // cors 해줘야함
    // reading 부분 , Event 부분 나눠야함
    string uri = _request.request_uri().to_string();
    vector<string> uri_tokens = string_split(uri, '/');

    http_response response;
    json::value response_json;

    response.headers().add("Access-Control-Allow-Origin", "*");
    response.headers().add("Access-Control-Allow-Credentials", "true");
    response.headers().add("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PATCH");
    response.headers().add("Access-Control-Max-Age", "3600");
    response.headers().add("Access-Control-Allow-Headers", "Content-Type, Accept, X-Requested-With, remember-me, X-Auth-Token");
    response.headers().add("Access-Control-Expose-Headers", "X-Auth-Token, Location");

    if(uri_tokens[1] == "reading")
    {
        reading_operation(_request, response);
    }
    // else if(uri_tokens[1] == "event")
    // {
    //     event_operation(_request, response);
    // } // 사용x
    else
    {
        json::value j = get_error_json("Not Supported URI");
        response.set_status_code(status_codes::BadRequest);
        response.set_body(j);
    }

    _request.reply(response);
    return ;
}

void reading_operation(http_request _request, http_response &_response)
{
    // DB Reading Table관련 기능 수행하는 핸들러처리
    // #1 uri 설계..
    // /log/reading까지 들어오고 /log/reading/[module]/[type]/[detail]/[time_option]
    // [module] : 모듈id,, 어느 모듈의 센서측정값들인지 // BMC는 자기 모듈꺼만 받는다
    // [type] : 센서의 위치타입,, thermal / power 등등...
    // [detail] : 해당 센서타입에서의 상세구분정보 ex> 타입power일때 powercontrol, powersupply 같은 정보
    // [time_option] : 시간조건,, 시간조건없이 최신 x개 긁어오기 / 최근1시간이내 / 최근30분이내 등등...
    // time_option 변경점 : 센서가 1분단위씩으로 주기적으로 측정됨.. 그래서 min으로 주면 분단위로 30분(즉 30개)
    // hour가 시간단위로 30시간(30개) 
    web::uri uri = _request.request_uri();
    string uri_string = uri.to_string();
    vector<string> uri_tokens = uri::split_path(uri_string);

    cout << "READING OPERATION" << endl;
    cout << "Request URI : " << uri_string << endl;
    // cout << "URI Tokens ---" << endl;
    // for(int i=0; i<uri_tokens.size(); i++)
    // {
    //     cout << uri_tokens[i] << endl;
    // }

    if(uri_tokens.size() < 6)
    {
        _response.set_status_code(status_codes::BadRequest);
        _response.set_body(get_error_json("URI Format >> /log/reading/[module]/[type]/[detail]/[time_option]"));
        return ;
    }

    string param_module, param_type, param_detail, param_time_option;

    
    // [module] 처리 // BMC는 자기 자신 모듈이름인지 체크
    // if(module_id_table.find(uri_tokens[2]) == module_id_table.end())
    // 여기를 이제 모듈id가 유동적으로 바뀌기때문에 체크 몬함
    // if(uri_tokens[2] != MODULE_NAME)
    // {
    //     _response.set_status_code(status_codes::BadRequest);
    //     _response.set_body(get_error_json("Module ID [" + uri_tokens[2] + "] does not Exist"));
    //     return ;
    // }
    param_module = uri_tokens[2];
    

    // [type] 처리
    if(!check_reading_type(uri_tokens[3]))
    {
        _response.set_status_code(status_codes::BadRequest);
        _response.set_body(get_error_json("Reading Type [" + uri_tokens[3] + "] is Incorrect"));
        return ;
    }
    param_type = uri_tokens[3];
    
    // [detail] 처리
    if(!check_reading_detail(param_type, uri_tokens[4]))
    {
        _response.set_status_code(status_codes::BadRequest);
        _response.set_body(get_error_json("Reading Detail [" + uri_tokens[4] + "] is Incorrect"));
        return ;
    }
    param_detail = uri_tokens[4];

    // [time_option] 처리
    if(!check_reading_time_option(uri_tokens[5]))
    {
        _response.set_status_code(status_codes::BadRequest);
        _response.set_body(get_error_json("Time Option [" + uri_tokens[5] + "] is Incorrect"));
        return ;
    }
    param_time_option = uri_tokens[5];
    

    // cout << param_module << " / " << param_type << " / " << param_detail << " / " << param_time_option << endl;

    // 만들어진 param가지고 함수 호출
    json::value result_jv;
    if(param_time_option == "min")
        result_jv = select_min_reading(param_module, param_type, param_detail, 30);
    else if(param_time_option == "hour")
        result_jv = select_hour_reading(param_module, param_type, param_detail, 30);
    // result_jv = select_all_reading()

    
    _response.set_status_code(status_codes::OK);
    _response.set_body(result_jv);
    return ;
    
}

void event_operation(http_request _request, http_response &_response)
{

}

bool check_reading_type(string _types)
{
    if(_types == "thermal" || _types == "power")
        return true;
    else
        return false;
}
// type하고 detail이 생기면서 type검사에 power, thermal과 같은걸로 넣어주어야하고
// 그 type에 따라서 detail값이 temp,fan // powercontrol powervoltage powersupply 로 나뉨
bool check_reading_detail(string _type, string _detail)
{
    if(_type == "thermal")
    {
        if(_detail == "temperature" || _detail == "fan" || _detail == "smartheater")
            return true;
        
    }
    else if(_type == "power")
    {
        if(_detail == "powercontrol" || _detail == "powervoltage" || _detail == "powersupply")
            return true;
    }

    return false;
}

bool check_reading_time_option(string _option)
{
    // hour: 1시간 간격 30개로그 , min: 1분 간격 30개로그...
    if(_option == "hour" || _option == "min")
        return true;
    else
        return false;
}