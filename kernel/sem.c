#include "include/sem.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/spinlock.h"
#include "include/timer.h"
#include "include/types.h"

void sem_init(struct semaphore *sem, uint8 value) {
  // debug_print("sem_init %p\n", sem);
  sem->value = value;
  sem->valid = 1;
  initlock(&sem->lock, "semaphore");
}

void sem_wait(struct semaphore *sem) {
  // debug_print("sem_wait %p\n", sem);
  acquire(&sem->lock);
  while (sem->value <= 0) {
    sleep(sem, &sem->lock);
  }
  sem->value--;
  release(&sem->lock);
}

uint32 sem_wait_with_milli_timeout(struct semaphore *sem,
                                   time_t milli_timeout) {
  // debug_print("sem_wait_with_milli_timeout %p\n", sem);
  time_t begin, end;
  begin = get_timeval().tv_usec;
  end = begin + milli_timeout * 1000;
  acquire(&sem->lock);
  while (sem->value <= 0) {
    end = get_timeval().tv_usec;
    if (milli_timeout * 1000 <= end - begin) {
      release(&sem->lock);
      return milli_timeout;
    }
    sleep(sem, &sem->lock);
  }
  sem->value--;
  release(&sem->lock);
  return end - begin;
}

void sem_post(struct semaphore *sem) {
  // debug_print("sem_post %p\n", sem);
  acquire(&sem->lock);
  sem->value++;
  wakeup(sem);
  release(&sem->lock);
}

void sem_destroy(struct semaphore *sem) {
  // debug_print("sem_destroy %p\n", sem);
  acquire(&sem->lock);
  sem->value = 0;
  sem->valid = 0;
  release(&sem->lock);
}

void sem_set_invalid(struct semaphore *sem) {
  // debug_print("sem_set_invalid %p\n", sem);
  acquire(&sem->lock);
  sem->valid = 0;
  release(&sem->lock);
}

int sem_is_valid(struct semaphore *sem) {
  // debug_print("sem_is_valid %p\n", sem);
  int valid;
  acquire(&sem->lock);
  valid = sem->valid;
  release(&sem->lock);
  return valid;
}
