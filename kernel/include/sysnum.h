#ifndef __SYSNUM_H
#define __SYSNUM_H

// System call numbers
#define SYS_fork         300
#define SYS_exit        93
#define SYS_exit_group  94
#define SYS_wait         3
#define SYS_pipe         4
#define SYS_read        63
#define SYS_kill         129
#define SYS_exec         7
#define SYS_execve     221
#define SYS_fstat       80
#define SYS_chdir       49
#define SYS_dup         23
#define SYS_dup3        24
#define SYS_getpid      172
#define SYS_sbrk        12 //from 12 to 214
#define SYS_brk         214
#define SYS_sleep       13
#define SYS_uptime      14
#define SYS_open        15
#define SYS_write       64
#define SYS_remove      87  //from 25 to 87
#define SYS_trace       18
#define SYS_sysinfo     179 //from 19 to 179
#define SYS_mkdir       1030
#define SYS_close       57
#define SYS_test_proc   22
#define SYS_dev         10
#define SYS_readdir     21      //comedymaker changed: from 24 to 21
#define SYS_getcwd      17
#define SYS_rename      26
#define SYS_getppid     173
#define SYS_mkdirat     34
#define SYS_nanosleep   101
#define SYS_clone       220
#define SYS_pipe2       59
#define SYS_wait4       260
#define SYS_getdents64  61
#define SYS_openat      56
#define SYS_gettimeofday 169
#define SYS_mmap        222
#define SYS_munmap      215
#define SYS_sched_yield 124
#define SYS_uname       160
#define SYS_unlinkat    35
#define SYS_mount       40
#define SYS_umount      39
#define SYS_times       153
#define SYS_getuid      174
#define SYS_setgid      144
#define SYS_setuid      146
#define SYS_geteuid     175
#define SYS_getgid      176
#define SYS_getegid     177
#define SYS_lseek       62
#define SYS_exit_group  94
#define SYS_set_tid_address 96
#define SYS_clock_gettime   113
#define SYS_futex       98
#define SYS_utimensat   88
#define SYS_syslog      116
#define SYS_writev      66
#define SYS_readv      65
#define SYS_ioctl       29
#define SYS_faccessat   48
#define SYS_fstatat     79
#define SYS_sendfile    71
#define SYS_fcntl       25
#define SYS_renameat2   276
#define SYS_rt_sigaction 134
#define SYS_rt_sigprocmask 135
#define SYS_rt_sigreturn 139
#define SYS_ppoll       73
#define SYS_getpgid     155
#define SYS_setpgid     154
#define SYS_tgkill      131
#define SYS_gettid      178
#define SYS_umask       166
#define SYS_readlinkat  78
#define SYS_sync        81
#define SYS_fsync        82
#define SYS_ftruncate   46
#define SYS_rt_sigtimedwait 137
#define SYS_prlimit64   261
#define SYS_statfs      43
#define SYS_setitimer   103
#define SYS_tkill       130
#define SYS_copy_file_range 285

#define SYS_shutdown    1000

// network syscall
#define SYS_socket      198
#define SYS_bind        200
#define SYS_listen      201
#define SYS_accept      202
#define SYS_connect     203
#define SYS_getsockname 204
#define SYS_sendto      206
#define SYS_recvfrom    207
#define SYS_setsockopt  208
#define SYS_getsockopt  209
#define SYS_pread       67
#define SYS_mprotect    226
#define SYS_madvise     233
#define SYS_getrusage   165
#define SYS_sched_getscheduler 120
#define SYS_sched_getparam 121
#define SYS_sched_getaffinity 123
#define SYS_sched_setscheduler 119
#define SYS_clock_getres    114
#define SYS_pselect6    72

#endif