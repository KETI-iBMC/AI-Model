#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <sstream>

#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;

#define KETI_Edge "KETI-Edge"
#define KETI_REST "KETI-REST"
#define WDT_PATH "/dev/watchdog"

class KETI_FILE{
  private:
    string file_path="./";
    string file_name;
    int error_count = 0;
    
  public :
    KETI_FILE(string file_name){
      this->file_path+=file_name;
      this->file_name = file_name;
      this->error_count = 0;
    };
    KETI_FILE(){};
    void set_file_name(string file_name){
      this->file_name = file_name;
      this->file_path +=file_name;
    }
    string get_file_name(){
      return this->file_name;
    }
    string get_file_path(){
      return this->file_path;
    }
    void error_count_up(){
      this->error_count++;
    }
    int get_error_count(){
      return error_count;
    }
    ~KETI_FILE(){};
    void pid_find();
    void pid_execution();
    
};
