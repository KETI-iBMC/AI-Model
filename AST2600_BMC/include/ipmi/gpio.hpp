/*************************************************************/
#ifndef GPIO_UNIQUE_NAME_HERE
#define GPIO_UNIQUE_NAME_HERE

#include <ipmi/ipmi.hpp>
#define AST_GPIO_BASE 0x1e780000
#define AST_MAC0_BASE 0x1e660000
#define AST_SCU_BASE 0x1e6e2000
#define AST_PWM_BASE 0x1e786000
//#define AST_GPIO_DEBUG

#ifdef AST_GPIO_DEBUG
//#define GPIODBUG(fmt, args...) printk("%s() " fmt, __FUNCTION__, ## args)
#define GPIODBUG(fmt, args...) printf(fmt, ##args)

#else
#define GPIODBUG(fmt, args...)
#endif
#define GPIO_PORTA 0x0
#define GPIO_PORTB 0x1
#define GPIO_PORTC 0x2
#define GPIO_PORTD 0x3
#define GPIO_PORTE 0x4
#define GPIO_PORTF 0x5
#define GPIO_PORTG 0x6
#define GPIO_PORTH 0x7
#define GPIO_PORTI 0x8
#define GPIO_PORTJ 0x9
#define GPIO_PORTK 0xa
#define GPIO_PORTL 0xb
#define GPIO_PORTM 0xc
#define GPIO_PORTN 0xd
#define GPIO_PORTO 0xe
#define GPIO_PORTP 0xf
#define GPIO_PORTQ 0x10
#define GPIO_PORTR 0x11
#define GPIO_PORTS 0x12
// AST2300 didn't have PORT TT
#define GPIO_PORTT 0x13
#if 1 // defined(AST_SOC_G4) || defined(CONFIG_AST2400_BMC)
#define GPIO_PORTU 0x14
#define GPIO_PORTV 0x15
#define GPIO_PORTW 0x16
#define GPIO_PORTX 0x17
#define GPIO_PORTY 0x18
#define GPIO_PORTZ 0x19
#define GPIO_PORTAA 0x1a
#define GPIO_PORTAB 0x1b
#endif

#define GPIO_INPUT_MODE 0
#define GPIO_OUTPUT_MODE 1

#define GPIO_RISING_EDGE 1
#define GPIO_FALLING_EDGE 0

#define GPIO_LEVEL_HIGH 1
#define GPIO_LEVEL_LOW 1

#define GPIO_EDGE_MODE 0
#define GPIO_LEVEL_MODE 1

#define GPIO_EDGE_LEVEL_MODE 0
#define GPIO_DUAL_EDGE_MODE 1

#define GPIO_NO_DEBOUNCE 0
#define GPIO_DEBOUNCE_TIMER0 2 // GPIO 50 as debounce timer
#define GPIO_DEBOUNCE_TIMER1 1 // GPIO 54 as debounce timer
#define GPIO_DEBOUNCE_TIMER2 3 // GPIO 58 as debounce timer

#define GPIO_CMD_ARM 0
#define GPIO_CMD_LPC 1
#define GPIO_CMD_COPROCESSOR 2

