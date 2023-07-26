#ifndef __IPMB_H__
#define __IPMB_H__

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/asio/write.hpp>
#include <filesystem>
#include <fstream>
#include <ipmi/ipmbdefines.hpp>
#include <tuple>
#include <unordered_map>
#define SOCK_PATH_IPMB "/tmp/ipmb_socket"
#define MAX_BYTES 300

#define BRIDGE_SLAVE_ADDR 0x16
/**
 * @brief IPMB ZEROSUM 확인
 *
 */
#define ZERO_CKSUM_CONST 0x100

// rqSA, rsSA, rqSeq, hdrCksum, dataCksum
#define IPMB_HDR_SIZE 5

// rqSA, NetFn, hdrCksum
#define IPMB_DATA_OFFSET 3

// Slot#0 is on I2C Bus1
#define IPMB_BUS_SLOT0 1
#define IPMB_I2C_BUS 5
// TODO: Some IPMB responses take about 5-6 seconds
// Need to add a timeout parameter to IPMB request
// For now changing global timeout to 8 seconds
#define TIMEOUT_IPMB 8
#define MAX_IPMB_RES_LEN 300
#define IPMI_MSG_MAX_LENGTH 32
#define I2C_CLIENT_PEC 0x0004 /* Use Packet error checking */
#define I2C_S_EN 0x0002       /* Slave mode enable */
#define I2C_S_ALT 0x0004      /* Slave issue slt */

/***********************************************************
 * I2C-DEV Addendum
 **********************************************************/
/* Additional ioctl commands for /dev/i2c-X */
#define I2C_SLAVE_RD 0x0710   /* Slave Read */
#define I2C_SLAVE_WR 0x0711   /* Slave /Write */
#define I2C_SLAVE_RDWR 0x0712 /* Slave Read/Write */

typedef struct _ipmb_req_t {
  uint8_t res_slave_addr;
  uint8_t netfn_lun;
  uint8_t hdr_cksum;
  uint8_t req_slave_addr;
  uint8_t seq_lun;
  uint8_t cmd;
  uint8_t dest_LUN;
  uint8_t src_LUN;
  uint8_t data[IPMI_MSG_MAX_LENGTH];
  //  uint8_t data[];
} ipmb_req_t;
/**
 * @brief Response IPMB Node to Node Message Format
 * @param req_slave_addr-rq Slave Addr.(rqSQ)
 * @param netfn_lun-rqLUN
 * @param res_slave_addr-rs SlaveAddr.(rsSA)
 * @param seq_lun-squ lun
 * @param cmd-command
 * @param cc-completion code
 * @param data-response data byte(0 or more)
 */
typedef struct _ipmb_res_t {
  uint8_t req_slave_addr;
  uint8_t netfn_lun;
  uint8_t hdr_cksum;
  uint8_t res_slave_addr;
  uint8_t seq_lun;
  uint8_t cmd;
  uint8_t cc;
  uint8_t data[];
} ipmb_res_t;

void lib_ipmb_handle(unsigned char bus_id, unsigned char *request,
                     unsigned char req_len, unsigned char *response,
                     unsigned char *res_len);
void ipmb_open(uint8_t ipmb_bus_num);
void ipmb_close(void);
void ipmb_rx_handler(void *bus_num);
void ipmb_get_deviceid(void);
void ipmb_get_cpu_temp(void);
void ipmb_i2c_open(unsigned char busm);
/**
 * @brief plat_ipmb_init I2C 초기화
 *
 */
void plat_ipmb_init(void);
/**
 * @brief Maximum count of messages to be sent
 */
#define IPMB_TXQUEUE_LEN 2
/**
 * @brief Maximum count if received messages to be delivered to client task
 */
#define IPMB_CLIENT_QUEUE_LEN 2

/**
 * @brief Maximum retries made by IPMB TX Task when sending a message
 */
#define IPMB_MAX_RETRIES 3

/**
 * @brief Timeout limit between the end of a request and start of a response
 * (defined in IPMB timing specifications)
 */
#define IPMB_MSG_TIMEOUT 250 / portTICK_PERIOD_MS

/**
 * @brief Timeout limit waiting a free space in client queue to put a received
 * message
 */
#define CLIENT_NOTIFY_TIMEOUT 5

