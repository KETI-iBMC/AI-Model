#include "ipmi/ipmi.hpp"
//#include <ipmi/channel.hpp>
#include <redfish/resource.hpp>

#define SENSOR_FRU_MAX 1
/**
 * @brief SEL Header 구조체
 *
 */
sel_hdr_t g_sel_hdr;
/**
 * @brief SEL Data 구조체
 *
 */
sel_msg_t g_sel_data[SEL_ELEMS_MAX];

/**
 * @brief Platform Event Filtering Entry Data 구조체
 *
 */
pef_table_entry_t g_eft_data[EVENT_FILTER_TABLE_ELEMS_MAX];

char *Alert_Strings[4];
char uuid_str[37];
int redfish_power_ctrl;
char uuid_hex[16];
uint16_t dcmi_sample_time;
uint16_t min_power, max_power, avg_power;
uuid_t uuid;
/**
 * @brief redfish 모니터를 위한 뮤텍스
 * @details 0= account 1= chassis 2=system 3=manager 4= session
 *
 */
std::mutex redfish_m_mutex[5];
/**
 * @brief redfish 모니터를 위한 뮤텍스조건
 * @details 0= account 1= chassis 2=system 3=manager 4= session
 *
 */
std::condition_variable redfish_m_cond[5];

/**
 * @brief 로그 파일 디스크립터
 *
 */
static FILE *fp_log_file;
/**
 * @brief 로그 파일 prefix 문자열
 *
 */
static char log_file_prefix[64] = "bmc";
/**
 * @brief 로그 디렉토리 경로
 *
 */
static char log_folder[1024] = "/var/log";
/**
 * @brief 로그 Trace Level
 *
 */
static int log_level = LOG_LVL_TRACE;

void time_stamp_fill(uint8_t *ts) {
  std::time_t lt_int = std::time(nullptr);

  ts[3] = (lt_int & 0xff000000) >> 24;
  ts[2] = (lt_int & 0xff0000) >> 16;
  ts[1] = (lt_int & 0xff00) >> 8;
  ts[0] = (lt_int & 0xff);
}

int LOGsetInfo(const char *dir, const char *prefix) {
  if (dir == NULL || dir[0] == 0x00) {
    fprintf(stderr, "log directory set error.\n");
    return -1;
  }
  if (prefix == NULL || prefix[0] == 0x00) {
    fprintf(stderr, "log file prefix set error.\n");
    return -1;
  }
  if (strcmp(dir, log_folder) == 0 && strcmp(prefix, log_file_prefix) == 0) {
    return 0;
  }

  strncpy(log_file_prefix, prefix, 64);
  strncpy(log_folder, dir, 1024);

  if (fp_log_file != NULL) {
    fclose(fp_log_file);
    fp_log_file = NULL;
  }
  return 0;
}
int LogSetLevel(int log_lvl) {
  int tmp = LogGetLevel();

  log_level = log_lvl;

  return tmp;
}
int LogGetLevel(void) {
  char *log_env;
  static int is_env_check = 0;

  if (is_env_check == 0) {
    if ((log_env = getenv("LOG_LEVEL")) == NULL) {
      log_level = LOG_LVL_DISABLE;
    } else {
      if (strcmp(log_env, "TRACE") == 0) {
        log_level = LOG_LVL_TRACE;
      } else if (strcmp(log_env, "INFO") == 0) {
        log_level = LOG_LVL_INFO;
      } else if (strcmp(log_env, "WARNING") == 0) {
        log_level = LOG_LVL_WARNING;
      } else if (strcmp(log_env, "ALL") == 0) {
        log_level = LOG_LVL_ALL_INFO;
      } else {
        log_level = LOG_LVL_DISABLE;
      }
    }
    is_env_check = 1;
  }
  return log_level;
}

