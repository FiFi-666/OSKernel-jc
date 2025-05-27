
#include "include/syscall.h"
#include "include/kalloc.h"
#include "include/memlayout.h"
#include "include/param.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/sbi.h"
#include "include/spinlock.h"
#include "include/string.h"
#include "include/sysinfo.h"
#include "include/sysnum.h"
#include "include/types.h"
#include "include/vm.h"

// Fetch the uint64 at addr from the current process.
int fetchaddr(uint64 addr, uint64 *ip) {
  struct proc *p = myproc();
  /*
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  // if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
  if(copyin2((char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  */
  if (copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0) {
    printf("fetchaddr: copyin failed\n");
    return -1;
  }
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int fetchstr(uint64 addr, char *buf, int max) {
  struct proc *p = myproc();
  int err = copyinstr(p->pagetable, buf, addr, max);
  // int err = copyinstr2(buf, addr, max);
  if (err < 0)
    return err;
  return strlen(buf);
}

static uint64 argraw(int n) {
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
int argint(int n, int *ip) {
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int argaddr(int n, uint64 *ip) {
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int argstr(int n, char *buf, int max) {
  uint64 addr;
  if (argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}

uint64 sys_shutdown() {
  sbi_shut_down();
  return 0;
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_dup3(void);
extern uint64 sys_exec(void);
extern uint64 sys_execve(void);
extern uint64 sys_exit(void);
extern uint64 sys_exit_group(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_readv(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_brk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_writev(void);
extern uint64 sys_uptime(void);
extern uint64 sys_test_proc(void);
extern uint64 sys_dev(void);
extern uint64 sys_readdir(void);
extern uint64 sys_getcwd(void);
extern uint64 sys_remove(void);
extern uint64 sys_trace(void);
extern uint64 sys_sysinfo(void);
extern uint64 sys_rename(void);
extern uint64 sys_getppid(void);
extern uint64 sys_mkdirat(void);
extern uint64 sys_nanosleep(void);
extern uint64 sys_clone(void);
extern uint64 sys_pipe2(void);
extern uint64 sys_wait4(void);
extern uint64 sys_getdents64(void);
extern uint64 sys_openat(void);
extern uint64 sys_gettimeofday(void);
extern uint64 sys_mmap(void);
extern uint64 sys_munmap(void);
extern uint64 sys_yield();
extern uint64 sys_uname();
extern uint64 sys_unlinkat();
extern uint64 sys_mount();
extern uint64 sys_umount();
extern uint64 sys_times();
extern uint64 sys_getuid();
extern uint64 sys_setgid();
extern uint64 sys_setuid();
extern uint64 sys_geteuid();
extern uint64 sys_getgid();
extern uint64 sys_getegid();
extern uint64 sys_lseek();
extern uint64 sys_exit_group();
extern uint64 sys_set_tid_address();
extern uint64 sys_futex();
extern uint64 sys_utimensat();
extern uint64 sys_clock_gettime();
extern uint64 sys_syslog();
extern uint64 sys_ioctl();
extern uint64 sys_faccessat();
extern uint64 sys_fstatat();
extern uint64 sys_sendfile();
extern uint64 sys_fcntl();
extern uint64 sys_renameat2();
extern uint64 sys_rt_sigaction(void);
extern uint64 sys_rt_sigprocmask(void);
extern uint64 sys_rt_sigreturn(void);
extern uint64 sys_ppoll();
extern uint64 sys_getpgid();
extern uint64 sys_setpgid();
extern uint64 sys_tgkill();
extern uint64 sys_gettid();
extern uint64 sys_umask();
extern uint64 sys_readlinkat();
extern uint64 sys_sync();
extern uint64 sys_fsync();
extern uint64 sys_ftruncate();
extern uint64 sys_rt_sigtimedwait();
extern uint64 sys_prlimit64();
extern uint64 sys_statfs();
extern uint64 sys_setitimer();
extern uint64 sys_sched_getscheduler();
extern uint64 sys_sched_getparam();
extern uint64 sys_sched_getaffinity();
extern uint64 sys_pselect6();
extern uint64 sys_tkill();
extern uint64 sys_copy_file_range();

// socket syscalls
extern uint64 sys_socket(void);
extern uint64 sys_bind(void);
extern uint64 sys_listen(void);
extern uint64 sys_accept(void);
extern uint64 sys_connect(void);
extern uint64 sys_sendto(void);
extern uint64 sys_recvfrom(void);
extern uint64 sys_getsockname(void);
extern uint64 sys_setsockopt(void);
extern uint64 sys_getsockopt(void);
extern uint64 sys_pread();
extern uint64 sys_mprotect();
extern uint64 sys_madvise();
extern uint64 sys_getrusage();
extern uint64 sys_sched_setscheduler();
extern uint64 sys_clock_getres();

static uint64 (*syscalls[])(void) = {
    [SYS_fork] sys_fork,
    [SYS_exit] sys_exit,
    [SYS_wait] sys_wait,
    [SYS_pipe] sys_pipe,
    [SYS_read] sys_read,
    [SYS_kill] sys_kill,
    [SYS_exec] sys_exec,
    [SYS_execve] sys_execve,
    [SYS_fstat] sys_fstat,
    [SYS_chdir] sys_chdir,
    [SYS_dup] sys_dup,
    [SYS_dup3] sys_dup3,
    [SYS_getpid] sys_getpid,
    [SYS_sbrk] sys_sbrk,
    [SYS_brk] sys_brk,
    [SYS_sleep] sys_sleep,
    [SYS_uptime] sys_uptime,
    [SYS_open] sys_open,
    [SYS_write] sys_write,
    [SYS_mkdir] sys_mkdir,
    [SYS_close] sys_close,
    [SYS_test_proc] sys_test_proc,
    [SYS_dev] sys_dev,
    [SYS_readdir] sys_readdir,
    [SYS_getcwd] sys_getcwd,
    [SYS_remove] sys_remove,
    [SYS_trace] sys_trace,
    [SYS_sysinfo] sys_sysinfo,
    [SYS_rename] sys_rename,
    [SYS_getppid] sys_getppid,
    [SYS_mkdirat] sys_mkdirat,
    [SYS_nanosleep] sys_nanosleep,
    [SYS_clone] sys_clone,
    [SYS_pipe2] sys_pipe2,
    [SYS_wait4] sys_wait4,
    [SYS_getdents64] sys_getdents64,
    [SYS_openat] sys_openat,
    [SYS_gettimeofday] sys_gettimeofday,
    [SYS_mmap] sys_mmap,
    [SYS_munmap] sys_munmap,
    [SYS_sched_yield] sys_yield,
    [SYS_uname] sys_uname,
    [SYS_unlinkat] sys_unlinkat,
    [SYS_mount] sys_mount,
    [SYS_umount] sys_umount,
    [SYS_times] sys_times,
    [SYS_getuid] sys_getuid,
    [SYS_setgid] sys_setgid,
    [SYS_setuid] sys_setuid,
    [SYS_geteuid] sys_geteuid,
    [SYS_getgid] sys_getgid,
    [SYS_getegid] sys_getegid,
    [SYS_lseek] sys_lseek,
    [SYS_exit_group] sys_exit_group,
    [SYS_set_tid_address] sys_set_tid_address,
    [SYS_clock_gettime] sys_clock_gettime,
    [SYS_syslog] sys_syslog,
    [SYS_writev] sys_writev,
    [SYS_readv] sys_readv,
    [SYS_utimensat] sys_utimensat,
    [SYS_ioctl] sys_ioctl,
    [SYS_faccessat] sys_faccessat,
    [SYS_fstatat] sys_fstatat,
    [SYS_sendfile] sys_sendfile,
    [SYS_fcntl] sys_fcntl,
    [SYS_rt_sigaction] sys_rt_sigaction,
    [SYS_rt_sigprocmask] sys_rt_sigprocmask,
    [SYS_rt_sigreturn] sys_rt_sigreturn,
    [SYS_renameat2] sys_renameat2,
    [SYS_ppoll] sys_ppoll,
    [SYS_getpgid] sys_getpgid,
    [SYS_setpgid] sys_setpgid,
    [SYS_tgkill] sys_tgkill,
    [SYS_gettid] sys_gettid,
    [SYS_umask] sys_umask,
    [SYS_readlinkat] sys_readlinkat,
    [SYS_rt_sigtimedwait] sys_rt_sigtimedwait,
    [SYS_prlimit64] sys_prlimit64,
    [SYS_statfs] sys_statfs,
    [SYS_pread] sys_pread,
    [SYS_mprotect] sys_mprotect,
    [SYS_sync] sys_sync,
    [SYS_fsync] sys_fsync,
    [SYS_ftruncate] sys_ftruncate,
    [SYS_setitimer] sys_setitimer,
    [SYS_pselect6] sys_pselect6,
    [SYS_tkill] sys_tkill,
    [SYS_copy_file_range] sys_copy_file_range,

    // socket syscalls
    [SYS_socket] sys_socket,
    [SYS_bind] sys_bind,
    [SYS_listen] sys_listen,
    [SYS_accept] sys_accept,
    [SYS_connect] sys_connect,
    [SYS_sendto] sys_sendto,
    [SYS_recvfrom] sys_recvfrom,
    // [SYS_shutdown]    sys_shutdown,
    [SYS_getsockname] sys_getsockname,
    // [SYS_getpeername] sys_getpeername,
    // [SYS_socketpair]  sys_socketpair,
    [SYS_setsockopt] sys_setsockopt,
    [SYS_getsockopt] sys_getsockopt,
    [SYS_madvise] sys_madvise,
    [SYS_futex] sys_futex,
    [SYS_getrusage] sys_getrusage,
    [SYS_sched_getscheduler] sys_sched_getscheduler,
    [SYS_sched_getparam] sys_sched_getparam,
    [SYS_sched_getaffinity] sys_sched_getaffinity,
    [SYS_sched_setscheduler] sys_sched_setscheduler,
    [SYS_clock_getres] sys_clock_getres,
    [SYS_shutdown] sys_shutdown,
};

static char *sysnames[] = {
    [SYS_fork] "fork",
    [SYS_exit] "exit",
    [SYS_wait] "wait",
    [SYS_pipe] "pipe",
    [SYS_read] "read",
    [SYS_kill] "kill",
    [SYS_exec] "exec",
    [SYS_execve] "execve",
    [SYS_fstat] "fstat",
    [SYS_chdir] "chdir",
    [SYS_dup] "dup",
    [SYS_dup3] "dup3",
    [SYS_getpid] "getpid",
    [SYS_sbrk] "sbrk",
    [SYS_brk] "brk",
    [SYS_sleep] "sleep",
    [SYS_uptime] "uptime",
    [SYS_open] "open",
    [SYS_write] "write",
    [SYS_mkdir] "mkdir",
    [SYS_close] "close",
    [SYS_test_proc] "test_proc",
    [SYS_dev] "dev",
    [SYS_readdir] "readdir",
    [SYS_getcwd] "getcwd",
    [SYS_remove] "remove",
    [SYS_trace] "trace",
    [SYS_sysinfo] "sysinfo",
    [SYS_rename] "rename",
    [SYS_getppid] "getppid",
    [SYS_mkdirat] "mkdirat",
    [SYS_nanosleep] "nanosleep",
    [SYS_clone] "clone",
    [SYS_pipe2] "pipe2",
    [SYS_wait4] "wait4",
    [SYS_getdents64] "getdents64",
    [SYS_openat] "openat",
    [SYS_mmap] "mmap",
    [SYS_munmap] "munmap",
    [SYS_uname] "uname",
    [SYS_unlinkat] "unlinkat",
    [SYS_mount] "mount",
    [SYS_umount] "umount",
    [SYS_times] "times",
    [SYS_getuid] "getuid",
    [SYS_setgid] "setgid",
    [SYS_setuid] "setuid",
    [SYS_geteuid] "geteuid",
    [SYS_getgid] "getgid",
    [SYS_getegid] "getegid",
    [SYS_lseek] "lseek",
    [SYS_exit_group] "exit_group",
    [SYS_set_tid_address] "set_tid_address",
    [SYS_futex] "futex",
    [SYS_utimensat] "utimensat",
    [SYS_clock_gettime] "clock_gettime",
    [SYS_syslog] "syslog",
    [SYS_writev] "writev",
    [SYS_readv] "readv",
    [SYS_ioctl] "ioctl",
    [SYS_faccessat] "faccessat",
    [SYS_fstatat] "fstatat",
    [SYS_sendfile] "sendfile",
    [SYS_fcntl] "fcntl",
    [SYS_rt_sigaction] "rt_sigaction",
    [SYS_rt_sigprocmask] "rt_sigprocmask",
    [SYS_renameat2] "renameat2",
    [SYS_ppoll] "ppoll",
    [SYS_getpgid] "getpgid",
    [SYS_setpgid] "setpgid",
    [SYS_tgkill] "tgkill",
    [SYS_gettid] "gettid",
    [SYS_umask] "umask",
    [SYS_rt_sigtimedwait] "rt_sigtimedwait",
    [SYS_rt_sigtimedwait] "rt_sigtimedwait",
    [SYS_rt_sigreturn] "rt_sigreturn",
    [SYS_prlimit64] "prlimit64",
    [SYS_statfs] "statfs",
    [SYS_pread] "pread",
    [SYS_mprotect] "mprotect",
    [SYS_prlimit64] "prlimit64",
    [SYS_statfs] "statfs",
    [SYS_madvise] "madvise",
    [SYS_getrusage] "getrusage",
    [SYS_setitimer] "setitimer",
    [SYS_fsync] "fsync",
    [SYS_sync] "sync",
    [SYS_ftruncate] "ftruncate",
    [SYS_pselect6] "pselect6",
    [SYS_readlinkat] "readlinkat",
    [SYS_tkill] "tkill",
    [SYS_copy_file_range] "copy_file_range",

    // socket syscalls
    [SYS_socket] "socket",
    [SYS_bind] "bind",
    [SYS_listen] "listen",
    [SYS_accept] "accept",
    [SYS_connect] "connect",
    [SYS_sendto] "sendto",
    [SYS_recvfrom] "recvfrom",
    [SYS_getsockname] "getsockname",
    [SYS_setsockopt] "setsockopt",
    [SYS_madvise] "madvise",
    [SYS_sched_getscheduler] "sched_getscheduler",
    [SYS_sched_getparam] "sched_getparam",
    [SYS_sched_getaffinity] "sched_getaffinity",
    [SYS_sched_setscheduler] "sched_setscheduler",
    [SYS_clock_getres] "clock_getres",
    [SYS_shutdown] "shutdown",
};

void syscall(void) {
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    if (num != 64 && num != 63 && num != SYS_writev &&
        num != SYS_clock_gettime && num != SYS_sendto && num != SYS_recvfrom)
      debug_print("pid %d call %d: %s\n", p->pid, num, sysnames[num]);
    p->trapframe->a0 = syscalls[num]();
    if (num == SYS_openat && p->trapframe->a0 == -1) {
      printf("");
    }
    // trace
    if (num != SYS_read && num != SYS_write && num != SYS_writev &&
        num != SYS_sendto && num != SYS_recvfrom)
      debug_print("pid %d: %s -> %d\n", p->pid, sysnames[num],
                  p->trapframe->a0);
    // printf("pid %d call %d: %s a0:%p sp:%p\n", p->pid, num, sysnames[num],
    // p->trapframe->a0, p->trapframe->sp);
    if ((p->tmask & (1 << num)) != 0) {
      printf("pid %d: %s -> %d\n", p->pid, sysnames[num], p->trapframe->a0);
    }
  } else {
    debug_print("pid %d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}

uint64 sys_test_proc(void) {
  int n;
  argint(0, &n);
  printf("hello world from proc %d, hart %d, arg %d\n", myproc()->pid, r_tp(),
         n);
  return 0;
}

uint64 sys_sysinfo(void) {
  uint64 addr;
  if (argaddr(0, &addr) < 0) {
    return -1;
  }
  struct sysinfo info;
  memset(&info, 0, sizeof(info));

  info.uptime = r_time() / CLK_FREQ;
  info.totalram = PHYSTOP - KERNBASE;
  info.freemem = freemem_amount();
  info.bufferram = 512 * 2500; // attention
  info.nproc = procnum();
  info.mem_unit = PGSIZE;

  if (either_copyout(1, addr, (char *)&info, sizeof(info)) < 0)
    return -1;

  return 0;
  /*
  info.freemem = freemem_amount();
  info.nproc = procnum();
  */
  // if (copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0) {
  /*
  if (copyout2(addr, (char *)&info, sizeof(info)) < 0) {
    return -1;
  }
  */

  return 0;
}