#define PIN_GPIOA0 (0)
#define PIN_GPIOA1 (1)
#define PIN_GPIOA2 (2)
#define PIN_GPIOA3 (3)
#define PIN_GPIOA4 (4)
#define PIN_GPIOA5 (5)
#define PIN_GPIOA6 (6)
#define PIN_GPIOA7 (7)
#define PIN_GPIOB0 (8)
#define PIN_GPIOB1 (9)
#define PIN_GPIOB2 (10)
#define PIN_GPIOB3 (11)
#define PIN_GPIOB4 (12)
#define PIN_GPIOB5 (13)
#define PIN_GPIOB6 (14)
#define PIN_GPIOB7 (15)
#define PIN_GPIOC0 (16)
#define PIN_GPIOC1 (17)
#define PIN_GPIOC2 (18)
#define PIN_GPIOC3 (19)
#define PIN_GPIOC4 (20)
#define PIN_GPIOC5 (21)
#define PIN_GPIOC6 (22)
#define PIN_GPIOC7 (23)
#define PIN_GPIOD0 (24)
#define PIN_GPIOD1 (25)
#define PIN_GPIOD2 (26)
#define PIN_GPIOD3 (27)
#define PIN_GPIOD4 (28)
#define PIN_GPIOD5 (29)
#define PIN_GPIOD6 (30)
#define PIN_GPIOD7 (31)
#define PIN_GPIOE0 (32)
#define PIN_GPIOE1 (33)
#define PIN_GPIOE2 (34)
#define PIN_GPIOE3 (35)
#define PIN_GPIOE4 (36)
#define PIN_GPIOE5 (37)
#define PIN_GPIOE6 (38)
#define PIN_GPIOE7 (39)
#define PIN_GPIOF0 (40)
#define PIN_GPIOF1 (41)
#define PIN_GPIOF2 (42)
#define PIN_GPIOF3 (43)
#define PIN_GPIOF4 (44)
#define PIN_GPIOF5 (45)
#define PIN_GPIOF6 (46)
#define PIN_GPIOF7 (47)
#define PIN_GPIOG0 (48)
#define PIN_GPIOG1 (49)
#define PIN_GPIOG2 (50)
#define PIN_GPIOG3 (51)
#define PIN_GPIOG4 (52)
#define PIN_GPIOG5 (53)
#define PIN_GPIOG6 (54)
#define PIN_GPIOG7 (55)
#define PIN_GPIOH0 (56)
#define PIN_GPIOH1 (57)
#define PIN_GPIOH2 (58)
#define PIN_GPIOH3 (59)
#define PIN_GPIOH4 (60)
#define PIN_GPIOH5 (61)
#define PIN_GPIOH6 (62)
#define PIN_GPIOH7 (63)
#define PIN_GPIOI0 (64)
#define PIN_GPIOI1 (65)
#define PIN_GPIOI2 (66)
#define PIN_GPIOI3 (67)
#define PIN_GPIOI4 (68)
#define PIN_GPIOI5 (69)
#define PIN_GPIOI6 (70)
#define PIN_GPIOI7 (71)
#define PIN_GPIOJ0 (72)
#define PIN_GPIOJ1 (73)
#define PIN_GPIOJ2 (74)
#define PIN_GPIOJ3 (75)
#define PIN_GPIOJ4 (76)
#define PIN_GPIOJ5 (77)
#define PIN_GPIOJ6 (78)
#define PIN_GPIOJ7 (79)
#define PIN_GPIOK0 (80)
#define PIN_GPIOK1 (81)
#define PIN_GPIOK2 (82)
#define PIN_GPIOK3 (83)
#define PIN_GPIOK4 (84)
#define PIN_GPIOK5 (85)
#define PIN_GPIOK6 (86)
#define PIN_GPIOK7 (87)
#define PIN_GPIOL0 (88)
#define PIN_GPIOL1 (89)
#define PIN_GPIOL2 (90)
#define PIN_GPIOL3 (91)
#define PIN_GPIOL4 (92)
#define PIN_GPIOL5 (93)
#define PIN_GPIOL6 (94)
#define PIN_GPIOL7 (95)
#define PIN_GPIOM0 (96)
#define PIN_GPIOM1 (97)
#define PIN_GPIOM2 (98)
#define PIN_GPIOM3 (99)
#define PIN_GPIOM4 (100)
#define PIN_GPIOM5 (101)
#define PIN_GPIOM6 (102)
// static int PIN_GPIOM6 = 102;
#define PIN_GPIOM7 (103)
#define PIN_GPION0 (104)
#define PIN_GPION1 (105)
#define PIN_GPION2 (106)
#define PIN_GPION3 (107)
#define PIN_GPION4 (108)
#define PIN_GPION5 (109)
#define PIN_GPION6 (110)
#define PIN_GPION7 (111)
#define PIN_GPIOO0 (112)
#define PIN_GPIOO1 (113)
#define PIN_GPIOO2 (114)
#define PIN_GPIOO3 (115)
#define PIN_GPIOO4 (116)
#define PIN_GPIOO5 (117)
#define PIN_GPIOO6 (118)
#define PIN_GPIOO7 (119)
#define PIN_GPIOP0 (120)
#define PIN_GPIOP1 (121)
#define PIN_GPIOP2 (122)
#define PIN_GPIOP3 (123)
#define PIN_GPIOP4 (124)
#define PIN_GPIOP5 (125)
#define PIN_GPIOP6 (126)
#define PIN_GPIOP7 (127)
#define PIN_GPIOQ0 (128)
#define PIN_GPIOQ1 (129)
#define PIN_GPIOQ2 (130)
#define PIN_GPIOQ3 (131)
#define PIN_GPIOQ4 (132)
#define PIN_GPIOQ5 (133)
#define PIN_GPIOQ6 (134)
#define PIN_GPIOQ7 (135)
#define PIN_GPIOR0 (136)
#define PIN_GPIOR1 (137)
#define PIN_GPIOR2 (138)
#define PIN_GPIOR3 (139)
#define PIN_GPIOR4 (140)
#define PIN_GPIOR5 (141)
#define PIN_GPIOR6 (142)
#define PIN_GPIOR7 (143)
#define PIN_GPIOS0 (144)
#define PIN_GPIOS1 (145)
#define PIN_GPIOS2 (146)
#define PIN_GPIOS3 (147)
#define PIN_GPIOS4 (148)
#define PIN_GPIOS5 (149)
#define PIN_GPIOS6 (150)
#define PIN_GPIOS7 (151)
#if 1 // defined(AST_SOC_G4) || defined(CONFIG_AST2400_BMC)
#define PIN_GPIOT0 (152)
#define PIN_GPIOT1 (153)
#define PIN_GPIOT2 (154)
#define PIN_GPIOT3 (155)
#define PIN_GPIOT4 (156)
#define PIN_GPIOT5 (157)
#define PIN_GPIOT6 (158)
#define PIN_GPIOT7 (159)
#define PIN_GPIOU0 (160)
#define PIN_GPIOU1 (161)
#define PIN_GPIOU2 (162)
#define PIN_GPIOU3 (163)
#define PIN_GPIOU4 (164)
#define PIN_GPIOU5 (165)
#define PIN_GPIOU6 (166)
#define PIN_GPIOU7 (167)
#define PIN_GPIOV0 (168)
#define PIN_GPIOV1 (169)
#define PIN_GPIOV2 (170)
#define PIN_GPIOV3 (171)
#define PIN_GPIOV4 (172)
#define PIN_GPIOV5 (173)
#define PIN_GPIOV6 (174)
#define PIN_GPIOV7 (175)
#define PIN_GPIOW0 (176)
#define PIN_GPIOW1 (177)
#define PIN_GPIOW2 (178)
#define PIN_GPIOW3 (179)
#define PIN_GPIOW4 (180)
#define PIN_GPIOW5 (181)
#define PIN_GPIOW6 (182)
#define PIN_GPIOW7 (183)
#define PIN_GPIOX0 (184)
#define PIN_GPIOX1 (185)
#define PIN_GPIOX2 (186)
#define PIN_GPIOX3 (187)
#define PIN_GPIOX4 (188)
#define PIN_GPIOX5 (189)
#define PIN_GPIOX6 (190)
#define PIN_GPIOX7 (191)
#define PIN_GPIOY0 (192)
#define PIN_GPIOY1 (193)
#define PIN_GPIOY2 (194)
#define PIN_GPIOY3 (195)
#define PIN_GPIOY4 (196)
#define PIN_GPIOY5 (197)
#define PIN_GPIOY6 (198)
#define PIN_GPIOY7 (199)
#define PIN_GPIOZ0 (200)
#define PIN_GPIOZ1 (201)
#define PIN_GPIOZ2 (202)
#define PIN_GPIOZ3 (203)
#define PIN_GPIOZ4 (204)
#define PIN_GPIOZ5 (205)
#define PIN_GPIOZ6 (206)
#define PIN_GPIOZ7 (207)
#define PIN_GPIOAA0 (208)
#define PIN_GPIOAA1 (209)
#define PIN_GPIOAA2 (210)
#define PIN_GPIOAA3 (211)
#define PIN_GPIOAA4 (212)
#define PIN_GPIOAA5 (213)
#define PIN_GPIOAA6 (214)
#define PIN_GPIOAA7 (215)
#define PIN_GPIOAB0 (216)
#define PIN_GPIOAB1 (217)
#define PIN_GPIOAB2 (218)
#define PIN_GPIOAB3 (219)
#define PIN_GPIOAB4 (220)
#define PIN_GPIOAB5 (221)
#define PIN_GPIOAB6 (222)
#define PIN_GPIOAB7 (223)

