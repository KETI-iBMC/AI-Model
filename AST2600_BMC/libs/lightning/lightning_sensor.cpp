/**
 * @file i2c or gpio를 통한 센서값 리딩 smbus pmbus
 *
 */
#include <ipmi/ipmb.hpp>
#include <ipmi/ipmi.hpp>
#include <ipmi/lightning_sensor.hpp>
#include <redfish/logservice.hpp>
#include <util/peci-ioctl.h>
#include <util/smbus.hpp>
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

int32_t signextend32(uint32_t val, int idx) {
  uint8_t shift = 31 - idx;
  return (int32_t)(val << shift) >> shift;
}

/**
 * @brief 입력된 하드웨어 주소의 센서 값을 읽어오는 함수
 *
 * @param device 센서 하드웨어 주소
 * @param value 센서 값
 * @return int 성공 유무 (0 : 성공 / errno : 실패)
 **/
static int read_device(const char *device, int *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
    printf("failed to open device %s", device);
    return err;
  }
  // int ab=i2c_smbus_read_byte(fp);
  // cout<<"\n\n\n\n\n\n\nsmbus_readbyte = "<<ab<<endl;

  rc = fscanf(fp, "%d", value);
  printf("read device : %d\n", value);
  fclose(fp);

  if (rc != 1) {
    printf("failed to read device %s", device);
    return ENOENT;
  } else {
    return 0;
  }
}

