#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/socket.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include "video.h"
#include "regs-video.h"

#define JPEG_UPLOAD_TERM 10

static int GetINFData(PVIDEO_ENGINE_INFO VideoEngineInfo);
int int_width(int num);
int set_resolution_buffer(unsigned char* resolutionBuffer, int width, int height);
void *VideoStream(void *data);
void *TcpRun(void *data);
void *JpegSnapshot(void *data);
void _run_v_server();
