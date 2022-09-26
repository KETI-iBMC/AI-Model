#pragma once

#include <WDT_stdx.hpp>
#include <KETI-FILE.hpp>

#include <condition_variable>
#include <stdint.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <thread>

using namespace std;

#define WAITSECONDS 30

struct wdt_msq_t{
  long type;
  int flag = 0;
};

class KETI_Watchdog {
private:
  bool lockFlag;
  key_t ms_key;
  KETI_Watchdog(){};
  ~KETI_Watchdog(){};
  std::condition_variable WDT_cv;
  static KETI_Watchdog *ins_KETI_Watchdog; //instance
  static vector<string> set_wdt(string conf_path);
  static int file_check(string file_name);
public:
  static int set_wdt_flag;
  static void *messegequeue(void);
  static KETI_Watchdog *SingleInstance();
  static void wdt_start();
  void start();
};