# 线程
xv6系统本身不支持线程，在决赛第一阶段，我们实现了内核级线程，并通过了libctest中有关线程的强测试。所有的调度单位均以**线程**为单位，进程则为资源的一种管理单位。

## 线程管理模块
``` c
typedef struct thread {
    struct spinlock lock;

    // 当使用下面这些变量的时候，thread的锁必须持有
    enum threadState state;   // 线程的状态
    struct proc *p;    // 这个线程属于哪一个进程
    void *chan;   // 如果不为NULL,则在chan地址上睡眠
    int tid;   // 线程ID
    uint64 awakeTime; 

    // 使用下面这些变量的时候，thread的锁不需要持有
    uint64 kstack;   // 线程内核栈的地址,一个进程的不同线程所用的内核栈的地址应该不同
    uint64 vtf;   // 线程的trapframe的虚拟地址
    uint64 sz;   // 复制自进程的sz
    struct trapframe *trapframe;
    context context;  // 每个进程应该有自己的context
    uint64 kstack_pa;
    uint64 clear_child_tid;

    struct thread *next_thread;
    struct thread *pre_thread;
}thread;
```

空闲线程和进程管理的线程都用双向链表来管理。
对于每个进程，proc结构体中有thread_queue字段，表示管理线程中的第一个线程。
每个线程有pre_thread和next_thread字段，分别用来指代链表中的前一个线程和后一个线程。
在thread.c中定义有free_thread作为空闲线程的链表头。

## 线程的创建

`threadInit()`函数是初始化线程池，为每个线程设置初始状态为未使用（t_UNUSED），并建立线程之间的前后关系，最后将`free_thread`设置为线程池的第一个线程，这样我们就可以开始从这个线程池中分配和使用线程了。


## 获取当前运行的线程
每个进程都有一个主线程，表示在这个进程上正在执行的线程是哪一个。用main_thread来表示。
当要获取当前正在运行的线程时，只需要得到myproc()->main_thread即可。

## 线程编号
为了避免和进程编号重复，在thread.c中定义了静态变量nexttid，每当一个新线程从空闲线程链表中被取出时，则为其分配一个新的线程编号。

## 线程状态

``` c
enum threadState {
    t_UNUSED,
    t_SLEEPING,
    t_RUNNABLE,
    t_RUNNING,
    t_ZOMBIE,
    t_TIMING,
};
```

- `t_UNUSED` ：线程空闲

- `t_SLEEPING` ：线程等待某资源或调用 sleep 函数。

- `t_RUNNABLE` ：线程就绪，可以被运行。

- `t_RUNNING` ：该线程正在运行。

- `t_ZOMBIE` ：该线程的进程被杀死

- `t_TIMING` ：该线程被休眠了指定时间

## 创建线程
用户态程序调用pthread_create函数时，调用clone的参数flags中带有CLONE_VM标志位，在sys_clone函数中，若检测到此标记位，则进行线程的创建，否则简单fork。
``` c
uint64 thread_clone(uint64 stackVa,uint64 ptid,uint64 tls,uint64 ctid) {
  struct proc *p = myproc();
  thread *t = allocNewThread();
  t->p = p;
  if (mappages(p->kpagetable,p->kstack-PGSIZE * p->thread_num * 2,PGSIZE,(uint64)(t->trapframe),PTE_R | PTE_W) < 0)
    panic("thread_clone: mappages");
  t->vtf = p->kstack-PGSIZE * p->thread_num * 2;
  void *kstack_pa = kalloc();
  if (NULL == kstack_pa)
    panic("thread_clone: kalloc kstack failed");
  if (mappages(p->kpagetable,p->kstack-PGSIZE * (1 + p->thread_num * 2),PGSIZE,(uint64)kstack_pa,PTE_R | PTE_W) < 0)
    panic("thread_clone: mappages");
  thread_stack_param tmp;

  if (copyin(p->pagetable,(char*)(&tmp),stackVa,sizeof(thread_stack_param)) < 0) {
    panic("copy in thread_stack_param failed");
  }
  t->kstack_pa = (uint64)kstack_pa;
  t->kstack = p->kstack-PGSIZE * (1 + p->thread_num * 2);
  // printf("thread stack param:%p %p\n",tmp.func_point,tmp.arg_point);
  
  t->next_thread = p->thread_queue;
  if (NULL != p->thread_queue)
    p->thread_queue->pre_thread = t;
  p->thread_queue = t;
  
  copytrapframe(t->trapframe,p->trapframe);
  t->trapframe->a0 = tmp.func_point;
  t->trapframe->tp = tls;
  t->trapframe->kernel_sp = p->kstack-PGSIZE * (1 + p->thread_num * 2) + PGSIZE;
  t->trapframe->sp = stackVa;
  t->trapframe->epc = tmp.func_point;
  copycontext_from_trapframe(&t->context,t->trapframe);
  t->context.ra = (uint64)forkret;
  t->context.sp = t->trapframe->kernel_sp;
  if (ptid != 0) {
    if (either_copyout(1,ptid,(void*)&t->tid,sizeof(int)) < 0)
      panic("thread_clone: either_copyout");
  }
  t->clear_child_tid = ctid;
  p->thread_num++;

  return t->tid;
}
```
首先，函数从进程的线程池中分配一个新的线程，并初始化该线程的各个字段，包括线程所属的进程、线程的栈等信息。
接下来，函数将新线程的trapframe映射到进程的页表中。
然后，函数为新线程分配一个内核栈，并将内核栈映射到进程的页表中。
然后，函数复制调用线程的trapframe到新线程的trapframe，并设置新线程的trapframe的各个字段，包括函数指针、线程局部存储的地址、内核栈的地址、栈的地址和函数指针。
接下来，函数将新线程添加到进程的线程队列中。
最后，如果ptid参数不为0，函数将新线程的线程ID写入ptid指定的地址，并将新线程的清除ID写入ctid指定的地址，然后增加进程的线程数。
我们在进程的kpagetable中为每个线程分配并映射了独自的trapframe和kstack物理地址和地址空间。为了节省空间，每个进程创建的第一个线程的内核栈与进程的内核栈共用同一地址空间和物理地址。

