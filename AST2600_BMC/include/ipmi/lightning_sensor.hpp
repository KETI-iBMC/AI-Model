#ifndef __LIGHTNING_SENSOR_H__
#define __LIGHTNING_SENSOR_H__

#include "ipmi/common.hpp"
#include <ipmi/sensor_define.hpp>
#include <ipmi/ipmb.hpp>
#include <ipmi/ipmi.hpp> // sdr structure + ipmi command
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <pthread.h>
//#include <ipmi/ipmb.h>

 

 #include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
 #define LARGEST_DEVICE_NAME 128
 #define MAX_DVNAME_LEN  128
 
 #define I2C_DEV_PEB       "/dev/i2c-4"
 #define I2C_DEV_PDPB      "/dev/i2c-6"
 #define I2C_DEV_TEM      "/dev/i2c-6" //임시
 #define I2C_DEV_FCB       "/dev/i2c-5"
 #define I2C_DEV_NVA       "/dev/i2c-4"
 #define I2C_PSU_DEV_NVA       "/dev/i2c-7"
 
 #define I2C_BUS_PEB_DIR "/sys/class/i2c-adapter/i2c-4/"
 #define I2C_BUS_PDPB_DIR "/root/sys/class/i2c-adapter/i2c-6/"//"/sys/class/i2c-adapter/i2c-6/"
 #define I2C_BUS_FCB_DIR "/sys/class/i2c-adapter/i2c-5/"
 


 #define I2C_ADDR_PEB_HSC 0x11
 
 #define I2C_ADDR_PDPB_ADC  0x48
 
 #define I2C_ADDR_FCB_HSC  0x22
 #define I2C_ADDR_NCT7904  0x2d
 
 //#define ADC_DIR "/root/sys/devices/platform/ast_adc.0/"//"/sys/devices/platform/ast-adc.0"
 #define ADC_DIR "/sys/devices/platform/ahb/ahb:apb/1e6e9000.adc/driver/1e6e9000.adc/iio:device0"
  #define ADC_DIR1 "/sys/devices/platform/ahb/ahb:apb/1e6e9000.adc/driver/1e6e9100.adc/iio:device1"
 #define ADC_VALUE "in_voltage%d_raw"
 //tacho는 현재 ast2600a3 보드에 있지않음으로 읽히지 않음
 //#define TACHO_DIR "/root/sys/devices/platform/ast_pwm_tacho.0"//"/sys/devices/platform/ast_pwm_tacho.0"
 #define PWM_DIR "/sys/devices/platform/ahb/ahb:apb/1e610000.pwm-tacho-controller/hwmon/hwmon0"
 #define TACHO_DIR "/sys/devices/platform/ahb/ahb:apb/1e610000.pwm-tacho-controller/hwmon/hwmon0"
 #define PWM_VALUE "pwm%d"
 #define TACHO_VALUE "fan%d_input"
 #define UNIT_DIV 1000
 #define TPM75_TEMP_RESOLUTION 0.0625
 #define ADS1015_DEFAULT_CONFIG 0xe383
 #define MAX_SENSOR_NUM 0xFF
 #define ALL_BYTES 0xFF
 #define LAST_REC_ID 0xFFFF
 #define PEB_TMP75_U136_DEVICE I2C_BUS_PEB_DIR "4-004d"
 #define PEB_TMP75_U134_DEVICE I2C_BUS_PEB_DIR "4-004a"
 
 #define PDPB_TMP75_U47_DEVICE I2C_BUS_PDPB_DIR "6-0049"
 #define PDPB_TMP75_U48_DEVICE I2C_BUS_PDPB_DIR "6-004a"
 #define PDPB_TMP75_U49_DEVICE I2C_BUS_PDPB_DIR "6-004b"
 #define PDPB_TMP75_U51_DEVICE I2C_BUS_PDPB_DIR "6-004c"
 #define PDPB_TMP75_U52_DEVICE I2C_BUS_PDPB_DIR "6-004d"
 #define PDPB_TMP75_U53_DEVICE I2C_BUS_PDPB_DIR "6-004e"
 #define PDPB_TMP75_PSU1_DEVICE I2C_BUS_PDPB_DIR "6-004f"
 #define PDPB_TMP75_PSU2_DEVICE I2C_BUS_PDPB_DIR "6-0050"
 
 #define FAN_REGISTER 0x80
#define MAX_SDR_LEN           64
#define MAX_SENSOR_NUM        0xFF
#define MAX_SENSOR_THRESHOLD  8
#define MAX_RETRIES_SDR_INIT  30
#define THERMAL_CONSTANT      255
#define ERR_NOT_READY         -2

#define UPPER_TRAY 0x00
#define LOWER_TRAY 0x01

#define LOWER_TRAY_DONE 0x00
#define UPPER_TRAY_DONE 0x01

#define MAX_RETRY_TIMES 3

/**
 * @brief Full Sensor SDR record; IPMI/Section 43.1
 * @bug ipmi sdr old version so will be soon reorganizition
 * @author kicheol
 */
