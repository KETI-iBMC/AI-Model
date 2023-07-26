#include "ipmb.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>
uint8_t rx_done = 0;
static int i2c_fd;
static int gseq = 0;
int g_Tmax = 90;
int temp_data[64];
#define I2C_RETRIES_MAX 5
pthread_mutex_t m_i2c;
uint8_t rx_buf[MAX_BYTES] = {0};
uint8_t rx_len;
#define RMCP_UDP_PORT 623
#define IPMB_PKT_MIN_SIZE 6

static int g_bus_id = 0; // store the i2c bus ID for debug print
void exit_cleanup() {
  printf("\nCleanup..");
  // sensors_cleanup();
  printf(". done, exiting\n");
  exit(0);
}
static int i2c_write(int fd, uint8_t *buf, uint8_t len) {
  struct i2c_rdwr_ioctl_data data;
  struct i2c_msg msg;
  int rc;
  int i;
  //  struct timespec req;
  //  struct timespec rem;

  memset(&msg, 0, sizeof(msg));

  msg.addr = buf[0] >> 1;
  msg.flags = 0;
  msg.len = len - 1; // 1st byte in addr
  msg.buf = &buf[1];

  data.msgs = &msg;
  data.nmsgs = 1;

  // Setup wait time
  // req.tv_sec = 0;
  // req.tv_nsec = 20000000;//20mSec
  printf("i2c write~\n");
  pthread_mutex_lock(&m_i2c);
  printf("i2c write lock~ \n");
  for (i = 0; i < I2C_RETRIES_MAX; i++) {
    rc = ioctl(fd, I2C_RDWR, &data);
    if (rc < 0) {
      printf("rc = %d \n retrty i2c_write\n");
      sleep(1);
      continue;
    } else {
      break;
    }
  }

  if (rc < 0) {
    printf("bus: %d, Failed to do raw io %d \n", g_bus_id);
    pthread_mutex_unlock(&m_i2c);
    return -1;
  }

  pthread_mutex_unlock(&m_i2c);

  return 0;
}

static inline uint8_t calc_cksum(uint8_t *buf, uint8_t len) {
  uint8_t i = 0;
  uint8_t cksum = 0;

  for (i = 0; i < len; i++) {
    cksum += buf[i];
  }

  return (ZERO_CKSUM_CONST - cksum);
}
void get_ipmb_sensor(int id, unsigned char *response, unsigned char res_len) {
  printf("get_ipmb_sensor !!");
  ipmb_res_t *res = (ipmb_res_t *)response;
  temp_data[0] = res->data[3];
  temp_data[1] = res->data[4];
  int i = 0;
  for (i = 0; i < res_len; i++)
    printf("<%x>", res->data[i]);
  printf("CPU temp  \nCPU0 = %d\n CPU1 = %d\n", g_Tmax - temp_data[0],
         g_Tmax - temp_data[1]);
}
static void ipmb_handle(int fd, unsigned char *request, unsigned char req_len,
                        unsigned char *response, unsigned char *res_len) {
  ipmb_req_t *req = (ipmb_req_t *)request;
  ipmb_res_t *res = (ipmb_res_t *)response;

  int8_t index;

  int data = 0;
  printf("ipmb_handle start\n");
  if (data = i2c_write(fd, request, req_len)) {
  }
  // printf("i2c_write data =%d\n", data);
  return;
}
/**
 * @brief Encode IPMI msg struct to a byte formatted buffer
 *
 * This function formats the ipmi_msg struct fields into a byte array, following
 * the specification: <table> <caption>IPMB Messages</caption> <tr><th>
 * <th>REQUEST   <th>RESPONSE   <th>Bit Len   <th>Byte # <tr><td>Connection
 * <td>rsSA      <td>rqSA       <td>8         <td>1 <tr><td>Header <td>NetFN
 * <td>NetFN      <td>6         <td>2 <tr><td>                 <td>rsLUN
 * <td>rqLUN      <td>2         <td>2 <tr><td>Header Chksum    <td>Chksum
 * <td>Chksum     <td>8         <td>3 <tr><td>                 <td>rqSA <td>rsSA
 * <td>8         <td>4 <tr><td>Callback Info    <td>rqSeq     <td>rqSeq <td>6
 * <td>5 <tr><td>                 <td>rqLUN     <td>rsLUN      <td>2 <td>5
 * <tr><td>Command          <td>CMD       <td>CMD        <td>8         <td>6
 * <tr><td>Data             <td>          <td>CC         <td>8         <td>7
 * <tr><td>                 <td>Data      <td>Data       <td>8*N       <td>7+N
 * <tr><td>Message Chksum   <td>Chksum    <td>Checksum   <td>8         <td>7+N+1
 * </table>
 *
 * @param[out] buffer Byte buffer which will hold the formatted message
 * @param[in] msg The message struct to be formatted
 *
 * @retval ipmb_error_success The message was successfully formatted
 */
