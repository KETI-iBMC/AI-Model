#include "keti_dbus.hpp"
#include <dbus-c++/interface.h>
#include <dbus-c++/message.h>
#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

using namespace std;
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <limits.h>
#include <math.h>
#include <util/peci-ioctl.h>
//#include <redfish/logservice.hpp>

#define BUFFER_SIZE 64

static const char *devpath = PECI_DEVICE;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;
// static const char *METHOD_NAME = "org.freedesktop.keti.bmc.ADC";
// static const char *METHOD_PATH = "/org/freedesktop/keti/bmc/ADC";
static const char *SERVER_NAME = "org.freedesktop.keti.bmc.dbus";
static const char *SERVER_PATH = "/org/freedesktop/keti/bmc/dbus";


int g_Tmax = 90;

KETI_Dbus::KETI_Dbus(DBus::Connection &connection)
  :  DBus::ObjectAdaptor(connection, SERVER_PATH)
{
}
DBus::BusDispatcher dispatcher;




void niam(int sig)
{
  dispatcher.leave();
}
struct timing_negotiation {
  u8 msg_timing;
  u8 addr_timing;
};
/**
 * @brief limit power
 * @author 박기철
 */
static pthread_mutex_t m_sensor;
int temp_data[64];
int tacho_data[16];
double lm75_data[16];
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

//  enum ads1015_registers {
//    ADS1015_CONVERSION = 0,
//    ADS1015_CONFIG,
//  };

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

//  enum adc_pins {
//    ADC_PIN0 = 0,
//    ADC_PIN1,
//    ADC_PIN2,
//    ADC_PIN3,
//    ADC_PIN4,
//    ADC_PIN5,
//    ADC_PIN6,
//    ADC_PIN7,
//    ADC_PIN8,
//    ADC_PIN9,
//    ADC_PIN10,
//    ADC_PIN11,
//    ADC_PIN12,
//    ADC_PIN13,
//    ADC_PIN14,
//    ADC_PIN15,
//  };

// Helper function for msleep

void msleep(int msec) {
  struct timespec req;

  req.tv_sec = 0;
  req.tv_nsec = msec * 1000 * 1000;

  while (nanosleep(&req, &req) == -1 && errno == EINTR) {
    continue;
  }
}

/**
 * @brief 입력된 하드웨어 디바이스 드라이버의 센서 값를 읽어오는 함수
 *
 * @param device 디바이스 드라이버 경로
 ex)/root/sys/devices/platform/ast_adc.0//adc0_value
 * @param value 센서 값 ex)150
 * @return int 성공 유무 (0 : 성공 / errno : 실패)
 **/
int read_device_int(const char *device, int *value) {
  FILE *fp;
  int rc;
  char tmp[10];
  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
    printf("failed to open device %s", device);
    fclose(fp);
    return err;
  }

  rc = fscanf(fp, "%s", tmp);
  fclose(fp);

  if (rc != 1) {

    // printf("failed to read device %s", device);
    // tacho데이터를 읽을때 0이하는 읽을수없다 ㅇ
    return ENOENT;
  }

  *value = atoi(tmp);
  return 0;
}
/**
 * @brief AST2600 ADC 포팅을 위한 함수
 *
 * @param pin
 * @param device
 * @param value
 * @return int
 */

int read_adc_value_KTNF(const int pin, const char *device, int *value) {
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];
  int val = 0;
  int ret;
  string adc_dir = ADC_DIR;
   //cout<<"read_adc_value_KTNF pin ="<<pin<<endl;
  int real_pin = pin;
  if (pin >= 7) {
    adc_dir = ADC_DIR1;
    real_pin = pin % 8;
  }
  snprintf(device_name, adc_dir.c_str(), device, real_pin);
   cout << "read_adc_value_KTNF : device_name =" << device_name << endl;
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", adc_dir.c_str(),
           device_name);
   cout << "read_adc_value_KTNF : full_name =" << full_name << endl;
  try {
    ret = read_device_int(full_name, &val);
  } catch (const std::exception &e) {
     //cout << "read_adc_value_KTNF error : not exist " << full_name << endl;
  }
  *value = val; //>>2
  return (ret);
}

