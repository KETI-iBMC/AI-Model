// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 Aspeed Technology Inc.
 */

/*
 * ============================
 * this test code is from
 * https://www.kernel.org/doc/Documentation/usb/gadget_hid.txt
 * Send and receive HID reports
 * ============================
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_LEN 512
#define MAX_VALUE_COUNT (64)
#define KEYBOARD (0)
#define MOUSE (1)
#define JOYSTICK (2)
#define PAUSE_MS (200) // pause between each value

// #define DEBUG

struct options {
  const char *opt;
  unsigned char val;
};

static struct options kmod[] = {{.opt = "--left-ctrl", .val = 0x01},
                                {.opt = "--right-ctrl", .val = 0x10},
                                {.opt = "--left-shift", .val = 0x02},
                                {.opt = "--right-shift", .val = 0x20},
                                {.opt = "--left-alt", .val = 0x04},
                                {.opt = "--right-alt", .val = 0x40},
                                {.opt = "--left-meta", .val = 0x08},
                                {.opt = "--right-meta", .val = 0x80},
                                {.opt = NULL}};

static struct options kval[] = {{.opt = "--return", .val = 0x28},
                                {.opt = "--esc", .val = 0x29},
                                {.opt = "--bckspc", .val = 0x2a},
                                {.opt = "--tab", .val = 0x2b},
                                {.opt = "--spacebar", .val = 0x2c},
                                {.opt = "--caps-lock", .val = 0x39},
                                {.opt = "--f1", .val = 0x3a},
                                {.opt = "--f2", .val = 0x3b},
                                {.opt = "--f3", .val = 0x3c},
                                {.opt = "--f4", .val = 0x3d},
                                {.opt = "--f5", .val = 0x3e},
                                {.opt = "--f6", .val = 0x3f},
                                {.opt = "--f7", .val = 0x40},
                                {.opt = "--f8", .val = 0x41},
                                {.opt = "--f9", .val = 0x42},
                                {.opt = "--f10", .val = 0x43},
                                {.opt = "--f11", .val = 0x44},
                                {.opt = "--f12", .val = 0x45},
                                {.opt = "--insert", .val = 0x49},
                                {.opt = "--home", .val = 0x4a},
                                {.opt = "--pageup", .val = 0x4b},
                                {.opt = "--del", .val = 0x4c},
                                {.opt = "--end", .val = 0x4d},
                                {.opt = "--pagedown", .val = 0x4e},
                                {.opt = "--right", .val = 0x4f},
                                {.opt = "--left", .val = 0x50},
                                {.opt = "--down", .val = 0x51},
                                {.opt = "--kp-enter", .val = 0x58},
                                {.opt = "--up", .val = 0x52},
                                {.opt = "--num-lock", .val = 0x53},
                                {.opt = NULL}};

int keyboard_fill_report(char report[8], char buf[BUF_LEN], int *hold) {
  char *tok = strtok(buf, " ");
  int key = 0;
  int i = 0;

  for (; tok != NULL; tok = strtok(NULL, " ")) {

    if (strcmp(tok, "--quit") == 0)
      return -1;

    if (strcmp(tok, "--hold") == 0) {
      *hold = 1;
      continue;
    }

    if (key < 6) {
      for (i = 0; kval[i].opt != NULL; i++)
        if (strcmp(tok, kval[i].opt) == 0) {
          report[2 + key++] = kval[i].val;
          break;
        }
      if (kval[i].opt != NULL)
        continue;
    }

    if (key < 6)
      if (islower(tok[0])) {
        report[2 + key++] = (tok[0] - ('a' - 0x04));
        continue;
      }

    for (i = 0; kmod[i].opt != NULL; i++)
      if (strcmp(tok, kmod[i].opt) == 0) {
        report[0] = report[0] | kmod[i].val;
        break;
      }
    if (kmod[i].opt != NULL)
      continue;

    if (key < 6)
      fprintf(stderr, "unknown option: %s\n", tok);
  }

  return 8;
}

static struct options mmod[] = {{.opt = "--b1", .val = 0x01},
                                {.opt = "--b2", .val = 0x02},
                                {.opt = "--b3", .val = 0x04},
                                {.opt = NULL}};

int mouse_fill_report(char report[8], char buf[BUF_LEN], int *hold) {
  char *tok = strtok(buf, " ");
  int mvt = 0;
  int i = 0;
  int val;

  for (; tok != NULL; tok = strtok(NULL, " ")) {

    if (strcmp(tok, "--quit") == 0)
      return -1;

    if (strcmp(tok, "--hold") == 0) {
      *hold = 1;
      continue;
    }

    for (i = 0; mmod[i].opt != NULL; i++)
      if (strcmp(tok, mmod[i].opt) == 0) {
        report[0] = report[0] | mmod[i].val;
        break;
      }

    if (mmod[i].opt != NULL)
      continue;

    if (!(tok[0] == '-' && tok[1] == '-') && mvt < 4) {
      errno = 0;
      val = atoi(tok);
      report[4 + mvt++] = (char)(val & 0xff);
      report[4 + mvt++] = (char)((val >> 8) & 0xff);

      if (errno != 0) {
        fprintf(stderr, "Bad value:'%s'\n", tok);
        mvt -= 2;
        report[4 + mvt] = 0;
      }

      continue;
    }

    fprintf(stderr, "unknown option: %s\n", tok);
  }

#ifdef DEBUG
  printf("report[0~7]: ");
  for (i = 0; i < 8; i++)
    printf("0x%x ", report[i]);

  printf("\n");
#endif

  return 8;
}

static struct options jmod[] = {
    {.opt = "--b1", .val = 0x10},         {.opt = "--b2", .val = 0x20},
    {.opt = "--b3", .val = 0x40},         {.opt = "--b4", .val = 0x80},
    {.opt = "--hat1", .val = 0x00},       {.opt = "--hat2", .val = 0x01},
    {.opt = "--hat3", .val = 0x02},       {.opt = "--hat4", .val = 0x03},
    {.opt = "--hatneutral", .val = 0x04}, {.opt = NULL}};

int joystick_fill_report(char report[8], char buf[BUF_LEN], int *hold) {
  char *tok = strtok(buf, " ");
  int mvt = 0;
  int i = 0;

  *hold = 1;

  /* set default hat position: neutral */
  report[3] = 0x04;

  for (; tok != NULL; tok = strtok(NULL, " ")) {

    if (strcmp(tok, "--quit") == 0)
      return -1;

    for (i = 0; jmod[i].opt != NULL; i++)
      if (strcmp(tok, jmod[i].opt) == 0) {
        report[3] = (report[3] & 0xF0) | jmod[i].val;
        break;
      }
    if (jmod[i].opt != NULL)
      continue;

    if (!(tok[0] == '-' && tok[1] == '-') && mvt < 3) {
      errno = 0;
      report[mvt++] = (char)strtol(tok, NULL, 0);
      if (errno != 0) {
        fprintf(stderr, "Bad value:'%s'\n", tok);
        report[mvt--] = 0;
      }
      continue;
    }

    fprintf(stderr, "unknown option: %s\n", tok);
  }
  return 4;
}

