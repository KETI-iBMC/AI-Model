#pragma once
#include "khandler.hpp"
#include <csignal>
#include <iostream>
#include <ipmi/apps.hpp>
#include <ipmi/common.hpp>
#include <ipmi/fru.hpp>
#include <ipmi/ipmi.hpp>
#include <ipmi/sdr.hpp>
#include <ipmi/setting_service.hpp>
#include <mutex>
#include <redfish/resource.hpp>
#include <thread>
#include <uuid/uuid.h>
// #include "handler.hpp"
#include <redfish/logservice.hpp>
#include<ipmi/ipmb.hpp>

// #include <boost/log/trivial.hpp>
// unique_ptr<Handler> g_listener;
// unordered_map<string, Resource *> g_record;
// src::severity_logger<severity_level> g_logger;
// ServiceRoot *g_service_root;

#define RMCP_UDP_PORT 623
using namespace SOL;
void exit_cleanup(int signo) {
  std::cout << "Cleanup ... " << std::endl;
  mutex_destroy();
  exit(1);
}

int main(void) {
  //system("websockify 7787 :8878 &");
  extern char uuid_hex[16];
  extern char uuid_str[37];
  extern uuid_t uuid;
  // BOOST_LOG_SEV(g_logger, info);
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::info);
  uint8_t dev_node[100] = "";
  int sockfd = 0, enable = 1, kcs_fd, result;
  struct sockaddr_in server;
  pthread_t tid, tid_dcmi_power, tid_kcs;
log(info)<<"kcs dev open";
 sprintf(dev_node, "/dev/ipmi-kcs%d", 2);
 std::thread *t_kcs_handler;
  kcs_fd = open(dev_node, O_RDWR | O_APPEND);
  log(info)<<"kcs_fd="<<kcs_fd;
  init_dbus_check();
  if (kcs_fd > 0) {
   t_kcs_handler =new thread(kcs_handler, &kcs_fd);
  }
  else{
    std::cout << "KCS ERROR" << std::endl;
  }
  
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    std::cout << "Socket opening error" << std::endl;
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(RMCP_UDP_PORT);

  mutex_init();

  signal(SIGINT, exit_cleanup);
  signal(SIGKILL, exit_cleanup);
  signal(SIGTERM, exit_cleanup);
  log(info)<<"sol dev open";
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    log(info) << "keti-Ibmc erorr :  setsockopt SOL_SOCKET ";
  }
    log(info)<<"rmcp dev open";
  if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
    std::cout << "Socket bind error" << std::endl;
    exit_cleanup(0);
    exit(0);
  }

  std::cout << "Starting UDP Server Port : " << RMCP_UDP_PORT << std::endl;

  uuid_generate_time_safe(uuid);
  uuid_unparse_upper(uuid, uuid_str);

  system("/etc/init.d/S50nginx restart");

 

  json::value jv;

  memcpy(uuid_hex, uuid, sizeof(uuid_t));
  plat_ipmi_init(); // user, dcmi, lan, buildtime, sdr, sysinfo init
  plat_fru_device_init();
  if (plat_sdr_init() == -1)
    fprintf(stderr, "sdr init failed\n");
  if (load_g_setting() == -1)
    fprintf(stderr, "load g setting failed\n");
  std::thread t_lanplus(lanplus_handler, &sockfd);
  // std::thread t_dcmi(dcmi_power_handler, &sockfd);
  std::thread t_redfish(redfish_handler, &sockfd);
  std::thread t_ssdp(ssdp_handler);

  int bus = IPMB_I2C_BUS;
  std::thread t_ipmb(ipmb_handler, &bus);
  std::thread t_timer(timer_handler);

  std::thread t_random(random_db_data_handler);
  std::thread t_tmpdb(tmp_insert_db);
  // wdt msq 22.04.26
  // std::thread t_wdt_msq(wdt_msq);

  while (1) {
    pause();
  }
  // exit_cleanup(0);
  t_redfish.join();
  t_lanplus.join();
  t_ipmb.join();
  t_ssdp.join();
  t_timer.join();
  t_random.join();
  t_tmpdb.join();
  t_kcs_handler->join();
  
}