typedef struct {
  // Sensor Record Header
  unsigned char rec_id[2];
  unsigned char ver;
  unsigned char type;
  unsigned char len;
  // Record Key Bytes
  unsigned char owner;
  unsigned char lun;
  unsigned char sensor_num;
  // Record Body Bytes
  unsigned char ent_id;
  unsigned char ent_inst;
  unsigned char sensor_init;
  unsigned char sensor_caps;
  unsigned char sensor_type;
  unsigned char evt_read_type;
  unsigned char lt_read_mask[2];
  unsigned char ut_read_mask[2];
  unsigned char set_thresh_mask[2];
  unsigned char sensor_units1;
  unsigned char sensor_units2;
  unsigned char sensor_units3;
  unsigned char linear;
  unsigned char m_val;
  unsigned char m_tolerance;
  unsigned char b_val;
  unsigned char b_accuracy;
  unsigned char accuracy_dir;
  unsigned char rb_exp;
  unsigned char analog_flags;
  unsigned char nominal;
  unsigned char normal_max;
  unsigned char normal_min;
  unsigned char max_reading;
  unsigned char min_reading;
  unsigned char unr_thresh;
  unsigned char uc_thresh;
  unsigned char unc_thresh;
  unsigned char lnr_thresh;
  unsigned char lc_thresh;
  unsigned char lnc_thresh;
  unsigned char pos_hyst;
  unsigned char neg_hyst;
  unsigned char rsvd[2];
  unsigned char oem;
  unsigned char str_type_len;
  char str[16];
} sdr_full_t;

enum tmp75_peb_sensors {
    PEB_TMP75_U136 = 0x4d,
    PEB_TMP75_U134 = 0x4a,
    PEB_TMP421_U15 = 0x4c,
    PEB_MAX6654_U4 = 0x18,
};

enum tmp75_pdpb_sensors {
    PDPB_TMP75_U70 = 0x4f,
    PDPB_TMP75_U71 = 0x49,
    PDPB_TMP75_U72 = 0x4A,
    PDPB_TMP75_U73 = 0x48,
    PDPB_TMP75_U69 = 0x4E,// check duplicate address
    PDPB_TMP75_U205 = 0x4C,
};
/**
 * @brief KTNF AST2600a3 타겟보드의 LM75 센서값
 * 
 */
enum tmp75_pdpb_KTNF_sensors{
    PDPB_REAR_RIGHT =0x4e,
    PDPB_CPU_AMBIENT =0x4f,
    PDPB_FRONT_RIGHT =0X49,
    PDPB_PCIE_AMBIENT = 0X4A,
    PDPB_FRONT_LEFT = 0X48,
};

enum adc_pins {
    ADC_PIN0 = 0,
    ADC_PIN1,
    ADC_PIN2,
    ADC_PIN3,
    ADC_PIN4,
    ADC_PIN5,
    ADC_PIN6,
    ADC_PIN7,
    ADC_PIN8,
    ADC_PIN9,
    ADC_PIN10,
    ADC_PIN11,
    ADC_PIN12,
    ADC_PIN13,
    ADC_PIN14,
    ADC_PIN15,
};

enum tmp75_nva_sensors {
    NVA_TMP75_1 = 0x48,
    NVA_TMP75_2 = 0x49,
    NVA_TMP75_3 = 0x4a,
    NVA_TMP75_4 = 0x4b,
};

// enum psu_nva_sensors {
//     NVA_PSU_1 = 0x58,
//     NVA_PSU_2 = 0x59,
// };

enum psu_nva_regs{
    NVA_PSU_TEMP=0x8d,
    NVA_PSU_TEMP2=0x8e,
    NVA_PSU_FAN1=0x90,
    NVA_PSU_FAN2=0x91,
    NVA_PSU_WATT=0x96,
};


typedef struct _sensor_info_t {
  bool valid;
  sdr_full_t sdr;
} sensor_info_t;

extern const uint8_t peb_sensor_pmc_list[];

extern const uint8_t peb_sensor_plx_list[];

extern const uint8_t pdpb_u2_sensor_list[];

extern const uint8_t pdpb_m2_sensor_list[];

extern const uint8_t fcb_sensor_list[];

extern const uint8_t nva_sensor_list[];

extern size_t peb_sensor_pmc_cnt;

extern size_t peb_sensor_plx_cnt;

extern size_t pdpb_u2_sensor_cnt;

extern size_t pdpb_m2_sensor_cnt;

extern size_t fcb_sensor_cnt;

extern size_t nva_sensor_cnt;





int lightning_sensor_name(uint8_t fru, uint8_t sensor_num, char *name);
int lightning_sensor_units(uint8_t fru, uint8_t sensor_num, char *units);

/**
 * @bug 필요시 구현 요망
 */
int lightning_sensor_sdr_path(uint8_t fru, char *path);
int lightning_sensor_threshold(uint8_t fru, uint8_t sensor_num, uint8_t thresh, float *value);
int lightning_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo);

void sensor_thresh_array_init();


void get_ipmb_sensor(int id, unsigned char *response, unsigned char res_len);

void get_ipmb_sensor1(int id, unsigned char *response, unsigned char res_len);

int read_tmp75_temp_value(const char *device, int *value);
int set_value_to_SMLTR(uint8_t sid, uint8_t val);
int read_adc_value(const int pin, const char *device, int *value);
#endif /* __LIGHTNING_SENSOR_H__ */