#ifndef __SENSOR_DEFINE_HPP__
#define __SENSOR_DEFINE_HPP__
#pragma once
#include <ipmi/ipmi.hpp>
#include <ipmi/common.hpp>
#include<util/iniparser.hpp>
#define SENSOR_INI "/conf/ipmi/sensor.ini"
#define SENSOR_FRU_MAX 1 //fru
#define SENSOR_TYPE_TEMPERATURE		0x01
#define SENSOR_TYPE_VOLTAGE				0x02 
#define SENSOR_TYPE_CURRENT				0x03
#define SENSOR_TYPE_FAN						0x04
#define SENSOR_TYPE_POWER_UNIT		0x09
#define SENSOR_TYPE_FRU             0x0a
#define SENSOR_EVENT_READING_THRESHOLD				0x01
#define SENSOR_EVENT_READING_GENERIC					0x02 // ~ 0x0C
#define SENSOR_EVENT_READING_SENSOR_SPECIFIC	0x6F

#define SENSOR_EVENT_ASSERTION_LS 				0x85 // 
#define SENSOR_EVENT_ASSERTION_MS 				0x02
#define SENSOR_EVENT_ASSERTION_LS_NL 			0x80 // Lower event none LS
#define SENSOR_EVENT_ASSERTION_MS_NL 			0x01 // Lower event none MS
#define SENSOR_MASK_READ_SETTABLE_LS_NL 	0x18
#define SENSOR_MASK_READ_SETTABLE_MS_NL 	0x00
#define SENSOR_MASK_READ_SETTABLE_LS 			0x1B
#define SENSOR_MASK_READ_SETTABLE_MS 			0x00
#define SENSOR_MASK_READ_SETTABLE_LS_NONE 0x00 // 
#define SENSOR_MASK_READ_SETTABLE_MS_NONE 0x00

#define SENSOR_UNIT_CELSIUS			0x01
#define SENSOR_UNIT_VOLTS				0x04 
#define SENSOR_UNIT_AMPS				0x05 
#define SENSOR_UNIT_WATTS				0x06 
#define SENSOR_UNIT_RPM					0x12 

#define M_VAL_BASE 0
#define M_TOLERANCE 2

#define PEB_SENSOR_NUM_BASE PEB_SENSOR_ADC_P12V_PSU1

#define P12V_VOUT_UNR_THRESHOLD 0xE4 // 14.4
#define P12V_VOUT_UCR_THRESHOLD 0xDA // 13.8
#define P12V_VOUT_UNC_THRESHOLD 0xD1 // 13.2
#define P12V_VOUT_LNC_THRESHOLD 0xAB // 10.8
#define P12V_VOUT_LCR_THRESHOLD 0xA1 // 10.2
#define P12V_VOUT_LNR_THRESHOLD 0x98 // 9.6

#define P5V_VOUT_UNR_THRESHOLD  0xE6 // 3.98
#define P5V_VOUT_UCR_THRESHOLD  0xDB // 3.8
#define P5V_VOUT_UNC_THRESHOLD 0xD1 // 3.62
#define P5V_VOUT_LNC_THRESHOLD 0xAB // 2.96
#define P5V_VOUT_LCR_THRESHOLD 0xA2 // 2.8
#define P5V_VOUT_LNR_THRESHOLD 0x99 // 2.66

#define P3V3_VOUT_UNR_THRESHOLD 0xE6 // 3.98
#define P3V3_VOUT_UCR_THRESHOLD 0xDB // 3.8
#define P3V3_VOUT_UNC_THRESHOLD 0xD1 // 3.62
#define P3V3_VOUT_LNC_THRESHOLD 0xAB // 2.96
#define P3V3_VOUT_LCR_THRESHOLD 0xA2 // 2.8
#define P3V3_VOUT_LNR_THRESHOLD 0x99 // 2.66

#define P3V_VOUT_UNR_THRESHOLD 0xB8 // 3.68
#define P3V_VOUT_UCR_THRESHOLD 0xAF // 3.5
#define P3V_VOUT_UNC_THRESHOLD 0xA6 // 3.32
#define P3V_VOUT_LNC_THRESHOLD 0x85 // 2.66
#define P3V_VOUT_LCR_THRESHOLD 0x7D // 2.5
#define P3V_VOUT_LNR_THRESHOLD 0x76 // 2.36

