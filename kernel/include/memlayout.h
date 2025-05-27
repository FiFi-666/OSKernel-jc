#ifndef __MEMLAYOUT_H
#define __MEMLAYOUT_H
#include "param.h"
// Physical memory layout

// k210 peripherals
// (0x0200_0000, 0x1000),      /* CLINT     */
// // we only need claim/complete for target0 after initializing
// (0x0C20_0000, 0x1000),      /* PLIC      */
// (0x3800_0000, 0x1000),      /* UARTHS    */
// (0x3800_1000, 0x1000),      /* GPIOHS    */
// (0x5020_0000, 0x1000),      /* GPIO      */
// (0x5024_0000, 0x1000),      /* SPI_SLAVE */
// (0x502B_0000, 0x1000),      /* FPIOA     */
// (0x502D_0000, 0x1000),      /* TIMER0    */
// (0x502E_0000, 0x1000),      /* TIMER1    */
// (0x502F_0000, 0x1000),      /* TIMER2    */
// (0x5044_0000, 0x1000),      /* SYSCTL    */
// (0x5200_0000, 0x1000),      /* SPI0      */
// (0x5300_0000, 0x1000),      /* SPI1      */
// (0x5400_0000, 0x1000),      /* SPI2      */
// (0x8000_0000, 0x600000),    /* Memory    */

// visionfive 2 peripherals
// (0x200_0000, 0x10000),      /*CLINT      */
// (0xc00_0000, 0x400_0000)    /*PLIC       */

// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0 
// 10001000 -- virtio disk 
// 80000000 -- boot ROM jumps here in machine mode
//             -kernel loads the kernel here
// unused RAM after 80000000.

#define VIRT_OFFSET             0x3F00000000L

// qemu puts UART registers here in physical memory.
#define UART                    0x10000000L
#define UART_V                  (UART + VIRT_OFFSET)

#define SD_BASE            0x16020000
#define SD_BASE_V               (SD_BASE + VIRT_OFFSET)

#ifdef QEMU
// virtio mmio interface
#define VIRTIO0                 0x10001000
#define VIRTIO0_V               (VIRTIO0 + VIRT_OFFSET)
#endif

// local interrupt controller, which contains the timer.
#define CLINT                   0x02000000L
#define CLINT_V                 (CLINT + VIRT_OFFSET)

#define PLIC                    0x0c000000L
#define PLIC_V                  (PLIC + VIRT_OFFSET)

#define PLIC_PRIORITY           (PLIC_V + 0x0)
#define PLIC_PENDING            (PLIC_V + 0x1000)
#define PLIC_MENABLE(hart)      (PLIC_V + 0x2000 + (hart) * 0x100)
#define PLIC_SENABLE(hart)      (PLIC_V + 0x2080 + (hart) * 0x100)
#define PLIC_MPRIORITY(hart)    (PLIC_V + 0x200000 + (hart) * 0x2000)
#define PLIC_SPRIORITY(hart)    (PLIC_V + 0x201000 + (hart) * 0x2000)
#define PLIC_MCLAIM(hart)       (PLIC_V + 0x200004 + (hart) * 0x2000)
#define PLIC_SCLAIM(hart)       (PLIC_V + 0x201004 + (hart) * 0x2000)


// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80200000 to PHYSTOP.
#define KERNBASE                0x80200000

// #define PHYSTOP                 0x240000000
// #define PHYSTOP                 0x80a00000
//128M  0x800_0000  4G  0x10000_0000
#ifdef QEMU
#define PHYSTOP                 0x88000000
#else
#define PHYSTOP                 0x140000000
#endif

// map the trampoline page to the highest address,
// in both user and kernel space.
#define TRAMPOLINE              (MAXVA - PGSIZE)

// 0x3F00000000L 上面是 mmio地址
// map kernel stacks beneath the trampoline,
// each surrounded by invalid guard pages.
// #define KSTACK(p)               (TRAMPOLINE - ((p) + 1) * 2 * PGSIZE)
#define VKSTACK                 0x3EC0000000L //0x3E_C000_0000
#define KSTACKSIZE              6 * PGSIZE
#define EXTRASIZE               2 * PGSIZE
#define PROCVKSTACK(paddrnum)        (VKSTACK - ((((paddrnum) + 1) % NPROC) * (KSTACKSIZE + EXTRASIZE)) + EXTRASIZE)

// User memory layout.
// Address zero first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
//   ...
//   TRAPFRAME (p->trapframe, used by the trampoline)
//   TRAMPOLINE (the same page as in the kernel)
#define TRAPFRAME               (TRAMPOLINE - PGSIZE)
#define SIGTRAMPOLINE           (TRAPFRAME - PGSIZE)

#define MAXUVA                  0x80000000L
#define USER_STACK_BOTTOM (MAXUVA - (2*PGSIZE))
#define USER_MMAP_START (USER_STACK_BOTTOM - 0x10000000)

//注意，上面的user_stack_bottom定义不管他，目前用下面这个
#define USER_STACK_TOP MAXUVA - PGSIZE
#define USER_STACK_DOWN USER_MMAP_START + PGSIZE

#define KERNEL_PROCESS_SP_TOP (1UL << 36)

#endif
