#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include "../../../include/types.h"
#include "../../../include/spinlock.h"
#include "../../../include/proc.h"
#include "../../../include/sem.h"
#include "../../../include/intr.h"
#include "../../../include/printf.h"

#include "cc.h"

#define LWIP_COMPAT_MUTEX 1

#define SYS_MBOX_NULL NULL
#define SYS_SEM_NULL NULL

#define SYS_ARCH_TIMEOUT 0xffffffffUL

typedef struct proc* sys_thread_t;

typedef uint64 sys_prot_t;

// #define SYS_MBOX_NULL NULL
typedef struct sys_mbox {
#define MBOXSLOTS 32
  struct spinlock s;
  struct semaphore sem;
  volatile int invalid;
  volatile int head;
  volatile int tail;
  void *msg[MBOXSLOTS];
} sys_mbox_t;

typedef struct semaphore sys_sem_t;

#define SYS_ARCH_NOWAIT  0xfffffffe

#endif