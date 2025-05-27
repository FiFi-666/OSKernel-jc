#include "include/printf.h"
#include "include/proc.h"
#include "include/signal.h"
#include "include/syscall.h"
#include "include/types.h"
#include "include/vm.h"
/**
 * syscall rt_sigaction将会调用此函数
 * @param signum 信号编号
 * @param act 信号处理函数结构体
 * @param oldact 旧的信号处理函数结构体
 * @return 成功返回0，失败返回-1
 */

uint64 sys_rt_sigaction(void) {
  int signum;
  sigaction *ptr_act = NULL;    // struct sigaction const *act
  sigaction *ptr_oldact = NULL; // struct sigaction *oldact

  argint(0, &signum);
  if (signum < 0 || signum > SIGRTMAX) {
    return -1;
  }
  argaddr(1, (uint64 *)&ptr_act);
  argaddr(2, (uint64 *)&ptr_oldact);

  // copy struct sigaction from user space
  sigaction act = {0};
  sigaction oldact = {0};

  if (ptr_act) {
    if (copyin(myproc()->pagetable, (char *)&(act), (uint64)ptr_act,
               sizeof(sigaction)) < 0) {
      return -1;
    }
  }

  if (set_sigaction(signum, ptr_act ? &act : NULL,
                    ptr_oldact ? &oldact : NULL) < 0) {
    return -1;
  }

  if (ptr_oldact) {
    if (copyout(myproc()->pagetable, (uint64)ptr_oldact, (char *)&(oldact),
                sizeof(sigaction)) < 0) {
      return -1;
    }
  }
  debug_print("sys_rt_sigaction: signum = %d, act fp:%p \n", signum,
              act.__sigaction_handler.sa_handler);
  return 0;
}

/**
 * syscall rt_sigprocmask将会调用此函数
 * @param how 信号屏蔽字的操作
 * @param set 信号屏蔽字
 * @param oldset 旧的信号屏蔽字
 * @return 成功返回0，失败返回-1
 */
uint64 sys_rt_sigprocmask(void) {
  int how;
  uint64 uptr_set, uptr_oldset;

  __sigset_t set, oldset;

  argint(0, &how);
  argaddr(1, &uptr_set);
  argaddr(2, &uptr_oldset);

  if (uptr_set &&
      copyin(myproc()->pagetable, (char *)&set, uptr_set, SIGSET_LEN * 8) < 0) {
    return -1;
  }

  if (sigprocmask(how, &set, uptr_oldset ? &oldset : NULL)) {
    return -1;
  }

  if (uptr_oldset && copyout(myproc()->pagetable, uptr_oldset, (char *)&oldset,
                             SIGSET_LEN * 8) < 0) {
    return -1;
  }
  debug_print("sys_rt_sigprocmask: how = %d, set = %p\n", how, set.__val[0]);
  return 0;
}

/**
 * syscall rt_sigreturn将会调用此函数
 * @param regs 保存的寄存器信息
 * @return 成功返回0，失败返回-1
 */
uint64 sys_rt_sigreturn(void) { return rt_sigreturn(); }

/**
 * @param pid
 * @param tid
 * @param sig
 * @return tgkill返回值
 */
uint64 sys_tgkill(void) {
  int sig;
  int tid;
  int pid;
  argint(0, &pid);
  argint(1, &tid);
  argint(2, &sig);
  return tgkill(tid, pid, sig);
}

uint64 sys_rt_sigtimedwait() { return 0; }