int KETI_Dbus:: read_adc_value(const int32_t& pin, const std::string& device) {
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];
  int *value;
  int val = 0;
  int ret;

  snprintf(device_name, LARGEST_DEVICE_NAME, device.c_str(), pin);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", ADC_DIR, device_name);
  ret = read_device_int(full_name, &val);
  *value = val; //>>2
  return (value);
}


/**
 * @brief 팬의 RPM 속도를 읽어오는 함수
 *
 * @param pin 팬의 Index 번호
 * @param value 팬 RPM 값
 * @return int 성공 유무
 **/
static int read_tacho_value(const int pin, int *value) {
  char device_name[MAX_DVNAME_LEN] = {0};
  char full_name[MAX_DVNAME_LEN] = {0};
  int ret;
  int val = 0;

  snprintf(device_name, MAX_DVNAME_LEN, TACHO_VALUE, pin);
  // printf("device name : %s\n", device_name);
  snprintf(full_name, MAX_DVNAME_LEN, "%s/%s", TACHO_DIR, device_name);
  // printf("full name : %s\n", full_name);

  ret = read_device_int(full_name, &val);
  *value = val; /// 100;
  return ret;
}

static int i2c_readw(int i2c_dev, int addr) {
  unsigned char val[2] = {0};
  unsigned char rval[2] = {0};
  int ret;

  val[0] = addr;

  write(i2c_dev, &val[0], 1);
  read(i2c_dev, &rval[0], 2);

  ret = rval[0] * 256 + rval[1];
  return ret;
}
static int read_i2c_value(char *device, uint8_t addr, uint8_t type, uint32_t *value) {

  int dev;
  int ret;
  int32_t res;
  // cout << "read_i2c_value device" << device << endl;
  dev = open(device, O_RDWR);
  if (dev < 0) {
    perror("read_i2c_value: open() failed");
    return -1;
  }
  // cout << "read_i2c_value dev" << dev << endl;
  // cout << "read_i2c_value addr" << addr << endl;
  if (ioctl(dev, I2C_SLAVE_FORCE, addr) < 0) {
    perror("ioctl() assigning i2c addr failed");
    close(dev);
    return -1;
  }
  if (i2c_smbus_write_word_data(dev, 0, 255) < 0) {
    perror("i2c_smbus_write_word_data not ");
  }
  unsigned char values;

  values = i2c_smbus_read_word_data(dev, 0x03);
  // printf("Values: X MSB: %d\n", values);

  // cout << "read_i2c_value type" << static_cast<int>(type) << endl;
  res = i2c_readw(dev, type);
  res >>= 4;
  // cout << "read_i2c_value res" << (int)res << endl;
  *value = static_cast<uint32_t>(res);
  // cout << "read_i2c_value value" << (int)*value << endl;
  close(dev);
  return 1;
}
/**
 * @brief TMP75 센서의 온도값을 읽어오는 함수\n
 * TMP75의 주소값을 통해 I2C 통신으로 센서 값을 읽어옴
 *
 * @param device 읽을 센서의 디바이스 드라이버
 * @param addr 센서 주소값
 * @param type 센서 타입
 * @param value 센서 값
 * @return int 성공 유무
 */
int read_temp_value(char *device, uint8_t addr, uint8_t type, uint32_t *value) {
  int ret;
  int val;
  // cout << "read_temp_value" << endl;
  ret = read_i2c_value(device, addr, type, &val);
  // cout << "ret" << endl;
  *value = val >> 3;

  return ret;
}

/**
 *@brief FAN ADC 데이터를 읽는 곳
 *@bug KTNF-AST2600a3보드에선 0~5개까지 존재함
 */
