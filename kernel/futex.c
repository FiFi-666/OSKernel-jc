#include "include/futex.h"
#include "include/proc.h"
#include "include/thread.h"
#include "include/timer.h"
#include "include/trap.h"
#include "include/types.h"

typedef struct FutexQueue {
  uint64 addr;
  thread *thread;
  uint8 valid;
} FutexQueue;

FutexQueue futexQueue[FUTEX_COUNT];

void futexWait(uint64 addr, thread *th, TimeSpec2 *ts) {
  for (int i = 0; i < FUTEX_COUNT; i++) {
    if (!futexQueue[i].valid) {
      futexQueue[i].valid = 1;
      futexQueue[i].addr = addr;
      futexQueue[i].thread = th;
      if (ts) {
        th->awakeTime = ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
        th->state = t_TIMING;
      } else {
        th->state = t_SLEEPING;
      }
      acquire(&th->p->lock);
      th->p->state = RUNNABLE;
      sched();
      release(&th->p->lock);
    }
  }
  panic("No futex Resource!\n");
}

void futexWake(uint64 addr, int n) {
  for (int i = 0; i < FUTEX_COUNT && n; i++) {
    if (futexQueue[i].valid && futexQueue[i].addr == addr) {
      futexQueue[i].thread->state = t_RUNNABLE;
      futexQueue[i].thread->trapframe->a0 = 0;
      futexQueue[i].valid = 0;
      n--;
    }
  }
}

void futexRequeue(uint64 addr, int n, uint64 newAddr) {
  for (int i = 0; i < FUTEX_COUNT && n; i++) {
    if (futexQueue[i].valid && futexQueue[i].addr == addr) {
      futexQueue[i].thread->state = t_RUNNABLE;
      futexQueue[i].thread->trapframe->a0 = 0;
      futexQueue[i].valid = 0;
      n--;
    }
  }
  for (int i = 0; i < FUTEX_COUNT; i++) {
    if (futexQueue[i].valid && futexQueue[i].addr == addr) {
      futexQueue[i].addr = newAddr;
    }
  }
}

void futexClear(thread *thread) {
  for (int i = 0; i < FUTEX_COUNT; i++) {
    if (futexQueue[i].valid && futexQueue[i].thread == thread) {
      futexQueue[i].valid = 0;
    }
  }
}