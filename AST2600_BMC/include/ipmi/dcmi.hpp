#ifndef __DCMI_HPP__
#define __DCMI_HPP__

#include <sys/sysinfo.h>
#include "ipmi/common.hpp"
#include "ipmi/sdr.hpp"
#include "ipmi/ipmi.hpp"

#define DCMI_SIZE_GRP_ID 1

#define DCMI_SIZE_GUID_TIME_L 4
#define DCMI_SIZE_GUID_TIME_M 2
#define DCMI_SIZE_GUID_TIME_H 2
#define DCMI_SIZE_GUID_CLOCK_SQ_LH 2
#define DCMI_SIZE_GUID_NODE 6

#define DCMI_SIZE_CAPA_CONFORM 2
#define DCMI_SIZE_CAPA_REV 1
#define DCMI_SIZE_CAPA_DATA 5

#define DCMI_SIZE_POWER_RD_WATT 2
#define DCMI_SIZE_POWER_RD_MIN 2
#define DCMI_SIZE_POWER_RD_MAX 2
#define DCMI_SIZE_POWER_RD_AVG 2
#define DCMI_SIZE_POWER_RD_PERIOD 4
#define DCMI_SIZE_POWER_RD_STATE 1

#define DCMI_SIZE_CONF_DHCP_ACTIVE 1
#define DCMI_SIZE_CONF_DHCP_DISCOVER 1
#define DCMI_SIZE_DHCP_TIMING 5

#define DCMI_SIZE_ASSET_TAG 64
#define DCMI_SIZE_MNG_CTRL 64

#define DCMI_SIZE_POWER_LMT_RESERVED 2
#define DCMI_SIZE_POWER_LMT_ACTION 1
#define DCMI_SIZE_POWER_LMT_LIMIT 2
#define DCMI_SIZE_POWER_LMT_CORRECTION 4
#define DCMI_SIZE_POWER_LMT_SAMPLE 2

class Dcmiconfiguration
{
     public:
        std::vector<uint8_t> dcmi_guid; // UUID와 동일함.
        std::vector<uint8_t> dcmi_capa;
        std::vector<uint8_t> dcmi_mandatory_capa;
        std::vector<uint8_t> dcmi_optional_capa;
        std::vector<uint8_t> dcmi_mgnt_access_capa;
        std::vector<uint8_t> dcmi_power_rd;
        std::vector<uint8_t> dcmi_conf_prm;
        std::vector<uint8_t> dcmi_asset_tag;
        std::vector<uint8_t> dcmi_mngctrl;
        std::vector<uint8_t> dcmi_power_lmt;
        

   
        uint8_t power_reading_state;
        Dcmiconfiguration();
        ~Dcmiconfiguration()
        {}
        
        void dcmi_discover(uint8_t *request, uint8_t *response, uint8_t *res_len);
        void dcmi_set_power_limit(uint8_t *request, uint8_t *response, uint8_t *res_len);
        void dcmi_get_power_reading(uint8_t *request, uint8_t *response, uint8_t *res_len);
        void dcmi_set_mngctrl(uint8_t *request, uint8_t *response, uint8_t *res_len);
        void dcmi_get_mngctrl(uint8_t *request, uint8_t *response, uint8_t *res_len);
        void plat_dcmi_capa_init();
        void plat_dcmi_power_lmt_init();
        /**
* @brief DCMI 전체 정보 저장 함수. DCMI 관련 정보를 모두 파일에 분리하여 저장한다.\n
(1)	DCMI_CAPA_PATH - dcmi_capable_vals 정보 저장\n
(2)	DCMI_MANDATORY_CAPA_PATH - dcmi_mandatory_plat_capa 정보 저장\n
(3)	DCMI_OPTION_CAPA_PATH - dcmi_optional_plat_capa 정보 저장\n
(4)	DCMI_MGNT_CAPA_PATH - dcmi_management_access_capa정보 저장\n
(5)	DCMI_POWER_LIMIT_PATH - dcmi_power_lmt 정보 저장\n
(6)	DCMI_ASSET_MNGCTRL_PATH - dcmi_asset_mngctrl 정보 저장\n
* @param void
* @return void
*/
        void plat_dcmi_data_save(void);

        /**
* @brief DCMI Power Limit을 설정하는 함수.\n
IPMI Request 데이터에 따라 Power limit, Sample Time, Action, Correction 등 기능을 설정한다.\n
호출 CMD : CMD_DCMI_POWER_SET_LIMIT (0x04)\n
* @param unsigned char *request - IPMI Request message pointer\n
* @param unsigned char *response - IPMI Response message pointer\n
* @param unsigned char *res_len - IPMI Response message length\n
* @return void
*/
        void dcmi_power_set_limit(unsigned char *request, unsigned char *response, unsigned char *res_len);

