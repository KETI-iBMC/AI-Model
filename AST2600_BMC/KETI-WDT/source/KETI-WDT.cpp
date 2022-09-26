#pragma once
#include <KETI-WDT.hpp>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <thread>
#include <mutex>

std::mutex define_mu;
int KETI_Watchdog::set_wdt_flag = 0;

KETI_Watchdog *KETI_Watchdog::ins_KETI_Watchdog=nullptr;
KETI_Watchdog *KETI_Watchdog::SingleInstance()
{
  if(KETI_Watchdog::ins_KETI_Watchdog==nullptr){
    KETI_Watchdog::ins_KETI_Watchdog=new KETI_Watchdog();
  }
    return KETI_Watchdog::ins_KETI_Watchdog;

}

void KETI_Watchdog::start(){

  std::thread t_wdt_msq( KETI_Watchdog::messegequeue);
  std::thread t_wdt_start( KETI_Watchdog::wdt_start);
  while(true){
    pause();
  }
  t_wdt_start.join();
  t_wdt_msq.join();
}
/**
* @brief wdt msg queue
* @author kys
* @return void
* @details thread action msg queue
*/
static void *KETI_Watchdog::messegequeue(void){

  int loop_count=0;
  int msqid_key;
  struct   msqid_ds msqstat;
 
  //req msg queue create 
  if ( -1 == ( msqid_key = msgget( (key_t)1037, IPC_CREAT | 0666))){
    perror ("msgget in req fail");
    exit(1);
  }
  
  //msg queue reset
  // msgctl(msqid_key, IPC_RMID, NULL);

  wdt_msq_t msq_req;
  wdt_msq_t msq_rsp;
  
  while(1){
    cout << "wait rcv " << endl;
    if ( -1 == msgrcv( msqid_key, &msq_rsp, sizeof(wdt_msq_t)-sizeof(long), 0, 0))
    {
      perror("msgrcv in req failed");      
    }
    
  cout << " msg rcv " << msq_rsp.flag << endl;
  
  if(msq_rsp.flag >= 1){
    
    KETI_Watchdog::set_wdt_flag = 1;
    msq_rsp.flag = 0;

    msq_req.type = 2;
    msq_req.flag = 1;

    cout << " msg snd "<<endl;
    if ( -1 == msgsnd(msqid_key, &msq_req, sizeof(wdt_msq_t)-sizeof(long), 0))
    {
        perror( "msgsnd() in req실패");
    }
  }
  sleep(1);
  }
}

/**
* @brief set_wdt
* @author kys
* @return success 0 error -1
* @details watchdog setting timeout
*/
vector<string> KETI_Watchdog::set_wdt(string conf_path){
  int errno;
  int fd;

  ReadConfigurationFile(conf_path.c_str());

  if((fd = open(WDT_PATH, O_WRONLY))<0) {
    cout << "/dev/watchdog open error" <<endl;

  }
  //Timeout
  errno=ioctl(fd, WDIOC_SETTIMEOUT,&KETI_WDT_define::Timeout);
  if(errno)
    printf("ioctl error WDIOC_SETTIMEOUT %s \n",strerror(errno));
  //Pretimeout
  ioctl(fd, WDIOC_SETPRETIMEOUT,&KETI_WDT_define::Pretimeout);
  if(errno)
    printf("ioctl error WDIOC_SETPRETIMEOUT %s \n",strerror(errno));
  
  close(fd);

cout << "split start" <<endl;
vector<string> proc_file;
char *ptr = KETI_WDT_define::Pidfile;
istringstream ss(ptr);
string buf;
proc_file.clear();

while(getline(ss,buf,' ')){
  if(file_check(buf)!= -1)
  proc_file.push_back(buf);
}
cout<< "end split"<<endl;
  return proc_file;
}
/**
* @brief file_check
* @author kys
* @return success 1 error -1
* @details KETI exection file check
*/
static int KETI_Watchdog::file_check(string file_name){

  FILE *fp ;
  char cmd[128]={0},buffer[512]={0};
  string ps_buffer;
  int count=0;

  cout << "find file"<<endl;
  cout << file_name<<endl;
  sprintf(cmd, "find | grep %s" ,file_name.c_str());
  fp = popen(cmd, "r");
  fread(buffer, sizeof(char), sizeof(buffer), fp);
  ps_buffer.clear();
  ps_buffer += buffer;

  if (ps_buffer.find(file_name) == string::npos){ 
    cout << "no such file : " << file_name <<endl;
    pclose(fp);
    return -1;
  }
  else cout << "find file : " << file_name <<endl;

  pclose(fp);
  return 1;
}

static void KETI_Watchdog::wdt_start(){
  int flag = 1;
  int errno;
  int get_wdt_time;
  int file_num =0 ;
  int i =0;
  int fd;
  vector<string> proc_file;
  proc_file = set_wdt(ConfigurationFileDir);

  file_num = proc_file.size();
  KETI_FILE proc_name[file_num];

  for(i =0 ;i<proc_file.size();i++){
    proc_name[i].set_file_name(proc_file[i]);
  }
  // watchdog kicking
    if((fd = open(WDT_PATH, O_WRONLY))<0) {
    cout << "/dev/watchdog open error" <<endl;
    return;
  }

  while(flag){

  if( KETI_Watchdog::set_wdt_flag){ 
    proc_file = set_wdt(ConfigurationFileDir);

    file_num = proc_file.size();
    KETI_FILE proc_name[file_num];

    for(i =0 ;i<proc_file.size();i++){
      proc_name[i].set_file_name(proc_file[i]);
    }
    KETI_Watchdog::set_wdt_flag = 0;
  }

  for(i =0 ;i<proc_file.size();i++){
    proc_name[i].pid_find();
    if(5 < proc_name[i].get_error_count()){
      KETI_WDT_define::Reset_count++;
      WriteConfigurationFile(ConfigurationFileDir);
      // flag =0;  // 재부팅됨
    }
  }

    ioctl(fd, WDIOC_KEEPALIVE,0);
    if(errno)
      printf("ioctl error WDIOC_KEEPALIVE %s \n",strerror(errno));
    
    ioctl(fd, WDIOC_GETTIMELEFT,&get_wdt_time);
    if(errno)
      printf("ioctl error WDIOC_GETTIMELEFT %s \n",strerror(errno));  
    cout<< "get_wdt_time : "<< get_wdt_time<<endl;
    sleep(KETI_WDT_define::Interval);
  // sleep(1);
  }
  close(fd);
  return ;
}
