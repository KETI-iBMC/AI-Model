/*
 * Copyright (c) 2020 Sven Herrmann
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>

void main(void)
{
	const char *const devname = DT_LABEL(DT_INST(0, honeywell_mpr));
	const struct device *dev = device_get_binding(devname);
	int rc;

	if (dev == NULL) {
		printf("Device %s not found.\n", devname);
		return;
	}

	while (1) {
		struct sensor_value pressure;

		rc = sensor_sample_fetch(dev);
		if (rc != 0) {
			printf("sensor_sample_fetch error: %d\n", rc);
			break;
		}

		rc = sensor_channel_get(dev, SENSOR_CHAN_PRESS, &pressure);
		if (rc != 0) {
			printf("sensor_channel_get error: %d\n", rc);
			break;
		}

		printf("pressure: %u.%u kPa\n", pressure.val1, pressure.val2);

		k_sleep(K_SECONDS(1));
	}
}
