/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include "mem_protect.h"

static K_APP_DMEM(ztest_mem_partition) int var = 1356;
static K_APP_BMEM(ztest_mem_partition) int zeroed_var = 20420;
static K_APP_BMEM(ztest_mem_partition) int bss_var;

/**
 * @brief Test assigning global data and BSS variables to memory partitions
 *
 * @details Test that system supports application assigning global data and BSS
 * variables using macros K_APP_BMEM() and K_APP_DMEM
 *
 * @ingroup kernel_memprotect_tests
 */
void test_mem_part_assign_bss_vars_zero(void)
{
	/* The global variable var will be inside the bounds of
	 * ztest_mem_partition and be initialized with 1356 at boot.
	 */
	zassert_true(var == 1356, NULL);

	/* The global variable zeroed_var will be inside the bounds of
	 * ztest_mem_partition and must be zeroed at boot size K_APP_BMEM() was
	 * used, indicating a BSS variable.
	 */
	zassert_true(zeroed_var == 0, NULL);

	/* The global variable var will be inside the bounds of
	 * ztest_mem_partition and must be zeroed at boot size K_APP_BMEM() was
	 * used, indicating a BSS variable.
	 */
	zassert_true(bss_var == 0, NULL);
}

K_APPMEM_PARTITION_DEFINE(part_arch);
K_APP_BMEM(part_arch) uint8_t __aligned(MEM_REGION_ALLOC)
	buf_arc[MEM_REGION_ALLOC];

/**
 * @brief Test partitions sized per the constraints of the MPU hardware
 *
 * @details
 * - MEM_REGION_ALLOC is pre-sized to naturally fit in the target hardware's
 *   memory management granularity. Show that the partition size matches.
 * - Show that the base address of the partition is properly set, it should
 *   match the base address of buf_arc.
 *
 * @ingroup kernel_memprotect_tests
 */
void test_mem_part_auto_determ_size(void)
{
	zassert_true(part_arch.size == MEM_REGION_ALLOC, NULL);
	zassert_true(part_arch.start == (uintptr_t)buf_arc,
	   "Base address of memory partition not determined at build time");
}
