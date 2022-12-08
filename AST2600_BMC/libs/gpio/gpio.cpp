
#include <cstdio>
#include <fcntl.h> /* For O_RDWR */
#include <iostream>
#include <ipmi/gpio.hpp>
#include <ipmi/ipmi.hpp>
#include <ipmi/lightning_sensor.hpp>
#include <sys/mman.h>/*For mamory map MAP_SHARED */
#include <unistd.h>  /* For open(), creat() */

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

using namespace std;
struct ast_gpio_bank {
  // TODO remove base
  unsigned int index;
  unsigned int data_offset;
  unsigned int dir_offset;
  unsigned int int_en_offset;
  unsigned int int_type_offset;
  unsigned int int_sts_offset;
  unsigned int rst_tol_offset;
  unsigned int debounce_offset;
  unsigned int cmd_source_offset;
};
#define AST_GPIO_BANK(index_no, data, dir, int_en, int_type, int_sts, rst_tol, \
                      debounce, cmd_s)                                         \
  {                                                                            \
    .index = index_no, .data_offset = data, .dir_offset = dir,                 \
    .int_en_offset = int_en, .int_type_offset = int_type,                      \
    .int_sts_offset = int_sts, .rst_tol_offset = rst_tol,                      \
    .debounce_offset = debounce, .cmd_source_offset = cmd_s,                   \
  }

static struct ast_gpio_bank ast_gpio_gp[] = {
    AST_GPIO_BANK(0, 0x000, 0x004, 0x008, 0x00c, 0x018, 0x01c, 0x040, 0x060),
    AST_GPIO_BANK(1, 0x000, 0x004, 0x008, 0x00c, 0x018, 0x01c, 0x040, 0x060),
    AST_GPIO_BANK(2, 0x000, 0x004, 0x008, 0x00c, 0x018, 0x01c, 0x040, 0x060),
    AST_GPIO_BANK(3, 0x000, 0x004, 0x008, 0x00c, 0x018, 0x01c, 0x040, 0x060),
    AST_GPIO_BANK(0, 0x020, 0x024, 0x028, 0x02c, 0x038, 0x03c, 0x048, 0x068),
    AST_GPIO_BANK(1, 0x020, 0x024, 0x028, 0x02c, 0x038, 0x03c, 0x048, 0x068),
    AST_GPIO_BANK(2, 0x020, 0x024, 0x028, 0x02c, 0x038, 0x03c, 0x048, 0x068),
    AST_GPIO_BANK(3, 0x020, 0x024, 0x028, 0x02c, 0x038, 0x03c, 0x048, 0x068),
    AST_GPIO_BANK(0, 0x070, 0x074, 0x098, 0x09c, 0x0a8, 0x0ac, 0x0b0, 0x090),
    AST_GPIO_BANK(1, 0x070, 0x074, 0x098, 0x09c, 0x0a8, 0x0ac, 0x0b0, 0x090),
    AST_GPIO_BANK(2, 0x070, 0x074, 0x098, 0x09c, 0x0a8, 0x0ac, 0x0b0, 0x090),
    AST_GPIO_BANK(3, 0x070, 0x074, 0x098, 0x09c, 0x0a8, 0x0ac, 0x0b0, 0x090),
    AST_GPIO_BANK(0, 0x078, 0x07c, 0x0e8, 0x0ec, 0x0f8, 0x0fc, 0x100, 0x0e0),
    AST_GPIO_BANK(1, 0x078, 0x07c, 0x0e8, 0x0ec, 0x0f8, 0x0fc, 0x100, 0x0e0),
    AST_GPIO_BANK(2, 0x078, 0x07c, 0x0e8, 0x0ec, 0x0f8, 0x0fc, 0x100, 0x0e0),
    AST_GPIO_BANK(3, 0x078, 0x07c, 0x0e8, 0x0ec, 0x0f8, 0x0fc, 0x100, 0x0e0),
    AST_GPIO_BANK(0, 0x080, 0x084, 0x118, 0x11c, 0x128, 0x12c, 0x130, 0x110),
    AST_GPIO_BANK(1, 0x080, 0x084, 0x118, 0x11c, 0x128, 0x12c, 0x130, 0x110),
    AST_GPIO_BANK(2, 0x080, 0x084, 0x118, 0x11c, 0x128, 0x12c, 0x130, 0x110),
#if 1 // defined(CONFIG_ARCH_AST2400)
    AST_GPIO_BANK(3, 0x080, 0x084, 0x118, 0x11c, 0x128, 0x12c, 0x130, 0x110),
    AST_GPIO_BANK(0, 0x088, 0x08c, 0x148, 0x14c, 0x158, 0x15c, 0x160, 0x140),
    AST_GPIO_BANK(1, 0x088, 0x08c, 0x148, 0x14c, 0x158, 0x15c, 0x160, 0x140),
    AST_GPIO_BANK(2, 0x088, 0x08c, 0x148, 0x14c, 0x158, 0x15c, 0x160, 0x140),
    AST_GPIO_BANK(3, 0x088, 0x08c, 0x148, 0x14c, 0x158, 0x15c, 0x160, 0x140),
    AST_GPIO_BANK(0, 0x1e0, 0x1e4, 0x178, 0x17c, 0x188, 0x18c, 0x190, 0x170),
    AST_GPIO_BANK(1, 0x1e0, 0x1e4, 0x178, 0x17c, 0x188, 0x18c, 0x190, 0x170),
    AST_GPIO_BANK(2, 0x1e0, 0x1e4, 0x178, 0x17c, 0x188, 0x18c, 0x190, 0x170),
    AST_GPIO_BANK(3, 0x1e0, 0x1e4, 0x178, 0x17c, 0x188, 0x18c, 0x190, 0x170),
#endif
};

