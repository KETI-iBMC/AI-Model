#pragma once
#include <condition_variable>
#include <iostream>
using namespace std;
class KETI_define {
public:
  static string NETWORK_CHANGE_PATH;
  static std::condition_variable network_cv;
  static int global_enabler;
  /**
   * @brief ip 주소를 바꿔주는 함수 eth0만 바꾸어준다.
   * @param ipv4 ip 주소 모든 정보가 입력되어야만 동작
   * @param netmask netmask 모든 정보가 입력되어야만 동작
   * @param gateway gatway 모든 정보가 입력되어야만 동작
   */
  static void IPv4_Change_File(string ipv4, string netmask, string gateway);
};