#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <termios.h>

#include <sys/mman.h>

/*************************************************************************************/
#define MAX_XFER_BUFF_SIZE 4096

struct ast_xdma_xfer {
	unsigned char	stream_dir;
	unsigned char	*xfer_buff;
	unsigned int	xfer_len;
	unsigned int	bmc_addr;
	unsigned int	host_addr_low;
	unsigned int	host_addr_high;
};

#define XDMAIOC_BASE       'D'

#define AST_XDMA_IOCXFER		_IOWR(XDMAIOC_BASE, 0x0, struct ast_xdma_xfer*)
/*************************************************************************************/
static void
usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
		"Usage: %s [options]\n\n"
		"Options:\n"
		" -h | --help                   Print this message\n"
		" -n | --node                   xdma device node\n"
		" -t | --xfer                   xdma xfer\n"
		" -r | --recv                   xdma recv\n"
		" -b | --baddr                  xdma bmc addr\n"
		" -x | --haddr                  xdma host addr\n"
		" -l | --len                    xdma xfer len \n"
		" -c | --loop                   xdma loop count \n"
		"",
		argv[0]);
}

static const char short_options [] = "htrb:h:x:l:c:";

static const struct option
	long_options [] = {
	{ "help",       no_argument,            NULL,   'h' },
	{ "xfer",       no_argument,            NULL,   't' },
	{ "recv",       no_argument,            NULL,   'r' },
	{ "bmc",        required_argument,      NULL,   'b' },
	{ "host",       required_argument,      NULL,   'x' },
	{ "len",        required_argument,      NULL,   'l' },
	{ "loop",       required_argument,      NULL,   'c' },
	{ 0, 0, 0, 0 }
};

/*************************************************************************************/
/*				AST XDMA LIB					*/
int ast_xdma_xfer(int fd, struct ast_xdma_xfer *xfer)
{
	int retval;

	retval = ioctl(fd, AST_XDMA_IOCXFER, xfer);
	if (retval == -1) {
		perror("ioctl XDMA fail!\n");
		return -1;
	}

	return 0;
}

unsigned int StrToHex(char *p)
{
	int i, sum;
	int temp, length;
	char c;
	sum = 0;
	length = strlen(p);
	for (i = 0; i < (length) ; i++) {
		c = *p;
		if (c >= 'a' && c <= 'z') {
			temp = c - 87;
			sum += ((temp) << (4 * (length - i - 1)));
		} else if (c >= 'A' && c <= 'Z') {
			temp = c - 55;
			sum += ((temp) << (4 * (length - i - 1)));
		} else {
			temp = c - 48;
			sum = sum + ((temp) << (4 * (length - i - 1)));
		}

		p = p + 1;
	}
	return sum;
}