// PVNN, P1V05
#define PVNN_VOUT_UNR_THRESHOLD 0xBA // 1.3
#define PVNN_VOUT_UCR_THRESHOLD 0xB1 // 1.23
#define PVNN_VOUT_UNC_THRESHOLD 0xAA // 1.19
#define PVNN_VOUT_LNC_THRESHOLD 0x7E // 0.9
#define PVNN_VOUT_LCR_THRESHOLD 0x77 // 0.85
#define PVNN_VOUT_LNR_THRESHOLD 0x70 // 0.76

// P1V8
#define P1V8_VOUT_UNR_THRESHOLD 0xE5 // 2.16
#define P1V8_VOUT_UCR_THRESHOLD 0xDB // 2.07
#define P1V8_VOUT_UNC_THRESHOLD 0xD2 // 1.98
#define P1V8_VOUT_LNC_THRESHOLD 0xA2 // 1.53
#define P1V8_VOUT_LCR_THRESHOLD 0x9A // 1.46
#define P1V8_VOUT_LNR_THRESHOLD 0x98 // 1.44

// PVCCIN CPU0, 1
#define PVCCIN_VOUT_UNR_THRESHOLD 0xE4 // 2.16
#define PVCCIN_VOUT_UCR_THRESHOLD 0xDB // 2.07
#define PVCCIN_VOUT_UNC_THRESHOLD 0xD1 // 1.98
#define PVCCIN_VOUT_LNC_THRESHOLD 0x90 // 1.36
#define PVCCIN_VOUT_LCR_THRESHOLD 0x87 // 1.28
#define PVCCIN_VOUT_LNR_THRESHOLD 0x7F // 1.2

// PVDDQ CPU0, 1
#define PVDDQ_VOUT_UNR_THRESHOLD 0xC7 // 1.42
#define PVDDQ_VOUT_UCR_THRESHOLD 0xBF // 1.36
#define PVDDQ_VOUT_UNC_THRESHOLD 0xB7 // 1.3
#define PVDDQ_VOUT_LNC_THRESHOLD 0x98 // 1.08
#define PVDDQ_VOUT_LCR_THRESHOLD 0x8F // 1.02
#define PVDDQ_VOUT_LNR_THRESHOLD 0x87 // 0.96

// PVCCO CPU 0, 1
#define PVCCO_VOUT_UNR_THRESHOLD 0xB7 // 1.3
#define PVCCO_VOUT_UCR_THRESHOLD 0xAD // 1.23
#define PVCCO_VOUT_UNC_THRESHOLD 0xA7 // 1.19
#define PVCCO_VOUT_LNC_THRESHOLD 0x7E // 0.98
#define PVCCO_VOUT_LCR_THRESHOLD 0x77 // 0.85
#define PVCCO_VOUT_LNR_THRESHOLD 0x6B // 0.76

// PSU_TEMP1, 2
#define PSU_TEMP_UNR_THRESHOLD 0x46
#define PSU_TEMP_UCR_THRESHOLD 0x3C
#define PSU_TEMP_UNC_THRESHOLD 0x37
#define PSU_TEMP_LNC_THRESHOLD 0x5
#define PSU_TEMP_LCR_THRESHOLD 0x2
#define PSU_TEMP_LNR_THRESHOLD 0x0

#define PSU_WATT_UNR_THRESHOLD THRESH_NOT_AVAILABLE
#define PSU_WATT_UCR_THRESHOLD 0xFF
#define PSU_WATT_UNC_THRESHOLD 0xF5
#define PSU_WATT_LNC_THRESHOLD THRESH_NOT_AVAILABLE
#define PSU_WATT_LCR_THRESHOLD THRESH_NOT_AVAILABLE
#define PSU_WATT_LNR_THRESHOLD THRESH_NOT_AVAILABLE

#define NVA_FAN_UNR_THRESHOLD 0x46
#define NVA_FAN_UCR_THRESHOLD 0x3C
#define NVA_FAN_UNC_THRESHOLD 0x37
#define NVA_FAN_LNC_THRESHOLD 0x5
#define NVA_FAN_LCR_THRESHOLD 0x2
#define NVA_FAN_LNR_THRESHOLD 0x0

