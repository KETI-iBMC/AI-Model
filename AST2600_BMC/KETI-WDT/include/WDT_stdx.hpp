#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define CONFIG_LINE_LEN 100
#define IPMI_DAEMON "Daemon"
#define IPMI_TIMEOUT "Timeout"
#define IPMI_PRETIMEOUT "Pretimeout"
#define IPMI_INTERVAL "Interval"
#define IPMI_PRETIMEOUTINTERRUPT "INT_Pretimeout"
#define IPMI_ACTION "Action"
#define IPMI_PIDFILE "Pidfile"
#define IPMI_RESETCOUNT "Reset_count"
#define ConfigurationFileDir "/etc/ipmiwatchdog.conf"


class KETI_WDT_define {
public:
  static int Interval, Timeout, Pretimeout,
      PretimeoutInterrupt, Action, Reset_count;
  static char *Daemon, *Pidfile, *Debug;
  static string file_path;
  static string NETWORK_CHANGE_PATH;
};


int ReadConfigurationFile(char *file);
int WriteConfigurationFile(char *file);
static int spool(char *line, int *i, int offset);