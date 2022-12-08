#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <util/pmbus.hpp>
#include <string.h>

static double new_voltage = 1.8;

enum { PM_VOLTAGE, PM_NONVOLTAGE, PM_STATUS } pm_types ;

struct pm_list {
  unsigned char command;
  int size;
  char *name;
  double diversion;
  char *units;
  int type;
};

static struct pm_list pm_paged[] = {
  { 0x25, 2, "VOUT_MARGIN_HIGH", 5.0, "V", PM_VOLTAGE },
  { 0x26, 2, "VOUT_MARGIN_LOW", -5.0, "V", PM_VOLTAGE },
  { 0x5e, 2, "POWER_GOOD_ON", -5.0, "V", PM_VOLTAGE },
  { 0x5f, 2, "POWER_GOOD_OFF", -8.0, "V", PM_VOLTAGE },
  { 0x43, 2, "VOUT_UV_WARN_LIMIT", -10.0, "V", PM_VOLTAGE },
  { 0x44, 2, "VOUT_UV_FAULT_LIMIT", -15.0, "V", PM_VOLTAGE },
  { 0x21, 2, "VOUT_COMMAND", 0.0, "V", PM_VOLTAGE },
  { 0x40, 2, "VOUT_OV_FAULT_LIMIT", 15.0, "V", PM_VOLTAGE },
  { 0x42, 2, "VOUT_OV_WARN_LIMIT", 10.0, "V", PM_VOLTAGE },
 { }
};

static unsigned short new_val(struct pm_list *p) {
  return (unsigned short) ((4096.0 * new_voltage * (1.0 + p->diversion / 100.0)) + 0.5);
}

static void dumpval(int pmbus_addr, struct pm_list *p) {
  unsigned short val;

  pmbus_read(pmbus_addr, p->command, p->size, (void *) &val);

  switch (p->type) {
  case PM_VOLTAGE:
    printf("  %s = %.4f %s -> %.4f %s (0x%04x -> 0x%04x)\n",
	   p->name, linear2voltage(val),
	   p->units, linear2voltage(new_val(p)), p->units,
	   val, new_val(p));
    break;

  case PM_NONVOLTAGE:
    printf("  %s = %.4f %s\n", p->name, linear2nonvoltage(val),
	   p->units);
    break;

  default:
    if (p->size == 1)
      printf("  %s = 0x%02x\n", p->name, val & 0xff);
    else
      printf("  %s = 0x%04x\n", p->name, val);
    break;
  }
}

int main(int argc, char *argv[]) {
  int dry_run = 0;
  char *wrote = "WRITING";

  struct pm_list *p;

  unsigned char phase_info[4];

  int pmbus_addr = 52 * 2;
  int page = 3;
  char yes[10];

  get_phase_info(pmbus_addr, phase_info);

  if (phase_info[page] == 0) {
    fprintf(stderr, "Requested page %d is not active on device\n", page);
    return 1;
  }
  printf("\nPage %d: Controlling DPWM:", page);
  print_dpwms(phase_info[page]);
  printf("\n");

  set_page(pmbus_addr, page);

  for (p = pm_paged; p->name; p++)
    dumpval(pmbus_addr, p);

  printf("\nUpdate these new values? (Type uppercase YES): ");
  fgets(yes, sizeof(yes), stdin);

  if (strcmp(yes, "YES\n")) {
    printf("\nDidn't get YES. Therefore dry-running.\n");
    dry_run = 1;
    wrote = "Would write";
  }

  for (p = pm_paged; p->name; p++) {
    unsigned short v = new_val(p);
    printf("%s value 0x%04x (%.4f V) to %s (command 0x%02x)\n",
	   wrote, v, linear2voltage(v), p->name, p->command);

    if (!dry_run) {
      unsigned short r;
      pmbus_write(pmbus_addr, p->command, 2, (void *) &v);
      pmbus_read(pmbus_addr, p->command, 2, (void *) &r);

      if (v != r) {
	fprintf(stderr, "Readback failed! Wrote 0x%04x, read back 0x%04x\n",
		v, r);
	exit(1);
      }
    }
  }

  if (dry_run)
    return 0;

  printf("\nCheck the voltage now.\n\nStore current settings to non-volatile memory? (Type uppercase STORE): ");
  fgets(yes, sizeof(yes), stdin);

  if (strcmp(yes, "STORE\n")) {
    printf("OK, did nothing.\n");
  } else {
    printf("Setting page back to 0\n");
    set_page(pmbus_addr, 0);

    printf("SENDING a STORE_DEFAULT_ALL command\n");
    pmbus_write(pmbus_addr, 0x11, 0, NULL);
    printf("Done.\n");
  }

  return 0;
}