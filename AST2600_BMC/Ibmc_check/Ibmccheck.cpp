#include <iostream>
#include <thread>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <cstdlib>

using namespace std;

char *get_popen_string(char *command)
{
    FILE *fp = popen(command, "r");
    char *temp = (char *)malloc(sizeof(char)*256);
    if(fp != NULL)
    {
        while(fgets(temp, 256, fp) != NULL)
        {
        }
        pclose(fp);
    }
    if (temp[strlen(temp) -1] == '\n')
        temp[strlen(temp) -1] = '\0';
    return temp;
}

string get_popen_string(string command)
{
    // log(warning) << command;
    FILE *fp = popen(command.c_str(), "r");
    char *temp = (char *)malloc(sizeof(char)*256);
    string ret;
    if (fp != NULL){
        while(fgets(temp, 256, fp) != NULL)
        {
            string str(temp);
            ret += temp;
        }
        pclose(fp);
    }
    if (ret.back() == '\n')
        ret.pop_back();
        
    // log(warning) << ret;
    return ret;
}

void *Ibmc_check_handler(void)
{
    while(1)
    {
        usleep(3000000);
        string cmd = "ps | grep KETI-Ibmc | grep -v grep";
        string result = get_popen_string(cmd.c_str());
        if(result == "")
        {
            // cout << " nnn" << endl;
            system("/firmware/KETI-Ibmc &");
        }
        else;
            // cout << " ddd" << endl;

    }
    
    
}


int main(void)
{
    std::thread t_Ibmc(Ibmc_check_handler);
    while (1) {
    pause();
    }
    t_Ibmc.join();
    return 0; 
}