/**
 * @brief Position of Header cheksum byte in a IPMI message
 */
#define IPMI_HEADER_CHECKSUM_POSITION 2

/**
 * @brief Maximum length in bytes of a IPMI message (including connection
 * header)
 */

/**
 * @brief Length of connection header in a request
 *
 * The connection header in a request is made by:
 * - rsSA -> Destination Address
 * - NetFN:6 rsLUN:2 -> Net function and dest LUN
 * - HeaderChecksum -> 2's complement of the sum of preceding bytes
 * - rqSA -> Source address
 * - rqSeq:6 rqLUN:2 -> Sequence Number and source LUN
 * - CMD -> Command
 * - Messagechecksum -> 2's complement of the sum of preceding bytes
 */
#define IPMB_REQ_HEADER_LENGTH 6

/**
 * @brief Length of connection header in a response
 *
 * The connection header in a response is made by:
 * - rqSA -> Destination Address
 * - NetFN:6 rqLUN:2 -> Net function and dest LUN
 * - HeaderChecksum -> 2's complement of the sum of preceding bytes
 * - rsSA -> Source address
 * - rqSeq:6 rsLUN:2 -> Sequence Number and source LUN
 * - CMD -> Command
 * - CC -> Completion Code
 * - Messagechecksum -> 2's complement of the sum of preceding bytes
 * @note Response header is 1 byte longer than request's because it must include
 * the completion code
 */
#define IPMB_RESP_HEADER_LENGTH 7

#define IPMB_NETFN_MASK 0xFC
#define IPMB_DEST_LUN_MASK 0x03
#define IPMB_SEQ_MASK 0xFC
#define IPMB_SRC_LUN_MASK 0x03

/**
 * @brief MicroTCA's MCH slave address
 */
#define MCH_ADDRESS 0x20

/**
 * @brief Macro to check is the message is a response (odd netfn)
 */
#define IS_RESPONSE(msg) (msg.netfn & 0x01)

/**
 * @brief Size of #IPMBL_TABLE
 */
#define IPMBL_TABLE_SIZE 27

/**
 * @brief Minimum clock cycles to the GPIO pin change take effect on some boards
 */
#define GPIO_GA_DELAY 10

/**
 * @brief GA pins definition
 */
typedef enum { GROUNDED = 0, POWERED, UNCONNECTED } GA_Pin_state;

/**
 * @brief IPMI message struct
 */
typedef struct ipmi_msg {
  uint8_t dest_addr;       /**< Destination slave address */
  uint8_t netfn;           /**< Net Function
                            * @see ipmi.h
                            */
  uint8_t dest_LUN;        /**< Destination LUN (Logical Unit Number) */
  uint8_t hdr_chksum;      /**< Connection Header Checksum */
  uint8_t src_addr;        /**< Source slave address */
  uint8_t seq;             /**< Sequence Number */
  uint8_t src_LUN;         /**< Source LUN (Logical Unit Number) */
  uint8_t cmd;             /**< Command
                            * @see ipmi.h
                            */
  uint8_t completion_code; /**< Completion Code
                            * @see ipmi.h
                            */
  uint8_t data_len;        /**< Amount of valid bytes in #data buffer */
  uint8_t data[IPMI_MSG_MAX_LENGTH]; /**< Data buffer <br>
                                      * Data field has 24 bytes:
                                      * 32 (Max IPMI msg len) - 7 header bytes -
                                      * 1 final chksum byte
                                      */
  uint8_t msg_chksum;                /**< Message checksum */
} ipmi_msg;

/**
 * @brief IPMB errors enumeration
 */
typedef enum ipmb_error {
  ipmb_error_unknown = 0, /**< Unknown error */
  ipmb_error_success,     /**< Generic no-error flag  */
  ipmb_error_failure,     /**< Generic failure on IPMB */
  ipmb_error_timeout,     /**< Error raised when a message takes too long to be
                             responded */
  ipmb_error_invalid_req, /**< A invalid request was received */
  ipmb_error_hdr_chksum,  /**< Invalid header checksum from incoming message */
  ipmb_error_msg_chksum,  /**< Invalid message checksum from incoming message */
  ipmb_error_queue_creation /**< Client queue couldn't be created. Invalid
                               pointer to handler was given */
} ipmb_error;

#endif /* __IPMB_H__ */