static int LOGcreateFile(struct tm *tml, const char *src_file) {
  char filename[1024];
  char *ext;

  if (log_folder[0] == 0x00) {
    strcpy(log_folder, ".");
  }
  if (log_file_prefix[0] == 0x00) {
    strncpy(log_file_prefix, src_file, sizeof(log_file_prefix));
    if ((ext = strchr(log_file_prefix, '.')) != NULL) {
      *ext = 0x00;
    }
  }
  snprintf(filename, 1024, "%s/%s-%04d%02d%02d.log", log_folder,
           log_file_prefix, 1900 + tml->tm_year, tml->tm_mon + 1, tml->tm_mday);

  if (fp_log_file != NULL) {
    fclose(fp_log_file);
    fp_log_file = NULL;
  }
  if ((fp_log_file = fopen(filename, "a")) == NULL) {
    return -1;
  }
  setvbuf(fp_log_file, NULL, _IOLBF, 0);
  return 0;
}

int LOGlogging(char log_type, const char *src_file, const char *func,
               int line_no, const char *fmt, ...) {
  va_list ap;
  int sz = 0;
  struct timeval tv;
  struct tm *tml;
  static int day = -1;
  static pid_t pid = -1;
  char src_info[128];

  gettimeofday(&tv, NULL);
  tml = localtime(&tv.tv_sec);
  va_start(ap, fmt);

  if (pid == -1) {
    pid = getpid();
  }

  if (day != tml->tm_mday) {
    if (LOGcreateFile(tml, src_file) != 0) {
      return -1;
    }
    day = tml->tm_mday;
  }

  sz += fprintf(fp_log_file, "(%c) ", log_type);
  sz += fprintf(fp_log_file, "[%04d-%02d-%02d %02d:%02d:%02d]",
                1900 + tml->tm_year, tml->tm_mon + 1, tml->tm_mday,
                tml->tm_hour, tml->tm_min, tml->tm_sec);
  snprintf(src_info, 128, "%s:%s(%d)", src_file, func, line_no);
  sz += fprintf(fp_log_file, ":%-50.50s: ", src_info);
  sz += vfprintf(fp_log_file, fmt, ap);
  sz += fprintf(fp_log_file, "\n");
  va_end(ap);

  return sz;
}
void test_ipmi(void) { printf("tset ipmi \n\n\n\n\n"); }
void delay(int millisec) {
  long pause;
  clock_t now, then;

  pause = millisec * (CLOCKS_PER_SEC / 1000);
  now = then = clock();
  while ((now - then) < pause)
    now = clock();
}
#define PEF_FILTER_ENTRY_COUNT 2
#define PEF_POLICY_ENTRY_COUNT 1

