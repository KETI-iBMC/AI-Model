#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <unistd.h>

#include "peci-ioctl.h"

#define BUFFER_SIZE 64

static const char short_options [] = "hpitcd:";

static const struct option
	long_options [] = {
	{ "help",	no_argument,		NULL,	'h' },
	{ "ping",	no_argument,		NULL,	'p' },
	{ "device",	required_argument,      NULL,	'd' },
	{ "info",	no_argument,		NULL,	'i' },
	{ "temp",	no_argument,		NULL,	't' },
	{ "customize",	no_argument,		NULL,	'c' },
	{ 0, 0, 0, 0 }
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
		"Usage: %s [options]\n\n"
		"Options:\n"
		" -h | --help                   Print this message\n"
		" -d | --device                 device node\n"
		" -p | --ping                   Ping command test\n"
		" -i | --devinfo                GetDIB command test\n"
		" -t | --temp                   GetTemp command test\n"
		" -c | --customize              Customized command test\n"
		"",
		argv[0]);
}

int main(int argc, char *argv[])
{
	char option;
	int ping = 0;
	int temp = 0;
	int id = 0;
    int cust = 0;
	int i = 0;

	int peci_fp;

	char devpath[20];
 
	uint8_t tx_buf[BUFFER_SIZE];

	while ((option = getopt_long(argc, argv, short_options, long_options, NULL)) != (char) -1) {
		switch (option) {
			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);
				break;
			case 'i':
				id = 1;
				break;
			case 'p':
				ping = 1;
				break;
			case 'd':
				memset(devpath, 0, 20);
				strcpy(devpath, optarg);
				break;
			case 't':
				temp = 1;
				break;
			case 'c':
				cust = 1;
				break;
			default:
				usage(stdout, argc, argv);
				exit(EXIT_FAILURE);
		}
	}

	memset(tx_buf, 0x00, BUFFER_SIZE);
	printf("Open devnode : %s\n", devpath);
	
	peci_fp = open(devpath, O_RDWR | O_CLOEXEC);

 	if (peci_fp < 0) {
 		printf("Couldn't open %s with flags O_RDWR: %s\n",
			devpath, strerror(errno));
 		return -errno;
 	}

	if (ping) {
		struct peci_ping_msg ping_msg;
		printf("ping address 0x30 \n");
		ping_msg.addr = 0x30;
		if (!ioctl(peci_fp, PECI_IOC_PING, &ping_msg))
			printf("ping ack \n");
		else
			printf("ping nack \n");
	}

	if (id) {
		printf("get address 0x30 \n");
		struct peci_xfer_msg msg;
		
		msg.addr = 0x30;
		msg.tx_len = 1;
		msg.rx_len = 8;
		msg.tx_buf[0] = 0xf7;
		if (!ioctl(peci_fp, PECI_IOC_XFER, &msg)) { 
			printf("ok \n");
			for( i = 0; i < msg.rx_len; i++) 
				printf("%x ", msg.rx_buf[i]);
			printf("\n");
		} else 
			printf("ping fail \n");
		
	}
	
	if (temp) {
		printf("get address 0x30 \n");
		struct peci_xfer_msg msg;
		
		msg.addr = 0x30;
		msg.tx_len = 1;
		msg.rx_len = 2; 
		msg.tx_buf[0] = 0x1;
		if (!ioctl(peci_fp, PECI_IOC_XFER, &msg)) { 
			printf("ok \n");
			for(i = 0; i < msg.rx_len; i++) 
				printf("%x ", msg.rx_buf[i]);
			printf("\n");
		} else 
			printf("temp fail \n");
	}

	if (cust) {
		printf("get address 0xf1 \n");
		struct peci_xfer_msg msg;

		msg.addr = 0xf1;
		msg.tx_len = 0;
		msg.rx_len = 1;
		if (!ioctl(peci_fp, PECI_IOC_XFER, &msg)) {
			printf("ok \n");
			for (i = 0; i < msg.rx_len; i++)
				printf("%x ", msg.rx_buf[i]);
			printf("\n");
		} else
			printf("cust fail\n");
	}

	close(peci_fp);
	return 0;
}
