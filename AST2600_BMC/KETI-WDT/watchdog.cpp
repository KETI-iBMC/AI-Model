
#pragma once
#include "WDT_stdx.hpp"
#include "watchdog.hpp"
#include <fcntl.h>
#include <linux/watchdog.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <thread>
// #include <libexplain/ioctl.h>

static bmc_watchdog_param_t g_watchdog_config;

#define PID_FILE_PATH "/conf/ipmi/WDT.pid"
// int main(int argc, char *argv[]) {

//   printf("configfile read\n");
//   ReadConfigurationFile(ConfigurationFileDir);

//   FILE *fp;
//   char cmd[128], buffer[512];
//   sprintf(cmd, "ps -ef | grep %s", "KETI-WDT");

//   fp = popen(cmd, "r");

//   fread(buffer, sizeof(char), sizeof(buffer), fp);
//   // printf("process pid : %s",buffer);
//   pclose(fp);

//   int msqid_req, msqid_rsp;
//   msgctl((key_t)1038, IPC_RMID, NULL);

//   // req msg queue create
//   if (-1 == (msqid_req = msgget((key_t)1038, IPC_CREAT | 0666))) {
//     perror("msgget in req fail");
//     exit(1);
//   }
//   // rsp msg queue create
//   if (-1 == (msqid_rsp = msgget((key_t)5037, IPC_CREAT | 0666))) {
//     perror("msgget in rsp fail");
//     exit(1);
//   }

//   msq_rsp_t msg_rsp;
//   msq_rsp_t msg_req;

//   msg_req.type = 1;

//   pid_t pid = getpid();

//   int count = 0;
//   int errsv = errno;

//   // thread 사용해야할듯
//   while (1) {
//     printf("loop count %d \n", count++);

//     int i, j; // WAITSECONDS 시간만큼만 응답 기다림
//     for (i = 0; i < WAITSECONDS; i++) {
//       printf("msg rcv %d \n", msg_rsp.data);
//       if (-1 ==
//           msgrcv(msqid_rsp, &msg_rsp, sizeof(msq_rsp_t) - sizeof(long), 0,
//           0)) {
//         perror("msgrcv in req failed");
//         exit(1); // while문 돌면서 request msg가 오기를 기다림
//         // usleep(500);
//         // continue;
//       } else
//         break;
//     }

//     printf("msg_rsp %d : \n", msg_rsp.data);
//     msg_req.type = 1;
//     strcpy(msg_req.data, "data");

//     printf("msg snd\n");
//     if (-1 ==
//         msgsnd(msqid_req, &msg_req, sizeof(msq_rsp_t) - sizeof(long), 0)) {
//       perror("msgsnd() in req실패");
//       exit(1);
//     }
//   }

//  int fd = open("/dev/watchdog", O_WRONLY);
//     int ret = 0;
//     errsv = errno;
//     printf("ioctl failed and returned errno %s \n",strerror(errsv));
//     if (fd == -1) {
//             perror("watchdog");
//             exit(EXIT_FAILURE);
//     }
//     while (1) {
//             ret = write(fd, "\0", 1);
//             if (ret != 1) {
//                     ret = -1;
//                     break;
//             }
//             sleep(10);
//     }
//     close(fd);
// int fd=open("/dev/watchdog",O_APPEND );
// int timeout = 90;
// printf("fileopen %d \n", fd);
//   errsv = errno;
//     printf("ioctl failed and returned errno %s \n",strerror(errsv));

// ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
// errsv = errno;
// printf("ioctl failed and returned errno %s \n",strerror(errsv));

// ioctl(fd, WDIOC_KEEPALIVE, 0);
// errsv = errno;
// printf("ioctl failed and returned errno %s \n",strerror(errsv));

// printf("The timeout was set to %d seconds\n", timeout);
// //printf("error_code %d\n",error_code);
// // fd=open("/dev/watchdog",O_WRONLY);
// int get_timeout;
// printf("fileopen\n");
// ioctl(fd, WDIOC_GETTIMEOUT, &get_timeout);
// errsv = errno;
// printf("ioctl failed and returned errno %s \n",strerror(errsv));
// close(fd);
// printf("The timeout was get to %d seconds\n", get_timeout);

// char message[3000];
// explain_message_errno_ioctl(message, sizeof(message), fd, WDIOC_SETTIMEOUT,
// &timeout)); printf("explain %s\n", message);

