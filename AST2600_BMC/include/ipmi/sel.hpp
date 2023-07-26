/**
 * @file sel.hpp
 * @brief sel(System Event Log)에 대한 설명
 * @date 2021-04-13
 * @brief 본 문서는 x86 서버보드 기술 개발의 KETI-IPMI 소프트웨어에 대한 설명입니다.\n
 * FRU 헤더파일에 대한 설명이  포함되어있습니다.
 * @version 0.0.1
 */
#pragma once
#ifndef SEL_UNIQUE_NAME_HERE
#define SEL_UNIQUE_NAME_HERE
#include "ipmi/efte.hpp"
#include <ipmi/common.hpp>
#include <ipmi/event_string.hpp>
#include <ipmi/ipmi.hpp>

/**
 * global enable
 */
void plat_globalenable_save(void);

/**
 * Req
 * - Generator ID(RqSA RqLUN)
 * 1 EvMRev 0x04 IPMI2.0 0x04 IPMI 0x03
 * 2 SensorType
 * 3 Sensor #
 * 4 Event Dir | Event Type
 * 5-7 EventData
 * Res
 * 1 Completion Code
 *
 */

#define IPMI_SEL_PATH "/conf/ipmi/sel.bin"
#define IPMI_SEL_VERSION 0x51

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

/// @brief Record ID can not be 0x0 (IPMI/Section 31)
#define SEL_RECID_MIN (SEL_INDEX_MIN + 1)
#define SEL_RECID_MAX (SEL_INDEX_MAX + 1)

/// @brief Special RecID value for first and last (IPMI/Section 31)
#define SEL_RECID_FIRST 0x0000
#define SEL_RECID_LAST 0xFFFF
#define SIZE_SEL_REC 16

void plat_sel_power_reset_add_entry();
void plat_sel_os_off_add_entry();
void plat_sel_power_limit_add_entry();
void plat_sel_power_off_add_entry();
void plat_sel_power_add_entry();

void plat_sel_ts_recent_add(time_stamp_t *ts);
void plat_sel_ts_recent_erase(time_stamp_t *ts);

int plat_sel_free_space(void);
int plat_sel_rsv_id();
int plat_sel_get_entry(int read_rec_id, sel_msg_t *msg, int *next_rec_id);
int plat_sel_add_entry(sel_msg_t *msg, int *rec_id);
int plat_sel_erase(int rsv_id);
int plat_sel_erase_status(int rsv_id, sel_erase_stat_t *status);
int plat_sel_time_get(sel_msg_t *msg);
int plat_sel_time_set(sel_msg_t *msg);
int plat_sel_init(void);
void sensor_event_get_num(ipmi_req_t *request, ipmi_res_t *response,
                          uint8_t *res_len);
void storage_set_time(ipmi_req_t *request, ipmi_res_t *response,
                      uint8_t *res_len);
void storage_get_time(ipmi_req_t *request, ipmi_res_t *response,
                      uint8_t *res_len);
void storage_clr_sel(ipmi_req_t *request, ipmi_res_t *response,
                     uint8_t *res_len);
void storage_del_sel_entry(ipmi_req_t *request, ipmi_res_t *response,
                           uint8_t *res_len);
void storage_add_sel(ipmi_req_t *request, ipmi_res_t *response,
                     uint8_t *res_len);
void storage_get_sel(ipmi_req_t *request, ipmi_res_t *response,
                     uint8_t *res_len);
void storage_rsv_sel(ipmi_res_t *response, uint8_t *res_len);
void storage_get_sel_info(ipmi_res_t *response, uint8_t *res_len);
int rest_get_eventlog_config(char *ret);
void storage_add_event(ipmi_req_t *request, ipmi_res_t *response,
                       uint8_t *res_len);
void plat_sel_last_rsv_id(unsigned char *data);
void plat_sel_threshold_add_entry(Sensor_tpye se_tpye, int se_num,
                                  Theshold_type th_type);

void ipmi_find_sel_desc(uint8_t *offset, uint8_t *data, uint8_t se_type,
                        const char *ev_desc, uint8_t ev_type);
void plat_sel_generater_add_entry(const char *ev_desc, uint8_t ev_type);
void ipmi_get_event_desc(char *desc, sel_msg_t *rec);

class IPMI_Handle_Event{
    public:
        static void ipmi_handle_sel(ipmi_req_t *request, ipmi_res_t *response, uint8_t *res_len);
        static void ipmi_kernel_panic();
};
#endif