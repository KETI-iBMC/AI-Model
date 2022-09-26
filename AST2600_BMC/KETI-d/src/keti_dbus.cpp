#include "keti_dbus.hpp"
#include <dbus-c++-1/dbus-c++/interface.h>
#include <dbus-c++-1/dbus-c++/message.h>
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


// static const char *METHOD_NAME = "org.freedesktop.keti.bmc.ADC";
// static const char *METHOD_PATH = "/org/freedesktop/keti/bmc/ADC";
static const char *SERVER_NAME = "org.freedesktop.keti.bmc.dbus";
static const char *SERVER_PATH = "/org/freedesktop/keti/bmc/dbus";

KETI_Dbus::KETI_Dbus(DBus::Connection &connection)
  :  DBus::ObjectAdaptor(connection, SERVER_PATH)
{
}
int KETI_Dbus::read_adc_value_KTNF(const int pin, const char *device, int *value) {
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];
  int val = 0;
  int ret;

  string adc_dir = ADC_DIR;

  int real_pin = pin;
  if (pin >= 7) {
    adc_dir = ADC_DIR1;
    real_pin = pin % 8;
  }
  string dev_f = "in_voltage";
  string dev_s = "_raw";
  string dev;

  dev = dev_f + to_string(real_pin) + dev_s;
  sprintf(device_name, adc_dir.c_str(), dev);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", device_name, dev.c_str());
   try {
    ret = KETI_Dbus::read_device_int(full_name, &val);
  } catch (const std::exception &e) {
    // cout << "read_adc_value_KTNF error : not exist " << full_name << endl;
  }
  cout <<"val "<<val<<endl;
  // *value = val; //>>2
    cout << "read device "<< full_name<<endl;
  return val;
}

/**
 * @brief 입력된 하드웨어 디바이스 드라이버의 센서 값를 읽어오는 함수
 *
 * z
 **/
int KETI_Dbus::read_device_int(const char *device, int *value) {
  FILE *fp;
  int rc;
  char tmp[10];
  // cout << "read _device_init" << endl;
  fp = fopen(device, "r");
  // cout << "file open" << endl;
  if (!fp) {
    int err = errno;
    printf("failed to open device %s", device);
    return err;
  }
  rc = fscanf(fp, "%s", tmp);
  fclose(fp);
  if (rc != 1) {

    // tacho데이터를 읽을때 0이하는 읽을수없다 ㅇ
    return ENOENT;
  }
  //  cout << "atoi start "<<endl;
  *value = atoi(tmp);
  //  cout << "atoi endl"<<endl;
  return 0;
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
  ret = KETI_Dbus::read_device_int(full_name, &tmp);
  *value = tmp;
  return ret;
}
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
int KETI_Dbus::lightning_sensor_read(const uint8_t fru, const uint8_t sensor_num,  int *value){
  cout<< "connect lightning senseor read " <<endl;
  int ret = 0;
  int tmp = 0;
  int val =0;
  switch (fru) {
  case FRU_PEB:
    switch (sensor_num) {
    case PEB_SENSOR_ADC_P12V_PSU1:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN0, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P12V_PSU2:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN1, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P3V3:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN2, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P5V:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN3, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_PCH:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN4, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V05:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN5, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V8:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN6, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_BAT:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN7, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVCCIN:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN8, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_PCH_CPU0:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN9, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V8_NACDELAY:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN10, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V2_VDDQ:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN11, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVNN_NAC:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN12, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_P1V05_NAC:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN13, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVPP:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN14, ADC_VALUE, (int *)value);
      break;
    case PEB_SENSOR_ADC_PVTT:
      ret = KETI_Dbus::read_adc_value_KTNF(ADC_PIN15, ADC_VALUE, (int *)value);
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
    case PDPB_SENSOR_TEMP_CPU0:
      // tmp = get_value_from_SMLTR(sensor_num);
      tmp = temp_data[0];
      if (tmp != 0xff)         //(temp_data[0] != 0xff)
        *value = g_Tmax - tmp; // temp_data[0];
      else
        *value = 0;
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1:
      // tmp = get_value_from_SMLTR(sensor_num);
      tmp = temp_data[1];
      if (tmp != 0xff)         //(temp_data[1] != 0xff)
        *value = g_Tmax - tmp; // temp_data[1];
      else
        *value = 0;
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[2];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[3];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[4];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[5];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[6];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[7];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[8];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[9];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[10];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[11];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[12];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[13];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[14];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[15];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[16];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[17];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[18];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[19];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[20];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[21];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[22];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[23];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[24];
      ret = 1;
      break;
    case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2:
      //*value = get_value_from_SMLTR(sensor_num); // temp_data[25];
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
      ret = tacho_data[0];
      break;
    case NVA_SYSTEM_FAN2:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[1]);
      ret = tacho_data[1];
      break;
    case NVA_SYSTEM_FAN3:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[2]);
      ret = tacho_data[2];
      break;
    case NVA_SYSTEM_FAN4:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[3]);
      ret = tacho_data[3];
      break;
    case NVA_SYSTEM_FAN5:
      cout << ("NVA_SENSOR_BP_FAN1 read: %d", tacho_data[4]);
      ret = tacho_data[4];
      break;
    }
    break;
  
  }
  // val = *value;
  cout << "read sensor end "<<endl;
  return ret;
}


std::string KETI_Dbus::rpc_test(const std::string& msg){
  std::string m = msg;
  return m;
}

DBus::BusDispatcher dispatcher;

void niam(int sig)
{
  dispatcher.leave();
}
/**
 *@brief FAN ADC 데이터를 읽는 곳
 *@bug KTNF-AST2600a3보드에선 0~5개까지 존재함
 */
int KETI_Dbus::read_fan_value(int fanno, int *value) {
  int ret;
  ret = read_tacho_value(fanno, &ret);
  tacho_data[fanno] = ret;
  cout << ("read_fan_value: fanno:%d value:%f", fanno, ret);
  return ret;
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
  cout << "tacho device name "<<device_name<<endl;
  // printf("device name : %s\n", device_name);
  snprintf(full_name, MAX_DVNAME_LEN, "%s/%s", TACHO_DIR, device_name);
  // printf("full name : %s\n", full_name);

  ret = KETI_Dbus::read_device_int(full_name, &val);
  // *value = val; /// 100;
  return val;
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
int KETI_Dbus::read_psu_value(std::string device, uint8_t addr, uint8_t type,
                          int *value) {

  int dev;
  int ret;
  int32_t res;
  int8_t n;
  int16_t y;
  dev = open(device.c_str(), O_RDWR);
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
