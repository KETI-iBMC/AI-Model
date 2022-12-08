
#include <cstdint>
#include <vector>
#include <string>


 #define LARGEST_DEVICE_NAME 128
 #define MAX_DVNAME_LEN  128
 
 #define I2C_DEV_PEB       "/dev/i2c-4"
 #define I2C_DEV_PDPB      "/dev/i2c-6"
 #define I2C_DEV_TEM      "/dev/i2c-6" //임시
 #define I2C_DEV_FCB       "/dev/i2c-5"
 #define I2C_DEV_NVA       "/dev/i2c-4"
 #define I2C_PSU_DEV_NVA       "/dev/i2c-7"
 

  #define I2C_ADDR_PEB_HSC 0x11
 
 #define I2C_ADDR_PDPB_ADC  0x48
 
 #define I2C_ADDR_FCB_HSC  0x22
 #define I2C_ADDR_NCT7904  0x2d

 #define I2C_BUS_PEB_DIR "/sys/class/i2c-adapter/i2c-4/"
 #define I2C_BUS_PDPB_DIR "/root/sys/class/i2c-adapter/i2c-6/"//"/sys/class/i2c-adapter/i2c-6/"
 #define I2C_BUS_FCB_DIR "/sys/class/i2c-adapter/i2c-5/"

#define ADC_DIR "/sys/devices/platform/ahb/ahb:apb/1e6e9000.adc/driver/1e6e9000.adc/iio:device0"
#define ADC_DIR1 "/sys/devices/platform/ahb/ahb:apb/1e6e9000.adc/driver/1e6e9100.adc/iio:device1"
#define ADC_VALUE "in_voltage%d_raw"
 //tacho는 현재 ast2600a3 보드에 있지않음으로 읽히지 않음
 //#define TACHO_DIR "/root/sys/devices/platform/ast_pwm_tacho.0"//"/sys/devices/platform/ast_pwm_tacho.0"
 #define PWM_DIR "/sys/devices/platform/ahb/ahb:apb/1e610000.pwm-tacho-controller/hwmon/hwmon0"
