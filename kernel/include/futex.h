//ref:https://man7.org/linux/man-pages/man2/futex.2.html
#ifndef __FUTEX_H
#define __FUTEX_H

#include "timer.h"
#include "thread.h"

#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_FD		2
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_WAIT_REQUEUE_PI	11
#define FUTEX_CMP_REQUEUE_PI	12
#define FUTEX_LOCK_PI2		13

#define FUTEX_PRIVATE_FLAG 128
#define FUTEX_COUNT 1024

void futexWait(uint64 addr, thread* th, TimeSpec2* ts);
void futexWake(uint64 addr, int n);
void futexRequeue(uint64 addr, int n, uint64 newAddr);
void futexClear(thread* thread);

#endif