static int ipmb_encode(uint8_t *buffer, ipmi_msg *msg) {
  /* Use this variable to address the buffer dynamically */
  uint8_t i = 0, j;

  buffer[i++] = msg->dest_addr; // rs Slave Addr. rsSA
  buffer[i++] = (((msg->netfn << 2) & IPMB_NETFN_MASK) |
                 (msg->dest_LUN & IPMB_DEST_LUN_MASK)); // Net Fn(even)/rsLUN
  buffer[i++] =
      calc_cksum(&buffer[0], IPMI_HEADER_CHECKSUM_POSITION); // Checksum
  buffer[i++] = msg->src_addr; // rq Slave Addr. rqSA
  buffer[i++] = (((msg->seq << 2) & IPMB_SEQ_MASK) |
                 (msg->src_LUN & IPMB_SRC_LUN_MASK)); // RQSeq/rqLUN
  buffer[i++] = msg->cmd;                             // CMD
  //    if (IS_RESPONSE((*msg))) {
  //        buffer[i++] = msg->completion_code;
  //    }
  if (msg->data_len) {
    memcpy(&buffer[i], &msg->data[0], msg->data_len);
    i += msg->data_len;
  }                                      // data bytes 0 or more
  buffer[i] = calc_cksum(&buffer[0], i); // checksum
  printf("<Q>");
  printf("IPMB Req : ");
  for (j = 0; j <= i; j++)
    printf("%02x ", buffer[j]);
  printf("\n");
  return (i + 1);
}

/**
 * @brief Decodes a buffer and copies to its specific fields in a ipmi_msg
 * struct
 *
 * @param[out] msg Pointer to a ipmi_msg struct which will hold the decoded
 * message
 * @param[in] buffer Pointer to a byte array that will be decoded
 * @param[in] len Length of \p buffer
 *
 * @retval ipmb_error_success The message was successfully decoded
 */
ipmb_error ipmb_decode(ipmi_msg *msg, uint8_t *buffer, uint8_t len);
static int i2c_open(uint8_t bus_num) {
  int fd;
  char fn[32];
  int rc;
  snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus_num);
  fd = open(fn, O_RDWR);
  if (fd == -1) {
    printf("Failed to open i2c device %s\n", fn);
    return -1;
  }

  rc = ioctl(fd, I2C_SLAVE, BRIDGE_SLAVE_ADDR);

  if (rc < 0) {
    printf(LOG_WARNING, "i2c_open Failed to open slave @ address 0x%x \n",
           BRIDGE_SLAVE_ADDR);
    close(fd);
    return -1;
  }

  return fd;
}

static int i2c_slave_read(int fd, uint8_t *buf, uint8_t *len) {
  struct i2c_rdwr_ioctl_data data;
  struct i2c_msg msg;
  int rc;
  memset(&msg, 0, sizeof(msg));

  msg.addr = BMC_SLAVE_ADDR;
  msg.flags = 0;
  msg.len = MAX_BYTES;
  msg.buf = buf;

  data.msgs = &msg;
  data.nmsgs = 1;
  rc = ioctl(fd, I2C_RDWR, &data);
  // rc = ioctl(fd, I2C_SLAVE_RDWR, &data);
  // rc 가 없음
  if (rc < 0) {
    // printf("rc =%d\n", rc);
    return -1;
  } else {
    printf("recv rc %d\n", rc);
  }

  *len = msg.len;

  // if (*len)
  //   printf("IPMB RX!!!\n");
  return 0;
}
static int i2c_slave_open(uint8_t bus_num) {
  int fd;
  char fn[32];
  int rc;
  struct i2c_rdwr_ioctl_data data;
  struct i2c_msg msg;
  uint8_t read_bytes[MAX_BYTES] = {0};

  snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus_num);
  fd = open(fn, O_RDWR);
  if (fd == -1) {
    syslog(LOG_WARNING, "Failed to open i2c device %s", fn);
    return -1;
  }
  memset(&msg, 0, sizeof(msg));
  msg.addr = BMC_SLAVE_ADDR;
  msg.flags = I2C_S_EN;
  msg.len = 1;
  msg.buf = read_bytes;
  msg.buf[0] = 1;
  data.msgs = &msg;
  data.nmsgs = 1;

  rc = ioctl(fd, I2C_RDWR, &data);
  // rc = ioctl(fd, I2C_RDWR, &data);
  if (rc < 0) {
    printf(" rc = %d i2c_slave_open Failed to open slave @ address 0x%x \n", rc,
           BMC_SLAVE_ADDR);
    close(fd);
  }

  return fd;
}