static unsigned int gpio_reg_base;
static unsigned int scu_reg_base;
static unsigned int pwm_reg_base;

// static inline unsigned int
unsigned int ast_pwm_read(unsigned int offset) {

  unsigned int data;

  data = *((unsigned int *)(pwm_reg_base + offset));

  cout << ("pwm_read: base = 0x%08x, offset = 0x%08x data = 0x%08x\n",
           pwm_reg_base, offset, data);
  return data;
}

static inline void ast_pwm_write(unsigned int val, unsigned int offset) {
  unsigned int data;

  //     data = *((unsigned int *) (gpio_reg_base + offset));
  //   GPIODBUG("gpio_write : base = 0x%08x, offset = 0x%08x val = 0x%08x\n",
  //   gpio_reg_base, offset, val);
  // GPIODBUG("gpio_write: base = 0x%08x, offset = 0x%08x, val = 0x%08x\n",
  // gpio_reg_base, offset, val);
  *((unsigned int *)(pwm_reg_base + offset)) = val;
  data = *((unsigned int *)(pwm_reg_base + offset));
  cout << ("gpio_write (after): base = 0x%08x, offset = 0x%08x data = 0x%08x\n",
           gpio_reg_base, offset, data)
       << endl;
}

void set_pwm_duty(int fannumb, int value) {
  unsigned int data;
  int val;

  val = 60 - value;
  if (val >= 0 && val <= 60) {
    data = 0x3c003c00;
    data |= val & 0x000000ff;         // data |= val&0x000000ff;
    data |= (val & 0x000000ff) << 16; // data |= (val&0x000000ff)<<16;
    switch (fannumb) {

    case 1:
      ast_pwm_write(data, 0x08);
      break;
    case 2:
      ast_pwm_write(data, 0x0C);
      break;
    case 3:
      ast_pwm_write(data, 0x48);
      break;
    }
    //				  ast_pwm_write(data, 0x08);
    //                ast_pwm_write(data, 0x0C);
    //                ast_pwm_write(data, 0x48);
    //                ast_pwm_write(data, 0x4C);
    //		printf(" read PWM duty val=%d, regs= 0x%08x\n", value,
    // ast_pwm_read(8));
  }
}

/*static inline*/ unsigned int ast_gpio_read(unsigned int offset) {
  unsigned int data;

  data = *((unsigned int *)(gpio_reg_base + offset));

  cout << ("gpio_read: base = 0x%08x, offset = 0x%08x data = 0x%08x\n",
           gpio_reg_base, offset, data);
  return data;
}

static inline unsigned int ast_reg_read(unsigned int reg_base,
                                        unsigned offset) {
  unsigned int data;

  data = *((unsigned int *)(reg_base + offset));

  cout << ("ast_reg_read: base = 0x%08x, offset = 0x%08x data = 0x%08x\n",
           reg_base, offset, data);
  return data;
}

/*static inline*/ void ast_gpio_write(unsigned int val, unsigned int offset) {
  unsigned int data;

  //     data = *((unsigned int *) (gpio_reg_base + offset));
  cout << ("gpio_write : base = 0x%08x, offset = 0x%08x val = 0x%08x\n",
           gpio_reg_base, offset, val)
       << endl;
  // GPIODBUG("gpio_write: base = 0x%08x, offset = 0x%08x, val = 0x%08x\n",
  // gpio_reg_base, offset, val);
  *((unsigned int *)(gpio_reg_base + offset)) = val;
  data = *((unsigned int *)(gpio_reg_base + offset));
  cout << ("gpio_write (after): base = 0x%08x, offset = 0x%08x data = 0x%08x\n",
           gpio_reg_base, offset, data)
       << endl;
}

