#ifndef _Thread_H_
#define _Thread_H_

#include "types.h"
#include "fat32.h"
#include "types.h"
#include "queue.h"
#include "riscv.h"
#include "context.h"

#define THREAD_NUM 10000

enum threadState {
    t_UNUSED,
    t_SLEEPING,
    t_RUNNABLE,
    t_RUNNING,
    t_ZOMBIE,
    t_TIMING,
};

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
    // TODO: signal
}thread;


void threadInit();
thread *allocNewThread();

#endif