#define BMC_HEARTBEAT PIN_GPIOD7
#define FT_LED_GREEN PIN_GPIOS4
#define FT_LED_AMBER PIN_GPIOS5
#define FRONT_ID_LED PIN_GPIOS6
#define PCH_PWR_GOOD PIN_GPIOC0
#define SYS_ERROR_LED PIN_GPIOD6
#define ALARM_CPU1_MEMGHJ_HOT PIN_GPIOC1
#define ALARM_CPU0_HOT PIN_GPIOC3
#define ALARM_CPU1_HOT PIN_GPIOC4
#define ALARM_CPU0_MEMDEF_HOT PIN_GPIOC5
#define ALARM_CPU1_MEMKLM_HOT PIN_GPIOC6
#define ALARM_CPU0_MEMABC_HOT PIN_GPIOC7
#define ALARM_SMI_ACTIVE PIN_GPIOD2
#define ALARM_SML_PMBUS PIN_GPIOAA1
#define ALARM_CPU0_PWR PIN_GPIOAA2
#define ALARM_CPU1_PWR PIN_GPIOAA3
#define ALARM_SML_ALERT PIN_GPIOS3
#define FAN_FAULT_LED0 PIN_GPIOM6
#define FAN_FAULT_LED1 PIN_GPIOM7
#define FAN_FAULT_LED2 PIN_GPIOH6
#define FAN_FAULT_LED3 PIN_GPIOAB1
#define FAN_FAULT_LED4 PIN_GPIOAB2
#define FAN_FAULT_LED5 PIN_GPIOAB3

extern int ast_gpio_open(void);
extern void ast_gpio_close(void);
extern int ast_set_gpio_value(unsigned gpio_pin, int value);
extern int ast_get_gpio_value(unsigned gpio_pin);
extern int ast_gpio_direction_input(unsigned gpio_pin);
extern int ast_gpio_direction_output(unsigned gpio_pin, int val);
extern int ast_get_reg_offset(unsigned int offset);
extern unsigned int ast_pwm_read(unsigned int offset);
extern void set_pwm_duty(int fannumb, int val);
void print_regs(void);
unsigned int ast_gpio_read(unsigned int offset);
void ast_gpio_write(unsigned int val, unsigned int offset);

/**
 * @brief pwm 데이터를 읽는 함수 BMC가 마스터인 경우의 수행(미확인)
 * @param i2cname pwm 이름ex) 12c-dev1
 * @param addr pwm slave 주소
 * @param type 센서타입 EX) NVA_PSU_FAN1 NVA_PSU_FAN2
 * @param value 반환값 인트만 나옴
 * @return error 시 false
 * @author Kicheol Park
 */
bool read_psu_value(string i2cname, uint8_t addr, uint8_t type, int *value);

#endif
#endif