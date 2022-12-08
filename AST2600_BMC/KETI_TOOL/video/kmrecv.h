#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#define BUF_LEN 32

struct options {
        const char    *opt;
        unsigned char val;
};

static struct options kmod[] = {
        {.opt = "--left-ctrl",          .val = 0x01},
        {.opt = "--right-ctrl",         .val = 0x10},
        {.opt = "--left-shift",         .val = 0x02},
        {.opt = "--right-shift",        .val = 0x20},
        {.opt = "--left-alt",           .val = 0x04},
        {.opt = "--right-alt",          .val = 0x40},
        {.opt = "--left-meta",          .val = 0x08},
        {.opt = "--right-meta",         .val = 0x80},
        {.opt = NULL} 
};

static struct options kval[] = {
        {.opt = "--return",     .val = 0x28},   //0
        {.opt = "--esc",        .val = 0x29},   //1
        {.opt = "--bckspc",     .val = 0x2a},   //2
        {.opt = "--tab",        .val = 0x2b},   //3
        {.opt = "--spacebar",   .val = 0x2c},   //4
        {.opt = "--caps-lock",  .val = 0x39},   //5
        {.opt = "--f1",         .val = 0x3a},   //6
        {.opt = "--f2",         .val = 0x3b},   //7
        {.opt = "--f3",         .val = 0x3c},   //8
        {.opt = "--f4",         .val = 0x3d},   //9
        {.opt = "--f5",         .val = 0x3e},   //10
        {.opt = "--f6",         .val = 0x3f},   //11
        {.opt = "--f7",         .val = 0x40},   //12
        {.opt = "--f8",         .val = 0x41},   //13
        {.opt = "--f9",         .val = 0x42},   //14
        {.opt = "--f10",        .val = 0x43},   //15
        {.opt = "--f11",        .val = 0x44},   //16
        {.opt = "--f12",        .val = 0x45},   //17
        {.opt = "--insert",     .val = 0x49},   //18
        {.opt = "--home",       .val = 0x4a},   //19 
	{.opt = "--pageup",     .val = 0x4b},   //20
        {.opt = "--del",        .val = 0x4c},   //21
        {.opt = "--end",        .val = 0x4d},   //22
        {.opt = "--pagedown",   .val = 0x4e},   //23
        {.opt = "--right",      .val = 0x4f},   //24
        {.opt = "--left",       .val = 0x50},   //25
        {.opt = "--down",       .val = 0x51},   //26
        {.opt = "--kp-enter",   .val = 0x58},   //27
        {.opt = "--up",         .val = 0x52},   //28
        {.opt = "--num-lock",   .val = 0x53},   //29
        {.opt = "--equals",     .val = 0x2e},   //30
        {.opt = "--obracket",   .val = 0x2f},   //31
        {.opt = "--bslash",     .val = 0x32},   //32
        {.opt = "--semicol",    .val = 0x33},   //33
        {.opt = "--squote",     .val = 0x34},   //34
        {.opt = "--bquote",     .val = 0x35},   //35
        {.opt = "--comma",      .val = 0x36},   //36
        {.opt = "--dot",        .val = 0x37},   //37
        {.opt = "--fslash",     .val = 0x38},   //38
        {.opt = "--cbracket",   .val = 0x30},   //39
        {.opt = "--hyphen",     .val = 0x2d},   //40
        {.opt = "--kp-slash",   .val = 0x54},   //41
        {.opt = "--kp-star",    .val = 0x55},   //42
        {.opt = "--kp-hiphen",  .val = 0x56},   //43
        {.opt = "--kp-plus",    .val = 0x57},   //44
        {.opt = "--kp-dot",     .val = 0x63},   //45
        //{.opt = "--",   .val = 0x},   //

        {.opt = NULL}
};

int keyboard_fill_report(char report[8], char buf[BUF_LEN], int* hold);
void keyboard_run();
void parse_k_input(char* input);
void mouse_run();
void parse_m_input(char* input);
void _run_km_server();