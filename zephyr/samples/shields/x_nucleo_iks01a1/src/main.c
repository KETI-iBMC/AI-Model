/*
 * Copyright (c) 2018 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <sys/util.h>

#ifdef CONFIG_LIS3MDL_TRIGGER
static int lis3mdl_trig_cnt;

static void lis3mdl_trigger_handler(const struct device *dev,
				    struct sensor_trigger *trig)
{
	sensor_sample_fetch_chan(dev, trig->chan);
	lis3mdl_trig_cnt++;
}
#endif

void main(void)
{
	struct sensor_value temp, hum, press;
	struct sensor_value magn_xyz[3], accel_xyz[3];
	const struct device *hts221 = device_get_binding(DT_LABEL(DT_INST(0, st_hts221)));
	const struct device *lis3mdl = device_get_binding(DT_LABEL(DT_INST(0, st_lis3mdl_magn)));
	const struct device *lsm6ds0 = device_get_binding(DT_LABEL(DT_INST(0, st_lsm6ds0)));
	const struct device *lps25hb = device_get_binding(DT_LABEL(DT_INST(0, st_lps25hb_press)));
#if defined(CONFIG_LIS3MDL_TRIGGER)
	struct sensor_trigger trig;
	int cnt = 1;
#endif

	if (hts221 == NULL) {
		printf("Could not get HTS221 device\n");
		return;
	}
	if (lis3mdl == NULL) {
		printf("Could not get LIS3MDL device\n");
		return;
	}
	if (lsm6ds0 == NULL) {
		printf("Could not get LSM6DS0 device\n");
		return;
	}
	if (lps25hb == NULL) {
		printf("Could not get LPS25HB device\n");
		return;
	}

#ifdef CONFIG_LIS3MDL_TRIGGER
	trig.type = SENSOR_TRIG_DATA_READY;
	trig.chan = SENSOR_CHAN_MAGN_XYZ;
	sensor_trigger_set(lis3mdl, &trig, lis3mdl_trigger_handler);
#endif

	while (1) {

		/* Get sensor samples */

		if (sensor_sample_fetch(hts221) < 0) {
			printf("HTS221 Sensor sample update error\n");
			return;
		}
		if (sensor_sample_fetch(lps25hb) < 0) {
			printf("LPS25HB Sensor sample update error\n");
			return;
		}
#ifndef CONFIG_LIS3MDL_TRIGGER
		if (sensor_sample_fetch(lis3mdl) < 0) {
			printf("LIS3MDL Sensor sample update error\n");
			return;
		}
#endif
		if (sensor_sample_fetch(lsm6ds0) < 0) {
			printf("LSM6DS0 Sensor sample update error\n");
			return;
		}

		/* Get sensor data */

		sensor_channel_get(hts221, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(hts221, SENSOR_CHAN_HUMIDITY, &hum);
		sensor_channel_get(lps25hb, SENSOR_CHAN_PRESS, &press);
		sensor_channel_get(lis3mdl, SENSOR_CHAN_MAGN_XYZ, magn_xyz);
		sensor_channel_get(lsm6ds0, SENSOR_CHAN_ACCEL_XYZ, accel_xyz);

		/* Display sensor data */

		/* Erase previous */
		printf("\0033\014");

		printf("X-NUCLEO-IKS01A1 sensor dashboard\n\n");

		/* temperature */
		printf("HTS221: Temperature: %.1f C\n",
		       sensor_value_to_double(&temp));

		/* humidity */
		printf("HTS221: Relative Humidity: %.1f%%\n",
		       sensor_value_to_double(&hum));

		/* pressure */
		printf("LPS25HB: Pressure:%.1f kpa\n",
		       sensor_value_to_double(&press));

		/* magneto data */
		printf(
		 "LIS3MDL: Magnetic field (gauss): x: %.1f, y: %.1f, z: %.1f\n",
		 sensor_value_to_double(&magn_xyz[0]),
		 sensor_value_to_double(&magn_xyz[1]),
		 sensor_value_to_double(&magn_xyz[2]));

		/* acceleration */
		printf(
		   "LSM6DS0: Acceleration (m.s-2): x: %.1f, y: %.1f, z: %.1f\n",
		   sensor_value_to_double(&accel_xyz[0]),
		   sensor_value_to_double(&accel_xyz[1]),
		   sensor_value_to_double(&accel_xyz[2]));


#if defined(CONFIG_LIS3MDL_TRIGGER)
		printk("%d:: lis3mdl trig %d\n", cnt, lis3mdl_trig_cnt);
		cnt++;
#endif

		k_sleep(K_MSEC(2000));
	}
}