/**
 * @brief ipmb 수신 명령어를 읽음
 *
 * @param bus_num i2c bus num
 * @return void*
 */
void *ipmb_rx_handler(void *bus_num) {
  uint8_t *bnum = (uint8_t *)bus_num;
  int fd;
  uint8_t tlun;
  struct pollfd ufds[1];

  ipmb_res_t *res = (ipmb_res_t *)rx_buf;
  int i;
  fd = i2c_slave_open(*bnum);
  if (fd < 0) {
    printf("i2c_slave_open fails\n");

    goto cleanup;
  }
  ufds[0].fd = fd;
  ufds[0].events = POLLIN;
  // fds : pollfd 의 구조체 포인터입니다.
  // pollfd fd = file descriptor  short events = requested events  shor revents
  // =returned events
  while (1) {
    // Read messages from i2c driver
    if (i2c_slave_read(fd, rx_buf, &rx_len) < 0) {
      poll(ufds, 1, 10);
      continue;
    }
    printf("rx_len= %d \n\n", rx_len);
    if (rx_len < IPMB_PKT_MIN_SIZE) {
      printf("bus: %d, IPMB Packet invalid size %d", g_bus_id, rx_len);
      continue;
    }
    if (res->hdr_cksum != calc_cksum(rx_buf, 2)) {
      printf("IPMB Header cksum does not match\n");
      continue;
    }

    if (rx_buf[rx_len - 1] != calc_cksum(&rx_buf[3], rx_len - 4)) {
      printf("IPMB Data cksum does not match\n");
      continue;
    }
    for (int i = 0; i < rx_len; i++)
      printf("0x%x ", rx_buf[rx_len - 1]);
    printf("\n");
    rx_done = 1;
    if (res->cc == 0) {
      if (gseq == (rx_buf[4] >> 2))
        rx_done = 1;
    }
  }
cleanup:
  if (fd > 0) {
    close(fd);
  }
  pthread_exit(NULL);
}

static void ipmb_get_cpu_temp(void) {
  unsigned char req_buf[128] = {0};
  unsigned char res_buf[128] = {0};

  int req_len, res_len;

  ipmi_msg req_msg;
  int i;
  int count;

  i = 0;
  req_msg.dest_addr = BRIDGE_SLAVE_ADDR << 1;
  req_msg.netfn = 0x2E; // OEM GROUP
  req_msg.dest_LUN = 0;
  req_msg.src_LUN = 0;
  req_msg.cmd = 0x4B; // get CPU and DIMM temperature
  req_msg.src_addr = BMC_SLAVE_ADDR << 1;
  if (gseq == 63)
    gseq = 0;
  req_msg.seq = ++gseq;     // seq_get_new();
  req_msg.data[i++] = 0x57; // me
  req_msg.data[i++] = 0x01; // me
  req_msg.data[i++] = 0x00; // me
  req_msg.data[i++] = 0x03; // CPU 0,1
  req_msg.data[i++] = 0xff; // CPU0 channel 0,1
  req_msg.data[i++] = 0x0f; // CPU0 channel 2
  // req_msg.data[i++] = 0xff; // CPU1 channel 0,1
  // req_msg.data[i++] = 0x0f; // CPU2 channel 2
  // req_msg.data[i++] = 0x00; // CPU3
  // req_msg.data[i++] = 0x00; // CPU3
  // req_msg.data[i++] = 0x00; // CPU4
  // req_msg.data[i++] = 0x00; // CPU4
  req_msg.data_len = i;
  req_len = ipmb_encode(req_buf, &req_msg);
  count = 0;
  rx_done = 0;

  ipmb_handle(i2c_fd, req_buf, req_len, res_buf, &res_len);
  while ((rx_done == 0)) {
    // printf("not working \n");
  }
  if (rx_done) {
    printf("Done!!\n");
    get_ipmb_sensor(0x4B, rx_buf, rx_len);
  }
}

