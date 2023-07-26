/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <irq_offload.h>
#include <ztest_error_hook.h>

#define TIMEOUT K_MSEC(100)
#define STACK_SIZE (512 + CONFIG_TEST_EXTRA_STACKSIZE)
#define STACK_LEN 2

static ZTEST_BMEM stack_data_t data[STACK_LEN];
extern struct k_stack stack;
K_THREAD_STACK_DEFINE(threadstack2, STACK_SIZE);
struct k_thread thread_data2;

static void stack_pop_fail(struct k_stack *stack)
{
	stack_data_t rx_data;

	/**TESTPOINT: stack pop returns -EBUSY*/
	zassert_equal(k_stack_pop(stack, &rx_data, K_NO_WAIT), -EBUSY, NULL);
	/**TESTPOINT: stack pop returns -EAGAIN*/
	zassert_equal(k_stack_pop(stack, &rx_data, TIMEOUT), -EAGAIN, NULL);
}

/**
 * @addtogroup kernel_stack_tests
 * @{
 */

/* Sub-thread entry */
void tStack_pop_entry(void *p1, void *p2, void *p3)
{
	zassert_true(k_stack_pop(p1, p2, K_FOREVER), "stack pop failed\n");
}

/**
 * @brief Verifies stack pop functionality
 * @see k_stack_init(), k_stack_pop()
 */
void test_stack_pop_fail(void)
{
	k_stack_init(&stack, data, STACK_LEN);

	stack_pop_fail(&stack);
}

/**
 * @brief Verifies cleanup a stack that still be needed by another
 * thread.
 * @see k_stack_cleanup()
 */
void test_stack_cleanup_error(void)
{
	stack_data_t rx_data[STACK_LEN - 1];

	k_stack_init(&stack, data, STACK_LEN);
	/* Creat a new thread */
	k_tid_t tid = k_thread_create(&thread_data2, threadstack2, STACK_SIZE,
					tStack_pop_entry, &stack,
					rx_data, NULL, K_PRIO_PREEMPT(0), 0,
					K_NO_WAIT);
	/* Delay for finishing some actions of the new thread */
	k_sleep(K_MSEC(500));
	/* Try to clean up the stack, that still be waited by the thread */
	zassert_true(k_stack_cleanup(&stack) == -EAGAIN, "The stack is cleanuped successful");
	/* clear the spawn thread to avoid side effect */
	k_thread_abort(tid);
}

/**
 * @brief Verifies push a data in the full stack.
 * @see k_stack_push()
 */
void test_stack_push_full(void)
{
	stack_data_t tx_data[STACK_LEN] = {0};
	stack_data_t data_tmp = 0;

	k_stack_init(&stack, data, STACK_LEN);
	for (int i = 0; i < STACK_LEN; i++) {
		zassert_true(k_stack_push(&stack, tx_data[i]) == 0, "push data into stack failed");
	}
	/* Verify that push a data in the full stack, a nagetive value will be met */
	zassert_true(k_stack_push(&stack, data_tmp) == -ENOMEM, "push data successful");
}

#ifdef CONFIG_USERSPACE
/**
 * @brief Verifies stack pop from a user thread
 * @see k_stack_init(), k_stack_pop()
 */
void test_stack_user_pop_fail(void)
{
	struct k_stack *alloc_stack = k_object_alloc(K_OBJ_STACK);

	zassert_not_null(alloc_stack, "couldn't allocate stack object");
	zassert_false(k_stack_alloc_init(alloc_stack, STACK_LEN),
		      "stack init failed");

	stack_pop_fail(alloc_stack);
}

/**
 * @brief Verifies stack alloc and initialize a null pointer.
 * @see k_stack_alloc_init()
 */
void test_stack_user_init_null(void)
{
	ztest_set_fault_valid(true);
	k_stack_alloc_init(NULL, STACK_LEN);
}

/**
 * @brief Verify that alloc and initialize a stack with
 * 0 memory.
 * @see k_stack_alloc_init()
 */
void test_stack_user_init_invalid_value(void)
{
	ztest_set_fault_valid(true);
	struct k_stack *alloc_stack = k_object_alloc(K_OBJ_STACK);

	zassert_not_null(alloc_stack, "couldn't allocate stack object");
	k_stack_alloc_init(alloc_stack, 0);
}

/**
 * @brief Verify that push some data into a NULL
 * pointer.
 * @see k_stack_push()
 */
void test_stack_user_push_null(void)
{
	ztest_set_fault_valid(true);
	k_stack_push(NULL, 0);
}

/**
 * @brief Verifies pop data from a NULL pointer.
 * @see k_stack_pop()
 */
void test_stack_user_pop_null(void)
{
	ztest_set_fault_valid(true);
	k_stack_pop(NULL, 0, K_NO_WAIT);
}

/**
 * @brief Verifies cleanup a stack that its data still be waited by
 * another thread.
 * @see k_stack_pop()
 */
void test_stack_user_pop_permission(void)
{
	ztest_set_fault_valid(true);
	struct k_stack *alloc_stack = k_object_alloc(K_OBJ_STACK);

	zassert_not_null(alloc_stack, "couldn't allocate stack object");
	zassert_false(k_stack_alloc_init(alloc_stack, STACK_LEN),
		      "stack init failed");
	/* Try to access and to write data at invalid address */
	k_stack_pop(alloc_stack, (stack_data_t *)alloc_stack, K_NO_WAIT);
}
#endif
/**
 * @}
 */
