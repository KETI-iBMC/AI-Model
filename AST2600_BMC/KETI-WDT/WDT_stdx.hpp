#pragma once
#include <fcntl.h>
#include <iostream>
using namespace std;

class KETI_WDT_define {
public:
  static int IPMI_Interval, IPMI_Timeout, IPMI_Pretimeout,
      IPMI_PretimeoutInterrupt, IPMI_Action;
  static char *IPMI_Daemon, *IPMI_Pidfile, *IPMI_debug;
  static string NETWORK_CHANGE_PATH;
};