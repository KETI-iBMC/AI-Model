#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>
//#include <sys/poll.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define LOOPBACK_DATALEN 30

static void
usage(FILE *fp, int argc, char **argv)
{
        fprintf(fp,
                "Usage: %s [options]\n\n"
                "Options:\n"
                " -h | --help          Print this message\n"
                " -f | --file          espi file node\n"
                " -r | --read          read \n"
                " -w | --write         write \n"
                "",
                argv[0]);
}

static const char short_options [] = "hwrlf:";

static const struct option
long_options [] = {
        { "help",               no_argument,                NULL,       'h' },
        { "file",               required_argument,          NULL,       'f' },
        { "read",               no_argument,          NULL,       'r' },
        { "write",    		no_argument,          NULL,       'w' },
        { "loopback",    		no_argument,          NULL,       'l' },
        { 0, 0, 0, 0 }
};

unsigned int StrToHex(char *p)
{
        int i, sum;
        int temp, length;
        char c;
        sum = 0;
        length = strlen(p);
        for( i = 0; i < (length) ; i++ )
        {
                c = *p;
                if( c >= 'a' && c <= 'z') {
                        temp = c - 87;
                        sum += ((temp) << (4*(length - i - 1)));
                } else if( c >= 'A' && c <= 'Z') {
                        temp = c - 55;
                        sum += ((temp) << (4*(length - i - 1)));
                } else {
                        temp = c - 48;
                        sum = sum + ((temp) << (4*(length - i - 1)));
                }

                p = p + 1;
        }
        return sum;
}

int main(int argc, char *argv[])
{
	char option;
	int mode_read = 0, mode_write = 0, loopback = 0;
	int i, r, len, tag, oob_type, pec_enable, oob_len;
	//smbus
	int dest_slave_addr, cmd_code, byte_count;
	//more mctp
	int src_slave_addr, hdr_ver, dest_eid, src_eid, mctp_msg;
	char filename[256];
	struct pollfd pfd;
	struct timespec ts;
	unsigned char data[1024];

	while ( (option=getopt_long(argc, argv, short_options, long_options, NULL)) != (char)-1 ) {
		switch(option) {
			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);
				break;
			case 'r':
				mode_read = 1;
				break;
			case 'w':
				mode_write = 1;
				break;
			case 'f':
				strcpy(filename, optarg);
				break;
			case 'l':
				loopback = 1;
				break;
		};
	}

	pfd.fd = open(filename, O_RDWR | O_NONBLOCK);
	if (pfd.fd < 0) {
		printf("can not open file %s \n", filename);
		return -1;
	}
	printf("oob channel : %s \n", filename);

	pfd.events = POLLPRI;