/*************************************************************************************/
int main(int argc, char *argv[])
{
	char dev_name[100] = "/dev/aspeed-xdma0";
	unsigned int i, j;
	char option;
	int cmp_err = 0;
	struct ast_xdma_xfer xdma_xfer;

	int loop = 0;
	int count = 0;
	unsigned char pattern = 0;
	unsigned char sed = 0;

	xdma_xfer.xfer_len = 0;
	xdma_xfer.bmc_addr = 0;
	xdma_xfer.host_addr_low = 0x00010000;
	xdma_xfer.host_addr_high = 0x00000000;
	if (!argv[1]) {
		usage(stdout, argc, argv);
		exit(EXIT_FAILURE);
	}

	while ((option = getopt_long(argc, argv, short_options, long_options, NULL)) != (char) -1) {
//		printf("option is c %c\n", option);
		switch (option) {
		case 'h':
			usage(stdout, argc, argv);
			exit(EXIT_SUCCESS);
			break;
		case 'n':
			strcpy(dev_name, optarg);
			if (!strcmp(dev_name, "")) {
				printf("No dev file name!\n");
				usage(stdout, argc, argv);
				exit(EXIT_FAILURE);
			}
			break;
		case 't':
			xdma_xfer.stream_dir = 1;
			break;
		case 'r':
			xdma_xfer.stream_dir = 0;
			break;
		case 'b':
			xdma_xfer.bmc_addr = StrToHex(optarg);
			break;
		case 'x':
			xdma_xfer.host_addr_low = StrToHex(optarg);
			break;
		case 'i':
			xdma_xfer.host_addr_high = StrToHex(optarg);
		case 'l':
			xdma_xfer.xfer_len = strtoul(optarg, 0, 0);
			if (xdma_xfer.xfer_len > 4096)
				xdma_xfer.xfer_len = 4096;
			break;
		case 'c':
			loop = 1;
			count = strtoul(optarg, 0, 0);
			break;
		default:
			usage(stdout, argc, argv);
			exit(EXIT_FAILURE);
			break;
		}
	}

	int xdma_fd = open(dev_name, O_RDWR);
	if (xdma_fd == -1) {
		perror("Can't open xdma device node, please install driver!! \n");
		//exit(EXIT_SUCCESS);v
		return -1;
	}

	if (loop)
		printf("Loop @ BMC addr : %x, Host addr : %x:%x \n", xdma_xfer.bmc_addr, xdma_xfer.host_addr_high, xdma_xfer.host_addr_low);
	else
		printf("%s @ BMC addr : %x, Host addr : %x:%x \n", xdma_xfer.stream_dir ? "Rx" : "Tx", xdma_xfer.bmc_addr, xdma_xfer.host_addr_high, xdma_xfer.host_addr_low);

	xdma_xfer.xfer_buff = malloc(4096);

	if (loop) {
		if (count == 0)
			goto out;

		for (i = 0; i < count ; i++) {
			memset(xdma_xfer.xfer_buff, 0, MAX_XFER_BUFF_SIZE);

			srand(time(NULL));
			pattern = rand() & 0xff;
			sed = 0x01;

			//8 byte align
			xdma_xfer.xfer_len = rand() & 0xfff;
			if (xdma_xfer.xfer_len == 0)
				xdma_xfer.xfer_len = 16;
			xdma_xfer.xfer_len += 16 - (xdma_xfer.xfer_len % 16);

			xdma_xfer.xfer_buff[0] = pattern;
			for (j = 1; j < xdma_xfer.xfer_len; j++) {
				xdma_xfer.xfer_buff[j] = xdma_xfer.xfer_buff[j - 1] + sed;
			}

			//print pattern
#if 0
			printf("----- Data ---- \n");
			for (j = 0; j < xdma_xfer.xfer_len; j++) {
				if (j % 16 == 0)
					printf("\n");
				printf("%02x ", xdma_xfer.xfer_buff[j]);
			}

			printf("\n");

			printf("====================================================================== \n");
			printf("BMC  Addr : 0x%x \nHost Addr : 0x%x (L) 0x%x (H) \n", xdma_xfer.bmc_addr, xdma_xfer.host_addr_low, xdma_xfer.host_addr_high);
			printf("Pattern :   0x%x, sed : 0x%x, xfer len =  %d \n", pattern, sed, xdma_xfer.xfer_len);
			printf("====================================================================== \n");
#endif
			printf("Count %d: xfer len %d ", i, xdma_xfer.xfer_len);
			//Tx
			xdma_xfer.stream_dir = 1;
			ast_xdma_xfer(xdma_fd, &xdma_xfer);
			memset(xdma_xfer.xfer_buff, 0, MAX_XFER_BUFF_SIZE);
			//Rx
			xdma_xfer.stream_dir = 0;
			if (ast_xdma_xfer(xdma_fd, &xdma_xfer)) {
				goto out;
			}
			//Cmp
			if (xdma_xfer.xfer_buff[0] != pattern) {
				printf("\n");
				cmp_err = 1;
				printf("[0] : %x != %x\n", xdma_xfer.xfer_buff[0], pattern);
			}

			for (j = 1; j < xdma_xfer.xfer_len; j++) {
				if (xdma_xfer.xfer_buff[j] != ((xdma_xfer.xfer_buff[j - 1] + sed) & 0xff)) {
					cmp_err = 1;
					printf("\n");
					printf("[%d] : %x != %x\n", j, xdma_xfer.xfer_buff[j], xdma_xfer.xfer_buff[j - 1] + sed);
				}
			}

			if (cmp_err  == 0) {
				printf("Pass\n");
			} else {
				printf("Fail\n");
				printf("====================================================================== \n");
				printf("xfer len %d from pattern %x, sed %x \n", xdma_xfer.xfer_len, pattern, sed);
				printf("0 : ");
				for (j = 0; j < xdma_xfer.xfer_len; j++) {
					if (j % 16 == 0) {
						printf("\n");
						printf("%d : ", j);
					}
					printf("%02x ", xdma_xfer.xfer_buff[j]);
				}
				printf("\n");
				printf("====================================================================== \n");
				goto out;
			}
		}
	} else {
		if (xdma_xfer.stream_dir) {
			memset(xdma_xfer.xfer_buff, 0, MAX_XFER_BUFF_SIZE);

			srand(time(NULL));
			pattern = rand() & 0xff;
			sed = 0x01;

			//8 byte align
			if (!xdma_xfer.xfer_len) {
				xdma_xfer.xfer_len = rand() & 0xfff;
				if (xdma_xfer.xfer_len == 0)
					xdma_xfer.xfer_len = 16;

				xdma_xfer.xfer_len += 16 - (xdma_xfer.xfer_len % 16);
			}

			xdma_xfer.xfer_buff[0] = pattern;
			for (i = 1; i < xdma_xfer.xfer_len; i++) {
				xdma_xfer.xfer_buff[i] = xdma_xfer.xfer_buff[i - 1] + sed;
			}

			//print pattern
#if 0
			printf("----- Data ---- \n");
			for (i = 0; i < xdma_xfer.xfer_len; i++) {
				if (i % 16 == 0)
					printf("\n");
				printf("%02x ", xdma_xfer.xfer_buff[i]);
			}

			printf("\n");
#endif
			printf("====================================================================== \n");
			printf("BMC  Addr : 0x%x \nHost Addr : 0x%x (L) 0x%x (H) \n", xdma_xfer.bmc_addr, xdma_xfer.host_addr_low, xdma_xfer.host_addr_high);
			printf("Pattern :   0x%x, sed : 0x%x, xfer len =  %d \n", pattern, sed, xdma_xfer.xfer_len);
			printf("====================================================================== \n");

			//Tx
			ast_xdma_xfer(xdma_fd, &xdma_xfer);
			printf("TX Done \n");

		} else {
			memset(xdma_xfer.xfer_buff, 0, MAX_XFER_BUFF_SIZE);

			srand(time(NULL));

			//8 byte align
			if (!xdma_xfer.xfer_len) {
				xdma_xfer.xfer_len = rand() & 0xfff;
				if (xdma_xfer.xfer_len == 0)
					xdma_xfer.xfer_len = 16;

				xdma_xfer.xfer_len += 16 - (xdma_xfer.xfer_len % 16);
			}
			printf("rx len = %d \n", xdma_xfer.xfer_len);
			if (ast_xdma_xfer(xdma_fd, &xdma_xfer)) {
				goto out;
			}
#if 1
			printf("----- Rx Data  -----\n");
			for (i = 0; i < xdma_xfer.xfer_len; i++) {
				if (i % 16 == 0)
					printf("\n");
				printf("%02x ", xdma_xfer.xfer_buff[i]);
			}

			printf("\n");
#endif
		}

	}
	printf("====================================================================== \n");
out:
	close(xdma_fd);

	return 0;
}
