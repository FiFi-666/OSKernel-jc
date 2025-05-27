
#include "include/proc.h"
#include "include/fat32.h"
#include "include/file.h"
#include "include/futex.h"
#include "include/intr.h"
#include "include/kalloc.h"
#include "include/memlayout.h"
#include "include/param.h"
#include "include/printf.h"
#include "include/riscv.h"
#include "include/spinlock.h"
#include "include/string.h"
#include "include/trap.h"
#include "include/types.h"
#include "include/vm.h"
#include "include/vma.h"
extern uchar initcode[];
extern int initcodesize;
extern thread *free_thread;
struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
extern int magic_count;
extern void swtch(struct context *, struct context *);
static void wakeup1(struct proc *chan);
static void freeproc(struct proc *p);

extern thread threads[];
extern char trampoline[];       // trampoline.S
extern char signalTrampoline[]; // signalTrampoline.S

void reg_info(void) {
  printf("register info: {\n");
  printf("sstatus: %p\n", r_sstatus());
  printf("sip: %p\n", r_sip());
  printf("sie: %p\n", r_sie());
  printf("sepc: %p\n", r_sepc());
  printf("stvec: %p\n", r_stvec());
  printf("satp: %p\n", r_satp());
  printf("scause: %p\n", r_scause());
  printf("stval: %p\n", r_stval());
  printf("sp: %p\n", r_sp());
  printf("tp: %p\n", r_tp());
  printf("ra: %p\n", r_ra());
  printf("}\n");
}

void cpuinit(void) {
  struct cpu *it;
  for (it = cpus; it < &cpus[NCPU]; it++) {
    it->proc = 0;
    it->intena = 0;
    it->noff = 0;
  }
}

