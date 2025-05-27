
#include "include/plic.h"
#include "include/memlayout.h"
#include "include/param.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/sbi.h"
#include "include/types.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

void plicinit(void) {
  writed(1, PLIC_V + DISK_IRQ * sizeof(uint32));
  writed(1, PLIC_V + UART_IRQ * sizeof(uint32));

#ifdef DEBUG
  printf("plicinit\n");
#endif
}

void plicinithart(void) {
  int hart = cpuid();
#ifdef QEMU
  // set uart's enable bit for this hart's S-mode.
  *(uint32 *)PLIC_SENABLE(hart) = (1 << UART_IRQ) | (1 << DISK_IRQ);
  // set this hart's S-mode priority threshold to 0.
  *(uint32 *)PLIC_SPRIORITY(hart) = 0;
#endif

#ifdef DEBUG
  printf("plicinithart\n");
#endif
}

// ask the PLIC what interrupt we should serve.
int plic_claim(void) {
  int hart = cpuid();
  int irq;
#ifndef QEMU
  irq = *(uint32 *)PLIC_MCLAIM(hart);
#else
  irq = *(uint32 *)PLIC_SCLAIM(hart);
#endif
  return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int irq) {
  int hart = cpuid();
#ifndef QEMU
  *(uint32 *)PLIC_MCLAIM(hart) = irq;
#else
  *(uint32 *)PLIC_SCLAIM(hart) = irq;
#endif
}
