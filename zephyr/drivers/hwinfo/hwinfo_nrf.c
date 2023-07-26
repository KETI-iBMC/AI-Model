/*
 * Copyright (c) 2018 Alexander Wachter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <drivers/hwinfo.h>
#include <string.h>
#include <hal/nrf_ficr.h>
#include <sys/byteorder.h>

struct nrf_uid {
	uint32_t id[2];
};

ssize_t z_impl_hwinfo_get_device_id(uint8_t *buffer, size_t length)
{
	struct nrf_uid dev_id;

	dev_id.id[0] = sys_cpu_to_be32(nrf_ficr_deviceid_get(NRF_FICR, 1));
	dev_id.id[1] = sys_cpu_to_be32(nrf_ficr_deviceid_get(NRF_FICR, 0));

	if (length > sizeof(dev_id.id)) {
		length = sizeof(dev_id.id);
	}

	memcpy(buffer, dev_id.id, length);

	return length;
}
