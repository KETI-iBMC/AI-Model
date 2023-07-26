/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <fs/fs.h>
#include "test_common.h"
#include "test_fat.h"
#include "test_fat_priv.h"


void test_fat_mkdir(void)
{
	zassert_true(test_mkdir(TEST_DIR_PATH, TEST_FILE) == TC_PASS, NULL);
}

void test_fat_readdir(void)
{
	zassert_true(test_lsdir(TEST_DIR_PATH) == TC_PASS, NULL);
}

void test_fat_rmdir(void)
{
	zassert_true(test_rmdir(TEST_DIR_PATH) == TC_PASS, NULL);
}
