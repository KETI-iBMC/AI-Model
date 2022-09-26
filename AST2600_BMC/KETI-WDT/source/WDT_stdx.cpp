#pragma once
#include "WDT_stdx.hpp"
// int KETI_WDT_define::global_enabler = 0;

int KETI_WDT_define::Interval = 10;
int KETI_WDT_define::Timeout = 60;
int KETI_WDT_define::Pretimeout = 10;
int KETI_WDT_define::PretimeoutInterrupt;
int KETI_WDT_define::Action = 1;
char *KETI_WDT_define::Daemon = NULL;
char *KETI_WDT_define::Pidfile = NULL;
char *KETI_WDT_define::Debug = NULL;
int KETI_WDT_define::Reset_count = 0;

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
          fprintf(stderr, " Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::Interval = atol(Configurationline + i);

          { printf(" IPMI_Interval = %d \n", KETI_WDT_define::Interval); }
        }
      }

      /*Timeout */
      else if (strncmp(Configurationline + i, IPMI_TIMEOUT,
                       strlen(IPMI_TIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_TIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::Timeout = atol(Configurationline + i);
          // g_watchdog_config.initial_countdown_lsb = IPMI_Timeout & 0xFF;
          // g_watchdog_config.initial_countdown_msb = IPMI_Timeout >> 8;
          printf(" IPMI_Timeout = %d \n",KETI_WDT_define::Timeout);
          // printf(" initial_countdown_lsb = %d \n",
          //        g_watchdog_config.initial_countdown_msb);
          // printf(" initial_countdown_msb = %d \n",
          //        g_watchdog_config.initial_countdown_msb);
        }
      }

      /*Pretimeout */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUT,
                       strlen(IPMI_PRETIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::Pretimeout = atol(Configurationline + i);
          printf(" IPMI_Pretimeout = %d \n", KETI_WDT_define::Pretimeout);
        }
      }

      /*Daemon */
      else if (strncmp(Configurationline + i, IPMI_DAEMON,
                       strlen(IPMI_DAEMON)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_DAEMON))) {
          KETI_WDT_define::Daemon = NULL;
        } else {
          KETI_WDT_define::Daemon = strdup(Configurationline + i);

          printf(" IPMI_Daemon = %s \n", KETI_WDT_define::Daemon);
        }
      }

      /*PretimeoutInterrupt */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUTINTERRUPT,
                       strlen(IPMI_PRETIMEOUTINTERRUPT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUTINTERRUPT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::PretimeoutInterrupt = atol(Configurationline + i);
          printf(" IPMI_PretimeoutInterrupt = %d \n",
                 KETI_WDT_define::PretimeoutInterrupt);
        }
      }

      /*Action */
      else if (strncmp(Configurationline + i, IPMI_ACTION,
                       strlen(IPMI_ACTION)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_ACTION))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::Action = atol(Configurationline + i);
          printf(" IPMI_Action = %d \n",KETI_WDT_define::Action);
        }
      }

      /*Pidfile */
      else if (strncmp(Configurationline + i, IPMI_PIDFILE,
                       strlen(IPMI_PIDFILE)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PIDFILE))) {
          KETI_WDT_define::Pidfile = NULL;
        } else {
          KETI_WDT_define::Pidfile = strdup(Configurationline + i);
          printf(" IPMI_Pidfile = %s \n",  KETI_WDT_define::Pidfile);
        }
      }

      /*Reset count */
      else if (strncmp(Configurationline + i, IPMI_RESETCOUNT,
                       strlen(IPMI_RESETCOUNT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_RESETCOUNT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          KETI_WDT_define::Reset_count = atol(Configurationline + i);

          printf(" Reset_count = %d \n", KETI_WDT_define::Reset_count);
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
    return 0;
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

static int WriteConfigurationFile(char *file) {

  vector<string> lines;
  ifstream readFromFile(ConfigurationFileDir);
  if (readFromFile.is_open()) {
    lines.clear();
    while (!readFromFile.eof()) {
      string tmp;
      getline(readFromFile, tmp);
      cout << tmp << endl;
      lines.push_back(tmp);
    }
    readFromFile.close();
  } else {
    printf("%s not exist", ConfigurationFileDir);
    return 1;
  }
 std::ofstream writeFile(ConfigurationFileDir);
  if (writeFile.is_open()) {
    for (int i = 0; i < lines.size(); i++) {
      string tmp = lines[i];
      cout << tmp << endl;
      if (tmp.find(IPMI_INTERVAL) != string::npos) {
        tmp = "Interval = " + to_string(KETI_WDT_define::Interval);
      } else if (tmp.find(IPMI_TIMEOUT) != string::npos) {
        tmp = "Timeout = " + to_string(KETI_WDT_define::Timeout);
      } else if (tmp.find(IPMI_PRETIMEOUT) != string::npos) {
        tmp = "Pretimeout = " + to_string(KETI_WDT_define::Pretimeout);
      } else if (tmp.find(IPMI_DAEMON) != string::npos) {
        if (KETI_WDT_define::Daemon != NULL) {
        tmp = "Daemon = " + string(KETI_WDT_define::Daemon);
        }
      } else if (tmp.find(IPMI_ACTION) != string::npos) {
        tmp = "Action = " + to_string(KETI_WDT_define::Action);
      } else if (tmp.find(IPMI_PIDFILE) != string::npos) {
        if (KETI_WDT_define::Pidfile != NULL) {
          tmp = "Pidfile = " + string(KETI_WDT_define::Pidfile);
        }
      }
      else if (tmp.find(IPMI_RESETCOUNT) != string::npos) {
        tmp = "Reset_count = " + to_string(KETI_WDT_define::Reset_count);
      }
      if (i != lines.size() - 1) {
        tmp += "\n";
      }
      cout << "change =" << tmp << endl;
      writeFile.write(tmp.c_str(), tmp.size());
    }

  } else {
    printf("file not exist");
  }
  writeFile.close();
  return 1;
}