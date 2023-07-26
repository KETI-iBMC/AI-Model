/*
 * Copyright (c) 2019, Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <toolchain.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>

#include <reent.h>

int _gettimeofday_r(struct _reent *r, struct timeval *__tp, void *__tzp)
{
	ARG_UNUSED(r);
	ARG_UNUSED(__tp);
	ARG_UNUSED(__tzp);

	return -1;
}