static int read_device_float(const char *device, float *value) {
  FILE *fp;
  int rc;
  char tmp[10];

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
#ifdef DEBUG
    LOG_ALL_INFO(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%s", tmp);
  fclose(fp);

  if (rc != 1) {
#ifdef DEBUG
    LOG_ALL_INFO(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  }

  *value = atof(tmp);

  return 0;
}

/**
 * @brief 입력된 하드웨어 디바이스 드라이버의 센서 값를 읽어오는 함수
 *
 * @param device 디바이스 드라이버 경로
 ex)/root/sys/devices/platform/ast_adc.0//adc0_value
 * @param value 센서 값 ex)150

 * @return int 성공 유무 (0 : 성공 / errno : 실패)
 **/
static int read_device_int(const char *device, int *value) {
  FILE *fp;
  int rc;
  char tmp[10];
  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
    printf("failed to open device %s", device);
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
  // cout<<"read_adc_value_KTNF pin ="<<pin<<endl;
  int real_pin = pin;
  if (pin >= 7) {
    adc_dir = ADC_DIR1;
    real_pin = pin % 8;
  }
  snprintf(device_name, adc_dir.c_str(), device, real_pin);
  // cout << "read_adc_value_KTNF : device_name =" << device_name << endl;
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", adc_dir.c_str(),
           device_name);
  // cout << "read_adc_value_KTNF : full_name =" << full_name << endl;
  try {
    ret = read_device_int(full_name, &val);
  } catch (const std::exception &e) {
    // cout << "read_adc_value_KTNF error : not exist " << full_name << endl;
  }
  *value = val; //>>2
  return (ret);
}

int read_adc_value(const int pin, const char *device, int *value) {
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];
  int val = 0;
  int ret;

  snprintf(device_name, LARGEST_DEVICE_NAME, device, pin);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", ADC_DIR, device_name);
  ret = read_device_int(full_name, &val);
  *value = val; //>>2
  return (ret);
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

static int i2c_write(int i2c_dev, int addr, char value) {
  unsigned char val[2] = {0};

  val[0] = addr;
  val[1] = value;

  write(i2c_dev, val, sizeof(val));
  return 0;
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
static int i2c_read(int i2c_dev, int addr) {
  unsigned char val[2] = {0};

  val[0] = addr;

  write(i2c_dev, &val[0], 1);
  read(i2c_dev, &val[1], 1);

  return val[1];
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

static unsigned int i2c_readw_r(int i2c_dev, int addr) {
  unsigned char val[2] = {0};
  unsigned char rval[2] = {0};
  unsigned int ret;

  val[0] = addr;

  write(i2c_dev, &val[0], 1);
  read(i2c_dev, &rval[0], 2);

  ret = rval[0] + rval[1] * 256;
  return ret;
}

/**
 * @brief AST2600 smltr에서 사용된 온습도센서
 *
 * @param device
 * @param value
 * @return int
 */
int read_tmp75_temp_value(const char *device, int *value) { // float *value
  char full_name[LARGEST_DEVICE_NAME + 1];
  int tmp, ret = 0;
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/temp1_input", device);
  ret = read_device_int(full_name, &tmp);
  *value = tmp;
  return ret;
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
static int read_i2c_value(char *device, uint8_t addr, uint8_t type,
                          uint32_t *value) {

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
 * @brief PSU(Power Supply Unit)의 정보를 읽어오는 함수\n
 * PMBus를 통하여 PSU의 온도, 팬 RPM, 전력량 등의 정보를 수집함.
 *
 * @param device PSU 디바이스 드라이버
 * @param addr PSU 번호
 * @param type 센서 타입 (온도, 팬, 전력량)
 * @param value 센서 값
 * @return int 성공 유무
 */
static int read_psu_value(char *device, uint8_t addr, uint8_t type,
                          int *value) {

  int dev;
  int ret;
  int32_t res;
  int8_t n;
  int16_t y;
  dev = open(device, O_RDWR);
  if (dev < 0) {
    cout << ("read_psu_value: open() failed") << endl;
    return -1;
  }

  if (ioctl(dev, I2C_SLAVE_FORCE, addr) < 0) {
    cout << ("read_i2c_value: ioctl() assigning i2c addr failed") << endl;
    close(dev);
    return -1;
  }
  res = i2c_readw_r(dev, type);

  close(dev);

  n = (res >> 11) & 0x1f;
  y = res & 0x7FF;
  if (n > 0x0f)
    n |= 0xE0;
  if (y > 0x3ff)
    y |= 0xF800;
  *value = (y * pow(2, n));

  if ((type == NVA_PSU_FAN1) || (type == NVA_PSU_FAN2)) {
    *value = (*value + 50) / 100;
  } else if (type == NVA_PSU_WATT) {
    *value = (*value + 5) / 10;
  }

  return 0;
}

static int read_temp(const char *device, float *value) {
  char full_name[LARGEST_DEVICE_NAME + 1];
  int tmp;

  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/temp1_input", device);
  if (read_device(full_name, &tmp)) {
    return -1;
  }

  *value = ((float)tmp) / UNIT_DIV;

  return 0;
}

static int read_hsc_value(uint8_t reg, char *device, uint8_t addr,
                          uint8_t cntlr, float *value) {

  int dev;
  int ret;
  int32_t res;

  dev = open(device, O_RDWR);
  if (dev < 0) {
    cout << ("read_hsc_value: open() failed");
    return -1;
  }

  /* Assign the i2c device address */
  ret = ioctl(dev, I2C_SLAVE, addr);
  if (ret < 0) {
    cout << ("read_hsc_value: ioctl() assigning i2c addr failed");
    close(dev);
    return -1;
  }
  close(dev);

  switch (reg) {
  case HSC_IN_VOLT:
    res &= 0xFFF; // This Parameter uses bits [11:0]
    if (cntlr == HSC_ADM1276 || cntlr == HSC_ADM1278) {
      *value = (1.0 / 19599) * ((res * 100) - 0);
    }
    break;
  case HSC_OUT_CURR:
    res &= 0xFFF; // This Parameter uses bits [11:0]
    if (cntlr == HSC_ADM1276) {
      *value = ((res * 10) - 20475) / (807 * 0.1667);
    } else if (cntlr == HSC_ADM1278) {
      *value = (1.0 / 1600) * ((res * 10) - 20475);
    }
    break;
  case HSC_IN_POWER:
    res &= 0xFFFF; // This Parameter uses bits [15:0]
    if (cntlr == HSC_ADM1276) {
      *value = ((res * 100) - 0) / (6043 * 0.1667);
    } else if (cntlr == HSC_ADM1278) {
      *value = (1.0 / 12246) * ((res * 100) - 0);
    }
    *value *= 0.99; // This is to compensate the controller reading offset value
    break;
  case HSC_TEMP:
    res &= 0xFFF; // This Parameter uses bits [11:0]
    if (cntlr == HSC_ADM1278) {
      *value = (1.0 / 42) * ((res * 10) - 31880);
    }
    *value *= 0.97; // This is to compensate the controller reading offset value
    break;
  default:
    cout << ("read_hsc_value: wrong param");
    return -1;
  }

  return 0;
}

static int read_nct7904_value(uint8_t reg, char *device, uint8_t addr,
                              float *value) {

  int dev;
  int ret;
  int res_h;
  int res_l;
  int bank;
  int retry;
  uint8_t peer_tray_exist;
  uint8_t location;
  uint8_t monitor_flag;
  uint16_t res;
  float multipler;

  dev = open(device, O_RDWR);
  if (dev < 0) {
    cout << ("read_nct7904_value: open() failed");
    return -1;
  }

  /* Assign the i2c device address */
  ret = ioctl(dev, I2C_SLAVE, addr);
  if (ret < 0) {
    cout << ("read_nct7904_value: ioctl() assigning i2c addr failed");
  }

  close(dev);

  if (reg >= FAN_REGISTER && reg <= FAN_REGISTER + 22) {
    res = ((res_h & 0xFF) << 5) | (res_l & 0x1F);
  } else {
    res = ((res_h & 0xFF) << 3) | (res_l & 0x7);
  }

  switch (reg) {
  case NCT7904_TEMP_CH1:
    multipler = (0.125 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_TEMP_CH2:
    multipler = (0.125 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_VSEN9:
    multipler = (12 /* Voltage Divider*/ * 0.002 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_VSEN6:
    multipler = (12 /* Voltage Divider*/ * 0.002 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_VSEN7:
    multipler = (12 /* Voltage Divider*/ * 0.002 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_3VDD:
    multipler = (0.006 /* NCT7904 Section 6.4 */);
    break;
  }

  if (reg >= FAN_REGISTER && reg <= FAN_REGISTER + 22) {
    /* fan speed reading */
    if (res == 0x1fff || res == 0)
      *value = 0;
    else
      *value = (float)(1350000 / res);
  } else {
    /* temp sensor reading */
    if (reg == NCT7904_TEMP_CH1 || reg == NCT7904_TEMP_CH2) {

      *value = signextend32(res, 10) * multipler;
      if (reg == NCT7904_TEMP_CH2)
        *value = *value - 2;
      /* other sensor reading */
    } else
      *value = (float)(res * multipler);
  }

  return 0;
}

static int read_ads1015_value(uint8_t channel, char *device, uint8_t addr,
                              float *value) {

  int dev;
  int ret;
  int32_t config, config2;
  int32_t res;

  dev = open(device, O_RDWR);
  if (dev < 0) {
    cout << ("read_ads1015_value: open() failed");
    return -1;
  }

  ret = ioctl(dev, I2C_SLAVE, addr);
  if (ret < 0) {
    cout << ("read_ads1015_value: ioctl() assigning i2c addr failed");
    close(dev);
    return -1;
  }

  config = ADS1015_DEFAULT_CONFIG;
  config |= (channel & 0x7) << 4;

  close(dev);
  res = ((res & 0x0FF) << 8) | ((res & 0xFF00) >> 8);
  *value = (float)(res >> 4) * (4.096 / 2048);

  return 0;
}
/**
 *@brief FAN ADC 데이터를 읽는 곳
 *@bug KTNF-AST2600a3보드에선 0~5개까지 존재함
 */
int read_fan_value(int fanno, int *value) {
  int ret;
  ret = read_tacho_value(fanno, value);
  tacho_data[fanno] = *value;
  // cout << ("read_fan_value: fanno:%d value:%f", fanno, *value);
  return ret;
}
/**
@brief IPMB의 Checksum을 확인하는 함수이다.
**/
static unsigned char calc_checksum(uint8_t *buf, uint8_t len) {
  uint8_t i = 0;
  uint8_t cksum = 0;

  for (i = 0; i < len; i++) {
    cksum += buf[i];
  }

  return (ZERO_CKSUM_CONST - cksum);
}

void get_ipmb_sensor(int id, unsigned char *response, unsigned char res_len) {
  ipmb_res_t *res = (ipmb_res_t *)response;
  int i;
  switch (id) {
  case 48:
    g_Tmax = res->data[0];
    break;
  case 0x4B:
    temp_data[0] = g_Tmax-res->data[3];
    //temp_data[1] = res->data[4]; cpu 1
    for (i = 0; i < 8; i++) {
      if (res->data[4 + i] != 0xff)
        temp_data[2 + i] = res->data[4 + i];
      else
        temp_data[2 + i] = 0;
    }
    break;
  default:
    break;
  }
  cout << "get_ipmb_sensor tempdata" << endl;
  for (size_t i = 0; i < 64; i++) {
    cout << "temp_data[" << i << "]=" << temp_data[i] << "  ";
  }
  cout << endl;
}
unsigned char ipmb_res_handle(unsigned char *response, unsigned char res_len) {

  ipmb_res_t *res = (ipmb_res_t *)response;
  unsigned char netfn;
  uint8_t fbyte;
  uint8_t len;
  uint8_t tlun;

  //  printf("ipmb_res_handle()\n");
  netfn = res->netfn_lun >> 2;
  len = res_len;
#if 1
  if (res->hdr_cksum != calc_checksum(response, 2)) {
    printf("IPMB Header cksum does not match\n");
    return 0xff;
  }
#if 0
    if (res->hdr_cksum != calc_cksum(response, 2)) {
      //handle wrong slave address
      if (res->req_slave_addr != BMC_SLAVE_ADDR<<1) {
        // Store the first byte
        fbyte = res->req_slave_addr;
        // Update the first byte with correct slave address
        res->req_slave_addr = BMC_SLAVE_ADDR<<1;
        // Check again if the cksum passes
        if (res->hdr_cksum != calc_cksum(response,2)) {
          //handle missing slave address
          // restore the first byte
          res->req_slave_addr = fbyte;
          //copy the buffer to temporary
          memcpy(tbuf, response, len);
          // correct the slave address
          res->req_slave_addr = BMC_SLAVE_ADDR<<1;
          // copy back from temp buffer
          // copy back from temp buffer
          memcpy(&response[1], tbuf, len);
          // increase length as we added slave address byte
          len++;
          // Check if the above hacks corrected the header
          if (res->hdr_cksum != calc_cksum(response,2)) {
            syslog(LOG_WARNING, "IPMB Header cksum error after correcting slave address\n");
	    printf("IPMB Header cksum error after correcting slave address\n");
            return ;
          }
        }
      } else {
          syslog(LOG_WARNING, "IPMB Header cksum does not match\n");
	  printf("IPMB Header cksum does not match\n");
          return ;
      }
    }
#endif
  // Verify the IPMB data cksum: data starts from 4-th byte
  if (response[len - 1] != calc_checksum(&response[3], len - 4)) {
    //    syslog(LOG_WARNING, "IPMB Data cksum does not match\n");
    printf("IPMB Data cksum does not match\n");
    return 0xff;
  }
#endif
  //  printf("IPMB Res NetFn = 0x%02x\n", netfn);

  // printf("<<<IPMI Handle>>>  netfn = %x, cmd=%x\n", netfn, req->cmd);
  switch (netfn) {
  case NETFN_SENSOR_RES:
    //	printf("IPMB NETFN_SENSOR_RES!\n");
    //     res->netfn_lun = NETFN_CHASSIS_RES << 2;
    //     ipmi_handle_chassis (request, req_len, response, res_len);
    // ipmb_handle_sensor(response, res_len);
    break;
  case NETFN_OEM_GROUP_RES:
    //	ipmb_handle_oem_group(response, res_len);
    //	printf("NETFN_OEM_GROUP_RES!\n");
    break;
  default:
    //   res->netfn_lun = (netfn + 1) << 2;
    break;
  }

  // This header includes NetFunction, Command, and Completion Code
  // *res_len += SIZE_IPMI_RES_HDR;

  return (res->cc);
}

int set_value_to_SMLTR(uint8_t sid, uint8_t val) {
  smltr_data_t req_data;

  int msqid;
  int ndx = 0;
  int ret;
  //	printf("get_value_from_SMLTR is called with sensor : %d \n", sensor_n);
  //	fflush(stdout);

  if (-1 == (msqid = msgget((key_t)8999, IPC_CREAT | 0666))) {
    perror("msgget() 실패");
    return;
    // exit(1);
  }
  printf("GET_VALUE_FROM__SMLTR : got the key for sensor  : %d \n", sid);
  fflush(stdout);
  req_data.data_type = 2;
  req_data.sensor_num = sid;
  req_data.value = val;
  if (-1 == msgsnd(msqid, &req_data, sizeof(smltr_data_t) - sizeof(long), 0)) {
    perror("msgsnd() 실패");
    return;
    // exit(1);
  }
}

int get_value_from_SMLTR(uint8_t sensor_n) {
  smltr_data_t req_data;
  smltr_data_t res_data;

  int msqid;
  int ndx = 0;
  int ret;

  if (-1 == (msqid = msgget((key_t)8998, IPC_CREAT | 0666))) {
    perror("msgget() 실패");
    exit(1);
  }

  req_data.data_type = 2;
  req_data.sensor_num = (unsigned int)sensor_n;

  if (-1 == msgsnd(msqid, &req_data, sizeof(smltr_data_t) - sizeof(long), 0)) {
    perror("msgsnd() 실패");
    perror("get_value_from_SMLTR 실패 ??");
    exit(1);
  }

  if (-1 ==
      msgrcv(msqid, &res_data, sizeof(smltr_data_t) - sizeof(long), 1, 0)) {
    perror("msgrcv() 실패\n");
  }
  ret = res_data.value;

  // printf("\t\t communicate with smltr, sensor_num : %d, ret = %d\n",
  // sensor_n, ret);
  return ret;
}

static const char *devpath = PECI_DEVICE;

#define BUFFER_SIZE 64
double peci_CPU_TEMP0() {
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
    log(info) << "read_lm75_value erorr not open" << LM75_I2C_BUS;
  } else {

    if (ioctl(fd, I2C_SLAVE, lm75_no_converter[no]) < 0) {
      log(info) << "read_lm75_value erorr not I2C_SLAVE " << std::hex << "0x"
                << lm75_no_converter[no];
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
        log(info) << "Failed to read from LM75 at bus ";
      }
    }
    close(fd);
  }
}
  /**
   *@brief LM75 ADC 데이터를 읽는 곳
   *@bug KTNF-AST2600a3보드에선 0~5개까지 존재함
   */
  int read_lm75_value(int no, double *value) {
    read_N520_lm75(no, value);
    lm75_data[no] = *value;
    return 1;
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
int lightning_sensor_read(uint8_t fru, uint8_t sensor_num, int *value) {
  int ret = 0;
  int tmp = 0;
  switch (fru) {
  case FRU_PEB:
    switch (sensor_num) {
    case PEB_SENSOR_ADC_P12V_PSU1:
      ret = read_adc_value_KTNF(ADC_PIN0, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P12V_PSU2:
      ret = read_adc_value_KTNF(ADC_PIN1, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P3V3:
      ret = read_adc_value_KTNF(ADC_PIN2, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P5V:
      ret = read_adc_value_KTNF(ADC_PIN3, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_PCH:
      ret = read_adc_value_KTNF(ADC_PIN4, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V05:
      ret = read_adc_value_KTNF(ADC_PIN5, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V8:
      ret = read_adc_value_KTNF(ADC_PIN6, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_BAT:
      ret = read_adc_value_KTNF(ADC_PIN7, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVCCIN:
      ret = read_adc_value_KTNF(ADC_PIN8, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_PCH_CPU0:
      ret = read_adc_value_KTNF(ADC_PIN9, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V8_NACDELAY:
      ret = read_adc_value_KTNF(ADC_PIN10, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V2_VDDQ:
      ret = read_adc_value_KTNF(ADC_PIN11, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_NAC:
      ret = read_adc_value_KTNF(ADC_PIN12, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V05_NAC:
      ret = read_adc_value_KTNF(ADC_PIN13, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVPP:
      ret = read_adc_value_KTNF(ADC_PIN14, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVTT:
      ret = read_adc_value_KTNF(ADC_PIN15, ADC_VALUE, (int *)value);
      break;

    default:
      ret = -1;
      break;
    }
    break;

  case FRU_PDPB:
    switch (sensor_num) {
    case PDPB_SENSOR_TEMP_REAR_RIGHT:
      ret = read_temp_value(I2C_DEV_PDPB, PDPB_REAR_RIGHT, LOCAL_SENSOR,
                            (uint32_t *)value);

      break;
    case PDPB_SENSOR_TEMP_CPU_AMBIENT:
      ret = read_temp_value(I2C_DEV_PDPB, PDPB_CPU_AMBIENT, LOCAL_SENSOR,
                            (uint32_t *)value);

      break;
    case PDPB_SENSOR_TEMP_FRONT_RIGHT:
      ret = read_temp_value(I2C_DEV_PDPB, PDPB_FRONT_RIGHT, LOCAL_SENSOR,
                            (uint32_t *)value);

      break;
    case PDPB_SENSOR_TEMP_PCIE_AMBIENT:
      ret = read_temp_value(I2C_DEV_PDPB, PDPB_PCIE_AMBIENT, LOCAL_SENSOR,
                            (uint32_t *)value);

      break;
    case PDPB_SENSOR_TEMP_FRONT_LEFT:
      ret = read_temp_value(I2C_DEV_PDPB, PDPB_FRONT_LEFT, LOCAL_SENSOR,
                            (uint32_t *)value);

      break;
    // peci 변경 N520 board는 CPU가 1개임
    case PDPB_SENSOR_TEMP_CPU0:
      *value = (int)peci_CPU_TEMP0();

      ret = 1;
      break;
    // peci 변경 N520 board는 CPU가 1개임
    case PDPB_SENSOR_TEMP_CPU1:
      //*value = (int)peci_CPU_TEMP0();
      ret = 1;
      break;
    case PDPB_SENSOR0_TEMP_LM75:
      //*value = (int)peci_CPU_TEMP0();
      *value = lm75_data[0];

      ret = 1;
      break;
    case PDPB_SENSOR1_TEMP_LM75:
      //*value = (int)peci_CPU_TEMP0();
      *value = lm75_data[1];
      ret = 1;
      break;
    case PDPB_SENSOR2_TEMP_LM75:
      //*value = (int)peci_CPU_TEMP0();
      *value = lm75_data[2];
      ret = 1;
      break;
    case PDPB_SENSOR3_TEMP_LM75:
      *value = lm75_data[3];
      ret = 1;
      break;
    case PDPB_SENSOR4_TEMP_LM75:
      *value = lm75_data[4];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0:
      *value = temp_data[2];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1:
      *value = temp_data[3];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2:
      *value = temp_data[4];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0:
      *value =  temp_data[5];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1:
      *value =  temp_data[6];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2:
      *value =  temp_data[7];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0:
      *value =  temp_data[8];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1:
      *value =  temp_data[9];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2:
      *value =  temp_data[10];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0:
      *value =  temp_data[11];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1:
      *value =  temp_data[12];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2:
      *value =  temp_data[13];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0:
      *value =  temp_data[14];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1:
      *value =  temp_data[15];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2:
      *value =  temp_data[16];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0:
      *value =  temp_data[17];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1:
      *value =  temp_data[18];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2:
      *value =  temp_data[19];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0:
      *value =  temp_data[20];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1:
      *value =  temp_data[21];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2:
      *value =  temp_data[22];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0:
      *value =  temp_data[23];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1:
      *value =  temp_data[24];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2:
      *value =  temp_data[25];
      ret = 1;
      break;
    default:
      ret = -1;
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
  return ret;
}
/**
 * @brief smbus test
 * @bug 테스트단계 보드가없어 확인불가
 */
int wiringPiI2CRead(int fd) {
  union i2c_smbus_data data;

  if (i2c_smbus_access(fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data))
    return -1;
  else
    return data.byte & 0xFF;
}
/**
 * @brief smbus test reg 버전
 * @bug 테스트단계 보드가없어 확인불가
 */
int wiringPiI2CReadReg8(int fd, int reg) {
  union i2c_smbus_data data;

  if (i2c_smbus_access(fd, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data))
    return -1;
  else
    return data.byte & 0xFF;
}