//  #define TACHO_DIR "/sys/devices/platform/ahb/ahb:apb/1e610000.pwm-tacho-controller/hwmon/hwmon0"
 #define TACHO_DIR "/sys/devices/platform/ahb/ahb:apb/1e610000.pwm-tacho-controller/hwmon/hwmon1"
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
enum {
  FRU_ALL = 0,
  FRU_PEB = 1,
  FRU_PDPB = 2,
  FRU_FCB = 3,
  FRU_NVA = 4,
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
enum {
  SENSOR_VALID = 0x0,
  UCR_THRESH = 0x01,
  UNC_THRESH,
  UNR_THRESH,
  LCR_THRESH,
  LNC_THRESH,
  LNR_THRESH,
  POS_HYST,
  NEG_HYST,
};
/**
 * @brief DIMM(dual in-line memory module) 등 센서 , psu
 */
enum {
  PDPB_SENSOR_TEMP_CPU0 = 0x01,
  PDPB_SENSOR_TEMP_CPU1 = 0x02,
  PDPB_SENSOR0_TEMP_LM75=0x40,
  PDPB_SENSOR1_TEMP_LM75,
  PDPB_SENSOR2_TEMP_LM75,
  PDPB_SENSOR3_TEMP_LM75,
  PDPB_SENSOR4_TEMP_LM75,
  PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2,
  PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2,
  PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2,
  PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2,
};

enum {
  PDPB_SENSOR_TEMP_REAR_RIGHT = 0x30,
  PDPB_SENSOR_TEMP_CPU_AMBIENT,
  PDPB_SENSOR_TEMP_FRONT_RIGHT,
  PDPB_SENSOR_TEMP_PCIE_AMBIENT,
  PDPB_SENSOR_TEMP_FRONT_LEFT,

};
enum tmp75_peb_sensors {
    PEB_TMP75_U136 = 0x4d,
    PEB_TMP75_U134 = 0x4a,
    PEB_TMP421_U15 = 0x4c,
    PEB_MAX6654_U4 = 0x18,
};
// Sensors KTNF under PEB
enum {
  PEB_SENSOR_ADC_P12V_PSU1 = 0x20,
  PEB_SENSOR_ADC_P12V_PSU2 = 0x21,
  PEB_SENSOR_ADC_P3V3 = 0x22,
  PEB_SENSOR_ADC_P5V = 0x23,
  PEB_SENSOR_ADC_PVNN_PCH = 0x24,
  PEB_SENSOR_ADC_P1V05 = 0x25,
  PEB_SENSOR_ADC_P1V8 = 0x26,
  PEB_SENSOR_ADC_BAT = 0x27,
  PEB_SENSOR_ADC_PVCCIN = 0x28,
  PEB_SENSOR_ADC_PVNN_PCH_CPU0 = 0x29,
  PEB_SENSOR_ADC_P1V8_NACDELAY = 0x2A,
  PEB_SENSOR_ADC_P1V2_VDDQ = 0x2B,
  PEB_SENSOR_ADC_PVNN_NAC = 0x2C,
  PEB_SENSOR_ADC_P1V05_NAC = 0x2D,
  PEB_SENSOR_ADC_PVPP = 0x2E,
  PEB_SENSOR_ADC_PVTT = 0x2F,
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
typedef struct {
  long data_type;
  uint8_t sensor_num;
  uint8_t value;
} smltr_data_t;
enum {
    NVA_SENSOR_TEMP1 = 0x65,
    NVA_SENSOR_TEMP2 = 0x66,
    NVA_SENSOR_TEMP3 = 0x67,
    NVA_SENSOR_TEMP4 = 0x68,
    NVA_SENSOR_PSU1_TEMP = 0x90,
    NVA_SENSOR_PSU2_TEMP = 0x91,
    NVA_SENSOR_PSU1_FAN1 = 0x92,
    NVA_SENSOR_PSU1_FAN2 = 0x93,
    NVA_SENSOR_PSU1_WATT = 0x94,
    NVA_SENSOR_PSU2_WATT = 0x95,
    NVA_SENSOR_PSU2_FAN1 = 0x96,
    NVA_SENSOR_PSU2_FAN2 = 0x97,
    NVA_SENSOR_PSU1_TEMP2 = 0x98,
    NVA_SENSOR_PSU2_TEMP2 = 0x99,
  NVA_SYSTEM_FAN1 = 0xA0,
  NVA_SYSTEM_FAN2 = 0xA1,
  NVA_SYSTEM_FAN3 = 0xA2,
  NVA_SYSTEM_FAN4 = 0xA3,
  NVA_SYSTEM_FAN5 = 0xA4,
};


enum psu_nva_regs{
    NVA_PSU_TEMP=0x8d,
    NVA_PSU_TEMP2=0x8e,
    NVA_PSU_FAN1=0x90,
    NVA_PSU_FAN2=0x91,
    NVA_PSU_WATT=0x96,
};

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;
struct timing_negotiation {
  u8 msg_timing;
  u8 addr_timing;
};
/**
 * @brief limit power
 * @author 박기철
 */
int g_Tmax = 90;
static pthread_mutex_t m_sensor;
int temp_data[64];
int tacho_data[16];

enum temp_sensor_type {
  LOCAL_SENSOR = 0,
  REMOTE_SENSOR,
};

enum nct7904_registers {
  NCT7904_TEMP_CH1 = 0x42,
  NCT7904_TEMP_CH2 = 0x46,
  NCT7904_VSEN6 = 0x4A,
  NCT7904_VSEN7 = 0x4C,
  NCT7904_VSEN9 = 0x50,
  NCT7904_3VDD = 0x5C,
  NCT7904_MONITOR_FLAG = 0xBA,
  NCT7904_BANK_SEL = 0xFF,
};

enum hsc_controllers {
  HSC_ADM1276 = 0,
  HSC_ADM1278,
};

enum hsc_commands {
  HSC_IN_VOLT = 0x88,
  HSC_OUT_CURR = 0x8c,
  HSC_IN_POWER = 0x97,
  HSC_TEMP = 0x8D,
};


enum ads1015_channels {
  ADS1015_CHANNEL0 = 0,
  ADS1015_CHANNEL1,
  ADS1015_CHANNEL2,
  ADS1015_CHANNEL3,
  ADS1015_CHANNEL4,
  ADS1015_CHANNEL5,
  ADS1015_CHANNEL6,
  ADS1015_CHANNEL7,
};
