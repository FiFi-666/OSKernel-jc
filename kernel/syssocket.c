#include "include/fcntl.h"
#include "include/file.h"
#include "include/kalloc.h"
#include "include/mmap.h"
#include "include/param.h"
#include "include/pipe.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/sleeplock.h"
#include "include/socket.h"
#include "include/spinlock.h"
#include "include/stat.h"
#include "include/string.h"
#include "include/syscall.h"
#include "include/sysinfo.h"
#include "include/sysnum.h"
#include "include/types.h"
#include "include/vm.h"
#include "lwip/include/arch/errno.h"

int tcp_start_listen;
static int fdalloc(struct file *f) {
  int fd;
  struct proc *p = myproc();

  for (fd = 0; fd < NOFILEMAX(p); fd++) {
    if (p->ofile[fd] == 0) {
      p->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

static int argfd(int n, int *pfd, struct file **pf) {
  int fd;
  struct file *f;

  if (argint(n, &fd) < 0) {
    debug_print("argfd: argint error\n");
    return -1;
  }
  // mmap映射匿名区域的时候会需要fd为-1
  if (fd == -1) {
    return -2;
  }

  if (fd == -100) {
    *pfd = fd;
    return -1;
  }

  if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == NULL) {
    debug_print("fd: %d argfd: fd error\n", fd);
    return -1;
  }

  if (pfd)
    *pfd = fd;
  if (pf)
    *pf = f;
  return 0;
}

uint64 sys_socket(void) {
  int domain, type, protocol;
  if (argint(0, &domain) < 0) {
    printf("sys_socket: argint(0, &domain) < 0\n");
    return -1;
  }

  if (argint(1, &type) < 0) {
    printf("sys_socket: argint(1, &type) < 0\n");
    return -1;
  }

  if (argint(2, &protocol) < 0) {
    printf("sys_socket: argint(2, &protocol) < 0\n");
    return -1;
  }
  // if(domain != AF_INET){
  //     return -97;
  // }
  struct file *f;
  int fd = 0;
  if ((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0) {
    if (f) {
      fileclose(f);
    }
  }
  debug_print("[sys_socket] type %p\n", type);
  f->type = FD_SOCK;
  f->off = 0;
  f->ep = 0;
  f->readable = 1;
  f->writable = 1;
  f->socket_type = type;
  f->socketnum = do_socket(domain, type, protocol);
  if (type & SOCK_CLOEXEC) {
    myproc()->exec_close[fd] = 1;
  }
  // if(type & 0x800){
  //     int argp = 1;
  //     lwip_ioctl(f->socketnum, FIONBIO, (void*)&argp);
  // }
  return fd;
}

uint64 sys_bind(void) {
  int sockfd;
  struct sockaddr_in_compat *addr;
  socklen_t addrlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_bind: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argaddr(1, (void *)&addr) < 0) {
    printf("sys_bind: argaddr(1, (void *)&addr) < 0\n");
    return -1;
  }
  if (argint(2, (int *)&addrlen) < 0) {
    printf("sys_bind: argint(2, &addrlen) < 0\n");
    return -1;
  }
  // printf("sys_bind: sockfd:%d, addr:%p, addrlen:%d\n", sockfd, addr,
  // addrlen);
  struct sockaddr_in_compat in_compat;
  if (copyin(myproc()->pagetable, (char *)&in_compat, (uint64)addr,
             sizeof(struct sockaddr_in_compat)) < 0)
    return -1;
  struct sockaddr_in in = {.sin_len = 16,
                           .sin_family = in_compat.sin_family,
                           .sin_port = in_compat.sin_port,
                           .sin_addr = in_compat.sin_addr,
                           .sin_zero = {0}};
  if (in.sin_addr.s_addr == 0) {
    in.sin_addr.s_addr = 16777343;
  }
  return do_bind(f->socketnum, (struct sockaddr *)&in, addrlen);
}

uint64 sys_listen(void) {
  int sockfd, backlog;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_listen: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argint(1, &backlog) < 0) {
    printf("sys_listen: argint(1, &backlog) < 0\n");
    return -1;
  }
  // debug_print("sys_listen sockfd:%d, backlog:%d\n", sockfd, backlog);
  tcp_start_listen = 1;
  printf("tcp_start_listen = 1\n");
  return do_listen(f->socketnum, backlog);
}

uint64 sys_connect(void) {
  int sockfd;
  struct sockaddr_in_compat *addr;
  socklen_t addrlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_connect: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argaddr(1, (void *)&addr) < 0) {
    printf("sys_connect: argaddr(1, (void *)&addr) < 0\n");
    return -1;
  }
  if (argint(2, (int *)&addrlen) < 0) {
    printf("sys_connect: argint(2, &addrlen) < 0\n");
    return -1;
  }
  struct sockaddr_in_compat in_compat;
  if (copyin(myproc()->pagetable, (char *)&in_compat, (uint64)addr,
             sizeof(struct sockaddr_in_compat)) < 0)
    return -1;
  struct sockaddr_in in = {.sin_len = 16,
                           .sin_family = in_compat.sin_family,
                           .sin_port = in_compat.sin_port,
                           .sin_addr = in_compat.sin_addr,
                           .sin_zero = {0}};
  // in.sin_family = 2;
  // if (in.sin_port == 0 && in.sin_addr.s_addr == 0) {
  //     in.sin_port = 65535;
  //     in.sin_addr.s_addr = 16777343;
  // }
  while (tcp_start_listen == 0) {
    printf("tcp_start_listen = 0\n");
  }
  printf("tcp_start_listen = 1\n");
  int result = do_connect(f->socketnum, (struct sockaddr *)&in, addrlen);
  printf("sys_connect: result = %d errorno :%d \n", result, errno);
  if (0 == result)
    return 0;
  return -97;
}

