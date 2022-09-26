//http://billauer.co.il/blog/2018/01/c-pmbus-xilinx-fpga-kc705/
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <util/pmbus.hpp>
#define PMBUSPATH "/dev/KETI_pmbus"

// Note that crc8 can be pre-calculated into an array of 256 bytes
static unsigned char crc8(unsigned char x) {
  int i;

  for (i=0; i<8; i++) {
    char toxor = (x & 0x80) ? 0x07 : 0;

    x = x << 1;
    x = x ^ toxor;
  }

  return x;
}

static unsigned char smbus_pec(unsigned char crc, const unsigned char *p,
			       int count) {
  int i;

  for (i = 0; i < count; i++)
    crc = crc8(crc ^ p[i]);
  return crc;
}

void pmbus_write(int addr, int command, int length, unsigned char *data) {
  int fd, rc;

  unsigned char sendbuf[131];
  int sent, tosend;

  if (length > 128)
    exit(1);

  fd = open("/dev/xillybus_pmbus", O_WRONLY);
  if (fd < 0) {
    perror("Failed to open /dev/xillybus_pmbus write-only");
    exit(1);
  }

  sendbuf[0] = addr & 0xfe;
  sendbuf[1] = command;
  memcpy(sendbuf + 2, data, length);
  sendbuf[length + 2] = smbus_pec(0, sendbuf, length+2);

  for (sent = 0, tosend = length + 3; sent < tosend; sent += rc) {
    rc = write(fd, (void *) (sendbuf + sent), tosend - sent);

    if ((rc < 0) && (errno == EINTR))
      continue;

    if (rc < 0)
      perror("pmbus_write() failed to write");

    if (rc == 0)
      fprintf(stderr, "pmbus_write reached write EOF (?!)\n");

    if (rc <= 0) {
      close(fd);
      exit(1);
    }
  }

  close(fd);
}

void pmbus_read(int addr, int command, int length, unsigned char *data) {
  int fdr, fdw, rc, i;

  unsigned char cmdbuf[2] = { addr & 0xfe, command };
  unsigned char dummybuf[130];
  unsigned char crc;

  const unsigned char *sendbuf[2] = { cmdbuf, dummybuf };
  const int tosend[2] = { 2, length + 2 };
  int sent, recvd;

  if (length > 128)
    exit(1);

  memset(dummybuf, 0, sizeof(dummybuf));
  dummybuf[0] = addr | 1;

  fdr = open("/dev/xillybus_pmbus", O_RDONLY);

  if (fdr < 0) {
    perror("Failed to open /dev/xillybus_pmbus read-only");
    exit(1);
  }

  for (i=0; i<2; i++) {
    fdw = open("/dev/xillybus_pmbus", O_WRONLY);
    if (fdw < 0) {
      perror("Failed to open /dev/xillybus_pmbus write-only");
      exit(1);
    }

    for (sent = 0; sent < tosend[i]; sent += rc) {
      rc = write(fdw, (void *) (sendbuf[i] + sent), tosend[i] - sent);

      if ((rc < 0) && (errno == EINTR))
	continue;

      if (rc < 0)
	perror("pmbus_read() failed to write");

      if (rc == 0)
	fprintf(stderr, "pmbus_read reached write EOF (?!)\n");

      if (rc <= 0) {
	close(fdr);
	close(fdw);
	exit(1);
      }
    }

    // Force a restart on the PMBUS by closing fdw. Had fdr been closed as
    // well, we would get a stop condition instead.

    close(fdw);
  }

  crc = smbus_pec(0, sendbuf[0], tosend[0]);
  crc = smbus_pec(crc, sendbuf[1], 1);

  // Collect the data that was read during the dummy write data cycles
  for (recvd = 0; recvd < length + 1; recvd += rc) {
    rc = read(fdr, (void *) (dummybuf + recvd), length + 1 - recvd);

    if ((rc < 0) && (errno == EINTR))
      continue;

    if (rc < 0)
      perror("pmbus_read() failed to read");

    if (rc == 0)
      fprintf(stderr, "pmbus_read reached EOF (?!)\n");

    if (rc <= 0) {
      close(fdr);
      exit(1);
    }
  }

  close(fdr);

  memcpy(data, dummybuf, length);

  crc = smbus_pec(crc, dummybuf, length);

  if (crc != dummybuf[length]) {
    fprintf(stderr, "Failed CRC check: PEC = 0x%02x, received 0x%02x\n",
	    crc, dummybuf[length]);
    exit(1);
  }
}

void set_page(int addr, unsigned char page) {
  unsigned char read_page;

  pmbus_write(addr, 0x00, 1, &page);

  pmbus_read(addr, 0x00, 1, &read_page);

  if (page != read_page) {
    fprintf(stderr, "Failed to set page to %d (read back %d instead)\n",
	    page, read_page);
    exit(1);
  }
}

void get_phase_info(int addr, unsigned char *data) {
  unsigned char readbytes[5];

  pmbus_read(addr, 0xd2, 5, readbytes);

  if (readbytes[0] != 4) {
    fprintf(stderr, "PHASE_INFO responded with block length %d != 4\n",
	    readbytes[0]);
    exit(1);
  }

  memcpy(data, readbytes + 1, 4);
}

void print_dpwms(unsigned char phase_info) {
  const char *dpwm[8] = { "1A", "1B", "2A", "2B", "3A", "3B", "4A", "4B" };
  int i;

  for (i=0; i<8; i++) {
    if (phase_info & 1)
      printf(" %s", dpwm[i]);
    phase_info >>= 1;
  }
}

void clear_faults(int addr) {
  pmbus_write(addr, 0x03, 0, NULL);
}

double linear2voltage(unsigned short word) {
  return word / 4096.0;
}

double linear2nonvoltage(unsigned short word) {
  double x = word & 0x7ff;
  int exponent = (word >> 11) & 0xf;

  if (word & 0x8000)
    x /= 1 << (16 - exponent);
  else
    x *= 1 << exponent;

  return x;
}