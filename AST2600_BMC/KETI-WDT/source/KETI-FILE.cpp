#include "KETI-FILE.hpp"

using namespace std;

/**
* @brief file pid find
* @author kys
* @return success 0 error -1 
* @details KETI file pid find
*/
void KETI_FILE::pid_find(){

  FILE *fp ;
  char cmd[128]={0},buffer[512]={0};
  int count=0;

  cout << "find pid"<<endl;
  cout << this->get_file_name()<<endl;
  sprintf(cmd, "ps -ef | grep %s" ,this->get_file_name().c_str());
  fp = popen(cmd, "r");
  fread(buffer, sizeof(char), sizeof(buffer), fp);
  
  for(int i=0;i<512;i++){
    if(buffer[i]=='\n')
      count++;
  }
  if(count < 3){  
    this->error_count_up();
    pid_execution();
  }
  pclose(fp);
}

/**
* @brief file_execution
* @author kys
* @return void
* @details error file restart
*/
void KETI_FILE::pid_execution(){
  
  cout<< "file execution"<<endl;
  string restart = this->get_file_path()+" &";
  cout <<restart<<endl;
  if(system(restart.c_str()) == -1)
    cout << "pid execution error" << endl;
}

