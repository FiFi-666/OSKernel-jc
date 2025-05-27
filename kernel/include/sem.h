#ifndef SEM_H
#define SEM_H

#include "types.h"
#include "spinlock.h"

struct semaphore {
    int value;
    int valid;
    struct spinlock lock;
};

void sem_init(struct semaphore *sem, uint8 value);

void sem_wait(struct semaphore *sem);

uint32 sem_wait_with_milli_timeout(struct semaphore *sem, time_t milli_timeout);

void sem_post(struct semaphore *sem);

void sem_destroy(struct semaphore *sem);

int sem_is_valid(struct semaphore *sem);

void sem_set_invalid(struct semaphore *sem);

#endif