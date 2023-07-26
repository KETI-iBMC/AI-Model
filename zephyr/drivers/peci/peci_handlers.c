/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/peci.h>
#include <syscall_handler.h>


static inline int z_vrfy_peci_config(const struct device *dev,
				     uint32_t bitrate)
{
	Z_OOPS(Z_SYSCALL_DRIVER_PECI(dev, config));

	return z_impl_peci_config(dev, bitrate);
}
#include <syscalls/peci_config_mrsh.c>

static inline int z_vrfy_peci_enable(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_PECI(dev, enable));

	return z_impl_peci_enable(dev);
}
#include <syscalls/peci_enable_mrsh.c>

static inline int z_vrfy_peci_disable(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_PECI(dev, disable));

	return z_impl_peci_disable(dev);
}
#include <syscalls/peci_disable_mrsh.c>

static inline int z_vrfy_peci_transfer(const struct device *dev,
				       struct peci_msg *msg)
{
	struct peci_msg msg_copy;

	Z_OOPS(Z_SYSCALL_DRIVER_PECI(dev, transfer));
	Z_OOPS(z_user_from_copy(&msg_copy, msg, sizeof(*msg)));

	return z_impl_peci_transfer(dev, &msg_copy);
}
#include <syscalls/peci_transfer_mrsh.c>