int ipmb_sensor_reading(uint8_t id) {
  unsigned char req_buf[128] = {0};
  unsigned char res_buf[128] = {0};

  int req_len, res_len;

  ipmi_msg req_msg;
  int i;
  int count;

  i = 0;
  printf("IPMB sensor reading!!\n");
  req_msg.dest_addr = BRIDGE_SLAVE_ADDR << 1;
  req_msg.netfn = 0x04; // SENSOR
  req_msg.dest_LUN = 0;
  req_msg.src_LUN = 0;
  req_msg.cmd = 0x2d; // Sensor Reading
  req_msg.src_addr = BMC_SLAVE_ADDR << 1;
  if (gseq == 63)
    gseq = 0;
  req_msg.seq = ++gseq; // seq_get_new();
  req_msg.data[i++] = id;
  req_msg.data_len = i;
  rx_buf[7] = 0;
  req_len = ipmb_encode(req_buf, &req_msg);
  rx_done = 0;
  count = 0;
  ipmb_handle(i2c_fd, req_buf, req_len, res_buf, &res_len);
  while ((rx_done == 0) && count < 10) {
    count++;
    printf("sleep count %d \n", count);
    sleep(1);
  }
  if (rx_done) {
    get_ipmb_sensor(id, rx_buf, rx_len);
  }
  return 0;
}
int ipmb_self_Test() {
  printf("self test\n");
  unsigned char req_buf[128] = {0};
  unsigned char res_buf[128] = {0};

  int req_len, res_len;

  ipmi_msg req_msg;
  int i;
  int count;

  i = 0;
  printf("IPMB sensor reading!!\n");
  req_msg.dest_addr = BRIDGE_SLAVE_ADDR << 1;
  // req_msg.dest_addr = 0x00;
  req_msg.netfn = 0x04; // SENSOR
  req_msg.dest_LUN = 0;
  req_msg.src_LUN = 0;
  req_msg.cmd = 0x2d; // Sensor Reading
  // req_msg.src_addr = 0x44;
  req_msg.src_addr = BMC_SLAVE_ADDR << 1;
  if (gseq == 63)
    gseq = 0;
  req_msg.seq = ++gseq; // seq_get_new();
  req_msg.data[i++] = 0x55;
  req_msg.data[i++] = 0x00;
  req_msg.data_len = i;
  rx_buf[7] = 0;
  req_len = ipmb_encode(req_buf, &req_msg);
  rx_done = 0;
  count = 0;
  ipmb_handle(i2c_fd, req_buf, req_len, res_buf, &(res_len));
  while ((rx_done == 0) && count < 3) {
    count++;
    sleep(1);
  }
  if (rx_done) {
    printf("rx_buf[7] =%d\n", rx_buf[7]);
    // if (rx_buf[7] < 110 && rx_buf[7] >= 70) {
    //   get_ipmb_sensor(id, rx_buf, rx_len);
    //   return 1;
    // }
  }
  return 0;
}
int main() {
  unsigned int count = 0, beat = 0, tcount = 0;
  pthread_t tid, tid2, tipmb;
  ;
  int slave_bus_num;

  slave_bus_num = I2C_BUS_NUM;
  // init random seed
  signal(SIGINT, exit_cleanup);
  i2c_fd = i2c_open(slave_bus_num);
  if (i2c_fd == -1) {
    printf("fail\n");
    return;
  }

  srand(time(NULL));
  if (pthread_create(&tipmb, NULL, ipmb_rx_handler, (void *)&slave_bus_num) <
      0) {
    printf("ipmi-server: pthread_create failed\n");
    exit(0);
  }
  sleep(1);
  // ipmb_sensor_reading(1);
  // ipmb_self_Test();
  // ipmb_sensor_reading(28);
  // ipmb_sensor_reading(29);
  // ipmb_sensor_reading(30);
  // ipmb_sensor_reading(3);
  // ipmb_sensor_reading(4);
  // ipmb_sensor_reading(5);
  // ipmb_sensor_reading(6);
  ipmb_get_cpu_temp();
  while (1) {
  };
  pthread_detach(tipmb);
  close(i2c_fd);
}
