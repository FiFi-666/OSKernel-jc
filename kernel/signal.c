#include "include/signal.h"
#include "include/defs.h"
#include "include/kalloc.h"
#include "include/memlayout.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/types.h"

int set_sigaction(int signum, sigaction const *act, sigaction *oldact) {

  struct proc *p = myproc();
  if (p->sigaction[signum].__sigaction_handler.sa_handler != NULL &&
      oldact != NULL) {
    *oldact = p->sigaction[signum];
  }
  if (act != NULL) {
    p->sigaction[signum] = *act;
  }
  return 0;
}

int sigprocmask(int how, __sigset_t *set, __sigset_t *oldset) {
  struct proc *p = myproc();

  for (int i = 0; i < SIGSET_LEN; i++) {
    if (oldset != NULL) {
      oldset->__val[i] = p->sig_set.__val[i];
    }
    if (set == NULL)
      continue;
    switch (how) {
    case SIG_BLOCK:
      p->sig_set.__val[i] |= set->__val[i];
      break;
    case SIG_UNBLOCK:
      p->sig_set.__val[i] &= ~(set->__val[i]);
      break;
    case SIG_SETMASK:
      p->sig_set.__val[i] = set->__val[i];
      break;
    default:
      break;
      // panic("invalid how\n");
    }
  }
  // SIGTERM SIGKILL SIGSTOP cannot be masked
  p->sig_set.__val[0] &= 1ul << SIGTERM | 1ul << SIGKILL | 1ul << SIGSTOP;
  return 0;
}

uint64 rt_sigreturn(void) {
  struct proc *p = myproc();
  memcpy(p->trapframe, p->sig_tf, sizeof(struct trapframe));
  kfree(p->sig_tf);
  p->sig_tf = 0;
  return p->trapframe->a0;
}

void sighandle(void) {
  struct proc *p = myproc();
  int signum = p->killed;
  // debug_print("sighandle %p a0:%d a7:%d\n", p->sig_pending.__val[0],
  // p->trapframe->a0, p->trapframe->a7);
  if (p->sigaction[signum].__sigaction_handler.sa_handler != NULL) {
    p->sig_tf = kalloc();
    memcpy(p->sig_tf, p->trapframe, sizeof(struct trapframe));
    p->trapframe->epc =
        (uint64)p->sigaction[signum].__sigaction_handler.sa_handler;
    p->trapframe->ra = (uint64)SIGTRAMPOLINE;
    p->trapframe->sp = p->trapframe->sp - PGSIZE;
    debug_print("sighandle epc:%p ra:%p\n", p->trapframe->epc,
                p->trapframe->ra);
    p->sig_pending.__val[0] &= ~(1ul << signum);
    if (p->sig_pending.__val[0] == 0) {
      p->killed = 0;
    }
  } else {
    exit(-1);
  }
}