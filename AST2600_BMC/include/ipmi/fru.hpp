/** @file fru.hpp
 * @author Park Ki Cheol
 * @date 2021-04-13
 * @brief 본 문서는 x86 서버보드 기술 개발의 KETI-IPMI 소프트웨어에 대한
 * 설명입니다.\n FRU 헤더파일에 대한 설명이  포함되어있습니다.
 *
 */
#ifndef FRU_UNIQUE_NAME_HERE
#define FRU_UNIQUE_NAME_HERE
#include "ipmi/common.hpp"
#include "ipmi/ipmi.hpp"
#define FRU_DEBUG 1

using namespace std;

// /**
//  * @brief
//  * FRU Header의 Board 헤더 길이 매크로입니다.
//  */
// #define FRU_BOARD_HEADER_LENGTH 6
// /**
//  * @brief
//  * FRU Header의 Product 헤더 길이 매크로입니다.
//  */
// #define FRU_PRODUCT_HEADER_LENGTH 3
// /**
//  * @brief
//  * FRU Header의 Chassis 헤더 길이 매크로입니다.
//  */
// #define FRU_CHASSIS_HEADER_LENGTH 2
// /**
//  * @brief
//  * storage_get_fruid_data 요청 시 클라이언트 ipmitool로 전송할 메세지의 최대
//  길이를 정의합니다.\n
//  * ipmitool fru edit 명령어 실행 시 request message의 4번째 Byte 데이터는 FRU
//  Data의 길이를 나타냅니다.\n
//  * 만약 FRU Data length가 MAX_READ_SIZE와 동일할 경우 edit_flag를 1로
//  바꿉니다.\n
//  */
// #define MAX_READ_SIZE 0x1d
// #define MIN_READ_SIZE 0x1d

enum {
  // SENSOR_FRU_MAX =1,
  FRU_BOARD_HEADER_LENGTH = 6,
  FRU_PRODUCT_HEADER_LENGTH = 3,
  FRU_CHASSIS_HEADER_LENGTH = 2,
  MAX_READ_SIZE = 0x1d,
  MIN_READ_SIZE = 0x1d,
  // MAX_READ_SIZE = 0x6,
  // MIN_READ_SIZE = 0x7,

};

void plat_fru_device_init();
void storage_get_fru(ipmi_req_t *request, ipmi_res_t *response,
                     uint8_t *res_len);
void storage_get_fru_info(ipmi_req_t *request, ipmi_res_t *response,
                          uint8_t *res_len);
void plat_fru_data_init();

int rest_get_fru_config(char *res);
void rest_set_fru_header(int id, char h_board, char h_chassis, char h_product);
void rest_set_fru_board(int id, char f_mfg_date[4], char f_mfg[LEN_MFG],
                        char f_product[LEN_PRODUCT], char f_serial[LEN_SERIAL],
                        char f_part_num[LEN_PART_NUM]);
void rest_set_fru_product(int id, char* f_name, char* f_mfg, char* f_version,
	char *f_serial, char *f_part_num);
void rest_set_fru_chassis(int id, char* f_type, char* f_serial, char* f_part_num);
void plat_fru_device_save();

class Ipmifru {
public:
  fru_info_t fru;
  fru_board_info_t fru_board;
  fru_chassis_info_t fru_chassis;
  fru_header_t fru_header;
  ipmi_fruid_info_t fruid;
  fru_product_info_t product;
  /// @brief storage_get_fruid_data 함수 호출 시 클라이언트로 FRU Inventory Data
  /// 및 header 정보를 보낼 때 보낸 데이터의 길이를 저장하는 변수
  unsigned char temp_write = 0;
  unsigned char header_flag = 0;
  unsigned char edit_flag = 0;
  /// @brief IPMI FRU 보드정보 버퍼
  char fru_board_buf[256] = {0};
  /// @brief IPMI FRU 샤시정보 버퍼
  char fru_chassis_buf[256] = {0};
  /// @brief IPMI FRU 프로덕트정보 버퍼
  char fru_product_buf[256] = {0};

  /// @brief 샤시, 보드, 프로덕트별 FRU Inventory Data 길이를 저장하는 변수
  int chassis_total, board_total, product_total = {0};

  /// @brief plat_fru_device_init 함수 호출 시 chassis, board, product 정보가
  /// 있을 경우 flag를 1로 설정
  unsigned char chassis_flag, board_flag, product_flag = {0};

  /**
   * @brief ipmitool  fru edit 명령어 실행 시 수정된 FRU Inventory Data 를
   * 저장하는 문자열 변수\n ipmitool fru edit 명령어 실행 시 FRU Inventory Data
   * 중 수정할 부분만 새로 수정된 데이터가 Request Message로 전송됩니다.\n
   * Example) <FRU Header><FRU Chassis><FRU Board><FRU Product> 와 같은 배열로
   * 되어있으며, Board 정보를 수정할 경우 <FRU Board>만 교체되어 전체 메세지가
   * 전송됩니다.
   *
   */
  unsigned char write_buffer[255];

  // [테스트] fru chassis type string 
  static vector<string> chassis_type_desc_fru;

  Ipmifru(fru_header_t _fru_header, fru_chassis_info_t _fru_chassis,
          fru_board_info_t _fru_board, fru_product_info_t _product) {
    this->fru_header = _fru_header;
    this->fru_board = _fru_board;
    this->fru_chassis = _fru_chassis;
    this->product = _product;
  }
  std::vector<uint8_t> fru_get_rec_to_vector();
  ~Ipmifru(){};
};
#endif