#include "ipmi/sel.hpp"
#include "ipmi/network.hpp"
extern sel_msg_t g_sel_data[SEL_ELEMS_MAX];
extern sel_hdr_t g_sel_hdr;
extern int find_sensor_name(uint8_t sType, uint8_t sNum, string &msg);

//14:29


/**
 * @brief system time 변경 함수
 *
 * @param ts 1970.01.01 부터의 시간초를 입력
 * @return int 성공 0 실패 -1
 * @bug kernel 5.1 -> 5.4로 변경하면서 stime 함수가 사라짐(커널단에서 삭제)
 */
int time_stamp_set(unsigned char *ts)
{
    struct timeval tv;
    unsigned int time;
    int ret;

    memcpy(&time, ts, 4);
    tv.tv_sec = time;

    //** 수정코드
    // NULL 오류시 gettimeofday 함수를 통해얻어와야함
    //stime
    try {
        
      ret = settimeofday(&tv, NULL);
    } catch (const std::exception &a) {
      return -1;
    }
    if (ret == -1) {
      return -1;
    }
    return 0;
}
/// @brief SEL Header magic number
#define SEL_HDR_MAGIC 0xFBFBFBFB

/// @brief SEL Header version number
#define SEL_HDR_VERSION 0x01

/// @brief SEL Data offset from file beginning
#define SEL_DATA_OFFSET 0x100

/// @brief SEL reservation IDs can not be 0x00 or 0xFFFF
#define SEL_RSVID_MIN 0x01
#define SEL_RSVID_MAX 0xFFFE

/// @brief Number of SEL records before wrap

/// @brief Index for circular array
#define SEL_INDEX_MIN 0x00

/// @brief Special RecID value for first and last (IPMI/Section 31)
#define SEL_RECID_FIRST 0x0000
#define SEL_RECID_LAST 0xFFFF

/// @brief Keep track of last Reservation ID
int g_rsv_id = 0x01;

/// @brief Cached version of SEL Header and data

/**
 * @brief 처음&마지막 index, timestamp 등이 저장된 g_sel_hdr을 /var/sel.bin의 0번지로부터 불러오는 함수
 * @param void
 * @return 성공  시 0 / 실패 시 -1을 반환
 */
int file_get_sel_hdr(void);
/**
 * @brief vent 내용이 저장된 g_sel_data을 /var/sel.bin의 SEL_DATA_OFFSET(0x100)번지로부터 불러오는 함수\n
 * @param void
 * @return 성공  시 0 / 실패 시 -1을 반환
 */
int file_get_sel_data(void);
/**
 * @brief 처음&마지막 index, timestamp 등이 저장된 g_sel_hdr을 IPMI_SEL_PATH에 저장하는 함수\n
 * @param void
 * @return 성공  시 0 / 실패 시 -1을 반환
 */
int file_store_sel_hdr(void);
/**
 * @brief sel list에서 출력될 index(가장 왼쪽 column)를 g_sel_data(data->msg)에 붙이는 함수\n
 * add 시에 호출되며 delete 시에 index 정리를 위해 다시 호출됨\n
 * @param int recId - sel의 index가 될 번호
 * @param sel_msg_t *data - IPMI Request 메시지로 부터 받은 g_sel_data 를 포함한 데이터
 * @return 성공  시 0을 반환
 */
int plat_sel_add_entry_record_id(int recId, sel_msg_t *data);
/**
 * @brief  add sel 시에, IPMI_SEL_PATH의 맨 뒤에 sel 하나를 저장하는 함수
 * @param int recId - sel의 index가 될 번호
 * @param sel_msg_t *data - IPMI Request 메시지로 부터 받은 g_sel_data 를 포함한 데이터
 * @return 성공  시 0을 반환
 */
int file_store_sel_data(int recId, sel_msg_t *data);

/**
 * @brief plat_sel_num_entries()로 구한 sel 갯수로부터 sel free space를 계산
 * @param void
 * @return (Total Space - Used Space)
 */
int plat_sel_free_space(void);
/**
 * @brief SEL Reservation ID를 1만큼 증가시키며, SEL_RSVID_MAX값에 도달한 경우 SEL_RSVID_MIN으로 설정하는 함수.
 * @param void
 * @return g_rsv_id
 */
int plat_sel_rsv_id();
/**
 * @brief 주어진 Record ID를 사용하여 SEL Entry 정보를 반환하는 함수.\n
 * 주어진 Record ID가 SEL 헤더 엔트리 최대 / 최소 값인지 확인 후 범위 내의 값인 경우 해당 Index를 사용하여\n
 * SEL Entry 정보를 추출하여 반환함.
 * @param int read_rec_id - 탐색할 Record ID
 * @param sel_msg_t *msg - 반환받을 SEL Entry 포인터
 * @param int *next_rec_id - 다음에 탐색할 Record ID
 * @return 성공시 0 / 실패시 -1을 반환
 */
int plat_sel_get_entry(int read_rec_id, sel_msg_t *msg, int *next_rec_id);
/**
 * @brief SEL Record가 가득 찬 경우 마지막 Entry에 가득 찼음을 알리는 로그를 저장하는 함수.
 * @param void
 * @return void
 */
int add_to_sel_full();
/**
 * @brief 주어진 SEL Record ID를 사용하여 SEL Entry에 로그를 저장하는 함수.
 * @param sel_msg_t *msg - 저장할 System Event Log 포인터
 * @param int *rec_id - 저장할 SEL Record ID
 * @return 성공시 0 / 실패시 -1을 반환
 */
int plat_sel_add_entry(sel_msg_t *msg, int *rec_id);
/**
 * @brief SEL 데이터를 파일에 저장하는 함수\n
 * g_sel_data 구조체에 포함된 모든 데이터를 IPMI_SEL_PATH 파일에 저장함.\n
 * 파일이 존재할 경우 SEL_DATA_OFFSET 만큼 커서를 이동시켜 저장함.\n
 * @param void
 * @return void
 */
int file_store_all_sel_data(void);
/**
 * @brief 주어진 Record ID를 사용하여 SEL Record를 제거하는 함수.\n
 * 주어진 Record ID의 SEL Record를 삭제한 뒤 이후 Record를 이동시켜 SEL Record를 재배열함.\n
 * 재배열 후 SEL 파일에 g_sel_data를 저장함.
 * @param int id - 제거할 SEL Record ID
 * @return 성공시 0 / 실패시 -1을 반환
 */
int delete_sel_entry(int id);
/**
 * @brief SEL 전체 Record를 제거하는 함수\n
 * SEL Record를 제거한 후 첫 번째 Record에 SEL Erase 로그를 남김.\n
 * @param rsv_id - SEL Record ID
 * @return 성공시 0 / 실패시 -1을 반환
 */
int plat_sel_erase(int rsv_id);
/**
 * @brief SEL Record 제거가 완료된 경우 SEL Erase Status를 SEL_ERASE_DONE으로 변경하는 함수\n
 * @param void
 * @return 성공시 0 / 실패시 -1을 반환
 */
int plat_sel_erase_status(int rsv_id, sel_erase_stat_t *status);
/**
 * @brief 주어진 SEL Record 의 Timestamp를 현재 시간으로 설정하는 함수\n
 * @param sel_msg_t *msg - 현재 시간을 설정할 SEL Record
 * @return 성공시 0을 반환
 */