        /**
* @brief DCMI Power Limit 설정 정보를 반환하는 함수.\n
dcmi_power_limit_t 구조체에 Power Limit에 관련된 정보가 모두 포함되어 있으며, DCMI Power Reading 기능이 비활성화 된 경우 Invalid Command로 설정되어 정보가 표시되지 않는다.\n
호출 CMD : CMD_DCMI_POWER_GET_LIMIT (0x03)\n
* @param unsigned char *request - IPMI Request message pointer\n
* @param unsigned char *response - IPMI Response message pointer\n
* @param unsigned char *res_len - IPMI Response message length\n
* @return void
*/
        void dcmi_power_get_limit(unsigned char *request, unsigned char *response, unsigned char *res_len);
        /**
* @brief DCMI Power Limit 기능을 활성화 시키는 함수.\n
초기 상태에는 DCMI Power Limit 기능이 비활성화 되어있기 때문에 dcmi_power_activate 기능을 호출하여 활성화 시켜야 Power Limit 기능이 활성화된다.\n
Deactivate 명령 전송 시 Power Limit 기능이 비활성화된다.\n
IPMI Request Data [1] << 6 값을 power_reading_state에 반영하여 Power Limit 기능을 활성화 / 비활성화 한다.\n
호출 CMD : CMD_DCMI_POWER_ACT (0x05)\n
* @param unsigned char *request - IPMI Request message pointer\n
* @param unsigned char *response - IPMI Response message pointer\n
* @param unsigned char *res_len - IPMI Response message length\n
* @return void
*/
        void dcmi_power_activate(unsigned char *request, unsigned char *response, unsigned char *res_len);
        /**
* @brief DCMI Power Reading 값을 반환하는 함수.\n
전원이 켜진 이후 PSU 전원 사용량을 측정하여 반환한다.\n
본 함수에 포함되어있는 기능은 현재 전원 사용량, 최대 전력량, 최소 전력량, 평균 전력량 등이 포함되어있다.\n
호출 CMD : CMD_DCMI_POWER_READING (0x02)\n
* @param unsigned char *request - IPMI Request message pointer\n
* @param unsigned char *response - IPMI Response message pointer\n
* @param unsigned char *res_len - IPMI Response message length\n
* @return void
*/
        void dcmi_power_reading(unsigned char *request, unsigned char *response, unsigned char *res_len);
        /**
* @brief DCMI Group Extension 값을 반환하는 함수.\n
호출 CMD : CMD_DCMI_GETSNSR (0x07)\n
* @param unsigned char *request - IPMI Request message pointer\n
* @param unsigned char *response - IPMI Response message pointer\n
* @param unsigned char *res_len - IPMI Response message length\n
* @return void
*/
        void dcmi_get_disvry(unsigned char *request, unsigned char *response, unsigned char *res_len);
        /**
* @brief DCMI Asset Tag 설정 값을 반환하는 함수.\n
호출 CMD : CMD_DCMI_GETASSET(0x06)\n
* @param unsigned char *request - IPMI Request message pointer\n
* @param unsigned char *response - IPMI Response message pointer\n
* @param unsigned char *res_len - IPMI Response message length\n
* @return void
*/
};

void ipmi_handle_dcmi(uint8_t *request, uint8_t *response, uint8_t *res_len);
void plat_dcmi_init();
void dcmi_power_reading (unsigned char *request, unsigned char *response, unsigned char *res_len);
enum
{
    CMD_DCMI_DISCOVER = 0x00,
    CMD_DCMI_COMPACT = 0x01,
    CMD_DCMI_POWER_READING = 0x02,
    CMD_DCMI_POWER_GET_LIMIT = 0x03,
    CMD_DCMI_POWER_SET_LIMIT = 0x04,
    CMD_DCMI_POWER_ACT = 0x05,
    CMD_DCMI_GETASSET = 0x06,
    CMD_DCMI_GETSNSR = 0x07,
    CMD_DCMI_SETASSET = 0x08,
    CMD_DCMI_GETMNGCTRLIDS = 0x09,
    CMD_DCMI_SETMNGCTRLIDS = 0x0A,
    CMD_DCMI_SETTERMALLIMIT = 0x0B,
    CMD_DCMI_GETTERMALLIMIT = 0x0C,
    CMD_DCMI_GETTEMPRED = 0x10,
    CMD_DCMI_SETCONFPARAM = 0x12,
    CMD_DCMI_GETCONFPARAM = 0x13,
};

#endif