void print_options(char c) {
  int i = 0;

  if (c == 'k') {
    printf("	keyboard options:\n"
           "		--hold\n");
    for (i = 0; kmod[i].opt != NULL; i++)
      printf("\t\t%s\n", kmod[i].opt);
    printf("\n	keyboard values:\n"
           "		[a-z] or\n");
    for (i = 0; kval[i].opt != NULL; i++)
      printf("\t\t%-8s%s", kval[i].opt, i % 2 ? "\n" : "");
    printf("\n");
  } else if (c == 'm') {
    printf("	mouse options:\n"
           "		--hold\n");
    for (i = 0; mmod[i].opt != NULL; i++)
      printf("\t\t%s\n", mmod[i].opt);
    printf("\n	mouse values:\n"
           "		Two signed numbers\n"
           "--quit to close\n");
  } else {
    printf("	joystick options:\n");
    for (i = 0; jmod[i].opt != NULL; i++)
      printf("\t\t%s\n", jmod[i].opt);
    printf("\n	joystick values:\n"
           "		three signed numbers\n"
           "--quit to close\n");
  }
}

void commandLineUsage(void) {
  printf("\n==================================================================="
         "==========\n");
  printf("\tUsage:  hid_gadget_test devname mouse|keyboard|joystick  "
         "[inputFile N]\n\n");
  printf("\tOption: inputFile N:\n");
  printf("\t\tinputFile - keyboard or mouse values (max 64 values) separated "
         "by newline.\n");
  printf("\t\tN    - times to run the values, 0 - run it till you stop this "
         "test\n");
  printf("\t\te.g:\n");
  printf("\t\t==interactive== hid_gadget_test /dev/hidg0 keyboard\n");
  printf("====================================================================="
         "========\n\n");
}

int main(int argc, const char *argv[]) {
  const char *filename = NULL;
  int fd = 0;
  int fd_input = 0;
  char buf[BUF_LEN];
  int cmd_len;
  char report[8];
  int to_send = 8;
  int hold = 0;
  fd_set rfds;
  int retval, i;
  // if (argc < 3) {
  // 	fprintf(stderr, "Usage: %s devname mouse|keyboard|joystick\n", argv[0]);
  // 	return 1;
  // }

  // if (argv[2][0] != 'k' && argv[2][0] != 'm' && argv[2][0] != 'j')
  // 	return 2;

  // filename = argv[1];

  fd = open("/dev/hidg1", O_RDWR, 0666);
  if (fd == -1) {
    perror("not open");
    return 3;
  }
  int k = 0;
  while (1) {
    k += 100;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_SET(fd, &rfds);
    retval = select(fd + 1, &rfds, NULL, NULL, NULL);
    if (retval == -1 && errno == EINTR)
      continue;
    if (retval < 0) {
      perror("select()");
      return 4;
    }
    // memset(report, 0x0, sizeof(report));

    report[0] = k % 2;
    report[1] = (0xff & (k));
    report[2] = (0xff & (k >> 8));
    report[3] = 0x04;
    report[4] = (0xff & (k >> 8));
    report[5] = (0xff & (k >> 8));
    report[6] = (0xff & (k >> 8));
    report[7] = (0xff & (k >> 8));
    printf("1\n");
    char buf[BUF_LEN] = "45 10";
    char buf2[BUF_LEN] = "0 0";
    char buf3[BUF_LEN] = "1600 450";
    // to_send = joystick_fill_report(report, buf3, &hold);
    to_send = 8;
    printf("2\n");
    printf("send!%d\n", k);
    if (write(fd, report, to_send) != to_send) {
      perror("not send\n");
      return 5;
    }
  }
  close(fd);

  if (fd_input)
    close(fd_input);

  return 0;
}