int plat_sel_time_get(sel_msg_t *msg);
/**
 * @brief 주어진 SEL Record 의 Timestamp를 사용하여 시스템 시간의 Timestamp를 설정하는 함수\n
 * @param sel_msg_t *msg - 설정할 SEL Record
 * @return 성공시 0 / 실패시 -1 을 반환
 */
int plat_sel_time_set(sel_msg_t *msg);
/**
 * @brief IPMI_SEL_PATH파일이 존재할 경우 파일에서 SEL Record Header와 Data를 불러와 g_sel_hdr, g_sel_data에 초기화하는 함수\n
 * IPMI_SEL_PATH 파일이 존재하지 않는 경우 파일을 생성하고 기본 상태로 g_sel_hdr와 g_sel_data를 초기화함.
 * @param void
 * @return 성공시 0 / 실패시 -1 을 반환
 */
int plat_sel_init(void);
/**
 * @brief BMC에 저장된 SEL 정보를 반환하는 함수\n
 * 최근 추가된 SEL Record 시간, 최근 삭제된 SEL Record 시간, 여유 SEL 공간, 현재 SEL Record 개수 등 전반적인 SEL에 대한 정보를 제공함.
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_get_sel_info(ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief SEL Reservation ID를 반환하는 함수\n
 * plat_sel_rsv_id()함수를 호출하여 rsv_id를 받아와 IPMI Response Message로 전송함.\n
 * IPMI Response Data Format\n
 * [0:1] - Reservation ID\n
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_rsv_sel(ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief 요청한 SEL Record ID 로 SEL 데이터를 반환하는 함수\n
 * 요청한 Record ID를 사용하여 plat_sel_get_entry()함수의 인자로 사용, SEL Record 데이터를 반환한다.\n
 * IPMI Request Data Format\n
 * [2:3] Record ID\n
 * IPMI Response Data Format\n
 * [0:n] SEL Record Data\n
 * @param ipmi_req_t *request - IPMI Request Message Pointer
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_get_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief ipmitool로 전송된 데이터를 SEL Record에 추가하는 함수\n
 * 전송된 데이터를 SEL Record에 추가한 뒤 추가된 현재 Record ID를 Response Data로 전송함.\n
 * IPMI Request Data Format\n
 * [0:n] SEL Record Data
 * IPMI Response Data Format
 * [0:1] 현재 Record ID
 * @param ipmi_req_t *request - IPMI Request Message Pointer
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_add_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/* @brief 전송된 Record ID를 사용하여 SEL Record를 제거함\n
 * ipmitool로 전송된 SEL Record ID를 delete_sel_entry()함수의 인자로 사용하여 SEL Record를 제거함\n
 * IPMI Request Data Format\n
 * [2] SEL Record ID\n
 * @param ipmi_req_t *request - IPMI Request Message Pointer
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_del_sel_entry(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief ipmitool로 전송된 데이터가 CLR인 경우 SEL Record를 초기화 하는 함수\n
 * Reservation ID를 plat_sel_erase()함수의 인자로 사용하여 SEL Record를 초기화 함.\n
 * Request Data[5]가 IPMI_SEL_INIT_ERASE 인 경우 SEL Record를 초기화 함.\n
 * Request Data[5]가 IPMI_SEL_ERASE_STAT인 경우 plat_sel_erase_status() 함수를 호출하여 Erase Status를 Done으로 설정\n
 * IPMI Request Data Format\n
 * [2] - 'C'\n
 * [3] - 'L'\n
 * [4] - 'R'\n
 * [5] - SEL Erase Status\n
 * @param ipmi_req_t *request - IPMI Request Message Pointer
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_clr_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief 현재 시스템 시간의 Timestamp를 반환하는 함수\n
 * IPMI Response Data Format\n
 * [0:n] - 현재 시스템 시간 Timestamp\n
 * @param ipmi_req_t *request - IPMI Request Message Pointer
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_get_time(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief 전송된 데이터를 현재 시스템 시간을 설정하는 함수\n
 * 전송된 데이터로 현재 시스템 Timestamp를 설정함.\n
 * IPMI Request Data Format\n
 * [0:n] - 설정할 Timestamp \n
 * @param ipmi_req_t *request - IPMI Request Message Pointer
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void storage_set_time(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief ipmitool로 전송된 데이터를 SEL Record에 추가하는 함수\n
 * 전송된 데이터를 SEL Record에 추가한 뒤 추가된 현재 Record ID를 Response Data로 전송함.\n
 * IPMI Request Data Format\n
 * [0:n] SEL Record Data
 * IPMI Response Data Format
 * [0:1] 현재 Record ID
 * @param ipmi_req_t *request - IPMI Request Message Pointer
 * @param ipmi_res_t *response - IPMI Response Message Pointer
 * @param unsigned char *res_len - IPMI Response Message Length
 * @return void
 */
