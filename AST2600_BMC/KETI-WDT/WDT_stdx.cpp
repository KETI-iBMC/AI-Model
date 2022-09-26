#pragma once
#include "WDT_stdx.hpp"
#include <boost/shared_ptr.hpp>
#include <stdlib.h>
using namespace std;
// int KETI_WDT_define::global_enabler = 0;

int KETI_WDT_define::IPMI_Interval = 10;
int KETI_WDT_define::IPMI_Timeout = 600;
int KETI_WDT_define::IPMI_Pretimeout = 10;
int KETI_WDT_define::IPMI_PretimeoutInterrupt;
int KETI_WDT_define::IPMI_Action = 1;
char *KETI_WDT_define::IPMI_Daemon = NULL;
char *KETI_WDT_define::IPMI_Pidfile = NULL;
char *KETI_WDT_define::IPMI_debug = NULL;
