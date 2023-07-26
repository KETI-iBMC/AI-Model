/*
 * Copyright (c) 2019 Linaro Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/gpio.h>
#include <init.h>
#include <kernel.h>
#include <drivers/pinmux.h>
#include <sys/sys_io.h>

#include <pinmux/stm32/pinmux_stm32.h>

static const struct pin_config pinconf[] = {
	/* RF_CTX_PA */
	{STM32_PIN_PA4, STM32_PUSHPULL_PULLUP},
	/* RF_CRX_RX */
	{STM32_PIN_PB6, STM32_PUSHPULL_PULLUP},
	/* RF_CBT_HF */
	{STM32_PIN_PB7, STM32_PUSHPULL_PULLUP},
};

static int pinmux_stm32_init(const struct device *port)
{
	ARG_UNUSED(port);
	const struct device *gpioa, *gpiob, *gpioh;

	stm32_setup_pins(pinconf, ARRAY_SIZE(pinconf));

	gpioa = device_get_binding(DT_LABEL(DT_NODELABEL(gpioa)));
	if (!gpioa) {
		return -ENODEV;
	}

	gpiob = device_get_binding(DT_LABEL(DT_NODELABEL(gpiob)));
	if (!gpiob) {
		return -ENODEV;
	}

	gpioh = device_get_binding(DT_LABEL(DT_NODELABEL(gpioh)));
	if (!gpioh) {
		return -ENODEV;
	}

	gpio_pin_configure(gpioa, 4, GPIO_OUTPUT);
	gpio_pin_set(gpioa, 4, 1);

	gpio_pin_configure(gpiob, 6, GPIO_OUTPUT);
	gpio_pin_set(gpiob, 6, 1);

	gpio_pin_configure(gpiob, 7, GPIO_OUTPUT);
	gpio_pin_set(gpiob, 7, 0);

	gpio_pin_configure(gpioh, 1, GPIO_OUTPUT);
	gpio_pin_set(gpioh, 1, 1);

	return 0;
}

/* Need to be initialised after GPIO driver */
SYS_INIT(pinmux_stm32_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
