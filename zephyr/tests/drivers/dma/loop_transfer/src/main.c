/*
 * Copyright (c) 2021 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr.h>
#include <ztest.h>

extern void test_dma_m2m_loop(void);

void test_main(void)
{
	ztest_test_suite(dma_m2m_loop_test,
			 ztest_unit_test(test_dma_m2m_loop));
	ztest_run_test_suite(dma_m2m_loop_test);
}
