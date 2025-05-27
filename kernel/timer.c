// Timer Interrupt handler

#include "include/timer.h"
#include "include/defs.h"
#include "include/param.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/sbi.h"
#include "include/spinlock.h"
#include "include/syscall.h"
#include "include/types.h"
#include "include/vm.h"

extern struct proc proc[NPROC];

struct spinlock tickslock;
uint ticks;
timer timers[NTIMERS];
int hastimer = 0;

void timerinit() {
  initlock(&tickslock, "time");
  memset(timers, 0, sizeof(timers));
  ticks = 0;
  hastimer = 0;
#ifdef DEBUG
  printf("timerinit\n");
#endif
}

void set_next_timeout() {
  // There is a very strange bug,
  // if comment the `printf` line below
  // the timer will not work.

  // this bug seems to disappear automatically
  // printf("");
  sbi_set_timer(r_time() + INTERVAL);
}

void timer_tick() {
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
  set_next_timeout();

  // printf("ticks:%d\n",ticks);
  if (hastimer) {
    // printf("begin timer\n");
    for (int i = 0; i < NTIMERS; i++) {
      if (timers[i].pid == 0)
        continue;
      if (ticks - timers[i].ticks >= 80) {
        // printf("timer pid %d\n",timers[i].pid);
        kill(timers[i].pid, SIGALRM);
        timers[i].pid = 0;
        hastimer = 0;
      }
    }
  }
}

uint64 sys_times() {
  struct tms ptms;
  uint64 utms;
  argaddr(0, &utms);
  ptms.tms_utime = myproc()->utime;
  ptms.tms_stime = myproc()->ktime;
  ptms.tms_cstime = 1;
  ptms.tms_cutime = 1;
  struct proc *p;
  for (p = proc; p < proc + NPROC; p++) {
    acquire(&p->lock);
    if (p->parent == myproc()) {
      ptms.tms_cutime += p->utime;
      ptms.tms_cstime += p->ktime;
    }
    release(&p->lock);
  }
  copyout(myproc()->pagetable, utms, (char *)&ptms, sizeof(ptms));
  return 0;
}

uint64 setitimer(int which, const struct itimerval *value,
                 struct itimerval *ovalue) {
  int pid = myproc()->pid;
  struct timer *timer = NULL;
  for (int i = 0; i < NTIMERS; i++) {
    if (timers[i].pid == pid && timers[i].which == which) {
      timer = &timers[i];
      break;
    }
  }
  if (ovalue != NULL && timer != NULL) {
    copyout(myproc()->pagetable, (uint64)ovalue, (char *)&((timer->itimer)),
            sizeof(struct itimerval));
  }

  if (value != NULL) {
    if (value->it_value.tv_sec == 0 && value->it_value.tv_usec == 0 &&
        value->it_interval.tv_sec == 0 && value->it_interval.tv_usec == 0) {
      return 0;
    }
    if (timer == NULL) {
      for (int i = 0; i < NTIMERS; i++) {
        if (timers[i].pid == 0) {
          // printf("set timer pid %d\n", pid);
          timer = &timers[i];
          timer->pid = pid;
          timer->which = which;
          timer->ticks = ticks;
          break;
        }
      }
    } else {
      timer->itimer = *value;
      timer->which = which;
      timer->ticks = ticks;
      timer->pid = pid;
    }
    hastimer = 1;
  }
  return 0;
}

struct timeval get_timeval() {
  uint64 time = r_time();
  return (struct timeval){
      .tv_sec = time / (CLK_FREQ),
      .tv_usec = time / (CLK_FREQ / 1000),
  };
}