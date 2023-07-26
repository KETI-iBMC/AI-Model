/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>

#include <drivers/edac.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

#define STACKSIZE	1024
#define PRIORITY	7

static atomic_t handled;

/**
 * Callback called from ISR. Runs in supervisor mode
 */
static void notification_callback(const struct device *dev, void *data)
{
	/* Special care need to be taken for NMI callback:
	 * delayed_work, mutex and semaphores are not working stable
	 * here, using integer increment for now
	 */
	atomic_set(&handled, true);
}

#define DEVICE_NAME DT_LABEL(DT_NODELABEL(ibecc))

void main(void)
{
	const struct device *dev;

	dev = device_get_binding(DEVICE_NAME);
	if (!dev) {
		LOG_ERR("Cannot open EDAC device: %s", DEVICE_NAME);
		return;
	}

	if (edac_notify_callback_set(dev, notification_callback)) {
		LOG_ERR("Cannot set notification callback");
		return;
	}

	LOG_INF("EDAC shell application initialized");
}

void thread_function(void)
{
	LOG_DBG("Thread started");

	while (true) {
		if (atomic_cas(&handled, true, false)) {
			printk("Got notification about IBECC event\n");
			k_sleep(K_MSEC(300));
		}
	}
}

K_THREAD_DEFINE(thread_id, STACKSIZE, thread_function, NULL, NULL, NULL,
		PRIORITY, 0, 0);
