#pragma once
#include <vector>
#include <ipmi/ipmi.hpp>
#include <ipmi/common.hpp>
#include <ipmi/setting_service.hpp>

int plat_sdr_get_entry(int rsv_id, int read_rec_id, std::vector<uint8_t> *v_rec, int *next_rec_id);
int plat_sdr_num_entries(void);
int plat_sdr_free_space(void);
int plat_sdr_rsv_id();

/**
 * @brief SDR 레코드를 초기화하는 함수\n
 * 본 함수 실행시 SDR 레코드를 전부 초기화 하고 SEL 리스트에 초기화 로그를 남긴다.\n
 * @param ipmi_res_t *response - IPMI Response message pointer\n
 * @param unsigned char *res_len - IPMI Response message length\n
 * @return void
 */ 
void storage_clear_sdr_repository(ipmi_res_t *response, uint8_t *res_len);
void update_sensor_reading();
void sensor_get_reading(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
void sensor_get_threshod(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
void sensor_set_threshold(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief SDR 정보를 반환하는 함수\n
 * 본 함수 실행시 SDR 버전, 레코드 개수, 여유 공간, 최근 추가된 SDR 레코드 날짜, 오버플로우 유무\n
 * SDR Repository Update 지원 등 SDR 전체 정보에 대해 반환하는 함수이다.\n
 * 현재 저장된 SDR 레코드 개수는 plat_sdr_num_entries() 함수를, 여유 공간에 대한 정보는 plat_sdr_free_space() 함수를 호출하여 반환한다.\n
 * @param ipmi_res_t *response - IPMI Response message pointer\n
 * @param unsigned char *res_len - IPMI Response message length\n
 * @return void
 */ 
void storage_rsv_sdr(ipmi_res_t *response, uint8_t *res_len);
/**
 * @brief 현재 SDR 레코드 정보와 다음 SDR 레코드 번호를 반환하는 함수\n
 * 본 함수 실행시 입력된 현재 SDR 레코드 번호를 사용하여 SDR 정보를 반환하고, 다음 불러올 레코드 번호를 반환한다.\n
 * SDR 레코드에 대한 정보는 plat_sdr_get_entry 함수를 호출하여 반환받는다.\n
 * @param ipmi_req_t *request - IPMI Request message pointer\n
 * @param ipmi_res_t *response - IPMI Response message pointer\n
 * @param unsigned char *res_len - IPMI Response message length\n
 * @return void
 */ 
void storage_get_sdr(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
void storage_get_sdr_info(ipmi_res_t *response, uint8_t *res_len);
uint8_t plat_find_sdr_index(uint8_t s_num);
unsigned char get_power_status(void);

int rest_get_sensor_config (char* res);
/**
 * @brief RAW 값을 Float 값으로 변환하는 함수\n
 * sdr_full_t 구조체에 저장된 값으로 계산되어 변환됨.
 * @param sensor Full Type SDR 구조체
 * @param val 변환할 RAW 값
 * @return float 변환된 실수형 값
 */
float sdr_convert_raw_to_sensor_value(sensor_thresh_t *sensor, uint8_t val);

/**
 * @brief Float 값을 Raw 값으로 변환하는 함수\n
 * sdr_full_t 구조체에 저장된 값으로 계산되어 변환됨.
 * @param sensor Full Type SDR 구조체
 * @param val 변환할 Float 값
 * @return Raw 변환된 실수형 값
 */
uint8_t sdr_convert_sensor_value_to_raw(sensor_thresh_t * sensor, double val);

/**
 * @brief SDR 정보를 초기화하는 함수.\n
 * SDR 헤더 및 데이터 파일이 없을 경우 libipmi_sensor/sensor_define.h에 정의된\n
 * 센서 리스트로 SDR을 초기화 함
 * 
 * @return int 성공 시 0, 실패 시 -1을 반환
 */
int plat_sdr_init(void);
/**
 * @brief SDR 데이터 레코드에 입력된 SDR 레코드를 저장하는 함수\n
 * 
 * @param rec 저장할 SDR 레코드
 * @param rec_id 저장된 SDR 레코드 ID
 * @return int 실패 시 -1 (SDR 데이터 레코드가 가득 찬 경우)
 */
static int plat_sdr_add_entry(sdr_rec_t *rec, int *rec_id);
/**
 * @brief SDR 헤더 정보를 저장하는 함수\n
 * SDR 레코드 추가 및 변경으로 인한 헤더 변경 시 호출됨.
 * @return int 성공 시 0 / 실패 시 -1을 반환
 */
int file_store_sdr_hdr() ;
/**
 * @brief SDR 데이터를 저장하는 함수\n
 * SDR 레코드 추가 및 변경, 데이터 변경으로 인한 데이터에 변화가 생겼을 경우 호출됨\n
 * @param recId 기록할 SDR 레코드 ID
 * @param data 저장할 SDR 레코드 데이터
 * @return int 성공 시 0, 실패 시 -1 반환
 */
int file_store_sdr_data(int recId, sdr_rec_t *data);
/**
 * @brief 파일로부터 SDR 헤더 정보를 불러옴
 * 
 * @return int 성공 시 0, 실패 시 -1을 반환
 */
int file_get_sdr_hdr();


uint8_t plat_find_sdr_name(string name);
/**
 * @brief 파일로부터 SDR 데이터를 불러오는 함수
 * 
 * @return int 성공 시 0, 실패 시 -1을 반환
 */
int file_get_sdr_data();
/**
 * @brief SDR 센서 값을 읽어오는 함수
 * 
 * @param sensor_num 읽을 센서 번호
 * @return unsigned char 센서 값 RAW Value
 */
unsigned char sdr_sensor_read(int sensor_num);

/**
 * @brief ipmi 기준으로 된 sensor type 을 redfish 타입으로 변경 하기 위한 함수
 */
string sensor_tpye2string(uint8_t ipmisensor_type);
bool redfish_seonsor_sync(sensor_thresh_t *rec);

