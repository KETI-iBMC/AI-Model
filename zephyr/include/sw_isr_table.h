/*
 * Copyright (c) 2014, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Software-managed ISR table
 *
 * Data types for a software-managed ISR table, with a parameter per-ISR.
 */

#ifndef ZEPHYR_INCLUDE_SW_ISR_TABLE_H_
#define ZEPHYR_INCLUDE_SW_ISR_TABLE_H_

#if !defined(_ASMLANGUAGE)
#include <zephyr/types.h>
#include <toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Note the order: arg first, then ISR. This allows a table entry to be
 * loaded arg -> r0, isr -> r3 in _isr_wrapper with one ldmia instruction,
 * on ARM Cortex-M (Thumb2).
 */
struct _isr_table_entry {
	const void *arg;
	void (*isr)(const void *);
};

/* The software ISR table itself, an array of these structures indexed by the
 * irq line
 */
extern struct _isr_table_entry _sw_isr_table[];

/*
 * Data structure created in a special binary .intlist section for each
 * configured interrupt. gen_irq_tables.py pulls this out of the binary and
 * uses it to create the IRQ vector table and the _sw_isr_table.
 *
 * More discussion in include/linker/intlist.ld
 */
struct _isr_list {
	/** IRQ line number */
	int32_t irq;
	/** Flags for this IRQ, see ISR_FLAG_* definitions */
	int32_t flags;
	/** ISR to call */
	void *func;
	/** Parameter for non-direct IRQs */
	const void *param;
};

/** This interrupt gets put directly in the vector table */
#define ISR_FLAG_DIRECT BIT(0)

#define _MK_ISR_NAME(x, y) __MK_ISR_NAME(x, y)
#define __MK_ISR_NAME(x, y) __isr_ ## x ## _irq_ ## y

/* Create an instance of struct _isr_list which gets put in the .intList
 * section. This gets consumed by gen_isr_tables.py which creates the vector
 * and/or SW ISR tables.
 */
#define Z_ISR_DECLARE(irq, flags, func, param) \
	static Z_DECL_ALIGN(struct _isr_list) Z_GENERIC_SECTION(.intList) \
		__used _MK_ISR_NAME(func, __COUNTER__) = \
			{irq, flags, (void *)&func, (const void *)param}

#define IRQ_TABLE_SIZE (CONFIG_NUM_IRQS - CONFIG_GEN_IRQ_START_VECTOR)

#ifdef CONFIG_DYNAMIC_INTERRUPTS
void z_isr_install(unsigned int irq, void (*routine)(const void *),
		   const void *param);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_SW_ISR_TABLE_H_ */