/*	
	16 byte header : 
	generic SMBus : 0x1
		buf[0] = tag, 		 buf[1] = type 0x1, 	buf[2] = pec enable, 	buf[3] = payload length
		buf[4] = dest_s_addr buf[5] : cmd code, 	buf[6 ~ 15] = reserved 

	mctp : 0x2
		buf[0] = tag, 		 buf[1] = type 0x2, 	buf[2] = pec enable, 	buf[3] = oob_length
		buf[4] = dest_s_addr buf[5] : cmd code, 	buf[6] : byte count 	buf[7] : src slave addr 
		buf[8] : hdr ver.	 buf[9] = dest epid,    buf[10] = src epid		buf[11] : msg tag,		
		buf[12 ~ 15] : reserved

	Customize type : 0x4
		buf[0] = tag,		 buf[1] = type 0x4, 	buf[2] = pec enable,	buf[3] = oob_length
		buf[4 ~ 15] : reserved 
		
	DATA begin from buf[16]
		
*/
	if (loopback) {
		char rx_buf[LOOPBACK_DATALEN + 4];
		char tx_buf[16 + LOOPBACK_DATALEN];

		while (1) {
			/*
			 * receive
			 * slave_addr (1B) + cmd_code (1B) + byte_cnt (1B) + payload + PEC (1B)
			 */
			while ((r = read(pfd.fd, rx_buf, sizeof(rx_buf))) == 0);
			if (r <= 0)
				continue;

			printf("receive %d bytes:", r);

			/* update */
			for (i = 0; i < r; i++) {
				if ((i % 16) == 0)
					printf("\n");
				printf("%02x ", rx_buf[i]);
			}

			memset(tx_buf, 0, sizeof(tx_buf));
			tx_buf[0] = 0x5;
			tx_buf[1] = 0x1;
			tx_buf[2] = 0x1;
			tx_buf[3] = rx_buf[2];
			tx_buf[4] = rx_buf[0] >> 1;
			tx_buf[5] = rx_buf[1];

			for (i = 0; i < rx_buf[2]; ++i)
				tx_buf[i + 16] = rx_buf[i + 3];

			/* response */
			write(pfd.fd, tx_buf, sizeof(tx_buf));

			printf("\n");
		}

		return 0;
	}

	if (mode_write) {
		printf("OOB: type [0: SMBUS, 1, MCTP, 2 Customize]");
		scanf("%d", &oob_type);

		/* prepare header */
		printf("OOB: tag [dex]");
		scanf("%d", &tag);
		data[0] = tag;

		printf("OOB: pec enable [dex]");
		scanf("%d", &pec_enable);
		if(pec_enable)
			data[2] = 1;
		else
			data[2] = 0;

		printf("OOB: oob_len [dex]");
		scanf("%d", &oob_len);
		data[3] = oob_len;
		len = oob_len + 16 + data[2];
		
		switch(oob_type) {
			case 0:	//smbus
				data[1] = 0x1;
				printf("OOB: dest slave addr [dex]");
				scanf("%d", &dest_slave_addr);
				data[4] = dest_slave_addr;

				printf("OOB: cmd code [dex]");
				scanf("%d", &cmd_code);
				data[5] = cmd_code;
				
				printf("OOB: byte count [dex]");
				scanf("%d", &byte_count);
				data[6] = byte_count;

				if(((oob_len + data[2]) - byte_count) !=  3) {
					printf("oob_len %d - byte_count %d mismatch, pec_enable %d %d\n", oob_len, byte_count, data[2], ((oob_len + data[2]) - byte_count));
					return -1;
				}
				
				break;
			case 1:	//mctp
				data[1] = 0x2;
				printf("OOB: dest slave addr [dex]");
				scanf("%d", &dest_slave_addr);
				data[4] = dest_slave_addr;

				printf("OOB: cmd code [dex]");
				scanf("%d", &cmd_code);
				data[5] = cmd_code;
				
				printf("OOB: byte count [dex]");
				scanf("%d", &byte_count);
				data[6] = byte_count;

				printf("OOB: src slave addr [dex]");
				scanf("%d", &src_slave_addr);
				data[7] = src_slave_addr;

				printf("OOB: header ver [dex]");
				scanf("%d", &hdr_ver);
				data[8] = hdr_ver;

				printf("OOB: dest_eid [dex]");
				scanf("%d", &dest_eid);
				data[9] = dest_eid;

				printf("OOB: src eid [dex]");
				scanf("%d", &src_eid);
				data[10] = src_eid;

				printf("OOB: mctp msg [dex]");
				scanf("%d", &mctp_msg);
				data[11] = mctp_msg;

				if(((oob_len + data[2]) - byte_count) !=  3) {
					printf("oob_len %d - byte_count %d mismatch, pec_enable %d \n", oob_len, byte_count, pec_enable);
					return -1;
				}
				break;
			case 2:	//customize
				data[3] = 0x4;
				break;
			default:
				return -1;
				break;
		}

#if 1
		//fix pattern
		for(i = 0; i < byte_count; i++)
			data[16 + i] = i;
#else
#endif

#if 0
		printf("\n");
		for(i = 0; i < len; i++) {
			printf("%x ", data[i]);
		}
		printf("\n");
#endif		
		write(pfd.fd, data, len);
	}

	if (mode_read) {
		while (1) {
			r = poll(&pfd, 1, 5000);

			if (r < 0)
				break;

			if (r == 0 || !(pfd.revents & POLLPRI))
				continue;

			lseek(pfd.fd, 0, SEEK_SET);
			r = read(pfd.fd, data, sizeof(data));
			if (r <= 0)
				continue;

			clock_gettime(CLOCK_MONOTONIC, &ts);
			printf("[%ld.%.9ld] :", ts.tv_sec, ts.tv_nsec);
			for (i = 0; i < r; i++)
				printf(" %02x", data[i]);
			printf("\n");
		}
	}

	close(pfd.fd);

	return 0;
}