uint64 sys_accept(void) {
  int sockfd;
  struct sockaddr_in_compat *addr;
  socklen_t *addrlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_accept: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argaddr(1, (void *)&addr) < 0) {
    printf("sys_accept: argaddr(1, (void *)&addr) < 0\n");
    return -1;
  }
  if (argaddr(2, (void *)&addrlen) < 0) {
    printf("sys_accept: argaddr(2, (int *)addrlen) < 0\n");
    return -1;
  }
  struct sockaddr_in_compat in_compat;
  if (copyin(myproc()->pagetable, (char *)&in_compat, (uint64)addr,
             sizeof(struct sockaddr_in_compat)) < 0)
    return -1;
  struct sockaddr_in in = {.sin_len = 16,
                           .sin_family = in_compat.sin_family,
                           .sin_port = in_compat.sin_port,
                           .sin_addr = in_compat.sin_addr,
                           .sin_zero = {0}};
  socklen_t inlen;
  if (copyin(myproc()->pagetable, (char *)&inlen, (uint64)addrlen,
             sizeof(socklen_t)) < 0)
    return -1;
  int ret = do_accept(f->socketnum, (struct sockaddr *)&in, &inlen);
  if (ret > 0) {
    struct file *f2 = NULL;
    int fd;
    if ((f2 = filealloc()) == NULL || (fd = fdalloc(f2)) < 0) {
      if (f2) {
        fileclose(f2);
      }
      return -1;
    }
    f2->type = FD_SOCK;
    f2->off = 0;
    f2->ep = 0;
    f2->readable = 1;
    f2->writable = 1;
    f2->socketnum = ret;
    return fd;
  }
  return ret;
}

uint64 sys_sendto(void) {
  int sockfd;
  void *buf;
  size_t len;
  int flags;
  struct sockaddr_in_compat *dest_addr;
  socklen_t addrlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_sendto: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argaddr(1, (uint64 *)&buf) < 0) {
    printf("sys_sendto: argaddr(1, (void *)&buf) < 0\n");
    return -1;
  }
  if (argint(2, (int *)&len) < 0) {
    printf("sys_sendto: argint(2, &len) < 0\n");
    return -1;
  }
  if (argint(3, &flags) < 0) {
    printf("sys_sendto: argint(3, &flags) < 0\n");
    return -1;
  }
  if (argaddr(4, (uint64 *)&dest_addr) < 0) {
    printf("sys_sendto: argaddr(4, (void *)&dest_addr) < 0\n");
    return -1;
  }
  if (argint(5, (int *)&addrlen) < 0) {
    printf("sys_sendto: argint(5, &addrlen) < 0\n");
    return -1;
  }
  // printf("sys_sendto: sockfd:%d, buf:%p, len:%d, flags:%d, dest_addr:%p,
  // addrlen:%d\n", sockfd, buf, len, flags, dest_addr, addrlen);
  struct sockaddr_in_compat in_compat;
  if (copyin(myproc()->pagetable, (char *)&in_compat, (uint64)dest_addr,
             sizeof(struct sockaddr_in_compat)) < 0)
    return -1;
  struct sockaddr_in in = {.sin_len = 16,
                           .sin_family = in_compat.sin_family,
                           .sin_port = in_compat.sin_port,
                           .sin_addr = in_compat.sin_addr,
                           .sin_zero = {0}};
  return do_sendto(f->socketnum, buf, len, flags, (struct sockaddr *)&in,
                   addrlen);
}

uint64 sys_recvfrom(void) {
  int sockfd;
  void *buf;
  size_t len;
  int flags;
  struct sockaddr_in_compat *src_addr;
  socklen_t *addrlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_recvfrom: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argaddr(1, (uint64 *)&buf) < 0) {
    printf("sys_recvfrom: argaddr(1, (void *)&buf) < 0\n");
    return -1;
  }
  if (argint(2, (int *)&len) < 0) {
    printf("sys_recvfrom: argint(2, &len) < 0\n");
    return -1;
  }
  if (argint(3, &flags) < 0) {
    printf("sys_recvfrom: argint(3, &flags) < 0\n");
    return -1;
  }
  if (argaddr(4, (void *)&src_addr) < 0) {
    printf("sys_recvfrom: argaddr(4, (void *)&src_addr) < 0\n");
    return -1;
  }
  if (argaddr(5, (void *)&addrlen) < 0) {
    printf("sys_recvfrom: argint(5, (int *)addrlen) < 0\n");
    return -1;
  }
  struct sockaddr_in_compat in_compat;
  if (copyin(myproc()->pagetable, (char *)&in_compat, (uint64)src_addr,
             sizeof(struct sockaddr_in_compat)) < 0)
    return -1;
  struct sockaddr_in in = {.sin_len = 16,
                           .sin_family = in_compat.sin_family,
                           .sin_port = in_compat.sin_port,
                           .sin_addr = in_compat.sin_addr,
                           .sin_zero = {0}};
  socklen_t inlen;
  if (copyin(myproc()->pagetable, (char *)&inlen, (uint64)addrlen,
             sizeof(socklen_t)) < 0)
    return -1;
  return do_recvfrom(f->socketnum, buf, len, flags, (struct sockaddr *)&in,
                     &inlen);
}

