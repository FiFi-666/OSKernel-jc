/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __RISCV_IO_H__
#define __RISCV_IO_H__
#include "types.h"

static inline void __raw_writeb(uint8 val, volatile void *addr)
{
	asm volatile("sb %0, 0(%1)" : : "r"(val), "r"(addr));
}

static inline void __raw_writew(uint16 val, volatile void *addr)
{
	asm volatile("sh %0, 0(%1)" : : "r"(val), "r"(addr));
}

static inline void __raw_writel(uint32 val, volatile void *addr)
{
	asm volatile("sw %0, 0(%1)" : : "r"(val), "r"(addr));
}

static inline uint8 __raw_readb(const volatile void *addr)
{
	uint8 val;

	asm volatile("lb %0, 0(%1)" : "=r"(val) : "r"(addr));
	return val;
}

static inline uint16 __raw_readw(const volatile void *addr)
{
	uint16 val;

	asm volatile("lh %0, 0(%1)" : "=r"(val) : "r"(addr));
	return val;
}

static inline uint32 __raw_readl(const volatile void *addr)
{
	uint32 val;

	asm volatile("lw %0, 0(%1)" : "=r"(val) : "r"(addr));
	return val;
}

/* FIXME: These are now the same as asm-generic */

/* clang-format off */

#define __io_rbr()		do {} while (0)
#define __io_rar()		do {} while (0)
#define __io_rbw()		do {} while (0)
#define __io_raw()		do {} while (0)

#define readb_relaxed(c)	({ uint8  __v; __io_rbr(); __v = __raw_readb(c); __io_rar(); __v; })
#define readw_relaxed(c)	({ uint16 __v; __io_rbr(); __v = __raw_readw(c); __io_rar(); __v; })
#define readl_relaxed(c)	({ uint32 __v; __io_rbr(); __v = __raw_readl(c); __io_rar(); __v; })

#define writeb_relaxed(v,c)	({ __io_rbw(); __raw_writeb((v),(c)); __io_raw(); })
#define writew_relaxed(v,c)	({ __io_rbw(); __raw_writew((v),(c)); __io_raw(); })
#define writel_relaxed(v,c)	({ __io_rbw(); __raw_writel((v),(c)); __io_raw(); })

#define __io_br()	do {} while (0)
#define __io_ar()	__asm__ __volatile__ ("fence i,r" : : : "memory");
#define __io_bw()	__asm__ __volatile__ ("fence w,o" : : : "memory");
#define __io_aw()	do {} while (0)

#define readb(c)	({ uint8  __v; __io_br(); __v = __raw_readb(c); __io_ar(); __v; })
#define readw(c)	({ uint16 __v; __io_br(); __v = __raw_readw(c); __io_ar(); __v; })
#define readl(c)	({ uint32 __v; __io_br(); __v = __raw_readl(c); __io_ar(); __v; })

#define writeb(v,c)	({ __io_bw(); __raw_writeb((v),(c)); __io_aw(); })
#define writew(v,c)	({ __io_bw(); __raw_writew((v),(c)); __io_aw(); })
#define writel(v,c)	({ __io_bw(); __raw_writel((v),(c)); __io_aw(); })

/* clang-format on */

#endif