/***************************************************************************************/
/**
 * @brief gpio FIN에 데이터 저장
 */
int ast_set_gpio_value(unsigned int gpio_pin, int value) {
  unsigned int data;
  unsigned int wr;
  unsigned int gp, pin;
  printf("ast_set_gpio_value start");
  gp = gpio_pin / 8;
  pin = gpio_pin % 32;
  (data = ast_gpio_read(ast_gpio_gp[gp].data_offset));

  // if (gpio_pin == 102) printf("before > reg :0x%08x, pin %d, data = %x \n ",
  // ast_gpio_gp[gp].data_offset, pin, data);

  if (value)
    data |= (1 << pin);
  else
    data &= ~(1 << pin);

  //      GPIODBUG("gp : %d, pin %d, data = %x \n ", gp, pin, data);
  ast_gpio_write(data, ast_gpio_gp[gp].data_offset);
  (data = ast_gpio_read(ast_gpio_gp[gp].data_offset));
  //    if (gpio_pin == 102)  printf("after > reg :0x%08x, pin %d, data = %x \n
  //    ", ast_gpio_gp[gp].data_offset, pin, data);

  //        printf("after > gp : %d, pin %d, data = %x \n ", gp, pin, data);

  return 0;
}

int ast_get_gpio_value(unsigned int gpio_pin) {
  unsigned int data;
  unsigned int gp, pin;
  gp = gpio_pin / 8;
  pin = gpio_pin % 32;
  (data = ast_gpio_read(ast_gpio_gp[gp].data_offset));

  //    GPIODBUG("gp : %d, pin %d, data = %x, value = %d \n ", gp, pin, data,
  //    (data >> pin) & 1);

  return ((data >> pin) & 1);
}

int ast_get_reg_offset(unsigned int offset) {
  unsigned int data;
  (data = ast_gpio_read(offset));

  cout << ("offset %x, data = %x\n ", offset, data) << endl;

  return (data);
}

int ast_gpio_direction_input(unsigned int gpio_pin) {
  struct ast_gpio_bank *ast_gpio;
  unsigned int v;
  int ret = -1;

  unsigned int gp, pin;
  gp = gpio_pin / 8;
  pin = gpio_pin % 32;

  //  GPIODBUG("dir_in %x \n",gpio_pin);

  ast_gpio = &ast_gpio_gp[gp];

  (v = ast_gpio_read(ast_gpio->dir_offset));

  v &= ~(GPIO_OUTPUT_MODE
         << pin); // (ast_gpio->data_offset + (ast_gpio->index * 8)));
  ast_gpio_write(v, ast_gpio->dir_offset);

  ret = 0;

  return ret;
}

int ast_gpio_direction_output(unsigned int gpio_pin, int val) {
  struct ast_gpio_bank *ast_gpio;
  unsigned int v;
  int ret = -1;

  unsigned int gp, pin;
  gp = gpio_pin / 8;
  pin = gpio_pin % 32;

  // GPIODBUG("dir_out %x, val %d \n",gpio_pin, val);
  //	printf("dir_out %x, val %d \n",gpio_pin, val);
  ast_gpio = &ast_gpio_gp[gp];

  /* Drive as an output */
  (v = ast_gpio_read(ast_gpio->dir_offset));
  //	printf("dir before > reg : 0x%x, pin =%d, data = %x \n ",
  // ast_gpio->dir_offset, pin, v);

  v |= (GPIO_OUTPUT_MODE
        << pin); // (ast_gpio->data_offset + (ast_gpio->index * 8)));

  ast_gpio_write(v, ast_gpio->dir_offset);
  // printf("dir before > gp : %d, pin %d, data = %x \n ", gp, pin, v);

  (ast_set_gpio_value(gpio_pin, val));
  (v = ast_gpio_read(ast_gpio->dir_offset));
  //	printf("dir after > reg : 0x%x, pin = %d, data = %x \n ",
  // ast_gpio->dir_offset, pin, v);

  // printf("dir after > gp : %d, pin %d, data = %x \n ", gp, pin, v);

  ret = 0;
  return ret;
}

