/**
 * @file KETI_stdx.cpp
 * @author kichol Park (you@domain.com)
 * @brief 유일 공유변수를 C++ 스타일로 리팩토링
 * @version 0.1
 * @date 2022-04-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <ipmi/KETI_stdx.hpp>
#include <stdlib.h>
using namespace std;
string KETI_define::NETWORK_CHANGE_PATH = "/etc/network/network_change.sh";
int KETI_define::global_enabler = 0;
void KETI_define::IPv4_Change_File(string ipv4, string netmask,
                                   string gateway) {
  string b = NETWORK_CHANGE_PATH;
  b += " -c " + ipv4 + " -n " + netmask + " -g " + gateway;
  cout << "cli command =" << b << endl;
  system(b.c_str());
  cout << "change network" << endl;
}