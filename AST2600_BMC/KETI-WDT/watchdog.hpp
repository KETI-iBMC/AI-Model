#pragma once
#include <condition_variable>
#include <stdint.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

// #include "ipmi/apps.h"
/* Configuration file key works*/
#define IPMI_DAEMON "Daemon"
#define IPMI_TIMEOUT "Timeout"
#define IPMI_PRETIMEOUT "Pretimeout"
#define IPMI_INTERVAL "Interval"
#define IPMI_PRETIMEOUTINTERRUPT "INT_Pretimeout"
#define IPMI_ACTION "Action"
#define IPMI_PIDFILE "Pidfile"
#define IPMIWatchdogFile1 "/etc/init.d/ipmiwatchdog"
#define IPMIWatchdogFile2 "/etc/ipmiwatchdog.conf"
#define ConfigurationFileDir "/etc/ipmiwatchdog.conf"
#define CONFIG_LINE_LEN 100
#define WAITSECONDS 100

typedef struct {
  unsigned char timer_use;
  unsigned char timer_actions;
  unsigned char pre_timeout;
  unsigned char timer_use_exp;
  unsigned char initial_countdown_lsb;
  unsigned char initial_countdown_msb;
  unsigned char present_countdown_lsb;
  unsigned char present_countdown_msb;
  unsigned char pretimeoutInterrupt;
  bool Islogging = false;

} bmc_watchdog_param_t;

typedef struct {
  long type;
  int next;
  int res_len;
  unsigned char data[35000];
  // QSIZE
} msq_rsp_t;

static int ReadConfigurationFile(char *file);
static int WriteConfigurationFile(char *file);
static int spool(char *line, int *i, int offset);

class KETI_Watchdog {
private:
  bool lockFlag;
  KETI_Watchdog();
  ~KETI_Watchdog(){};
  std::condition_variable WDT_cv;
  static KETI_Watchdog *ins_KETI_Watchdog;

public:
  static KETI_Watchdog *SingleInstance();
  void start();
};

void *messegequeue(void *keyvalue)