void sensor_event_get_num(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief REST 요청을 수신한 경우 raw 파일로 SEL 로그를 저장함.\n
 * @param void
 * @return 성공시 0 / 실패시 -1 을 반환
 */
int rest_sel_write_raw();
/**
 * @brief REST 요청을 수신한 경우 SEL Record를 JSON 문자열로 반환함.\n
 * JSON 문자열은 전체 SEL Record를 배열 형태로 반환함.
 * @param char *ret - JSON 문자열 포인터
 * @return strlen(ret) - JSON 문자열 길이
 */
int rest_get_eventlog_config(char *ret);

void plat_sel_test_add_entry(unsigned char fst, unsigned char second)
{
    printf("Add SEL test entry\n");
    int rec_id;
    unsigned char ts_array[4];
    sel_msg_t entry;

    (time_stamp_fill(ts_array));
    memset(&entry, 0, sizeof(sel_msg_t));

    entry.msg[0] = 0x00;
    entry.msg[1] = 0x00;
    entry.msg[2] = 0x02;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0;
    entry.msg[9] = 0x04;
    entry.msg[10] = fst; // Sensor Type : Power Supply
    entry.msg[11] = PEB_SENSOR_ADC_P12V_PSU1;
    entry.msg[12] = 0x6f;
    entry.msg[13] = second;
    entry.msg[14] = 0x04;
    (plat_sel_add_entry(&entry, &rec_id));
}

void plat_sel_power_reset_add_entry()
{
    printf("Add SEL Entry to Power reset");
    int rec_id;
    unsigned char ts_array[4];
    sel_msg_t entry;

    (time_stamp_fill(ts_array));
    memset(&entry, 0, sizeof(sel_msg_t));

    entry.msg[0] = 0x00;
    entry.msg[1] = 0x00;
    entry.msg[2] = 0x02;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0;
    entry.msg[9] = 0x04;
    entry.msg[10] = 0x23; // Sensor Type : Watchdog
    entry.msg[11] = PEB_SENSOR_ADC_P12V_PSU1;
    entry.msg[12] = 0x6f;
    entry.msg[13] = 0xC1; // Command : Hard Reset
    entry.msg[14] = 0x04;
    (plat_sel_add_entry(&entry, &rec_id));
}
/**
 * @brief sel event power off
 * @details 3-Type offset 0x5 ofset(0x01)
 * event_string.hpp = Digital-Discrete Event States
 */
void plat_sel_power_limit_add_entry()
{

    printf("Add SEL Entry to Power off\n");
    int rec_id;
    unsigned char ts_array[4];
    sel_msg_t entry;

    (time_stamp_fill(ts_array));
    memset(&entry, 0, sizeof(sel_msg_t));

    entry.msg[0] = 0x00;
    entry.msg[1] = 0x00;
    entry.msg[2] = 0x02;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0;
    entry.msg[9] = 0x04;
    entry.msg[10] = 0x09; // Sensor Type : Power Supply
    entry.msg[11] = PEB_SENSOR_ADC_P12V_PSU1;
    entry.msg[12] = 0x05; // Digital-Discrete Event States 12 13 event byte set
    entry.msg[13] = 0x01; // Limit Exceeded
    entry.msg[14] = 0x01; // offset
    (plat_sel_add_entry(&entry, &rec_id));
}
/**
 * @brief sel event os
 * @details type Watchdog1 description BIOS OEM 0x04
 *
 */
void plat_sel_os_off_add_entry()
{
    printf("Add SEL Entry to Power off via OS\n");
    int rec_id;
    unsigned char ts_array[4];
    sel_msg_t entry;

    (time_stamp_fill(ts_array));
    memset(&entry, 0, sizeof(sel_msg_t));

    entry.msg[0] = 0x00;
    entry.msg[1] = 0x00;
    entry.msg[2] = 0x02;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0;
    entry.msg[9] = 0x04;
    entry.msg[10] = 0x11; // Sensor Type : Watchdog 1
    entry.msg[11] = PEB_SENSOR_ADC_P12V_PSU1;
    entry.msg[12] = 0x6f; // 11 /12 없음 따라서 OEM
    entry.msg[13] = 0xC2;
    entry.msg[14] = 0x04; // offset
    (plat_sel_add_entry(&entry, &rec_id));
}

/**
 * @brief
 *
 * @details
 * 3:6 ts_array = time stemp
 * 10 sensor type = ipmi_generic_sensor_type_vals
 * 11 sensor number
 *
 */
void plat_sel_power_off_add_entry()
{

    printf("Add SEL Entry to Power off");
    int rec_id;
    unsigned char ts_array[4];
    sel_msg_t entry;

    (time_stamp_fill(ts_array));
    memset(&entry, 0, sizeof(sel_msg_t));

    entry.msg[0] = 0x00;
    entry.msg[1] = 0x00;
    entry.msg[2] = 0x02;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0;
    entry.msg[9] = 0x04;
    entry.msg[10] = 0x09; // Sensor Type : Power Supply
    entry.msg[11] = PEB_SENSOR_ADC_P12V_PSU1;
    entry.msg[12] = 0x6f; // event type
    entry.msg[13] = 0xC0;
    entry.msg[14] = 0x04;
    (plat_sel_add_entry(&entry, &rec_id));
}
void plat_sel_power_add_entry()
{
    printf("Add SEL Entry to Power supply\n");
    int rec_id;
    unsigned char ts_array[4];
    sel_msg_t entry;

    (time_stamp_fill(ts_array));
    memset(&entry, 0, sizeof(sel_msg_t));

    entry.msg[0] = 0x00;
    entry.msg[1] = 0x00;
    entry.msg[2] = 0x02;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0;
    entry.msg[9] = 0x04;
    entry.msg[10] = 0x08; // Sensor Type : Power Supply
    entry.msg[11] = PEB_SENSOR_ADC_P12V_PSU1;
    entry.msg[12] = 0x6f;
    entry.msg[13] = 0xC3;
    entry.msg[14] = 0x04;
    (plat_sel_add_entry(&entry, &rec_id));
}

/* file_get_sel_hdr
 * Description : 처음&마지막 index, timestamp 등이 저장된 g_sel_hdr을 /var/sel.bin의 0번지로부터 불러오는 함수
 */
int file_get_sel_hdr(void)
{
    FILE *fp;

    fp = fopen(IPMI_SEL_PATH, "r");
    if (fp == NULL)
    {
        return -1;
    }

    if (fread(&g_sel_hdr, sizeof(sel_hdr_t), 1, fp) <= 0)
    {
        cout << ("file_get_sel_hdr: fread\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

/* file_get_sel_data
 * Description : event 내용이 저장된 g_sel_data을 /var/sel.bin의 SEL_DATA_OFFSET(0x100)번지로부터 불러오는 함수
 */
int file_get_sel_data(void)
{
    FILE *fp;

    fp = fopen(IPMI_SEL_PATH, "r");
    if (fp == NULL)
    {
        cout << ("file_get_sel_data: fopen\n");
        return -1;
    }

    if (fseek(fp, SEL_DATA_OFFSET, SEEK_SET) != 0)
    {
        cout << ("file_get_sel_data: fseek\n");
        fclose(fp);
        return -1;
    }

    unsigned char buf[SEL_ELEMS_MAX * 16];
    if (fread(buf, 1, SEL_ELEMS_MAX * sizeof(sel_msg_t), fp) <= 0)
    {
        cout << ("file_get_sel_data: fread\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    for (int i = 0; i < SEL_ELEMS_MAX; i++)
    {
        for (int j = 0; j < sizeof(sel_msg_t); j++)
        {
            g_sel_data[i].msg[j] = buf[i * 16 + j];
        }
    }

    return 0;
}

/* file_store_sel_hdr
 * Description : 처음&마지막 index, timestamp 등이 저장된 g_sel_hdr을 IPMI_SEL_PATH에 저장하는 함수
 */
int file_store_sel_hdr(void)
{
    FILE *fp;

    fp = fopen(IPMI_SEL_PATH, "r+");
    if (fp == NULL)
    {
        cout << ("file_store_sel_hdr: fopen\n");
        return -1;
    }

    if (fwrite(&g_sel_hdr, sizeof(sel_hdr_t), 1, fp) <= 0)
    {
        cout << ("file_store_sel_hdr: fwrite\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

/* plat_sel_add_entry_record_id
 * Param : int recId(sel의 index가 될 번호), sel_msg_t *data(req로부터 받은 g_sel_data를 포함한 요청메시지)
 * Description : sel list에서 출력될 index(가장 왼쪽 column)를 g_sel_data(data->msg)에 붙이는 함수
 *               add 시에 호출되며 delete 시에 index 정리를 위해 다시 호출됨
 */
int plat_sel_add_entry_record_id(int recId, sel_msg_t *data)
{
    int *tmp;
    tmp = (int *)data->msg;
    *tmp = recId;
    return 0;
}

/* file_store_sel_data
 * Param : int recId(sel의 index), sel_msg_t *data(req로부터 받은 g_sel_data를 포함한 요청메시지)
 * Description : add sel 시에, IPMI_SEL_PATH의 맨 뒤에 sel 하나를 저장하는 함수
 */
int file_store_sel_data(int recId, sel_msg_t *data)
{
    FILE *fp;
    int index;

    fp = fopen(IPMI_SEL_PATH, "r+");
    if (fp == NULL)
    {
        printf("file_store_sel_data: fopen\n");
        return -1;
    }

    // Records are stored using zero-based index
    index = (recId) * sizeof(sel_msg_t);
    // index = g_sel_hdr.end * sizeof(sel_msg_t);

    if (fseek(fp, SEL_DATA_OFFSET + index, SEEK_SET))
    {
        printf("file_store_sel_data: fseek\n");
        fclose(fp);
        return -1;
    }

    if (fwrite(data->msg, sizeof(sel_msg_t), 1, fp) <= 0)
    {
        printf("file_store_sel_data: fwrite\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

// Platform specific SEL API entry points
// Retrieve time stamp for recent add operation

/* plat_sel_free_space
 * Description : plat_sel_num_entries로 구한 sel 갯수로부터 sel free space를 계산
 */
int plat_sel_free_space(void)
{
    int total_space;
    int used_space;

    total_space = SEL_RECORDS_MAX * sizeof(sel_msg_t);
    (used_space = plat_sel_num_entries() * sizeof(sel_msg_t));

    return (total_space - used_space);
}

// Reserve an ID that will be used in later operations
// IPMI/Section 31.4
int plat_sel_rsv_id()
{
    // Increment the current reservation ID and return
    if (g_rsv_id++ == SEL_RSVID_MAX)
    {
        g_rsv_id = SEL_RSVID_MIN;
    }
    // g_rsv_id = g_sel_hdr.end;
    return g_rsv_id;
}

// Get the SEL entry for a given record ID
// IPMI/Section 31.5
int plat_sel_get_entry(int read_rec_id, sel_msg_t *msg, int *next_rec_id)
{
    int index;

    // Find the index in to array based on given index
    if (read_rec_id == 0)
    {
        index = g_sel_hdr.begin;
    }
    else if (read_rec_id == SEL_RECID_LAST)
    {
        if (g_sel_hdr.end)
        {
            index = g_sel_hdr.end;
        }
        else
        {
            index = SEL_INDEX_MAX;
        }
    }
    else
    {
        index = read_rec_id;
    }

    // If the log is empty return error
    if (plat_sel_num_entries() == 0)
    {
        printf("plat_sel_get_entry: No entries\n");
        return -1;
    }

    // Check for boundary conditions
    if ((index < SEL_INDEX_MIN) || (index > SEL_INDEX_MAX))
    {
        printf("plat_sel_get_entry: Invalid Record ID %d\n", read_rec_id);
        return -1;
    }

    // If begin < end, check to make sure the given id falls between
    if (g_sel_hdr.begin < g_sel_hdr.end)
    {
        if (index < g_sel_hdr.begin || index >= g_sel_hdr.end)
        {
            printf("plat_sel_get_entry: Wrong Record ID %d\n", read_rec_id);
            return -1;
        }
    }

    // If end < begin, check to make sure the given id is valid
    if (g_sel_hdr.begin > g_sel_hdr.end)
    {
        if (index >= g_sel_hdr.end && index < g_sel_hdr.begin)
        {
            printf("plat_sel_get_entry: Wrong Record ID2 %d\n", read_rec_id);
            return -1;
        }
    }

    memcpy(msg->msg, g_sel_data[read_rec_id].msg, sizeof(sel_msg_t));

    // Return the next record ID in the log
    // lika
    *next_rec_id = ++read_rec_id;

    if (*next_rec_id > SEL_INDEX_MAX)
    {
        *next_rec_id = SEL_INDEX_MIN;
    }

    // If this is the last entry in the log, return 0xFFFF
    if (*next_rec_id == g_sel_hdr.end)
    {
        *next_rec_id = SEL_RECID_LAST;
    }

    return 0;
}

int add_to_sel_full()
{
    sel_msg_t entry;
    uint8_t ts[4];
    (time_stamp_fill(ts));
    memset(&entry, 0, sizeof(sel_msg_t));
    entry.msg[2] = 2;
    memcpy(entry.msg + 3, ts, 4);
    entry.msg[7] = 0x20;
    entry.msg[9] = 4;
    entry.msg[10] = 0x10;
    entry.msg[11] = 0x72;
    entry.msg[12] = 0x6F;
    entry.msg[13] = 0b00010000 + 0x04;
    int rec_id;
    (plat_sel_add_entry(&entry, &rec_id));
}
// Add a new entry in to SEL log
// IPMI/Section 31.6
int plat_sel_add_entry(sel_msg_t *msg, int *rec_id)
{
    int i, j;
    int full_msg = 0;
    int entries = 0;

    (entries = plat_sel_num_entries());
    if (entries == SEL_RECORDS_MAX)
    {
        printf("SEL Entry is full");
        return -1;
    }
    // If the SEL if full, roll over. To keep track of empty condition, use
    // one empty location less than the max records.

    (entries = plat_sel_num_entries());
    if (entries == SEL_RECORDS_MAX - 2)
    {

        full_msg = 1;
    }
    //// 바이트 4에서 시작하는 메시지의 타임스탬프 업데이트
    // Update message's time stamp starting at byte 4
    (time_stamp_fill(&msg->msg[3]));

    // Retrun the newly added record ID
    *rec_id = g_sel_hdr.end;

    (entries = plat_sel_add_entry_record_id(*rec_id, msg));
    if (entries)
    {
        printf("plat_sel_add_entry: plat_sel_add_entry_record_id");
        return -1;
    }

    // Add the enry at end
    memcpy(g_sel_data[g_sel_hdr.end].msg, msg->msg, sizeof(sel_msg_t));

    g_sel_hdr.end++;

    // Update timestamp for add in header
    (time_stamp_fill(g_sel_hdr.ts_add.ts));

    // Store the structure persistently
    if (file_store_sel_hdr())
    {
        printf("plat_sel_add_entry: file_store_sel_hdr");
        return -1;
    }

    /////lika
    if (file_store_sel_data(*rec_id, msg))
    {
        printf("plat_sel_add_entry: file_get_sel_data");
        return -1;
    }

    if (full_msg == 1)
    {
        (add_to_sel_full());
    }
    return 0;
}

int file_store_all_sel_data(void)
{
    FILE *fp;
    fp = fopen(IPMI_SEL_PATH, "r+");
    if (fp == NULL)
    {
        printf("file_store_all_sel_data: fopen");
        return -1;
    }
    int i;
    if (fseek(fp, SEL_DATA_OFFSET, SEEK_SET))
    {
        printf("file_store_all_set_data: fseek");
        fclose(fp);
        return -1;
    }

    for (i = 0; i < g_sel_hdr.end + 1; i++)
    { // SEL data always starts from 0, not begin
        (plat_sel_add_entry_record_id(i, &g_sel_data[i]));
        if (fwrite(&g_sel_data[i], sizeof(g_sel_data), 1, fp) != 1)
        {
            printf("[%d] file_store_all_sel_data error: fwrite\n", i);
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int delete_sel_entry(int id)
{
    int e;
    if (id == g_sel_hdr.begin)
    {
        for (e = id; e < g_sel_hdr.end - 1; e++)
        {
            memcpy(&g_sel_data[e], &g_sel_data[e + 1], sizeof(sel_msg_t));
        }
        g_sel_hdr.end -= 1;
    }
    else if (id == g_sel_hdr.end - 1)
    {
        g_sel_hdr.end -= 1;
    }
    else
    {

        for (e = id; e < g_sel_hdr.end - 1; e++)
        {
            memcpy(&g_sel_data[e], &g_sel_data[e + 1], sizeof(sel_msg_t));
        }
        g_sel_hdr.end -= 1;
    }
    //  time_stamp_fill(g_sel_hdr.ts_erase.ts);

    // Store the structure persistently
    if (file_store_sel_hdr())
    {
        printf("plat_sel_add_entry: file_store_sel_hdr\n");
        return -1;
    }

    (file_store_all_sel_data());

    // file_get_sel_hdr();
    // file_get_sel_data();

    return 0;
}

// Erase the SEL completely
// IPMI/Section 31.9
// Note: To reduce wear/tear, instead of erasing, manipulating the metadata
// #define BR2_PACKAGE_NETSNMP 1
/**
 * @author 기철
 * @bug SNMP Alert 서버로 snmptrap 메시지를 전송하기위한 함수 미구현
 */
int plat_sel_erase(int rsv_id)
{

    if (rsv_id != g_rsv_id)
    {
        return -1;
    }

    // Erase SEL Logs
    g_sel_hdr.begin = SEL_INDEX_MIN;
    g_sel_hdr.end = SEL_INDEX_MIN;

    // Update timestamp for erase in header
    (time_stamp_fill(g_sel_hdr.ts_erase.ts));

    // Store the structure persistently
    if (file_store_sel_hdr())
    {
        printf("plat_sel_erase: file_store_sel_hdr");
        return -1;
    }
    int rec_id;
    uint8_t ts_array[4];
    (time_stamp_fill(ts_array));
    sel_msg_t entry;
    entry.msg[0] = 0x0;
    entry.msg[1] = 0x0;
    entry.msg[2] = 0x2;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0x0;
    entry.msg[9] = 0x04;
    entry.msg[10] = 0x10; // Event Logging
    entry.msg[11] = 0x72; // Sensor Number
    entry.msg[12] = 0x6F;
    entry.msg[13] = 0b00010000 + 0x02;

    (plat_sel_add_entry(&entry, &rec_id));
#if BR2_PACKAGE_NETSNMP
    unsigned char policy_set = 0;
    (policy_set = check_pef_table_sensor(0xff, 0x2f, 0x10));
    if (policy_set != -1)
        (prepare_alert_snmp(policy_set, "SEL cleared"));
#endif
    return 0;
}

// To get the erase status while erase happens
// IPMI/Section 31.2
// Note: Since we are not doing offline erasing, need not return in-progress state
int plat_sel_erase_status(int rsv_id, sel_erase_stat_t *status)
{

    if (rsv_id != g_rsv_id)
    {
        return -1;
    }

    // Since we do not do any offline erasing, always return erase done
    *status = SEL_ERASE_DONE;

    return 0;
}

// lika
int plat_sel_time_get(sel_msg_t *msg)
{

    (time_stamp_fill(&msg->msg[0]));
    return 0;
}

// lika
int plat_sel_time_set(sel_msg_t *msg)
{
    int ret = 0;
    (ret = time_stamp_set(&msg->msg[0]));

    return ret;
}

// Initialize SEL log file
int plat_sel_init(void)
{
    FILE *fp;
    // Check if the file exists or not
    if (access(IPMI_SEL_PATH, F_OK) == 0)
    {
        // Since file is present, fetch all the contents to cache
        if (file_get_sel_hdr())
        {
            printf("plat_init_sel: file_get_sel_hdr\n");
            return -1;
        }

        if (file_get_sel_data())
        {
            printf("plat_init_sel: file_get_sel_data\n");
            return -1;
        }

        return 0;
    }

    // File not present, so create the file
    fp = fopen(IPMI_SEL_PATH, "w+");
    if (fp == NULL)
    {
        printf("plat_init_sel: fopen\n");
        return -1;
    }

    fclose(fp);

    // Populate SEL Header in to the file
    g_sel_hdr.magic = SEL_HDR_MAGIC;
    g_sel_hdr.version = SEL_HDR_VERSION;
    g_sel_hdr.begin = SEL_INDEX_MIN;
    g_sel_hdr.end = SEL_INDEX_MIN;
    memset(g_sel_hdr.ts_add.ts, 0x0, 4);
    memset(g_sel_hdr.ts_erase.ts, 0x0, 4);

    if (file_store_sel_hdr())
    {
        printf("plat_init_sel: file_store_sel_hdr\n");
        return -1;
    }

    // Populate SEL Data in to the file
    for (int i = 1; i <= SEL_RECORDS_MAX; i++)
    {
        sel_msg_t msg = {0};
        if (file_store_sel_data(i, &msg))
        {
            printf("plat_init_sel: file_store_sel_data\n");
            return -1;
        }
    }
    return 0;
}

void storage_get_sel_info(ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];
    int num_entries;              // number of log entries
    int free_space;               // free space in SEL device in bytes
    time_stamp_t ts_recent_add;   // Recent Addition Timestamp
    time_stamp_t ts_recent_erase; // Recent Erasure Timestamp

    // Use platform APIs to get SEL information
    num_entries = plat_sel_num_entries();
    free_space = plat_sel_free_space();
    (plat_sel_ts_recent_add(&ts_recent_add));
    (plat_sel_ts_recent_erase(&ts_recent_erase));

    res->cc = CC_SUCCESS;

    *data++ = IPMI_SEL_VERSION;   // SEL version
    *data++ = num_entries & 0xFF; // number of log entries
    *data++ = (num_entries >> 8) & 0xFF;
    *data++ = free_space & 0xFF; // Free SEL Space
    *data++ = (free_space >> 8) & 0xFF;

    memcpy(data, ts_recent_add.ts, SIZE_TIME_STAMP);
    data += SIZE_TIME_STAMP;

    memcpy(data, ts_recent_erase.ts, SIZE_TIME_STAMP);
    data += SIZE_TIME_STAMP;

    cout << ("Get SEL Information");
    cout << ("Number of SEL Entries : %d", num_entries);
    cout << ("Free SEL Space : %d", free_space);

    *data++ = 0x02; // Operations supported

    *res_len = data - &res->data[0];

    return;
}

void storage_rsv_sel(ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];
    int rsv_id; // SEL reservation ID

    // Use platform APIs to get a SEL reservation ID
    rsv_id = plat_sel_rsv_id();
    if (rsv_id < 0)
    {
        res->cc = CC_SEL_ERASE_PROG;
        return;
    }

    res->cc = CC_SUCCESS;
    *data++ = rsv_id & 0xFF; // Reservation ID
    *data++ = (rsv_id >> 8) & 0XFF;

    cout << ("Get SEL Reservation ID");
    cout << ("Reservation ID : 0x%02x", rsv_id & 0xff);

    *res_len = data - &res->data[0];

    return;
}

void storage_get_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];

    int read_rec_id; // record ID to be read
    int next_rec_id; // record ID for the next msg
    sel_msg_t entry; // SEL log entry
    int ret;

    read_rec_id = (req->data[3] >> 8) | req->data[2];

    // Use platform API to read the record Id and get next ID
    (ret = plat_sel_get_entry(read_rec_id, &entry, &next_rec_id));
    if (ret)
    {
        res->cc = CC_UNSPECIFIED_ERROR;
        return;
    }

    res->cc = CC_SUCCESS;
    *data++ = next_rec_id & 0xFF; // next record ID
    *data++ = (next_rec_id >> 8) & 0xFF;

    memcpy(data, entry.msg, SIZE_SEL_REC);
    data += SIZE_SEL_REC;

    *res_len = data - &res->data[0];

    return;
}
#include <bitset>

void storage_add_event(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];
    int record_id; // Record ID for added entry
    int ret;
    // for (int i = 0; i < 8; i++)
    // {
    //     bitset<32> bit = bitset<32>(req->data[i]);
    //     cout<<int(req->data[i])<<endl;
    //     cout << "bit =="<<i<< endl;
    //     cout << bit << endl;
    //     cout << "==========" << endl;
    // }
    event_msg_t entry;
    entry.gen_id = 0x41;
    entry.evm_rev = (uint8_t)(req->data[0]);
    entry.sensor_type = (uint8_t)(req->data[1]);
    entry.sensor_num = (uint8_t)(req->data[2]);
    entry.event_dir = (uint8_t)(req->data[3] & 0x80) >> 7;
    entry.event_type = (uint8_t)(req->data[3] & 0x7f);
    entry.event_data[0] = (uint8_t)(req->data[4]);
    entry.event_data[1] = (uint8_t)(req->data[5]);
    entry.event_data[2] = (uint8_t)(req->data[6]);

    sel_msg_t selentry;
    selentry.msg[0] = 0x00;
    selentry.msg[1] = 0x00;
    selentry.msg[3] = 0x02; // record_type
    selentry.msg[4] = 0x00;
    selentry.msg[5] = 0x00;
    selentry.msg[6] = 0x00;
    selentry.msg[7] = 0x00;
    selentry.msg[8] = entry.gen_id; // genid;
    selentry.msg[9] = entry.evm_rev;
    selentry.msg[10] = entry.sensor_type;
    selentry.msg[11] = entry.sensor_num;
    selentry.msg[12] = entry.event_type;
    selentry.msg[13] = entry.event_data[0];
    selentry.msg[14] = entry.event_data[1];
    selentry.msg[15] = entry.event_data[2];

    (ret = plat_sel_add_entry(&selentry, &record_id));
    if (ret)
    {
        if (ret == -2)
            res->cc = CC_OUT_OF_SPACE;
        else
            res->cc = CC_UNSPECIFIED_ERROR;
        return;
    }

    res->cc = CC_SUCCESS;
    *data++ = record_id & 0xFF;
    *data++ = (record_id >> 8) & 0xFF;

    cout << ("SEL Add Entry") << endl;
    cout << ("SEL Record ID : 0x%02x", record_id) << endl;

    *res_len = data - &res->data[0];
}
void storage_add_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];
    int record_id; // Record ID for added entry
    int ret;

    sel_msg_t entry;
    memcpy(entry.msg, req->data, SIZE_SEL_REC);
    cout << "storage_add_sel::entry.msg " << endl;
    for (int i = 0; i < SIZE_SEL_REC; i++)
        cout << (int)entry.msg[i] << endl;
    (ret = plat_sel_add_entry(&entry, &record_id));
    if (ret)
    {
        if (ret == -2)
            res->cc = CC_OUT_OF_SPACE;
        else
            res->cc = CC_UNSPECIFIED_ERROR;
        return;
    }

    res->cc = CC_SUCCESS;
    *data++ = record_id & 0xFF;
    *data++ = (record_id >> 8) & 0xFF;

    cout << ("SEL Add Entry") << endl;
    cout << ("SEL Record ID : 0x%02x", record_id) << endl;
    char buf[500] = {
        0,
    };
    sprintf(buf, "SELAdd Entry Record ID : 0x%02x", record_id);
    // ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf,"Sensor","System"));
    *res_len = data - &res->data[0];

    return;
}
void storage_del_sel_entry(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];
    int id = req->data[2] & 0xff; // entry index to delete

    int ret = 0;
    (delete_sel_entry(id));

    cout << ("Delete SEL Entry");
    cout << ("Delete SEL ID : 0x%02x", id);
    char buf[500] = {
        0,
    };
    sprintf(buf, "Delete SEL ID : 0x%02x", id);
    // ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf,"Sensor","System"));
    res->cc = CC_SUCCESS;
    *res_len = data - &res->data[0];

    return;
}

void storage_clr_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];

    sel_erase_stat_t status;
    int ret;
    int rsv_id;

    // Verify the request to contain 'CLR' characters
    if ((req->data[2] != 'C') || (req->data[3] != 'L') || (req->data[4] != 'R'))
    {
        res->cc = CC_INVALID_PARAM;
        return;
    }

    // Populate reservation ID given in request
    rsv_id = (req->data[1] << 8) | req->data[0];

    // Use platform APIs to clear or get status
    if (req->data[5] == IPMI_SEL_INIT_ERASE)
    {
        (ret = plat_sel_erase(rsv_id));
    }
    else if (req->data[5] == IPMI_SEL_ERASE_STAT)
    {
        (ret = plat_sel_erase_status(rsv_id, &status));
    }
    else
    {
        res->cc = CC_INVALID_PARAM;
        return;
    }

    // Handle platform error and return
    if (ret)
    {
        res->cc = CC_UNSPECIFIED_ERROR;
        return;
    }

    res->cc = CC_SUCCESS;
    *data++ = status;

    cout << ("Clear SEL Entries");
    cout << ("Clear Reservation ID : 0x%02x", rsv_id);
    cout << ("SEL Status : 0x%02x", status);

    *res_len = data - &res->data[0];

    return;
}

// lika) storage_get_time
void storage_get_time(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];
    sel_msg_t entry;
    int ret;

    (ret = plat_sel_time_get(&entry));
    if (ret)
    {
        res->cc = CC_UNSPECIFIED_ERROR;
        return;
    }

    res->cc = CC_SUCCESS;
    memcpy(data, entry.msg, 4);
    cout << ("Get SEL Timestamp : 0x%02x%02x%02x%02x", data[3], data[2], data[1], data[0]);

    *res_len = 4;
    return;
}

// lika ) storage_set_time
void storage_set_time(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    sel_msg_t entry;
    int ret, i;

    memcpy(entry.msg, req->data, 4);

    (ret = plat_sel_time_set(&entry));
    if (ret)
    {
        res->cc = CC_UNSPECIFIED_ERROR;
        return;
    }

    cout << ("Set SEL Timestamp : 0x%02x%02x%02x%02x", req->data[3], req->data[2], req->data[1], req->data[0]);
    char buf[500] = {
        0,
    };
    sprintf(buf, "Set SEL Timestamp : 0x%02x%02x%02x%02x", req->data[3], req->data[2], req->data[1], req->data[0]);
    // ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf,"Maintenance","System"));
    res->cc = CC_SUCCESS;
    *res_len = 0;
    return;
}

void sensor_event_get_num(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len)
{
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *data = &res->data[0];
    int record_id;
    int ret;
    sel_msg_t entry;
    memcpy(&entry.msg[9], req->data, 7);

    (ret = plat_sel_add_entry(&entry, &record_id));
    if (ret)
    { // fail
        if (ret == -2)
            res->cc = CC_OUT_OF_SPACE;
        else
            res->cc = CC_UNSPECIFIED_ERROR;
        return;
    }

    // success
    res->cc = CC_SUCCESS;
    *data++ = record_id & 0xFF;
    cout << ("Get Sensor Event Number");
    cout << ("Record ID : 0x%02x", record_id & 0xFF);
    char buf[500] = {
        0,
    };
    sprintf(buf, "Record ID : 0x%02x", record_id & 0xFF);
    // ipmiLogEventHandler.Event_Registration(IpmiLogEvent(buf,"Sensor","System"));
    *res_len = data - &res->data[0];
    return;
}

static char *
ipmi_sel_timestamp_date(time_t timestamp)
{
    static char tbuf[11];
    //	time_t s = (time_t)stamp;
    strftime(tbuf, sizeof(tbuf), "%m/%d/%Y", gmtime(&timestamp));
    return tbuf;
}
static char *
ipmi_sel_timestamp(time_t stamp)
{
    static char tbuf[40];
    //	time_t s = (time_t)stamp;
    memset(tbuf, 0, 40);
    strftime(tbuf, sizeof(tbuf), "%m/%d/%Y %H:%M:%S", gmtime(&stamp));
    return tbuf;
}

int rest_sel_write_raw()
{
    FILE *fp = fopen("/conf/sel.log", "w");
    if ((fwrite(&g_sel_data, sizeof(g_sel_data), 1, fp)) < 1)
    {
        printf("fwrite in rest_sel_write_raw\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

ipmi_event_sensor_types *
ipmi_get_first_event_sensor_type(
    uint8_t sensor_type, uint8_t event_type)
{
    struct ipmi_event_sensor_types *evt, *start, *next = NULL;
    uint8_t code;

    if (event_type == 0x6f)
    {

        /* check generic sensor types */
        start = sensor_specific_event_types;
        code = sensor_type;
    }
    else
    {
        start = generic_event_types;
        code = event_type;
    }

    for (evt = start; evt->desc || next; evt++)
    {
        /* check if VITA sensor event types has finished */
        if (!evt->desc)
        {
            /* proceed with next table */
            evt = next;
            next = NULL;
        }

        if (code == evt->code)
            return evt;
    }

    return NULL;
}

struct ipmi_event_sensor_types *
ipmi_get_next_event_sensor_type(struct ipmi_event_sensor_types *evt)
{
    struct ipmi_event_sensor_types *start = evt;

    for (evt = start + 1; evt->desc; evt++)
    {
        if (evt->code == start->code)
        {
            return evt;
        }
    }

    return NULL;
}

void ipmi_get_event_desc(char *desc, sel_msg_t *rec)
{
    struct ipmi_event_sensor_types *evt = NULL;
    uint8_t offset = rec->msg[13];
    uint8_t offdata = rec->msg[14];

    for (evt = ipmi_get_first_event_sensor_type(rec->msg[10], rec->msg[12]); evt; evt = ipmi_get_next_event_sensor_type(evt))
    {
        if (evt->offset == offset)
        {
            if (evt->data == offdata)
                sprintf(desc, "%s", evt->desc);
        }
    }
    if (!(strcmp(desc, "NONE")))
        sprintf(desc, "Type(0x%x), Offset(0x%x) Data(0x%x)", rec->msg[12], rec->msg[13], rec->msg[14]);
}

const char *ipmi_generic_sensor_type_vals[] = {
    "reserved",
    "Temperature", "Voltage", "Current", "Fan",
    "Physical Security", "Platform Security", "Processor",
    "Power Supply", "Power Unit", "Cooling Device", "Other",
    "Memory", "Drive Slot / Bay", "POST Memory Resize",
    "System Firmwares", "Event Logging Disabled", "Watchdog1",
    "System Event", "Critical Interrupt", "Button",
    "Module / Board", "Microcontroller", "Add-in Card",
    "Chassis", "Chip Set", "Other FRU", "Cable / Interconnect",
    "Terminator", "System Boot Initiated", "Boot Error",
    "OS Boot", "OS Critical Stop", "Slot / Connector",
    "System ACPI Power State", "Watchdog2", "Platform Alert",
    "Entity Presence", "Monitor ASIC", "LAN",
    "Management Subsys Health", "Battery", "Session Audit",
    "Version Change", "FRU State",
    NULL};

// json 변경해야함-- 메모
// sel 이벤트 변경
int rest_get_eventlog_config(char *ret)
{
    int i = 0;
    int getfail, next;
    sel_msg_t msg;
    struct tm *t;
    time_t time;
    char buf[100];
    strcpy(ret, "{\n");
    strcat(ret, "\"EVENT_INFO\": {\n  \"SEL\":\n\t[\n");
    int eventlog_cnt = plat_sel_num_entries();
    while (i < eventlog_cnt)
    {
        (getfail = plat_sel_get_entry(i, &msg, &next));
        if (getfail)
            break;
        memcpy(&time, &msg.msg[3], sizeof(time_t));
        //    printf("time: %s\n", ipmi_sel_timestamp(time));
        (t = localtime(&time));

        strcat(ret, "\t\t{\n");

        if ((t->tm_year + 1900) < 2017)
            sprintf(buf, "\t\t \"TIME\": \"Pre-Init\",\n");
        else
            sprintf(buf, "\t\t \"TIME\": \"%04d-%02d-%02d %02d:%02d:%02d\",\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
        strcat(ret, buf);
        char sName[30];
        int namelen = 0;

        string sensorname;
        (namelen = find_sensor_name(msg.msg[10], msg.msg[11], sensorname));
        strcpy(sName, sensorname.c_str());
        if (namelen < 0)
            sprintf(sName, "%s : 0x%02X", ipmi_generic_sensor_type_vals[msg.msg[10]], msg.msg[11]);
        sprintf(buf, "\t\t \"NAME\": \"%s\",\n", sName); // need parsing through sensor enum
        strcat(ret, buf);

        sprintf(buf, "\t\t \"SEL_RD\": \"%x\",\n", msg.msg[12]); // no use
        strcat(ret, buf);

        sprintf(buf, "\t\t \"TYPE\": \"%s\",\n", ipmi_generic_sensor_type_vals[msg.msg[10]]);
        strcat(ret, buf);

        char buf_des[128] = {0};
        char buf_ret[256] = {0};

        msg.msg[13] = msg.msg[13] & 0x0f;

        if (msg.msg[10] != 0x08 && msg.msg[10] != 0x2b && msg.msg[10] != 0x0f)
            msg.msg[14] = 0xff;

        if (msg.msg[12] != 0x6f)
            msg.msg[14] = 0xff;

        ipmi_get_event_desc(buf_des, &msg);
        sprintf(buf_ret, "\t\t \"DESCRIPTION\": \"%s\",\n", buf_des);
        strcat(ret, buf_ret);

        // ----------------------------------- 14:25 
        // sel test
        // #if 0
        //     if (msg.msg[12] == 0x01) { // Threshold Sensor
        //       msg.msg[13] -= 0b01010000;
        //       switch (msg.msg[13]) {
        //         case 0x02:
        //           strcat(ret, "\t\t \"DESCRIPTION\": \"Lower critical going low\",\n");
        //           break;
        //         case 0x09:
        //           strcat(ret, "\t\t \"DESCRIPTION\": \"Upper critical going high\",\n");
        //           break;
        // 	case 0xb9:
        // 	  strcat(ret, "\t\t \"DESCRIPTION\": \"Sample-Upper critical going high\",\n");
        // 	  break;
        // 	default:
        // 	  sprintf(buf_des, "\t\t \"DESCRIPTION\": \"0x%x\",\n", msg.msg[13]);
        // 	  strcat(ret,buf_des);
        // 	  break;
        //       }
        //     }
        //     else if (msg.msg[12] == 0x08) {
        //       switch (msg.msg[13]) {
        //       	case 0x0:
        // 	  strcat(ret, "\t\t \"DESCRIPTION\": \"Device Absent\",\n");
        // 	  break;
        // 	case 0x1:
        // 	  strcat(ret, "\t\t \"DESCRIPTION\": \"Device Present\",\n");
        // 	  break;
        // 	default:
        // 	    sprintf(buf_des, "\t\t \"DESCRIPTION\": \"1-Type(0x08), Offset(0x%x)\",\n", msg.msg[13]);
        // 	    strcat(ret, buf_des);
        // 	    break;
        // 	}
        //     }
        //     else if (msg.msg[12] == 0x6f) {
        // 	switch (msg.msg[13]) {
        // 	    case 0x0:
        // 		strcat(ret, "\t\t \"DESCRIPTION\": \"Sample-Correctable ECC\",\n");
        // 		break;
        // 	    case 0x15:
        // 		strcat(ret, "\t\t \"DESCRIPTION\": \"Timestamp Clock Sync\",\n");
        // 		break;
        // 	    case 0x12:
        // 		strcat(ret, "\t\t \"DESCRIPTION\": \"SEL Cleared\",\n");
        // 		break;
        // 	    case 0x14:
        // 		strcat(ret, "\t\t \"DESCRIPTION\": \"SEL Full\",\n");
        // 		break;
        // 	    case 0xC3:
        // 	       strcat(ret, "\t\t \"DESCRIPTION\": \"Power Supply AC lost\",\n");
        // 	       break;
        // 	    case 0xC2:
        // 	       switch (msg.msg[14]) {
        // 		   case 0x03:
        // 		       strcat(ret, "\t\t \"DESCRIPTION\": \"Secondary CPU initialization\",\n");
        // 		       break;
        // 		   case 0x07:
        // 		       strcat(ret, "\t\t \"DESCRIPTION\": \"PCI resource configuration\",\n");
        // 		       break;
        // 		   case 0x09:
        // 		       strcat(ret, "\t\t \"DESCRIPTION\": \"Video initialization\",\n");
        // 		       break;
        // 		   case 0x08:
        // 		       strcat(ret, "\t\t \"DESCRIPTION\": \"Option ROM initialization\",\n");
        // 		       break;
        // 		   case 0x0c:
        // 		       strcat(ret, "\t\t \"DESCRIPTION\": \"Keyboard controller initialization\",\n");
        // 		       break;
        // 		   case 0x02:
        // 		       strcat(ret, "\t\t \"DESCRIPTION\": \"Hard-disk initialization\",\n");
        // 		       break;
        // 		   case 0x05:
        // 		       strcat(ret, "\t\t \"DESCRIPTION\": \"User-initiated system setup\",\n");
        // 		       break;
        // 		   default:
        // 		       sprintf(buf_des, "\t\t \"DESCRIPTION\": \"BIOS OEM 0x%x\",\n", msg.msg[14]);
        // 		       strcat(ret, buf_des);
        // 		       break;
        // 	       }
        // 	       break;
        // 	    case 0x5:
        // 	       strcat(ret, "\t\t \"DESCRIPTION\": \"Timstamp Clock Synch\",\n");
        // 	       break;
        // 	    default:
        //                sprintf(buf_des, "\t\t \"DESCRIPTION\": \"2-Type(0x%x), Offset(0x%x)\",\n", msg.msg[12], msg.msg[13]);
        //                strcat(ret, buf_des);
        //                break;
        // 	}
        //     }
        //     else {
        // 	sprintf(buf_des, "\t\t \"DESCRIPTION\": \"3-Type(0x%x), Offset(0x%x)\",\n", msg.msg[12], msg.msg[13]);
        // 	strcat(ret, buf_des);
        //     }

        // // #endif

                sprintf(buf, "\t\t \"SENSOR_ID\": \"%02X\"\n", i);
                strcat(ret, buf);
                if (++i < eventlog_cnt && strlen(ret) < 34000)
                {
                    strcat(ret, "\t\t},\n");
                }
                else
                {
                    strcat(ret, "\t\t}\n");
                    break;
                }
            }
            strcat(ret, "\t]\n  }\n}\n");
            return strlen(ret);
    
}

// Retrieve time stamp for recent erase operation
void plat_sel_ts_recent_erase(time_stamp_t *ts)
{
    memcpy(ts->ts, g_sel_hdr.ts_erase.ts, 0x04);
}
void plat_sel_ts_recent_add(time_stamp_t *ts)
{
    memcpy(ts->ts, g_sel_hdr.ts_add.ts, 0x04);
}

/** custom event 생성중 ...
 * @brief threashold event 생성
 * @param
 * @details
 * event_string.hpp = Digital-Discrete Event States
 *
 */
// void plat_sel_threshold_add_entry(Sensor_tpye se_tpye, int se_num, Theshold_type th_type)
// {

//     printf("Add Lower Non-critical going low\n");
//     int rec_id;
//     unsigned char ts_array[4];
//     sel_msg_t entry;

//     (time_stamp_fill(ts_array));
//     memset(&entry, 0, sizeof(sel_msg_t));

//     entry.msg[0] = 0x00;
//     entry.msg[1] = 0x00;
//     entry.msg[2] = 0x02;
//     entry.msg[3] = ts_array[3];
//     entry.msg[4] = ts_array[2];
//     entry.msg[5] = ts_array[1];
//     entry.msg[6] = ts_array[0];
//     entry.msg[7] = 0x20;
//     entry.msg[8] = 0;
//     entry.msg[9] = 0x04;
//     entry.msg[10] = static_cast<int>(se_tpye); // Sensor Type : Power Supply
//     entry.msg[11] = se_num;
//     entry.msg[12] = 0x01;                      // /* Threshold Based States */
//     entry.msg[13] = static_cast<int>(th_type); // Limit Exceeded
//     entry.msg[14] = 0x01;                      // offset
//     (plat_sel_add_entry(&entry, &rec_id));
// }


void ipmi_find_sel_desc(uint8_t *offset, uint8_t *data,uint8_t se_type, const char *ev_desc ,uint8_t ev_type )
{
    struct ipmi_event_sensor_types *evt = NULL;

    for (evt = ipmi_get_first_event_sensor_type(se_type, ev_type); evt; evt = ipmi_get_next_event_sensor_type(evt))
    {
        if (!strcmp(evt->desc , ev_desc))
        {
          *offset = evt->offset;
          *data = evt->data;
        return;
        }
    }
   
}

void plat_sel_generater_add_entry(const char *ev_desc,uint8_t ev_type)
{

    printf("plat_sel_generater_add_entry\n");
    int rec_id;
    unsigned char ts_array[4];
    sel_msg_t entry;

    uint8_t se_type = 0x08;
    uint8_t offset = 0x04;
    uint8_t data = 0xff;

    ipmi_find_sel_desc(&offset,&data,se_type,ev_desc,ev_type);
    
    (time_stamp_fill(ts_array));
    memset(&entry, 0, sizeof(sel_msg_t));

    entry.msg[0] = 0x00;
    entry.msg[1] = 0x00;
    entry.msg[2] = 0x02;
    entry.msg[3] = ts_array[3];
    entry.msg[4] = ts_array[2];
    entry.msg[5] = ts_array[1];
    entry.msg[6] = ts_array[0];
    entry.msg[7] = 0x20;
    entry.msg[8] = 0;
    entry.msg[9] = 0x04;
    entry.msg[10] = se_type;                   // sensor type 
    entry.msg[11] = 0x20;                      // sensor num
    entry.msg[12] = ev_type;                   // event type
    entry.msg[13] = offset;                    // offset
    entry.msg[14] = data;                      // data
    (plat_sel_add_entry(&entry, &rec_id));
}


void IPMI_Handle_Event::ipmi_handle_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len){
    uint8_t cmd = request->cmd;
    ipmi_req_t *req = (ipmi_req_t *)request;
    ipmi_res_t *res = (ipmi_res_t *)response;
    unsigned char *event = &res->data[0];
    cout << "event handle " << endl;
    printf("%x",event);
    // switch(){
    //     cout << " switch " <<endl;
    //     case 0x21: 
    //         cout<< "kernel panic "<<cmd<<endl;
    //         res->cc = CC_SUCCESS;
    //         break;
    //     case 0x41:
    //         cout << "shutdown " << endl;
    //         res->cc = CC_SUCCESS;
    //         break;
    // }
}

void IPMI_Handle_Event::ipmi_kernel_panic(){
    cout << "panic "<<endl;
}