int32_t KETI_Dbus::read_fan_value(const int32_t& fanno) {
  int *value;
  read_tacho_value(fanno, value);
  tacho_data[fanno] = *value;
  // cout << ("read_fan_value: fanno:%d value:%f", fanno, *value);
  return *value;
}
double KETI_Dbus::peci_CPU_TEMP0() {
  char option;
  int ping = 0;
  int temp = 0;
  int id = 0;
  int cust = 0;
  int i = 0;
  int peci_fp;
  uint8_t tx_buf[BUFFER_SIZE];
  temp = 1;
  peci_fp = open(devpath, O_RDWR | O_CLOEXEC);

  if (peci_fp < 0) {
    printf("Couldn't open %s with flags O_RDWR: %s\n", devpath,
           strerror(errno));
    return -errno;
  }
  while (true) {
    if (ping) {
      struct peci_ping_msg ping_msg;
      printf("ping address 0x30 \n");
      ping_msg.addr = 0x30; // Address of the target. Accepted values are 48-55
                            // (0x30-0x37). Default is 48 (0x30)
      if (!ioctl(peci_fp, PECI_IOC_PING, &ping_msg))
        printf("ping ack \n");
      else
        printf("ping nack \n");
    }
    if (temp) {
      struct peci_xfer_msg msg;

      msg.addr = 0x30; // Address of the target. Accepted values are 48-55
                       // (0x30-0x37). Default is 48 (0x30)
      msg.tx_len = 1;
      msg.rx_len = 2;
      msg.tx_buf[0] = 0x1;
      if (!ioctl(peci_fp, PECI_IOC_XFER, &msg)) {
        if (msg.rx_len >= 1)
          return msg.rx_buf[0];
        else
          return 0.0;
      } else {
        printf("temp fail \n");
        return -1.0;
      }
    }
  }
}

#define LM75_REG_TEMP (0x00) // Temperature Register
#define LM75_REG_CONF (0x01) // Configuration Register
#define LM75_ADDR (0x90)     // LM75 address
#define LM75_I2C_BUS "/dev/i2c-6"
int lm75_no_converter[5] = {0x48,0x49,0x4a,0x4e,0x4f};
/**
 * @brief N520_LM75_reading
 *
 * @param slave slave addr
 */
void read_N520_lm75(int no, double *value) {

  int32_t ps32Temp;
  uint8_t u8Buffer[2];
  int fd = open(LM75_I2C_BUS, O_RDWR);
  if (fd <= 0) {
    //log(info) << "read_lm75_value erorr not open" << LM75_I2C_BUS;
  } else {

    if (ioctl(fd, I2C_SLAVE, lm75_no_converter[no]) < 0) {
      //log(info) << "read_lm75_value erorr not I2C_SLAVE " << std::hex << "0x"
      //          << lm75_no_converter[no];
    } else {
      char data_write[2];
      char data_read[2];
      /*Set read pointer to address 0x00 */

      write(fd, (char)0x00, 1);

      if (read(fd, u8Buffer, 2) == 2) {
        int16_t i16 = (u8Buffer[0] << 8) | u8Buffer[1];
        float temp = i16 / 256.0;
        *value=temp;
        // printf("i16 =%x\n", i16);
        // printf("temp =%.3f\n", temp);
        ps32Temp = 0;
      } else {
        //log(info) << "Failed to read from LM75 at bus ";
      }
    }
    close(fd);
  }
}
  /**
   *@brief LM75 ADC 데이터를 읽는 곳
   *@bug KTNF-AST2600a3보드에선 0~5개까지 존재함
   */
double KETI_Dbus::read_lm75_value(const int32_t& no){
  double *value;
  read_N520_lm75(no, value);
  lm75_data[no] = *value;
  return *value;
}
/**
 * @brief PSU WATT FAN 등 다양한 센서값을 읽는 함수
 * @param value 전달할 데이터
 * @param fru fru 센서 정보
 * @param sensor_num fru 구역내 센서 값
 * @bug tacho 만 읽는 FAN CPU TEMP같은 센서값은 read_??_value를 통해 값을
 * 얻어와야 함 임시로구현
 * @author 박기철
 */
