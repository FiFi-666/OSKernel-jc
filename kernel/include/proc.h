#ifndef __PROC_H
#define __PROC_H

#include "param.h"
#include "riscv.h"
#include "types.h"
#include "spinlock.h"
#include "file.h"
#include "fat32.h"
#include "thread.h"
#include "trap.h"
#include "vma.h"
#include "signal.h"
#include "context.h"

#define CLONE_VM	0x00000100
#define CLONE_FS	0x00000200
#define CLONE_FILES	0x00000400
#define CLONE_SIGHAND	0x00000800
#define CLONE_PIDFD	0x00001000
#define CLONE_PTRACE	0x00002000
#define CLONE_VFORK	0x00004000
#define CLONE_PARENT	0x00008000
#define CLONE_THREAD	0x00010000
#define CLONE_NEWNS	0x00020000
#define CLONE_SYSVSEM	0x00040000
#define CLONE_SETTLS	0x00080000
#define CLONE_PARENT_SETTID	0x00100000
#define CLONE_CHILD_CLEARTID	0x00200000
#define CLONE_DETACHED	0x00400000
#define CLONE_UNTRACED	0x00800000
#define CLONE_CHILD_SETTID	0x01000000
#define CLONE_NEWCGROUP	0x02000000
#define CLONE_NEWUTS	0x04000000
#define CLONE_NEWIPC	0x08000000
#define CLONE_NEWUSER	0x10000000
#define CLONE_NEWPID	0x20000000
#define CLONE_NEWNET	0x40000000
#define CLONE_IO	0x80000000

// Saved registers for kernel context switches.

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context context;     // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  struct proc *parent;         // Parent process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID
  int uid;                     // Process User ID
  int gid;                     // Process Group ID
  int pgid;

  uint64 filelimit;

  // these are private to the process, so p->lock need not be held.
  thread *main_thread;         // Main thread per process
  thread *thread_queue;        // thread_queue
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  pagetable_t kpagetable;      // Kernel page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  int *exec_close;             // Open files
  struct dirent *cwd;          // Current directory
  char name[16];               // Process name (debugging)
  int tmask;                    // trace mask
  struct vma *vma;
  int ktime;
  int utime;
  int thread_num;
  int char_count;   // not used
  uint64 clear_child_tid;
  //signal
  sigaction sigaction[SIGRTMAX + 1]; // signal action
  __sigset_t sig_set; // signal mask
  __sigset_t sig_pending; // pending signal
  struct trapframe *sig_tf; // trapframe for signal

  //kernel thread
  void (*fn)(void *);
  void *arg;
};

typedef struct rlimit {
  uint64 rlim_cur;
  uint64 rlim_max;
}rlimit;

typedef struct thread_stack_param {
  uint64 func_point;
  uint64 arg_point;
}thread_stack_param;

#define NOFILEMAX(p) (p->filelimit<NOFILE?p->filelimit:NOFILE)
#define LOG_PROCESS_NUM 7
#define THREAD_TOTAL_NUMBER (1 << LOG_PROCESS_NUM)
#define PROCESS_OFFSET(processId) ((processId) & (THREAD_TOTAL_NUMBER - 1))


void            cpuinit(void);
void            reg_info(void);
int             cpuid(void);
void            exit(int);
int             fork(void);
int             growproc(int);
pagetable_t     proc_pagetable(struct proc *);
void            proc_freepagetable(pagetable_t, uint64);
int             kill(int, int);
int             tgkill(int, int, int);
struct cpu*     mycpu(void);
struct cpu*     getmycpu(void);
struct proc*    myproc();
void            procinit(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            setproc(struct proc*);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(uint64);
void            wakeup(void*);
void            yield(void);
void            t_yield(void);
int             either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
int             either_copyin(void *dst, int user_src, uint64 src, uint64 len);
void            procdump(void);
uint64          procnum(void);
void            test_proc_init(int);
int             wait4pid(int pid,uint64 addr,int options);
uint64            sys_yield();
uint64          clone(uint64 new_stack, uint64 new_fn);
uint64          thread_clone(uint64 stackVa,uint64 ptid,uint64 tls,uint64 ctid);

struct proc*    threadalloc(void (*fn)(void *), void *arg);
int             get_proc_addr_num(struct proc *p);
#endif