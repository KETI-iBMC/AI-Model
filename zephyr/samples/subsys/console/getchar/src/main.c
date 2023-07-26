/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <console/console.h>

void main(void)
{
	console_init();

	printk("Start typing characters to see their hex codes printed\n");

	while (1) {
		uint8_t c = console_getchar();

		printk("char: [0x%x] %c\n", c, c);
	}
}
