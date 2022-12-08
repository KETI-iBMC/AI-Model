#include <sys/ioctl.h>
/*
#include <linux/types.h>
*/
typedef unsigned char u8;
typedef unsigned int u32;
/***********************************************************************/
struct timing_negotiation {
  u8 msg_timing;
  u8 addr_timing;
};
struct xfer_msg {
  u8 client_addr;
  u8 tx_len;
  u8 rx_len;
  u8 tx_fcs;
  u8 rx_fcs;
  u8 fcs_en;
  u8 sw_fcs;
  u8 *tx_buf;
  u8 *rx_buf;
  u32 sts;
};
#define PECI_DEVICE "/dev/peci-0"
#define PECIIOC_BASE 'P'
#define AST_PECI_IOCRTIMING _IOR(PECIIOC_BASE, 0, struct timing_negotiation *)
#define AST_PECI_IOCWTIMING _IOW(PECIIOC_BASE, 1, struct timing_negotiation *)
#define AST_PECI_IOCXFER _IOWR(PECIIOC_BASE, 2, struct xfer_msg *)