int32_t KETI_Dbus::lightning_sensor_read(const uint8_t& fru, const uint8_t& sensor_num) {
  int tmp = 0;
  int *value;
  printf("\nlightning function success\n");
  switch (fru) {
  case FRU_PEB:
    switch (sensor_num) {
    case PEB_SENSOR_ADC_P12V_PSU1:
      read_adc_value_KTNF(ADC_PIN0, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P12V_PSU2:
      read_adc_value_KTNF(ADC_PIN1, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P3V3:
      read_adc_value_KTNF(ADC_PIN2, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P5V:
      read_adc_value_KTNF(ADC_PIN3, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_PCH:
      read_adc_value_KTNF(ADC_PIN4, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V05:
      read_adc_value_KTNF(ADC_PIN5, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V8:
      read_adc_value_KTNF(ADC_PIN6, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_BAT:
      read_adc_value_KTNF(ADC_PIN7, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVCCIN:
      read_adc_value_KTNF(ADC_PIN8, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_PCH_CPU0:
      read_adc_value_KTNF(ADC_PIN9, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V8_NACDELAY:
      read_adc_value_KTNF(ADC_PIN10, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V2_VDDQ:
      read_adc_value_KTNF(ADC_PIN11, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_NAC:
      read_adc_value_KTNF(ADC_PIN12, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V05_NAC:
      read_adc_value_KTNF(ADC_PIN13, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVPP:
      read_adc_value_KTNF(ADC_PIN14, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVTT:
      read_adc_value_KTNF(ADC_PIN15, ADC_VALUE, (int *)value);
      break;
    default:
      break;
    }
    break;

  case FRU_PDPB:
    switch (sensor_num) {
    case PDPB_SENSOR_TEMP_REAR_RIGHT:
      read_temp_value(I2C_DEV_PDPB, PDPB_REAR_RIGHT, LOCAL_SENSOR,
                            (uint32_t *)value);
      break;
    case PDPB_SENSOR_TEMP_CPU_AMBIENT:
      read_temp_value(I2C_DEV_PDPB, PDPB_CPU_AMBIENT, LOCAL_SENSOR,
                            (uint32_t *)value);
      break;
    case PDPB_SENSOR_TEMP_FRONT_RIGHT:
      read_temp_value(I2C_DEV_PDPB, PDPB_FRONT_RIGHT, LOCAL_SENSOR,
                            (uint32_t *)value);
      break;
    case PDPB_SENSOR_TEMP_PCIE_AMBIENT:
      read_temp_value(I2C_DEV_PDPB, PDPB_PCIE_AMBIENT, LOCAL_SENSOR,
                            (uint32_t *)value);
      break;
    case PDPB_SENSOR_TEMP_FRONT_LEFT:
      read_temp_value(I2C_DEV_PDPB, PDPB_FRONT_LEFT, LOCAL_SENSOR,
                            (uint32_t *)value);
      break;
    // peci 변경 N520 board는 CPU가 1개임
    case PDPB_SENSOR_TEMP_CPU0:
      static double tmp=40;
      double tmp_e=(double)peci_CPU_TEMP0();
      cout<<"PDPB_SENSOR_TEMP_CPU0 value="<<tmp_e<<endl;
        if(tmp_e>100| tmp_e<15)
          *value =tmp;
        else
        tmp=tmp_e;
        *value = (int)tmp;
        break;  
    // peci 변경 N520 board는 CPU가 1개임
    case PDPB_SENSOR_TEMP_CPU1:
      //*value = (int)KETI_Dbus::peci_CPU_TEMP0();      
      break;
    case PDPB_SENSOR0_TEMP_LM75:
      //*value = (int)KETI_Dbus::peci_CPU_TEMP0();
      *value = lm75_data[0];      
      break;
    case PDPB_SENSOR1_TEMP_LM75:
      //*value = (int)KETI_Dbus::peci_CPU_TEMP0();
      *value = lm75_data[1];      
      break;
    case PDPB_SENSOR2_TEMP_LM75:
      //*value = (int)KETI_Dbus::peci_CPU_TEMP0();
      *value = lm75_data[2];      
      break;
    case PDPB_SENSOR3_TEMP_LM75:
      *value = lm75_data[3];      
      break;
    case PDPB_SENSOR4_TEMP_LM75:
      *value = lm75_data[4];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0:
      *value = temp_data[2];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1:
      *value = temp_data[3];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2:
      *value = temp_data[4];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0:
      *value = temp_data[5];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1:
      *value = temp_data[6];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2:
      *value = temp_data[7];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0:
      *value = temp_data[8];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1:
      *value = temp_data[9];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2:
      *value = temp_data[10];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0:
      *value = temp_data[11];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1:
      *value = temp_data[12];      
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2:
      *value = temp_data[13];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0:
      *value = temp_data[14];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1:
      *value = temp_data[15];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2:
      *value = temp_data[16];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0:
      *value = temp_data[17];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1:
      *value = temp_data[18];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2:
      *value = temp_data[19];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0:
      *value = temp_data[20];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1:
      *value = temp_data[21];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2:
      *value = temp_data[22];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0:
      *value = temp_data[23];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1:
      *value = temp_data[24];      
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2:
      *value = temp_data[25];      
      break;
    default:
      break;
    }
    break;
  case FRU_NVA:
    switch (sensor_num) {
    // case NVA_SENSOR_PSU1_TEMP:
    //   // cout<<("lightning_sensor_read: %s %02X
    //   // %02X",I2C_PSU_DEV_NVA,NVA_PSU_1,NVA_PSU_TEMP);
    //   ret = read_tmp75_temp_value(
    //       // PDPB_TMP75_PSU1_DEVICE,
    //       // (int *)value);
    //   break;
    // case NVA_SENSOR_PSU2_TEMP:
    //   // cout<<("lightning_sensor_read: %s %02X
    //   // %02X",I2C_PSU_DEV_NVA,NVA_PSU_2,NVA_PSU_TEMP);
    //   // ret = read_tmp75_temp_value(
    //   //     PDPB_TMP75_PSU2_DEVICE,
    //   //     (int *)value);
    // case NVA_SENSOR_PSU1_TEMP2:

    //   break;
    // case NVA_SENSOR_PSU2_TEMP2:
    //   break;
    // case NVA_SENSOR_PSU1_FAN1:
    //   break;

    // case NVA_SENSOR_PSU2_FAN1:
    //   break;
    // case NVA_SENSOR_PSU1_WATT:
    //   *value = tacho_data[8];

    //   break;
    // case NVA_SENSOR_PSU2_WATT:
    //   *value = tacho_data[9];
    //   break;
    case NVA_SYSTEM_FAN1:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[0]);
      *value = tacho_data[0];
      break;
    case NVA_SYSTEM_FAN2:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[1]);
      *value = tacho_data[1];
      break;
    case NVA_SYSTEM_FAN3:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[2]);
      *value = tacho_data[2];
      break;
    case NVA_SYSTEM_FAN4:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[3]);
      *value = tacho_data[3];
      break;
    case NVA_SYSTEM_FAN5:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[4]);
      *value = tacho_data[4];
      break;
    }
    break;
  }
  // pthread_mutex_unlock(&m_sensor);
  return *value;
}

int main()
{

  std::cout << "  Server Start "<< std::endl;

  DBus::default_dispatcher = &dispatcher;

  DBus::Connection conn = DBus::Connection::SystemBus();
  conn.request_name(SERVER_NAME);

  KETI_Dbus server(conn);

  dispatcher.enter();

  return 0;
}
