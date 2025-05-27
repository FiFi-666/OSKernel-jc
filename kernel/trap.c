#include "include/trap.h"
#include "include/console.h"
#include "include/disk.h"
#include "include/memlayout.h"
#include "include/param.h"
#include "include/plic.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/sbi.h"
#include "include/spinlock.h"
#include "include/syscall.h"
#include "include/timer.h"
#include "include/types.h"
#include "include/uart.h"
#include "include/vm.h"
#include "include/vma.h"

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
extern void kernelvec();

int devintr();

// void
// trapinit(void)
// {
//   initlock(&tickslock, "time");
//   #ifdef DEBUG
//   printf("trapinit\n");
//   #endif
// }

// set up to take exceptions and traps while in the kernel.
void trapinithart(void) {
  w_stvec((uint64)kernelvec);
  w_sstatus(r_sstatus() | SSTATUS_SIE);
  // enable supervisor-mode timer interrupts.
  w_sie(r_sie() | SIE_SEIE | SIE_SSIE | SIE_STIE);
  set_next_timeout();
#ifdef DEBUG
  printf("trapinithart\n");
#endif
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void usertrap(void) {
  // printf("run in usertrap\n");
  int which_dev = 0;

  if ((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();

  // save user program counter.
  p->trapframe->epc = r_sepc();

  if (r_scause() == 8) {
    // system call
    if (p->killed == SIGTERM)
      exit(-1);
    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;
    // an interrupt will change sstatus &c registers,
    // so don't enable until done with those registers.
    intr_on();
    syscall();
  } else if ((r_scause() == 13 || r_scause() == 15) &&
             (handle_stack_page_fault(myproc(), r_stval()) == 0)) {
    // load page fault or store page fault
    // check if the page fault is caused by stack growth
    printf("handle stack page fault\n");
  } else if ((which_dev = devintr()) != 0) {
    // ok
  } else if (r_scause() == 3) {
    printf("ebreak\n");
    trapframedump(p->trapframe);
    p->trapframe->epc += 2;
  }

  else {
    uint64 ir = 0;
    copyin(myproc()->pagetable, (char *)&ir, r_sepc(), 8);
    serious_print("\nusertrap(): unexpected scause %p pid=%d %s\n", r_scause(),
                  p->pid, p->name);
    serious_print("            sepc=%p stval=%p ir: %p\n", r_sepc(), r_stval(),
                  ir);
    trapframedump(p->trapframe);
    p->killed = SIGTERM;
  }

  if (p->killed) {
    if (p->killed == SIGTERM) {
      exit(-1);
    }
    sighandle();
  }

  // give up the CPU if this is a timer interrupt.
  if (which_dev == 2) {
    p->utime++;
    yield();
  }

  usertrapret();
}

//
// return to user space
//
void usertrapret(void) {
  struct proc *p = myproc();

  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to trampoline.S
  w_stvec(TRAMPOLINE + (uservec - trampoline));

  // set up trapframe values that uservec will need when
  // the process next re-enters the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp(); // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.

  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  // printf("[usertrapret]p->pagetable: %p\n", p->pagetable);
  uint64 satp = MAKE_SATP(p->pagetable);
  // printf("usertrapret epc:%p\n", p->trapframe->epc);
  //  jump to trampoline.S at the top of memory, which
  //  switches to the user page table, restores user registers,
  //  and switches to user mode with sret.
  uint64 fn = TRAMPOLINE + (userret - trampoline);
  ((void (*)(uint64, uint64))fn)(TRAPFRAME, satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void kerneltrap() {
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();

  if ((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if (intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if ((which_dev = devintr()) == 0) {
    serious_print("\nscause %p\n", scause);
    serious_print("sepc=%p stval=%p hart=%d\n", r_sepc(), r_stval(), r_tp());
    struct proc *p = myproc();
    if (p != 0) {
      serious_print("pid: %d, name: %s\n", p->pid, p->name);
    }
    panic("kerneltrap");
  }
  // printf("which_dev: %d\n", which_dev);
  if (which_dev == 2 && myproc() != 0) {
    myproc()->ktime++;
  }

  // give up the CPU if this is a timer interrupt.
  if (which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING) {
    yield();
  }
  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

// Check if it's an external/software interrupt,
// and handle it.
// returns  2 if timer interrupt,
//          1 if other device,
//          0 if not recognized.
int devintr(void) {
  uint64 scause = r_scause();

  if ((0x8000000000000000L & scause) && 9 == (scause & 0xff)) {
    int irq = plic_claim();
    if (UART_IRQ == irq) {
      // keyboard input
      int c = sbi_console_getchar();
      if (-1 != c) {
        consoleintr(c);
      }
    } else if (DISK_IRQ == irq) {
      disk_intr();
    } else if (irq) {
      serious_print("unexpected interrupt irq = %d\n", irq);
    }

    if (irq) {
      plic_complete(irq);
    }

#ifdef k210
    w_sip(r_sip() & ~2); // clear pending bit
    sbi_set_mie();
#endif

    return 1;
  } else if (0x8000000000000005L == scause) {
    timer_tick();
    return 2;
  } else {
    serious_print("scause: %p\n", scause);
    serious_print("stval: %p\n", r_stval());
    return 0;
  }
}

void trapframedump(struct trapframe *tf) {
  serious_print("a0: %p\t", tf->a0);
  serious_print("a1: %p\t", tf->a1);
  serious_print("a2: %p\t", tf->a2);
  serious_print("a3: %p\n", tf->a3);
  serious_print("a4: %p\t", tf->a4);
  serious_print("a5: %p\t", tf->a5);
  serious_print("a6: %p\t", tf->a6);
  serious_print("a7: %p\n", tf->a7);
  serious_print("t0: %p\t", tf->t0);
  serious_print("t1: %p\t", tf->t1);
  serious_print("t2: %p\t", tf->t2);
  serious_print("t3: %p\n", tf->t3);
  serious_print("t4: %p\t", tf->t4);
  serious_print("t5: %p\t", tf->t5);
  serious_print("t6: %p\t", tf->t6);
  serious_print("s0: %p\n", tf->s0);
  serious_print("s1: %p\t", tf->s1);
  serious_print("s2: %p\t", tf->s2);
  serious_print("s3: %p\t", tf->s3);
  serious_print("s4: %p\n", tf->s4);
  serious_print("s5: %p\t", tf->s5);
  serious_print("s6: %p\t", tf->s6);
  serious_print("s7: %p\t", tf->s7);
  serious_print("s8: %p\n", tf->s8);
  serious_print("s9: %p\t", tf->s9);
  serious_print("s10: %p\t", tf->s10);
  serious_print("s11: %p\t", tf->s11);
  serious_print("ra: %p\n", tf->ra);
  serious_print("sp: %p\t", tf->sp);
  serious_print("gp: %p\t", tf->gp);
  serious_print("tp: %p\t", tf->tp);
  serious_print("epc: %p\n", tf->epc);
}