// initialize the proc table at boot time.
void procinit(void) {
  struct proc *p;
  magic_count = 0;
  initlock(&pid_lock, "nextpid");
  for (p = proc; p < &proc[NPROC]; p++) {
    initlock(&p->lock, "proc");
    p->state = UNUSED;
    p->parent = 0;
    p->main_thread = 0;
    p->chan = 0;
    p->killed = 0;
    p->xstate = 0;
    p->pid = 0;
    p->kstack = 0;
    p->sz = 0;
    p->pagetable = 0;
    p->kpagetable = 0;
    p->trapframe = 0;
    for (int i = 0; i < NOFILE; i++)
      p->ofile[i] = 0;
    p->cwd = 0;
    p->name[0] = 0;
    p->vma = 0;
    p->tmask = 0;
    p->ktime = 0;
    p->utime = 0;
    memset(p->sigaction, 0, sizeof(p->sigaction));
    memset(p->sig_set.__val, 0, sizeof(p->sig_set));
    memset(p->sig_pending.__val, 0, sizeof(p->sig_pending));
    p->sig_tf = NULL;
    // Allocate a page for the process's kernel stack.
    // Map it high in memory, followed by an invalid
    // guard page.
    // char *pa = kalloc();
    // // printf("[procinit]kernel stack: %p\n", (uint64)pa);
    // if(pa == 0)
    //   panic("kalloc");
    // uint64 va = KSTACK((int) (p - proc));
    // // printf("[procinit]kvmmap va %p to pa %p\n", va, (uint64)pa);
    // kvmmap(va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
    // p->kstack = va;
  }
  // kvminithart();

  memset(cpus, 0, sizeof(cpus));
#ifdef DEBUG
  printf("procinit\n");
#endif
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid() {
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu *mycpu(void) {
  int id = cpuid();
  struct cpu *c = &cpus[id];

  return c;
}

// Return the current struct proc *, or zero if none.
struct proc *myproc(void) {
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int allocpid() {
  int pid;

  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

static void copycontext(context *t1, context *t2) {
  t1->ra = t2->ra;
  t1->sp = t2->sp;
  t1->s0 = t2->s0;
  t1->s1 = t2->s1;
  t1->s2 = t2->s2;
  t1->s3 = t2->s3;
  t1->s4 = t2->s4;
  t1->s5 = t2->s5;
  t1->s6 = t2->s6;
  t1->s7 = t2->s7;
  t1->s8 = t2->s8;
  t1->s9 = t2->s9;
  t1->s10 = t2->s10;
  t1->s11 = t2->s11;
}

// copy trapframe f2 to trapframe f1
static void copytrapframe(struct trapframe *f1, struct trapframe *f2) {
  f1->kernel_satp = f2->kernel_satp;
  f1->kernel_sp = f2->kernel_sp;
  f1->kernel_trap = f2->kernel_trap;
  f1->epc = f2->epc;
  f1->kernel_hartid = f2->kernel_hartid;
  f1->ra = f2->ra;
  f1->sp = f2->sp;
  f1->gp = f2->gp;
  f1->tp = f2->tp;
  f1->t0 = f2->t0;
  f1->t1 = f2->t1;
  f1->t2 = f2->t2;
  f1->s0 = f2->s0;
  f1->s1 = f2->s1;
  f1->a0 = f2->a0;
  f1->a1 = f2->a1;
  f1->a2 = f2->a2;
  f1->a3 = f2->a3;
  f1->a4 = f2->a4;
  f1->a5 = f2->a5;
  f1->a6 = f2->a6;
  f1->a7 = f2->a7;
  f1->s2 = f2->s2;
  f1->s3 = f2->s3;
  f1->s4 = f2->s4;
  f1->s5 = f2->s5;
  f1->s6 = f2->s6;
  f1->s7 = f2->s7;
  f1->s8 = f2->s8;
  f1->s9 = f2->s9;
  f1->s10 = f2->s10;
  f1->s11 = f2->s11;
  f1->t3 = f2->t3;
  f1->t4 = f2->t4;
  f1->t5 = f2->t5;
  f1->t6 = f2->t6;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc *allocproc(void) {
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return NULL;

found:
  p->pid = allocpid();
  freemem_amount();
  // printf("alloc proc:%d freemem_mount:%p\n", p->pid, freemem_amount());
  p->vma = NULL;
  p->filelimit = NOFILE;
  p->ktime = 1;
  p->utime = 1;
  p->uid = 0;
  p->gid = 0;
  p->pgid = 0;
  p->thread_num = 0;
  p->char_count = 0;
  p->clear_child_tid = NULL;
  memset(p->sigaction, 0, sizeof(p->sigaction));
  memset(p->sig_set.__val, 0, sizeof(p->sig_set));
  memset(p->sig_pending.__val, 0, sizeof(p->sig_pending));
  // Allocate a trapframe page.
  if ((p->trapframe = (struct trapframe *)kalloc()) == NULL) {
    release(&p->lock);
    return NULL;
  }

  // An empty user page table.
  // And an identical kernel page table for this proc.
  if ((p->pagetable = proc_pagetable(p)) == NULL ||
      (p->kpagetable = proc_kpagetable(p)) == NULL) {
    freeproc(p);
    release(&p->lock);
    return NULL;
  }
  p->kstack = PROCVKSTACK(get_proc_addr_num(p));

  p->exec_close = kalloc();
  for (int fd = 0; fd < NOFILE; fd++)
    p->exec_close[fd] = 0;

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + KSTACKSIZE;
  p->main_thread = allocNewThread();
  copycontext(&p->main_thread->context, &p->context);
  p->thread_num++;
  p->main_thread->p = p;
  p->main_thread->sz = p->sz;
  p->main_thread->clear_child_tid = p->clear_child_tid;
  p->main_thread->kstack = p->kstack;
  p->thread_queue = p->main_thread;
  if (mappages(p->kpagetable, p->kstack - PGSIZE, PGSIZE,
               (uint64)(p->main_thread->trapframe), PTE_R | PTE_W) < 0)
    panic("allocproc: map thread trapframe failed");
  p->main_thread->vtf = p->kstack - PGSIZE;
  return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void freeproc(struct proc *p) {
  if (p->trapframe)
    kfree((void *)p->trapframe);

  kfree((void *)p->exec_close);
  p->trapframe = 0;

  thread *t = p->thread_queue;
  while (NULL != t) {
    thread *tmp = t->next_thread;
    if (NULL != t->next_thread)
      t->next_thread->pre_thread = NULL;
    t->state = t_UNUSED;
    t->next_thread = free_thread;
    if (NULL != free_thread)
      free_thread->pre_thread = t;
    free_thread = t;
    kfree((void *)t->trapframe);
    if (t->kstack != p->kstack) {
      vmunmap(p->kpagetable, t->kstack, 1, 0);
      kfree((void *)t->kstack_pa);
    }
    t = tmp;
  }

  if (p->kpagetable) {
    kvmfree(p->kpagetable, 1, p);
  }
  p->kpagetable = 0;
  if (p->pagetable) {
    free_vma_list(p);
    proc_freepagetable(p->pagetable, p->sz);
  }
  // TODO: free threads
  freemem_amount();
  // printf("free proc : %d freemem_mount:%p\n",p->pid, freemem_amount());
  p->pagetable = 0;
  p->vma = NULL;
  p->sz = 0;
  p->pid = 0;
  p->main_thread->state = t_UNUSED;
  p->main_thread = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

// Create a user page table for a given process,
// with no user memory, but with trampoline pages.
pagetable_t proc_pagetable(struct proc *p) {
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if (pagetable == 0)
    return NULL;

  if (NULL == vma_init(p)) {
    uvmfree(pagetable, 0);
    return NULL;
  }
  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if (mappages(pagetable, TRAMPOLINE, PGSIZE, (uint64)trampoline,
               PTE_R | PTE_X) < 0) {
    uvmfree(pagetable, 0);
    return NULL;
  }

  // map the trapframe just below TRAMPOLINE, for trampoline.S.
  if (mappages(pagetable, TRAPFRAME, PGSIZE, (uint64)(p->trapframe),
               PTE_R | PTE_W) < 0) {
    vmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return NULL;
  }
  if (mappages(pagetable, SIGTRAMPOLINE, PGSIZE, (uint64)signalTrampoline,
               PTE_R | PTE_X | PTE_U) < 0) {
    vmunmap(pagetable, TRAMPOLINE, 1, 0);
    vmunmap(pagetable, TRAPFRAME, 1, 0);
    uvmfree(pagetable, 0);
    return NULL;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void proc_freepagetable(pagetable_t pagetable, uint64 sz) {
  vmunmap(pagetable, TRAMPOLINE, 1, 0);
  vmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// Set up first user process.
void userinit(void) {
  struct proc *p;

  p = allocproc();
  initproc = p;

  // allocate one user page and copy init's instructions
  // and data into it.
  uvminit(p->pagetable, p->kpagetable, initcode, initcodesize);
  p->sz = initcodesize;
  p->sz = PGROUNDUP(p->sz);

  // uvmclear(p->pagetable, p->sz - stacksize);
  alloc_vma_stack(p);
  p->trapframe->sp = get_proc_sp(p); // user stack pointer

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0x0; // user program counter

  safestrcpy(p->name, "initcode", sizeof(p->name));

  p->state = RUNNABLE;

  p->tmask = 0;

  copytrapframe(p->main_thread->trapframe, p->trapframe);
  p->main_thread->state = t_RUNNABLE;

  release(&p->lock);
#ifdef DEBUG
  printf("userinit\n");
#endif
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n) {
  uint sz;
  struct proc *p = myproc();

  sz = p->sz;
  if (n > 0) {
    if ((sz = uvmalloc(p->pagetable, p->kpagetable, sz, sz + n,
                       PTE_R | PTE_W)) == 0) {
      return -1;
    }
  } else if (n < 0) {
    sz = uvmdealloc(p->pagetable, p->kpagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void) {
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if ((np = allocproc()) == NULL) {
    return -1;
  }

  // Copy user memory from parent to child.
  if (uvmcopy(p->pagetable, np->pagetable, np->kpagetable, p->sz) < 0) {
    freeproc(np);
    release(&np->lock);
    return -1;
  }

  struct vma *nvma = vma_copy(np, p->vma);
  if (NULL != nvma) {
    nvma = nvma->next;
    while (nvma != np->vma) {
      if (vma_map(p->pagetable, np->pagetable, nvma) < 0) {
        printf("clone: vma deep mapping failed\n");
        return -1;
      }
      nvma = nvma->next;
    }
  }
  /*
  if (NULL == (nvma = vma_copy(p,np->vma))) {
    printf("clone failed\n");
    return -1;
  }
  np ->vma = nvma;
  nvma = nvma->next;
  while (nvma != p->vma) {
    if (vma_map(p->pagetable,np->pagetable,nvma) < 0) {
      printf("clone: vma deep mapping failed\n");
      return -1;
    }
    nvma = nvma->next;
  }
  */

  np->sz = p->sz;

  np->parent = p;

  // copy tracing mask from parent.
  np->tmask = p->tmask;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;
  copytrapframe(np->main_thread->trapframe, np->trapframe);
  // increment reference counts on open file descriptors.
  for (i = 0; i < NOFILE; i++)
    if (p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = edup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  np->state = RUNNABLE;
  np->main_thread->state = t_RUNNABLE;

  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold p->lock.
void reparent(struct proc *p) {
  struct proc *pp;

  for (pp = proc; pp < &proc[NPROC]; pp++) {
    // this code uses pp->parent without holding pp->lock.
    // acquiring the lock first could cause a deadlock
    // if pp or a child of pp were also in exit()
    // and about to try to lock p.
    if (pp->parent == p) {
      // pp->parent can't change between the check and the acquire()
      // because only the parent changes it, and we're the parent.
      acquire(&pp->lock);
      pp->parent = initproc;
      // we should wake up init here, but that would require
      // initproc->lock, which would be a deadlock, since we hold
      // the lock on one of init's children (pp). this is why
      // exit() always wakes init (before acquiring any locks).
      release(&pp->lock);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void exit(int status) {
  struct proc *p = myproc();
  // if(p->pid == 3){ printf("pid 3 exit status %d ra:%p\n", r_ra());}

  if (p == initproc)
    panic("init exiting");

  // Close all open files.
  for (int fd = 0; fd < NOFILE; fd++) {
    if (p->ofile[fd]) {
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  eput(p->cwd);
  p->cwd = 0;
  checkup1(p);
  // we might re-parent a child to init. we can't be precise about
  // waking up init, since we can't acquire its lock once we've
  // acquired any other proc lock. so wake up init whether that's
  // necessary or not. init may miss this wakeup, but that seems
  // harmless.
  acquire(&initproc->lock);
  wakeup1(initproc);
  release(&initproc->lock);

  // grab a copy of p->parent, to ensure that we unlock the same
  // parent we locked. in case our parent gives us away to init while
  // we're waiting for the parent lock. we may then race with an
  // exiting parent, but the result will be a harmless spurious wakeup
  // to a dead or wrong process; proc structs are never re-allocated
  // as anything else.
  acquire(&p->lock);
  struct proc *original_parent = p->parent;
  release(&p->lock);

  // we need the parent's lock in order to wake it up from wait().
  // the parent-then-child rule says we have to lock it first.
  acquire(&original_parent->lock);

  acquire(&p->lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup1(original_parent);
  // Parent might be sleeping in wait4pid().
  if (original_parent->chan == p && original_parent->state == SLEEPING) {
    original_parent->state = RUNNABLE;
  }

  p->xstate = status;
  p->state = ZOMBIE;
  p->main_thread->state = t_ZOMBIE;

  release(&original_parent->lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr) {
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();

  // hold p->lock for the whole time to avoid lost
  // wakeups from a child's exit().
  acquire(&p->lock);

  for (;;) {
    // Scan through table looking for exited children.
    havekids = 0;
    for (np = proc; np < &proc[NPROC]; np++) {
      // this code uses np->parent without holding np->lock.
      // acquiring the lock first would cause a deadlock,
      // since np might be an ancestor, and we already hold p->lock.
      if (np->parent == p) {
        // np->parent can't change between the check and the acquire()
        // because only the parent changes it, and we're the parent.
        acquire(&np->lock);
        havekids = 1;
        if (np->state == ZOMBIE) {
          // Found one.
          pid = np->pid;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                   sizeof(np->xstate)) < 0) {
            release(&np->lock);
            release(&p->lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&p->lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || p->killed) {
      release(&p->lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &p->lock); // DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void) {
  struct proc *p;
  struct cpu *c = mycpu();
  extern pagetable_t kernel_pagetable;

  c->proc = 0;
  for (;;) {
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    int found = 0;
    for (p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if (p->state == RUNNABLE) {
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        // printf("[scheduler]found runnable proc with pid: %d\n", p->pid);

        // TODO: 改进线程枚举算法
        /*
        int i = 0;
        for (i = 0; i < THREAD_NUM; i++) {
          if (threads[i].state == t_UNUSED)
            break;
          if (threads[i].p == p && (threads[i].state == t_RUNNABLE ||
        (threads[i].state == t_TIMING && threads[i].awakeTime < r_time() + (1LL
        << 35)))) break;
        }
        if (threads[i].state == t_UNUSED)  //
        剩下线程池里的线程都是没有分配的，说明这个进程的线程都不能跑 continue;
        // 让threads[i]成为p的主线程
        p->main_thread = &threads[i];
        */
        thread *t = p->thread_queue;
        while (NULL != t) {
          if (t->state == t_RUNNABLE ||
              (t->state == t_TIMING && t->awakeTime < r_time() + (1LL << 35)))
            break;
          t = t->next_thread;
        }
        if (NULL == t)
          continue;
        if (p->thread_queue != t) { // 放到队首，避免死线程集中在首部
          if (NULL != t->next_thread) {
            t->next_thread->pre_thread = t->pre_thread;
          }
          if (NULL != t->pre_thread) {
            t->pre_thread->next_thread = t->next_thread;
          }
          t->pre_thread = NULL;
          t->next_thread = p->thread_queue;
          p->thread_queue->pre_thread = t;
          p->thread_queue = t;
        }
        p->main_thread = t;
        copycontext(&p->context, &p->main_thread->context);
        copytrapframe(p->trapframe, p->main_thread->trapframe);
        // debug_print("run proc %d ra %p sp %p kpt %p\n", p->pid,
        // p->context.ra, p->context.sp, p->kpagetable);
        p->main_thread->state = t_RUNNING;
        p->main_thread->awakeTime = 0;
        p->state = RUNNING;
        futexClear(p->main_thread);
        c->proc = p;
        w_satp(MAKE_SATP(p->kpagetable));
        sfence_vma();
        swtch(&c->context, &p->context);
        copycontext(&p->main_thread->context, &p->context);
        w_satp(MAKE_SATP(kernel_pagetable));
        sfence_vma();
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;

        found = 1;
      }
      release(&p->lock);
    }
    if (found == 0) {
      intr_on();
      asm volatile("wfi");
    }
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void) {
  int intena;
  struct proc *p = myproc();
  // debug_print("sched p->pid %d\n", p->pid);
  if (!holding(&p->lock))
    panic("sched p->lock");
  if (mycpu()->noff != 1) {
    debug_print("noff:%d\n", mycpu()->noff);
    panic("sched locks");
  }
  if (p->state == RUNNING || p->main_thread->state == t_RUNNING)
    panic("sched running");
  if (intr_get())
    panic("sched interruptible");

  copytrapframe(p->main_thread->trapframe, p->trapframe);
  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void) {
  struct proc *p = myproc();
  acquire(&p->lock);
  // printf("pid %d yield\n, epc: %p", p->pid, p->trapframe->epc);
  p->state = RUNNABLE;
  p->main_thread->state = t_RUNNABLE;
  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void) {

  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  if (first) {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    // printf("[forkret]first scheduling\n");
    first = 0;
    fat32_init();
    myproc()->cwd = ename("/");
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk) {
  struct proc *p = myproc();

  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.
  if (lk != &p->lock) { // DOC: sleeplock0
    acquire(&p->lock);  // DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;
  p->main_thread->state = t_RUNNABLE;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if (lk != &p->lock) {
    release(&p->lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void wakeup(void *chan) {
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->state == SLEEPING && p->chan == chan) {
      p->state = RUNNABLE;
    }
    release(&p->lock);
  }
}

// Wake up p if it is sleeping in wait(); used by exit().
// Caller must hold p->lock.
static void wakeup1(struct proc *p) {
  if (!holding(&p->lock))
    panic("wakeup1");
  if (p->chan == p && p->state == SLEEPING) {
    p->state = RUNNABLE;
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int kill(int pid, int sig) {
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->pid == pid) {
      p->sig_pending.__val[0] |= (1 << (sig));
      // printf("kill peding:%p, killed:%d\n", p->sig_pending.__val[0],
      // p->killed);
      if (p->killed == 0 || p->killed > sig) {
        p->killed = sig;
      }
      if (p->state == SLEEPING) {
        // Wake process from sleep().
        p->state = RUNNABLE;
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

// static int cmp_parent(int pid,int sid){
//   struct proc* p;
//   for(p = proc;p < &proc[NPROC];p++){
//     if(p->pid == sid) break;
//   }
//   while(p){
//     p = p->parent;
//     if(!p)break;
//     if(p->pid == pid) return 1;
//   }
//   return 0;
// }

int tgkill(int tid, int pid, int sig) {
  // if(!cmp_parent(pid,tid)) {printf("pid:%d, tid:%d\n");return -1;}
  // else return kill(tid,sig);
  printf("tgkill:%d %d %d\n", tid, pid, sig);
  return kill(tid, sig);
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len) {
  struct proc *p = myproc();
  if (user_dst) {
    return copyout(p->pagetable, dst, src, len);
    // return copyout2(dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(void *dst, int user_src, uint64 src, uint64 len) {
  struct proc *p = myproc();
  if (user_src) {
    return copyin(p->pagetable, dst, src, len);
    // return copyin2(dst, src, len);
  } else {
    memmove(dst, (char *)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void) {
  static char *states[] = {[UNUSED] "unused",
                           [SLEEPING] "sleep ",
                           [RUNNABLE] "runble",
                           [RUNNING] "run   ",
                           [ZOMBIE] "zombie"};
  struct proc *p;
  char *state;

  printf("\nPID\tSTATE\tNAME\tMEM\n");
  for (p = proc; p < &proc[NPROC]; p++) {
    if (p->state == UNUSED)
      continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d\t%s\t%s\t%d", p->pid, state, p->name, p->sz);
    printf("\n");
  }
}

uint64 procnum(void) {
  int num = 0;
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++) {
    if (p->state != UNUSED) {
      num++;
    }
  }

  return num;
}

struct proc *findchild(struct proc *p, int pid, struct proc **child) {
  *child = NULL;
  // iterator all process
  for (struct proc *np = proc; np < &proc[NPROC]; np++) {
    if ((pid == -1 || np->pid == pid) && np->parent == p) {
      acquire(&np->lock);
      *child = np;
      // printf("state : %d\n", np->state);
      if (np->state == ZOMBIE) {
        return np;
      }
      release(&np->lock);
    }
  }
  return NULL;
}

int wait4pid(int pid, uint64 addr, int options) {
  struct proc *p = myproc(), *child = NULL, *tchild = NULL;
  int kidpid;
  acquire(&p->lock);
  while (1) {
    kidpid = pid;
    // printf("arrive a\n");
    child = findchild(p, pid, &tchild);
    // printf("arrive b\n");
    if (NULL != child) {
      kidpid = child->pid;
      child->xstate <<= 8;
      if (addr != 0 && copyout(p->pagetable, addr, (char *)&child->xstate,
                               sizeof(child->xstate)) < 0) {
        release(&child->lock);
        release(&p->lock);
        return -1;
      }
      // printf("arrive freeproc, child pid : %d\n", child->pid);
      freeproc(child);
      release(&child->lock);
      release(&p->lock);

      return kidpid;
    }
    /*
    if (options & WNOHANG) {
      release(&p->lock);
      return 0;
    }
    */
    if (!tchild) {
      release(&p->lock);
      return -1;
    }
    if (pid == -1) {
      sleep(p, &p->lock);
    } else {
      // printf("arrive here: tchild: %d\n", tchild->pid);
      sleep(tchild, &p->lock);
      // pay attention!
    }
  }
  release(&p->lock);
  return 0;

  // TODO: deal with options
}

uint64 sys_yield() {
  yield();
  return 0;
}

static void copycontext_from_trapframe(context *t, struct trapframe *f) {
  t->ra = f->ra;
  t->sp = f->kernel_sp;
  t->s0 = f->s0;
  t->s1 = f->s1;
  t->s2 = f->s2;
  t->s3 = f->s3;
  t->s4 = f->s4;
  t->s5 = f->s5;
  t->s6 = f->s6;
  t->s7 = f->s7;
  t->s8 = f->s8;
  t->s9 = f->s9;
  t->s10 = f->s10;
  t->s11 = f->s11;
}

uint64 thread_clone(uint64 stackVa, uint64 ptid, uint64 tls, uint64 ctid) {
  struct proc *p = myproc();
  thread *t = allocNewThread();
  t->p = p;
  if (mappages(p->kpagetable, p->kstack - PGSIZE * p->thread_num * 2, PGSIZE,
               (uint64)(t->trapframe), PTE_R | PTE_W) < 0)
    panic("thread_clone: mappages");
  t->vtf = p->kstack - PGSIZE * p->thread_num * 2;
  void *kstack_pa = kalloc();
  if (NULL == kstack_pa)
    panic("thread_clone: kalloc kstack failed");
  if (mappages(p->kpagetable, p->kstack - PGSIZE * (1 + p->thread_num * 2),
               PGSIZE, (uint64)kstack_pa, PTE_R | PTE_W) < 0)
    panic("thread_clone: mappages");
  thread_stack_param tmp;

  if (copyin(p->pagetable, (char *)(&tmp), stackVa,
             sizeof(thread_stack_param)) < 0) {
    panic("copy in thread_stack_param failed");
  }
  t->kstack_pa = (uint64)kstack_pa;
  t->kstack = p->kstack - PGSIZE * (1 + p->thread_num * 2);
  // printf("thread stack param:%p %p\n",tmp.func_point,tmp.arg_point);

  t->next_thread = p->thread_queue;
  if (NULL != p->thread_queue)
    p->thread_queue->pre_thread = t;
  p->thread_queue = t;

  copytrapframe(t->trapframe, p->trapframe);
  t->trapframe->a0 = tmp.func_point;
  t->trapframe->tp = tls;
  t->trapframe->kernel_sp =
      p->kstack - PGSIZE * (1 + p->thread_num * 2) + PGSIZE;
  t->trapframe->sp = stackVa;
  t->trapframe->epc = tmp.func_point;
  copycontext_from_trapframe(&t->context, t->trapframe);
  t->context.ra = (uint64)forkret;
  t->context.sp = t->trapframe->kernel_sp;
  if (ptid != 0) {
    if (either_copyout(1, ptid, (void *)&t->tid, sizeof(int)) < 0)
      panic("thread_clone: either_copyout");
  }
  t->clear_child_tid = ctid;
  p->thread_num++;

  return t->tid;
}

uint64 clone(uint64 new_stack, uint64 new_fn) {
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if ((np = allocproc()) == NULL) {
    return -1;
  }

  // Copy user memory from parent to child.
  if (uvmcopy(p->pagetable, np->pagetable, np->kpagetable, p->sz) < 0) {
    freeproc(np);
    release(&np->lock);
    return -1;
  }

  struct vma *nvma = vma_copy(np, p->vma);
  if (NULL != nvma) {
    nvma = nvma->next;
    while (nvma != np->vma) {
      if (vma_map(p->pagetable, np->pagetable, nvma) < 0) {
        printf("clone: vma deep mapping failed\n");
        return -1;
      }
      nvma = nvma->next;
    }
  }

  np->sz = p->sz;

  np->parent = p;

  // copy tracing mask from parent.
  np->tmask = p->tmask;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for (i = 0; i < NOFILE; i++)
    if (p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = edup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  np->state = RUNNABLE;

  np->trapframe->epc = new_fn;
  np->trapframe->sp = new_stack;

  release(&np->lock);

  return pid;
}

void threadhelper(uint64 sp) {
  release(&myproc()->lock);
  printf("threadhelper sp %p\n", sp);
  void (*fn)(void *) = (void (*)(void *))myproc()->fn;
  void *arg = myproc()->arg;
  printf("threadhelper fn %p arg %p\n", fn, arg);
  fn(arg);
  exit(0);
}

extern void threadstub(void);

struct proc *threadalloc(void (*fn)(void *), void *arg) {
  struct proc *p;

  p = allocproc();

  p->tmask = 0;

  copytrapframe(p->main_thread->trapframe, p->trapframe);

  p->main_thread->context.ra = (uint64)threadstub;

  p->fn = fn;
  p->arg = arg;

  release(&p->lock);
  return p;
}

int get_proc_addr_num(struct proc *p) {
  for (int i = 0; i < NPROC; i++) {
    if (&proc[i] == p) {
      return i;
    }
  }
  return -1;
}