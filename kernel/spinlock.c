// Mutual exclusion spin locks.

#include "include/spinlock.h"
#include "include/intr.h"
#include "include/memlayout.h"
#include "include/param.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/types.h"

void initlock(struct spinlock *lk, char *name) {
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void acquire(struct spinlock *lk) {
  push_off(); // disable interrupts to avoid deadlock.
  // if(!(lk->name[0] == 'k' && lk->name[1] == 'm' && lk->name[2] == 'e' &&
  // lk->name[3] == 'm')
  // && !(lk->name[0] == 'p' && lk->name[1] == 'r' && lk->name[2] == 'o' &&
  // lk->name[3] == 'c')
  // && !(lk->name[0] == 'c' && lk->name[1] == 'o' && lk->name[2] == 'n' &&
  // lk->name[3] == 's')&&
  // !(lk->name[0] == 't' && lk->name[1] == 'i' && lk->name[2] == 'm' &&
  // lk->name[3] == 'e')){
  //   printstring("acquire:");
  //   printstring(lk->name);
  //   printstring("\n");
  // }
  if (holding(lk))
    panic("acquire");
  // printstring("acquire:"); printstring(lk->name); printstring("\n");
  // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while (__sync_lock_test_and_set(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  // Record info about lock acquisition for holding() and debugging.
  lk->cpu = mycpu();
}

// Release the lock.
void release(struct spinlock *lk) {
  if (!holding(lk))
    panic("release");

  lk->cpu = 0;

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked);

  // if(!(lk->name[0] == 'k' && lk->name[1] == 'm' && lk->name[2] == 'e' &&
  // lk->name[3] == 'm')
  // && !(lk->name[0] == 'p' && lk->name[1] == 'r' && lk->name[2] == 'o' &&
  // lk->name[3] == 'c')
  // && !(lk->name[0] == 'c' && lk->name[1] == 'o' && lk->name[2] == 'n' &&
  // lk->name[3] == 's')&&
  // !(lk->name[0] == 't' && lk->name[1] == 'i' && lk->name[2] == 'm' &&
  // lk->name[3] == 'e')){
  //   printstring("release:");
  //   printstring(lk->name);
  //   printstring("\n");
  // }
  pop_off();
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int holding(struct spinlock *lk) {
  int r;
  r = (lk->locked && lk->cpu == mycpu());
  return r;
}