// unsigned int *pid = getpid();
// printf("pid : %d\n",pid);
// FILE *fp;
// fp = fopen(PID_FILE_PATH, "w+");
// printf("file\n");
// if(fwrite(&pid, sizeof(unsigned int), 1, fp) <= 0){
// 	printf("set_eft_entry_num: fwrite");
// 	fclose(fp);
// 	return -1;
// }
//}
static int ReadConfigurationFile(char *file) {
  FILE *ReadConfigurationFile;

  /* Open the configuration file with readonly parameter*/
  printf("Trying the configuration file %s \n", ConfigurationFileDir);
  if ((ReadConfigurationFile = fopen(ConfigurationFileDir, "r")) == NULL) {
    printf("There is no configuration file, use default values for IPMI "
           "watchdog \n");
    return (1);
  }

  /* Check to see the configuration has data or not*/
  while (!feof(ReadConfigurationFile)) {
    char Configurationline[CONFIG_LINE_LEN];

    /* Read the line from configuration file */
    if (fgets(Configurationline, CONFIG_LINE_LEN, ReadConfigurationFile) ==
        NULL) {
      if (!ferror(ReadConfigurationFile)) {
        break;
      } else {
        return (1);
      }
    } else {
      int i, j;

      /* scan the actual line for an option , first remove the leading blanks*/
      for (i = 0; Configurationline[i] == ' ' || Configurationline[i] == '\t';
           i++)
        ;

      /* if the next sign is a '#' we have a comment , so we ignore the
       * configuration line */
      if (Configurationline[i] == '#') {
        continue;
      }

      /* also remove the trailing blanks and the \n */
      for (j = strlen(Configurationline) - 1;
           Configurationline[j] == ' ' || Configurationline[j] == '\t' ||
           Configurationline[j] == '\n';
           j--)
        ;

      Configurationline[j + 1] = '\0';

      /* if the line is empty now, we don't have to parse it */
      if (strlen(Configurationline + i) == 0) {
        continue;
      }

      /* now check for an option , interval first */

      /*Interval */
      if (strncmp(Configurationline + i, IPMI_INTERVAL,
                  strlen(IPMI_INTERVAL)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_INTERVAL))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::IPMI_Interval = atol(Configurationline + i);

          { printf(" IPMI_Interval = %d \n", KETI_WDT_define::IPMI_Interval); }
        }
      }

      /*Timeout */
      else if (strncmp(Configurationline + i, IPMI_TIMEOUT,
                       strlen(IPMI_TIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_TIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::IPMI_Timeout = atol(Configurationline + i);
          g_watchdog_config.initial_countdown_lsb =
              KETI_WDT_define::IPMI_Timeout & 0xFF;
          g_watchdog_config.initial_countdown_msb =
              KETI_WDT_define::IPMI_Timeout >> 8;
          printf(" IPMI_Timeout = %d \n", KETI_WDT_define::IPMI_Timeout);
          printf(" initial_countdown_lsb = %d \n",
                 g_watchdog_config.initial_countdown_msb);
          printf(" initial_countdown_msb = %d \n",
                 g_watchdog_config.initial_countdown_msb);
        }
      }

      /*Pretimeout */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUT,
                       strlen(IPMI_PRETIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::IPMI_Pretimeout = atol(Configurationline + i);

          g_watchdog_config.pre_timeout = KETI_WDT_define::IPMI_Pretimeout;
          printf(" IPMI_Pretimeout = %d \n", KETI_WDT_define::IPMI_Pretimeout);
        }
      }

      /*Daemon */
      else if (strncmp(Configurationline + i, IPMI_DAEMON,
                       strlen(IPMI_DAEMON)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_DAEMON))) {
          KETI_WDT_define::IPMI_Daemon = NULL;
        } else {
          KETI_WDT_define::IPMI_Daemon = strdup(Configurationline + i);

          printf(" IPMI_Daemon = %s \n", KETI_WDT_define::IPMI_Daemon);
        }
      }

      /*PretimeoutInterrupt */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUTINTERRUPT,
                       strlen(IPMI_PRETIMEOUTINTERRUPT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUTINTERRUPT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          g_watchdog_config.pretimeoutInterrupt = atol(Configurationline + i);
          printf(" IPMI_PretimeoutInterrupt = %d \n",
                 g_watchdog_config.pretimeoutInterrupt);
        }
      }

      /*Action */
      else if (strncmp(Configurationline + i, IPMI_ACTION,
                       strlen(IPMI_ACTION)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_ACTION))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::IPMI_Action = atol(Configurationline + i);
          g_watchdog_config.timer_actions = KETI_WDT_define::IPMI_Action;
          printf(" IPMI_Action = %d \n", KETI_WDT_define::IPMI_Action);
        }
      }

      /*Pidfile */
      else if (strncmp(Configurationline + i, IPMI_PIDFILE,
                       strlen(IPMI_PIDFILE)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PIDFILE))) {
          KETI_WDT_define::IPMI_Pidfile = NULL;
        } else {
          KETI_WDT_define::IPMI_Pidfile = strdup(Configurationline + i);

          printf(" IPMI_Pidfile = %s \n", KETI_WDT_define::IPMI_Pidfile);
        }
      }

      else {
        fprintf(stderr, "Ignoring config Configurationline: %s\n",
                Configurationline);
      }
    }
  }

  /* Close the configuration file */
  if (fclose(ReadConfigurationFile) != 0) {
    return (1);
  }
}

static int spool(char *line, int *i, int offset) {
  for ((*i) += offset; line[*i] == ' ' || line[*i] == '\t'; (*i)++)
    ;
  if (line[*i] == '=')
    (*i)++;
  for (; line[*i] == ' ' || line[*i] == '\t'; (*i)++)
    ;
  if (line[*i] == '\0')
    return (1);
  else
    return (0);
}

// void *messegequeue(void *keyvalue) {
//   key_t me_key = (key_t)keyvalue;

//   try {

//   } catch (const std::exception &) {
//   }
// }
KETI_Watchdog *KETI_Watchdog::ins_KETI_Watchdog = nullptr;
KETI_Watchdog::KETI_Watchdog() {
  cout << "Load WDT config" << endl;
  ReadConfigurationFile(ConfigurationFileDir);
}
KETI_Watchdog *KETI_Watchdog::SingleInstance() {
  if (KETI_Watchdog::ins_KETI_Watchdog == nullptr)
    KETI_Watchdog::ins_KETI_Watchdog = new KETI_Watchdog();
  return KETI_Watchdog::ins_KETI_Watchdog;
}

void KETI_Watchdog::start() {
  cout << "WDT-Service start" << endl;
  key_t key = "4885";
  // std::thread t_messegequeue(messegequeue, &key);
  std::mutex m;
  std::unique_lock<std::mutex> lk(m);
  while (true) {
    this->WDT_cv.wait(lk, [&] { return this->lockFlag; });
  }
  // t_messegequeue.join();
}