## 线程的调度
线程在thread_clone后,通过手动设置线程的trapframe的a0和epc寄存器，线程会在第一次被调度时先返回到forkret函数，接着由forkret返回返回到用户态中，从epc指向的位置继续执行。
``` c
void
forkret(void)
{
  
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
```
在每个CPU核心有scheduler()函数，作为调度器。这个调度器不断轮训所有进程是否为RUNNABLE状态，当有进程可以执行时，则从进程的thread_queue中选出状态为t_RUNNABLE的线程执行：
``` c
        thread *t = p->thread_queue;
        while (NULL != t) {
          if (t->state == t_RUNNABLE || (t->state == t_TIMING && t->awakeTime < r_time() + (1LL << 35)))
            break;
          t = t->next_thread;
        }
        if (NULL == t)
          continue;
        if (p->thread_queue != t) {   // 放到队首，避免死线程集中在首部
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
        copycontext(&p->context,&p->main_thread->context);
        copytrapframe(p->trapframe,p->main_thread->trapframe);
        p->main_thread->state = t_RUNNING;
        p->main_thread->awakeTime = 0;
        p->state = RUNNING;
        futexClear(p->main_thread);
        c->proc = p;
        w_satp(MAKE_SATP(p->kpagetable));
        sfence_vma();
        swtch(&c->context, &p->context);
        copycontext(&p->main_thread->context,&p->context);
        w_satp(MAKE_SATP(kernel_pagetable));
        sfence_vma();
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;

        found = 1;
```
确定要执行某一个线程时，进程将该线程作为主进程，并拷贝其trapframe和context(用于调度),拷贝的好处在于当内核需要访问进程管理的资源时，不必更改接口。

线程执行完毕时，一定会调用sched函数来让出CPU,在sched函数中将进程的trapframe和context拷贝到主线程的对应内存区域中。通过两次拷贝即实现了线程执行的“检查点-加载-存储”功能。
``` c
void
sched(void)
{
  int intena;
  struct proc *p = myproc();
  // printf("sched p->pid %d\n", p->pid);
  if(!holding(&p->lock))
    panic("sched p->lock");
  if(mycpu()->noff != 1){
    printf("noff:%d\n", mycpu()->noff);
    panic("sched locks");
  }
  if(p->state == RUNNING || p->main_thread->state == t_RUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  copytrapframe(p->main_thread->trapframe,p->trapframe);  
  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}
```

## 线程销毁

将线程链表前后连接起来，并加入到空闲线程链表中，释放掉trapframe和kstack的物理内存，并解除在kpagetable上的内存映射。

``` c
    thread *tmp = t->next_thread;
    if (NULL != t->next_thread)
      t->next_thread->pre_thread = NULL;
    t->state = t_UNUSED;
    t->next_thread = free_thread;
    if (NULL != free_thread)
      free_thread->pre_thread = t;
    free_thread = t;
    kfree((void*)t->trapframe);
    if (t->kstack != p->kstack){
      vmunmap(p->kpagetable, t->kstack, 1, 0);
      kfree((void*)t->kstack_pa);
    }
    t = tmp;
```