pef_table_entry_t pef_filter_table[PEF_FILTER_ENTRY_COUNT] = {
    {
        PEF_FILTER_DISABLED,      // data1
        PEF_CONFIG_PRECONFIGURED, // uint8_t config;
        PEF_ACTION_POWER_DOWN,    //	uint8_t action;
        0x0,                      // uint8_t policy_number;
        PEF_SEVERITY_CRITICAL,    // uint8_t severity;
        0x20,                     // uint8_t generator_ID_addr;
        0x00,                     // uint8_t generator_ID_lun;
        0x01,                // SENSOR_TYPE_TEMPERATURE,	// uint8_t sensor_type;
        PEB_SENSOR_NUM_BASE, // PECI_SENSOR_NUM_BASE -> EVENT_SENSOR_NUM_BASE
                             // uint8_t sensor_number;
        PEF_EVENT_TRIGGER_THRESHOLD, // uint8_t event_trigger;
        {0x00, 0x00},                // uint8_t event_data_1_offset_mask[2];
        0x00,                        // uint8_t event_data_1_AND_mask;
        0x00,                        // uint8_t event_data_1_compare_1;
        0x00,                        // uint8_t event_data_1_compare_2;
        0x00,                        // uint8_t event_data_2_AND_mask;
        0x00,                        // uint8_t event_data_2_compare_1;
        0x00,                        // uint8_t event_data_2_compare_2;
        0x00,                        // uint8_t event_data_3_AND_mask;
        0x00,                        // uint8_t event_data_3_compare_1;
        0x00                         // uint8_t event_data_3_compare_2;
    },
    {
        PEF_FILTER_DISABLED,      // data1
        PEF_CONFIG_PRECONFIGURED, // uint8_t config;
        PEF_ACTION_POWER_DOWN,    //	uint8_t action;
        0x0,                      // uint8_t policy_number;
        PEF_SEVERITY_CRITICAL,    // uint8_t severity;
        0x20,                     // uint8_t generator_ID_addr;
        0x00,                     // uint8_t generator_ID_lun;
        0x01, // SENSOR_TYPE_TEMPERATURE,	// uint8_t sensor_type;
        PEB_SENSOR_NUM_BASE +
            1, // PECI_SENSOR_NUM_BASE -> EVENT_SENSOR_NUM_BASE  uint8_t
               // sensor_number;
        PEF_EVENT_TRIGGER_THRESHOLD, // uint8_t event_trigger;
        {0x00, 0x00},                // uint8_t event_data_1_offset_mask[2];
        0x00,                        // uint8_t event_data_1_AND_mask;
        0x00,                        // uint8_t event_data_1_compare_1;
        0x00,                        // uint8_t event_data_1_compare_2;
        0x00,                        // uint8_t event_data_2_AND_mask;
        0x00,                        // uint8_t event_data_2_compare_1;
        0x00,                        // uint8_t event_data_2_compare_2;
        0x00,                        // uint8_t event_data_3_AND_mask;
        0x00,                        // uint8_t event_data_3_compare_1;
        0x00                         // uint8_t event_data_3_compare_2;
    }};

/* plat_sel_last_rsv_id
 * Param: unsigned char *data(바로 response data에 작성하기 위한 인자)
 * Description : 다음 index를 response의 data에 저장함. 마지막 index는 0xff로
 * 마지막임을 알려줌
 */
void plat_sel_last_rsv_id(unsigned char *data) {
  unsigned int src;
  unsigned char *tmp = data;

  if (plat_sel_num_entries() == 0) {
    printf("SEL Entry is zero\n");
    *data++ = 0xff;
    *data = 0xff;
  } else {
    src = (unsigned int)g_sel_hdr.end;
    *data++ = src & 0xff;
    *data = (unsigned char)(src >> 8);
  }
  return;
}
/* plat_sel_num_entries
 * Description : 현재 저장된 sel의 갯수. roll over 되지 않게 해놨으므로 항상 end
 * - begin
 */
int plat_sel_num_entries(void) {
  int rst;
  if (g_sel_hdr.begin <= g_sel_hdr.end) {
    return (g_sel_hdr.end - g_sel_hdr.begin);
  } else {
    return (g_sel_hdr.end + (SEL_INDEX_MAX - g_sel_hdr.begin + 1));
  }
}

uint8_t plat_lan_channel_selector(uint8_t net_priority) {
  uint8_t chan = 0;
  switch (net_priority) {
  case 1:
    chan = 0;
    break;
  case 6:
    chan = 1;
    break;
  case 7:
    chan = 2;
    break;
  case 8:
    chan = 3;
    break;
  default:
    chan = 0;
    break;
  }
  return chan;
}

uint8_t parse_policy_set(uint8_t policy) {
  uint8_t policy_set;
  if (policy == 0x00) {

    return 0;
  }
  policy_set = policy / 0x10;

  return policy_set;
}
/**
 * @brief global enable redfish 저장
 */
void plat_globalenable_save(void) {
  cout << "plat_globalenable_save " << endl;
  string _uri = ODATA_MANAGER_ID;
  if (record_is_exist(_uri)) {
    Manager *manager = (Manager *)g_record[_uri];
    resource_save_json(manager);
  }
}
