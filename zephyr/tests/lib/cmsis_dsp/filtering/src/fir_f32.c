/*
 * Copyright (c) 2020 Stephanos Ioannidis <root@stephanos.io>
 * Copyright (C) 2010-2020 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <zephyr.h>
#include <stdlib.h>
#include <arm_math.h>
#include "../../common/test_common.h"

#include "fir_f32.pat"

#define SNR_ERROR_THRESH	((float32_t)120)
#define REL_ERROR_THRESH	(3.0e-5)

static void test_arm_fir_f32(void)
{
	size_t sample_index, block_index;
	size_t block_size, tap_count;
	size_t sample_count = ARRAY_SIZE(in_config) / 2;
	size_t length = ARRAY_SIZE(ref_val);
	const uint16_t *config = in_config;
	const float32_t *input = (const float32_t *)in_val;
	const float32_t *coeff = (const float32_t *)in_coeff;
	const float32_t *ref = (const float32_t *)ref_val;
	float32_t *state, *output_buf, *output;
	arm_fir_instance_f32 inst;

	/* Allocate buffers */
	state = malloc(64 * sizeof(float32_t));
	zassert_not_null(state, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	output_buf = malloc(length * sizeof(float32_t));
	zassert_not_null(output_buf, ASSERT_MSG_BUFFER_ALLOC_FAILED);

	output = output_buf;

	/* Enumerate samples */
	for (sample_index = 0; sample_index < sample_count; sample_index++) {
		/* Resolve sample configurations */
		block_size = config[0];
		tap_count = config[1];

		/* Initialise instance */
		arm_fir_init_f32(&inst, tap_count, coeff, state, block_size);

		/* TODO: Add MEVF support */

		/* Reset input pointer */
		input = (const float32_t *)in_val;

		/* Enumerate blocks */
		for (block_index = 0; block_index < 2; block_index++) {
			/* Run test function */
			arm_fir_f32(&inst, input, output, block_size);

			/* Increment pointers */
			input += block_size;
			output += block_size;
		}

		/* Increment pointers */
		coeff += tap_count;
		config += 2;
	}

	/* Validate output */
	zassert_true(
		test_snr_error_f32(length, output_buf, ref, SNR_ERROR_THRESH),
		ASSERT_MSG_SNR_LIMIT_EXCEED);

	zassert_true(
		test_rel_error_f32(length, output_buf, ref, REL_ERROR_THRESH),
		ASSERT_MSG_REL_ERROR_LIMIT_EXCEED);

	/* Free buffers */
	free(state);
	free(output_buf);
}

void test_filtering_fir_f32(void)
{
	ztest_test_suite(filtering_fir_f32,
		ztest_unit_test(test_arm_fir_f32)
		);

	ztest_run_test_suite(filtering_fir_f32);
}
