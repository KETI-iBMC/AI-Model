/*
 * Copyright (c) 2018 Alexander Wachter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <sys/printk.h>

void main(void)
{
	const struct device *dev;
	struct sensor_value temperature, humidity;

	dev = device_get_binding(DT_LABEL(DT_INST(0, ams_ens210)));
	if (!dev) {
		printk("Failed to get device binding");
		return;
	}

	printk("device is %p, name is %s\n", dev, dev->name);

	while (1) {
		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);
		sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
		printk("Temperature: %d.%06d C; Humidity: %d.%06d%%\n",
			temperature.val1, temperature.val2,
			humidity.val1, humidity.val2);

		k_sleep(K_MSEC(1000));
	}
}