#define LM75_TEMP_UNR_THRESHOLD 0xB7 // 1.3
#define LM75_TEMP_UCR_THRESHOLD 0xAD // 1.23
#define LM75_TEMP_UNC_THRESHOLD 0xA7 // 1.19
#define LM75_TEMP_LNC_THRESHOLD 0x7E // 0.98
#define LM75_TEMP_LCR_THRESHOLD 0x77 // 0.85
#define LM75_TEMP_LNR_THRESHOLD 0x6B // 0.76

#define PDPB_COMMON_UNR_THRESHOLD THRESH_NOT_AVAILABLE
#define PDPB_COMMON_UCR_THRESHOLD 0x6A
#define PDPB_COMMON_UNC_THRESHOLD 0x58
#define PDPB_COMMON_LNC_THRESHOLD 0xA
#define PDPB_COMMON_LCR_THRESHOLD 0x0
#define PDPB_COMMON_LNR_THRESHOLD THRESH_NOT_AVAILABLE

#define PDPB_CPU_UNR_THRESHOLD 0x5A
#define PDPB_CPU_UCR_THRESHOLD 0x55
#define PDPB_CPU_UNC_THRESHOLD 0x50
#define PDPB_CPU_LNC_THRESHOLD 0xA
#define PDPB_CPU_LCR_THRESHOLD 0x5
#define PDPB_CPU_LNR_THRESHOLD 0x0

#define PDPB_MB_UNR_THRESHOLD 0x50
#define PDPB_MB_UCR_THRESHOLD 0x4E
#define PDPB_MB_UNC_THRESHOLD 0x4B
#define PDPB_MB_LNC_THRESHOLD 0x5
#define PDPB_MB_LCR_THRESHOLD 0x2
#define PDPB_MB_LNR_THRESHOLD 0x0

#define PEB_SENSOR_COUNT 16
#define PDPB_SENSOR_COUNT 7//peci 2개 LPC를통한 나머지 센서 init 필요 2022-08-22
#define NVA_SENSOR_COUNT 5

extern const float m_val_vol[PEB_SENSOR_COUNT];
extern const float input_volt[PEB_SENSOR_COUNT];
extern const float nomi[PEB_SENSOR_COUNT];
extern const float volt[PEB_SENSOR_COUNT];
extern  sensor_thresh_t peb_sensors[PEB_SENSOR_COUNT];
extern  sensor_thresh_t nva_sensors[NVA_SENSOR_COUNT];
extern  sensor_thresh_t pdpb_sensors[PDPB_SENSOR_COUNT];
// sensor_thresh_t fru_sensors[1] = {
//     BMC_SLAVE_ADDR,0x00,0x00,0x0A,0x00,0x7F,0x02,SENSOR_TYPE_FRU,SENSOR_EVENT_READING_THRESHOLD,
//     {SENSOR_EVzENT_ASSERTION_LS, SENSOR_EVENT_ASSERTION_MS},
//     {SENSOR_EVENT_ASSERTION_LS, SENSOR_EVENT_ASSERTION_MS},
//     {SENSOR_MASK_READ_SETTABLE_LS, SENSOR_MASK_READ_SETTABLE_MS},
//     0x00, SENSOR_UNIT_CELSIUS, 0x00, 0x00, 100, 0x00, 0x00, 0x0, 0x01, 0xE0, 0x01,
//     100, 0xFF, 0x00, 0xFF, 0x00,
//     PDPB_MB_UNR_THRESHOLD, PDPB_MB_UCR_THRESHOLD, PDPB_MB_UNC_THRESHOLD,
//     PDPB_MB_LNR_THRESHOLD, PDPB_MB_LCR_THRESHOLD, PDPB_MB_LNC_THRESHOLD,
//     0x00, 0x00, 0x0a, 0xCF, "KR422-SA1"};


bool plat_sensor_init(void);
/**
 * @brief PEB/PDPB/NVA 모두저장하는 함수
 * 
 */
void sensor_threshold_save(void);
#endif