int ast_gpio_open(void) {
  int fd;
  void *map_base;
  void *scu_base;
  void *pwm_base;
  int i;

  (fd = open("/dev/mem", O_RDWR | O_SYNC));
  if (fd == -1) {
    cout << ("Can't Open dev/mem!\n");
    return (-1);
  }

  //	printf("<7>\n");
  (map_base = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                   AST_GPIO_BASE));

  if (map_base == 0) {
    cout << ("NULL pointer!\n");
    return (-1);
  }

  //      printf("<8>\n");
  gpio_reg_base = (unsigned int)map_base;
  //	for (i=0; i<0x200; i=i+4){
  //		printf("gpio regs = 0x%x, data = 0x%08x\n", i,
  // ast_gpio_read(i));
  //	}
  (scu_base =
       mmap(NULL, 0x256, PROT_READ | PROT_WRITE, MAP_SHARED, fd, AST_SCU_BASE));

  if (scu_base == 0) {
    cout << ("NULL pointer!\n");
    return (-1);
  }

  scu_reg_base = (unsigned int)scu_base;
  //        for (i=0x70; i<=0xb0; i=i+4){
  //              printf("scu regs = 0x%x, data = 0x%08x\n", i,
  //              ast_reg_read(scu_reg_base, i));
  //    }
  (pwm_base =
       mmap(NULL, 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, AST_PWM_BASE));

  if (pwm_base == 0) {
    cout << ("NULL pointer!\n");
    return (-1);
  }

  pwm_reg_base = (unsigned int)pwm_base;
  //	printf("<9>\n");
  ast_pwm_write(0x3c433c43, 4);
  //	printf("<10>\n");

  return 0;
}

void print_regs(void) {
  int i;

  for (i = 0; i < 0x200; i = i + 4) {
    printf("gpio regs = 0x%x, data = 0x%08x\n", i, ast_gpio_read(i));
  }

  for (i = 0x80; i <= 0x94; i = i + 4) {
    printf("scu regs = 0x%x, data = 0x%08x\n", i,
           ast_reg_read(scu_reg_base, i));
  }
}
void ast_gpio_close(void) {
  munmap((void *)gpio_reg_base, 0x1000);
  munmap((void *)scu_reg_base, 0x1000);
}
/** @brief i2c 데이터 읽고 쓰기
/*  power 값을 읽을경우의 값임
**/
static unsigned int i2c_readw_r(int i2c_dev, int addr) {
  unsigned char val[2] = {0};
  unsigned char rval[2] = {0};
  unsigned int ret;

  val[0] = addr;

  // Set I2C request command(val) val[0]= index val[1]=command
  write(i2c_dev, &val[0], 1);
  read(i2c_dev, &rval[0], 2);

  ret = rval[0] + rval[1] * 256;
  return ret;
}

bool read_psu_value(string i2cname, uint8_t addr, uint8_t type, int *value) {
  int file;
  int32_t res;
  if ((file = open(i2cname.c_str(), O_RDWR)) < 0) {
    cout << "Failed to detect " << i2cname;
    return false;
  }
  // addr = SLAVE ADDR I2C_SLAVE_FORCE == 사용되더라도 강재로 확인
  else if (ioctl(file, I2C_SLAVE_FORCE, addr) < 0) {
    stringstream stream;
    stream << hex << addr;
    cout << "Failed to detect bus 0x" << string(stream.str());
    return false;
  }
  // power 파트인경우
  if (type == NVA_PSU_FAN1 || type == NVA_PSU_FAN2 || NVA_PSU_WATT) {
    int8_t n;
    int16_t y;
    res = i2c_readw_r(file, type);
    n = (res >> 11) & 0x1f;
    y = res & 0x7FF;
    if (n > 0x0f)
      n |= 0xE0;
    if (y > 0x3ff)
      y |= 0xF800;
    if ((type == NVA_PSU_FAN1) || (type == NVA_PSU_FAN2)) {
      *value = (*value + 50) / 100;
    } else {
      *value = (*value + 5) / 10;
    }
    return false;

  } else {
    cout << "미구현 " << endl;
  }

  close(file);
}

/**
 * @brief Chassis_Power On
 *
 * @return value return 1 = sucess
 * @author kcp
 */
int Chassis_Power_Off() {
  ast_set_gpio_value(PIN_GPIOD3, 0);
  return 1;
}
/**
 * @brief Chassis_Power Off
 *
 * @return value return 1 = sucess
 * @author kcp
 */
int Chassis_Power_On() {
  ast_set_gpio_value(PIN_GPIOD3, 1);
  return 1;
}
