/*
 * Copyright (c) 2018 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Board configuration macros for EM Starter kit board
 *
 * This header file is used to specify and describe board-level
 * aspects for the target.
 */

#ifndef _SOC_H_
#define _SOC_H_

#include <sys/util.h>

/* default system clock */
#define SYSCLK_DEFAULT_IOSC_HZ			MHZ(16)

/*
 * UART: use lr and sr to access subsystem uart IP
 */
#define UART_NS16550_ACCESS_IOPORT
#define UART_REG_ADDR_INTERVAL 4


/* ARC EM Core IRQs */
#define IRQ_TIMER0				16
#define IRQ_TIMER1				17
#include "soc_irq.h"

#define BASE_ADDR_SYSCONFIG		0xF000A000

#ifndef _ASMLANGUAGE


#include <sys/util.h>
#include <random/rand32.h>


#endif /* !_ASMLANGUAGE */

#endif /* _SOC_H_ */