uint64 sys_getsockname(void) {
  // libctest的socket在这里的调用addr会是0，很奇怪
  //  return 0;
  int sockfd;
  struct sockaddr_in_compat *addr;
  socklen_t *addrlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_getsockname: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argaddr(1, (void *)&addr) < 0) {
    printf("sys_getsockname: argaddr(1, (void *)&addr) < 0\n");
    return -1;
  }
  if (argaddr(2, (void *)&addrlen) < 0) {
    printf("sys_getsockname: argint(2, (int *)addrlen) < 0\n");
    return -1;
  }
  printf("sys_getsockname: addr = %p, addrlen = %p\n", addr, addrlen);
  struct sockaddr_in_compat in_compat;
  if (copyin(myproc()->pagetable, (char *)&in_compat, (uint64)addr,
             sizeof(struct sockaddr_in_compat)) < 0)
    return -1;
  struct sockaddr_in in = {.sin_len = 16,
                           .sin_family = in_compat.sin_family,
                           .sin_port = in_compat.sin_port,
                           .sin_addr = in_compat.sin_addr,
                           .sin_zero = {0}};
  socklen_t inlen;
  if (copyin(myproc()->pagetable, (char *)&inlen, (uint64)addrlen,
             sizeof(socklen_t)) < 0)
    return -1;
  int ret = do_getsockname(f->socketnum, (struct sockaddr *)&in, &inlen);
  in_compat.sin_family = in.sin_family;
  in_compat.sin_addr = in.sin_addr;
  in_compat.sin_port = in.sin_port;
  if (copyout(myproc()->pagetable, (uint64)addr, (char *)&in_compat,
              sizeof(struct sockaddr_in_compat)) < 0)
    return -1;
  return ret;
}

uint64 sys_setsockopt(void) {
  int sockfd;
  int level;
  int optname;
  void *optval;
  socklen_t optlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_setsockopt: argint(0, &sockfd) < 0\n");
    return -1;
  }
  if (argint(1, &level) < 0) {
    printf("sys_setsockopt: argint(1, &level) < 0\n");
    return -1;
  }
  if (argint(2, &optname) < 0) {
    printf("sys_setsockopt: argint(2, &optname) < 0\n");
    return -1;
  }
  if (argaddr(3, (void *)&optval) < 0) {
    printf("sys_setsockopt: argaddr(3, (void *)&optval) < 0\n");
    return -1;
  }
  if (argint(4, (int *)&optlen) < 0) {
    printf("sys_setsockopt: argint(4, &optlen) < 0\n");
    return -1;
  }
  void *koptval = kalloc();
  if (copyin(myproc()->pagetable, koptval, (uint64)optval, optlen) < 0) {
    kfree(koptval);
    return -1;
  }
  int ret = do_setsockopt(f->socketnum, level, optname, koptval, optlen);
  kfree(koptval);
  return ret;
}

void sys_getsockopt(void) {
  int sockfd;
  int level;
  int optname;
  void *optval;
  socklen_t *optlen;
  struct file *f;
  if (argfd(0, &sockfd, &f) < 0) {
    printf("sys_getsockopt: argint(0, &sockfd) < 0\n");
    return;
  }
  if (argint(1, &level) < 0) {
    printf("sys_getsockopt: argint(1, &level) < 0\n");
    return;
  }
  if (argint(2, &optname) < 0) {
    printf("sys_getsockopt: argint(2, &optname) < 0\n");
    return;
  }
  if (argaddr(3, (void *)&optval) < 0) {
    printf("sys_getsockopt: argaddr(3, (void *)&optval) < 0\n");
    return;
  }
  if (argaddr(4, (void *)&optlen) < 0) {
    printf("sys_getsockopt: argint(4, (int *)optlen) < 0\n");
    return;
  }
  char koptval[256];
  socklen_t koptlen;
  int ret = do_getsockopt(f->socketnum, level, optname, koptval, &koptlen);
  if (copyout(myproc()->pagetable, (uint64)optval, koptval, koptlen) < 0) {
    return;
  }
  if (copyout(myproc()->pagetable, (uint64)optlen, (char *)&koptlen,
              sizeof(socklen_t)) < 0) {
    return;
  }
  return;
}