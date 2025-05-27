#include "include/thread.h"
#include "include/kalloc.h"
#include "include/memlayout.h"
#include "include/printf.h"
#include "include/syscall.h"
#include "include/vm.h"

int nexttid = 1;

thread threads[THREAD_NUM];
thread *free_thread;

void threadInit() {
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].state = t_UNUSED;
    threads[i].pre_thread = NULL;
    threads[i].next_thread = NULL;
    if (i != 0) {
      threads[i].pre_thread = &threads[i - 1];
    }
    if (i + 1 < THREAD_NUM) {
      threads[i].next_thread = &threads[i + 1];
    }
  }
  free_thread = &threads[0];
}
// alloc main thread
thread *allocNewThread() {
  /*
  int i = 0;
  for (i = 0; i < THREAD_NUM; i++) {
      if (threads[i].state == t_UNUSED)
          break;
  }
  if (t_UNUSED != threads[i].state) {
      panic("allocNewThread: can not find unused thread");
  }
  if (NULL == (threads[i].trapframe = (struct trapframe *)kalloc())) {
      panic("allocNewThread: can not kalloc a page");
  }
  threads[i].awakeTime = 0;
  threads[i].state = t_RUNNABLE;
  threads[i].tid = nexttid++;


  return &threads[i];
  */
  if (NULL == free_thread) {
    panic("allocNewThread: can not find unused thread");
  }
  if (NULL == (free_thread->trapframe = (struct trapframe *)kalloc())) {
    panic("allocNewThread: can not kalloc a page");
  }
  free_thread->awakeTime = 0;
  free_thread->state = t_RUNNABLE;
  free_thread->tid = nexttid++;

  // 取出链表头部
  if (NULL != free_thread->next_thread) // 重要！链表操作要判断元素非空
    free_thread->next_thread->pre_thread = NULL;
  thread *tmp = free_thread;
  free_thread = free_thread->next_thread;
  tmp->next_thread = NULL;
  tmp->pre_thread = NULL;

  return tmp;
}

uint64 sys_tkill() {
  int tid;
  int signum;
  if (argint(0, &tid) < 0 || argint(1, &signum) < 0)
    return -1;
  debug_print("sys_tkill: tid = %d, signum = %d\n", tid, signum);
  // kill(myproc()->pid, SIGTERM);
  return 0;
}
