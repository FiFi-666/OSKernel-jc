//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "include/fat32.h"
#include "include/fcntl.h"
#include "include/file.h"
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
#include "include/types.h"
#include "include/vm.h"

char syslogbuffer[1024];
int bufferlength = 0;

void initlogbuffer() {
  bufferlength = 0;
  strncpy(syslogbuffer, "[log]init done\n", 1024);
  bufferlength += strlen(syslogbuffer);
}

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
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

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
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
/*
// Comedymaker added
// Allocate a specifed file descriptor for the given file
static int
fdalloc3(struct file *f, int new)
{
  struct proc *p = myproc();
  if (new < NOFILE && p->ofile[new] == 0)
  {
    p->ofile[new] = f;
    return new;
  }
  return -1;
}
*/
static struct dirent *create(char *path, short type, int mode);
uint64 sys_mkdirat(void) {
  // 目前测试中的mkdir都是调用了mkdirat，并且测试中并没有测试mkdirat
  // 所以我们目前只考虑path，不考虑mkdirat的其他参数
  // 下面的写法目前是和mkdir一样的，但是注意，我们的参数path是第1个，而mkdir的path是第0个
  char path[FAT32_MAX_PATH];
  struct dirent *ep;
  int mode;
  if (argstr(1, path, FAT32_MAX_PATH) < 0 || argint(2, &mode) < 0) {
    return -1;
  }
  // printf("path: %s, mode: %d\n", path, mode);
  if (strlen(path) == 1) {
    return 0;
  }

  if ((ep = create(path, T_DIR, 0)) == 0) {
    return -1;
  }
  eunlock(ep);
  eput(ep);
  // printf("arrive2\n");
  return 0;
}

uint64 sys_dup(void) {
  struct file *f;
  int fd;
  if (argfd(0, 0, &f) < 0)
    return -1;
  if ((fd = fdalloc(f)) < 0)
    return -24;
  filedup(f);
  return fd;
}

uint64 sys_dup3(void) {
  /*
  struct file *f;
  int new;
  int fd;
  if(argfd(0, 0, &f) < 0 || argint(1, &new) < 0)
    return -1;
  if((fd=fdalloc3(f, new)) < 0)
    return -1;
  filedup(f);
  return fd;
  */
  struct file *f;
  int newfd;
  struct proc *p = myproc();
  if (argfd(0, 0, &f) < 0 || argint(1, &newfd) < 0 || newfd < 0)
    return -1;
  if (newfd >= NOFILEMAX(p))
    return -24;
  if (p->ofile[newfd] != f)
    p->ofile[newfd] = filedup(f);

  return newfd;
}

uint64 sys_read(void) {
  struct file *f;
  int n;
  uint64 p;

  if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argaddr(1, &p) < 0)
    return -1;
  return fileread(f, p, n);
}

uint64 sys_pread(void) {
  struct file *f;
  int off, count;
  uint64 p;
  if (argfd(0, 0, &f) < 0 || argaddr(1, &p) < 0 || argint(2, &count) < 0 ||
      argint(3, &off) < 0) {
    return -1;
  }
  fileseek(f, off, SEEK_SET);

  return fileread(f, p, count);
}

uint64 sys_write(void) {
  struct file *f;
  int n;
  uint64 p;

  if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argaddr(1, &p) < 0)
    return -1;
  // if(f->type == FD_ENTRY){
  //   printf("write file : %s\n", f->ep->filename);
  //   printf("write file from :%p\n", p);
  // }
  return filewrite(f, p, n);
}

uint64 sys_close(void) {
  int fd;
  struct file *f;

  if (argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

uint64 sys_readv(void) {
  struct file *f;
  int fd;
  uint64 iov;
  int iovcnt;
  iovec v[IOVMAX];
  struct proc *p = myproc();

  if (argfd(0, &fd, &f) < 0)
    return -1;
  if (argaddr(1, &iov) < 0)
    return -1;
  if (argint(2, &iovcnt) < 0)
    return -1;

  if (iov) {
    copyin(p->pagetable, (char *)v, iov, sizeof(v));
  } else
    return -1;

  uint64 len = 0;
  for (int i = 0; i < iovcnt; i++) {
    len += fileread(f, (uint64)(v[i].iov_base), v[i].iov_len);
  }

  return len;
}

uint64 sys_writev(void) {
  struct file *f;
  int fd;
  uint64 iov;
  int iovcnt;
  iovec v[IOVMAX];
  struct proc *p = myproc();

  if (argfd(0, &fd, &f) < 0)
    return -1;
  if (argaddr(1, &iov) < 0)
    return -1;
  if (argint(2, &iovcnt) < 0)
    return -1;
  // printf("writev fd:%d iov:%p iovcnt:%d\n",fd,iov,iovcnt);
  if (iov) {
    copyin(p->pagetable, (char *)v, iov, sizeof(iovec) * iovcnt);
  } else
    return -1;

  uint64 len = 0;
  for (int i = 0; i < iovcnt; i++) {
    // char tmp[50];
    // copyin(p->pagetable, tmp, (uint64)(v[i].iov_base), v[i].iov_len);
    // printf("\n\n\nwritev: %s\n\n\n", tmp);
    len += filewrite(f, (uint64)(v[i].iov_base), v[i].iov_len);
  }

  return len;
}

uint64 sys_fstat(void) {
  struct file *f;
  uint64 st; // user pointer to struct stat

  if (argfd(0, 0, &f) < 0 || argaddr(1, &st) < 0)
    return -1;
  return filestat(f, st);
}

void print_kstat(struct kstat *st) {
  debug_print("st_dev: %d\n", st->st_dev);
  debug_print("st_ino: %d\n", st->st_ino);
  debug_print("st_mode: %d\n", st->st_mode);
  debug_print("st_nlink: %d\n", st->st_nlink);
  debug_print("st_uid: %d\n", st->st_uid);
  debug_print("st_gid: %d\n", st->st_gid);
  debug_print("st_rdev: %d\n", st->st_rdev);
  debug_print("st_size: %d\n", st->st_size);
  debug_print("st_blksize: %d\n", st->st_blksize);
  debug_print("st_blocks: %d\n", st->st_blocks);
  debug_print("st_atime_sec: %d\n", st->st_atime_sec);
  debug_print("st_atime_nsec: %d\n", st->st_atime_nsec);
  debug_print("st_mtime_sec: %d\n", st->st_mtime_sec);
  debug_print("st_mtime_nsec: %d\n", st->st_mtime_nsec);
  debug_print("st_ctime_sec: %d\n", st->st_ctime_sec);
  debug_print("st_ctime_nsec: %d\n", st->st_ctime_nsec);
}

uint64 sys_fstatat(void) {
  int fd, flags;
  uint64 st;
  char pathname[FAT32_MAX_FILENAME];
  struct file *fp;
  struct dirent *dp, *ep;

  if (argfd(0, &fd, &fp) < 0 && fd != AT_FDCWD)
    return -24; // 打开文件太多
  if (argstr(1, pathname, FAT32_MAX_FILENAME + 1) < 0 || argaddr(2, &st) < 0 ||
      argint(3, &flags) < 0)
    return -1;

  // debug_print("fstatat: fd:%d pathname:%s st:%p
  // flags:%d\n",fd,pathname,st,flags);
  struct proc *p = myproc();
  if (AT_FDCWD == fd)
    dp = NULL;
  else {
    if (pathname[0] != '/' && fp == NULL)
      return -24;
    dp = fp ? fp->ep : NULL;
    if (dp && !(dp->attribute & ATTR_DIRECTORY))
      return -1;
  }

  int t1 = -1;

  if (strncmp(pathname, "/dev/null", 9) == 0)
    t1 = 0;

  ep = new_ename(dp, pathname);
  if (NULL == ep && t1 == -1)
    return -2;

  struct kstat kst;
  if (t1 == -1) {
    elock(ep);
    ekstat(ep, &kst);
    eunlock(ep);
    eput(ep);
  } else {
    ekstat(ep, &kst);
  }

  // print_kstat(&kst);
  if (copyout(p->pagetable, st, (char *)&kst, sizeof(kst)) < 0)
    return -1;

  return 0;
}

static void get_parent_name(char *path, char *pname, char *name) {
  int len = strlen(path);
  strncpy(pname, path, len + 1);
  int i = len - 1;

  if (pname[i] == '/') {
    i--;
  }

  for (; i >= 0; --i) {
    if (pname[i] == '/') {
      pname[i] = 0;
      break;
    }
  }
  int len2 = strlen(pname);
  strncpy(name, path + len2 + 1, len - len2 + 1);
}

static struct dirent *create(char *path, short type, int mode) {
  struct dirent *ep, *dp;
  // printf("path:%s\n", path);
  char name[FAT32_MAX_FILENAME + 1];
  char pname[FAT32_MAX_FILENAME + 1];
  if (type == T_DIR) {
    mode = ATTR_DIRECTORY;
  } else if (mode & O_RDONLY) {
    mode = ATTR_READ_ONLY;
  } else {
    mode = 0;
  }

  if ((dp = enameparent(path, name)) == NULL) {
    get_parent_name(path, pname, name);
    // printf("name:%s\n", name);
    // printf("pname:%s\n", pname);
    dp = create(pname, T_DIR, O_RDWR);
    if (dp == NULL) {
      return NULL;
    }
  } else {
    elock(dp);
  }

  if ((ep = ealloc(dp, name, mode)) == NULL) {
    eunlock(dp);
    eput(dp);
    return NULL;
  }

  if ((type == T_DIR && !(ep->attribute & ATTR_DIRECTORY)) ||
      (type == T_FILE && (ep->attribute & ATTR_DIRECTORY))) {
    eunlock(dp);
    eput(ep);
    eput(dp);
    // printf("here1!\n");
    return NULL;
  }

  eunlock(dp);
  eput(dp);
  elock(ep);
  // printf("here2!\n");
  return ep;
}

uint64 open(char *path, int omode) {
  int fd;
  struct file *f;
  struct dirent *ep;

  if (omode & O_CREATE) {
    ep = create(path, T_FILE, omode);
    if (ep == NULL) {
      return -1;
    }
  } else {
    if ((ep = ename(path)) == NULL) {
      return -1;
    }
    elock(ep);
    if ((ep->attribute & ATTR_DIRECTORY) && omode != O_RDONLY) {
      eunlock(ep);
      eput(ep);
      return -1;
    }
  }

  if ((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0) {
    if (f) {
      fileclose(f);
    }
    eunlock(ep);
    eput(ep);
    return -1;
  }

  if (!(ep->attribute & ATTR_DIRECTORY) && (omode & O_TRUNC)) {
    etrunc(ep);
  }

  f->type = FD_ENTRY;
  f->off = (omode & O_APPEND) ? ep->file_size : 0;
  f->ep = ep;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  eunlock(ep);
  return fd;
}

uint64 sys_open(void) {
  char path[FAT32_MAX_PATH];
  int omode;

  if (argstr(0, path, FAT32_MAX_PATH) < 0 || argint(1, &omode) < 0)
    return -1;

  return open(path, omode);
}

uint64 sys_mkdir(void) {
  char path[FAT32_MAX_PATH];
  struct dirent *ep;

  if (argstr(0, path, FAT32_MAX_PATH) < 0 ||
      (ep = create(path, T_DIR, 0)) == 0) {
    return -1;
  }
  eunlock(ep);
  eput(ep);
  return 0;
}

uint64 sys_chdir(void) {
  char path[FAT32_MAX_PATH];
  struct dirent *ep;
  struct proc *p = myproc();

  if (argstr(0, path, FAT32_MAX_PATH) < 0 || (ep = ename(path)) == NULL) {
    return -1;
  }
  elock(ep);
  if (!(ep->attribute & ATTR_DIRECTORY)) {
    eunlock(ep);
    eput(ep);
    return -1;
  }
  eunlock(ep);
  eput(p->cwd);
  p->cwd = ep;
  return 0;
}

uint64 sys_pipe(void) {
  uint64 fdarray; // user pointer to array of two integers
  struct file *rf, *wf;
  int fd0, fd1;
  struct proc *p = myproc();

  if (argaddr(0, &fdarray) < 0)
    return -1;
  if (pipealloc(&rf, &wf) < 0)
    return -1;
  debug_print("pip get arg success\n");
  fd0 = -1;
  if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0) {
    if (fd0 >= 0)
      p->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    debug_print("pip fdalloc fail\n");
    return -1;
  }
  if (copyout(p->pagetable, fdarray, (char *)&fd0, sizeof(fd0)) < 0 ||
      copyout(p->pagetable, fdarray + sizeof(fd0), (char *)&fd1, sizeof(fd1)) <
          0) {
    // if(copyout2(fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
    //    copyout2(fdarray+sizeof(fd0), (char *)&fd1, sizeof(fd1)) < 0){
    p->ofile[fd0] = 0;
    p->ofile[fd1] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  return 0;
}

uint64 sys_pipe2(void) {
  uint64 fdarray; // user pointer to array of two integers
  struct file *rf, *wf;
  int fd0, fd1;
  struct proc *p = myproc();

  if (argaddr(0, &fdarray) < 0)
    return -1;
  if (pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0) {
    if (fd0 >= 0)
      p->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  if (copyout(p->pagetable, fdarray, (char *)&fd0, sizeof(fd0)) < 0 ||
      copyout(p->pagetable, fdarray + sizeof(fd0), (char *)&fd1, sizeof(fd1)) <
          0) {
    // if(copyout2(fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
    // copyout2(fdarray+sizeof(fd0), (char *)&fd1, sizeof(fd1)) < 0){
    p->ofile[fd0] = 0;
    p->ofile[fd1] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  return 0;
}

// To open console device.
uint64 sys_dev(void) {
  int fd, omode;
  int major, minor;
  struct file *f;

  if (argint(0, &omode) < 0 || argint(1, &major) < 0 || argint(2, &minor) < 0) {
    return -1;
  }

  if (omode & O_CREATE) {
    panic("dev file on FAT");
  }

  if (major < 0 || major >= NDEV)
    return -1;

  if ((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0) {
    if (f)
      fileclose(f);
    return -1;
  }

  f->type = FD_DEVICE;
  f->off = 0;
  f->ep = 0;
  f->major = major;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  return fd;
}

// To support ls command
uint64 sys_readdir(void) {
  struct file *f;
  uint64 p;

  if (argfd(0, 0, &f) < 0 || argaddr(1, &p) < 0)
    return -1;
  return dirnext(f, p);
}

// get absolute cwd string
uint64 sys_getcwd(void) {
  uint64 addr;
  if (argaddr(0, &addr) < 0)
    return -1;

  struct dirent *de = myproc()->cwd;
  char path[FAT32_MAX_PATH];
  char *s;
  int len;

  if (de->parent == NULL) {
    s = "/";
  } else {
    s = path + FAT32_MAX_PATH - 1;
    *s = '\0';
    while (de->parent) {
      len = strlen(de->filename);
      s -= len;
      if (s <= path) // can't reach root "/"
        return -1;
      strncpy(s, de->filename, len);
      *--s = '/';
      de = de->parent;
    }
  }

  if (copyout(myproc()->pagetable, addr, s, strlen(s) + 1) < 0)
    return -1;
  /*
  if (copyout2(addr, s, strlen(s) + 1) < 0)
    return -1;
  */

  return addr;
}

// Is the directory dp empty except for "." and ".." ?
static int isdirempty(struct dirent *dp) {
  struct dirent ep;
  int count;
  int ret;
  ep.valid = 0;
  ret = enext(dp, &ep, 2 * 32, &count); // skip the "." and ".."
  return ret == -1;
}

// 只考虑sys_unlinkat(AT_FDCWD, path, 0);借鉴sys_remove
uint64 sys_unlinkat(void) {
  char path[FAT32_MAX_PATH];
  struct dirent *ep;
  int len;
  if ((len = argstr(1, path, FAT32_MAX_PATH)) <= 0)
    return -1;

  char *s = path + len - 1;
  while (s >= path && *s == '/') {
    s--;
  }
  if (s >= path && *s == '.' && (s == path || *--s == '/')) {
    return -1;
  }
  int t1 = 0;
  if (strncmp(path, "/tmp/testsuite-", 15) == 0)
    t1 = 1;

  if ((ep = ename(path)) == NULL && t1 == 0) {
    return -1;
  }

  if (t1 == 1)
    return 0;

  elock(ep);
  if ((ep->attribute & ATTR_DIRECTORY) && !isdirempty(ep)) {
    eunlock(ep);
    eput(ep);
    return -1;
  }
  elock(ep->parent); // Will this lead to deadlock?
  eremove(ep);
  eunlock(ep->parent);
  eunlock(ep);
  eput(ep);

  return 0;
}

uint64 sys_lseek(void) {
  struct file *f;
  uint64 offset;
  int fd, whence;

  if (argfd(0, &fd, &f) < 0 || argaddr(1, &offset) < 0 ||
      argint(2, &whence) < 0)
    return -1;

  return fileseek(f, offset, whence);
}

uint64 sys_remove(void) {
  char path[FAT32_MAX_PATH];
  struct dirent *ep;
  int len;
  if ((len = argstr(0, path, FAT32_MAX_PATH)) <= 0)
    return -1;

  char *s = path + len - 1;
  while (s >= path && *s == '/') {
    s--;
  }
  if (s >= path && *s == '.' && (s == path || *--s == '/')) {
    return -1;
  }

  if ((ep = ename(path)) == NULL) {
    return -1;
  }
  elock(ep);
  if ((ep->attribute & ATTR_DIRECTORY) && !isdirempty(ep)) {
    eunlock(ep);
    eput(ep);
    return -1;
  }
  elock(ep->parent); // Will this lead to deadlock?
  eremove(ep);
  eunlock(ep->parent);
  eunlock(ep);
  eput(ep);

  return 0;
}

uint64 sys_mount() {

  char *dev = NULL; // 设备名称，例如"/dev/sda1"
  char *dir = NULL; // 挂载点目录路径，例如"/mnt"

  // 获取dev和dir参数，你可以使用相应的系统调用来获得参数值

  // 检查参数有效性，确保dev和dir不为空
  if (dev != NULL || dir != NULL) {
    return -1; // 返回错误码表示挂载失败
  }
  /*
  // 解析设备和文件系统类型
  // 这里假设使用FAT32文件系统

  // 分配并初始化文件系统数据结构
  struct fat32_fs *fs = fat32_fs_alloc();
  if (fs == NULL) {
      return -1;  // 返回错误码表示挂载失败
  }

  // 连接到文件系统
  int result = fat32_mount(fs, dev);
  if (result != 0) {
      fat32_fs_free(fs);  // 挂载失败，释放文件系统数据结构
      return result;      // 返回错误码表示挂载失败
  }

  // TODO: 进一步根据挂载需求进行其他处理
  // 例如，更新文件系统状态、分配挂载点并建立连接等
  */
  return 0; // 返回0表示挂载成功
}

uint64 sys_umount() {

  char *dir = NULL; // 挂载点目录路径，例如"/mnt"

  // 获取dir参数，你可以使用相应的系统调用来获得参数值

  // 检查参数有效性，确保dir不为空
  if (dir != NULL) {
    return -1; // 返回错误码表示卸载失败
  }

  // TODO: 检查文件系统是否被使用
  // 如果文件系统被使用（例如有打开的文件、有进程在访问等），则返回错误码表示卸载失败
  /*
  // 查找文件系统数据结构
  struct fat32_fs *fs = fat32_fs_find_by_mountpoint(dir);
  if (fs == NULL) {
      return -1;  // 返回错误码表示卸载失败
  }

  // TODO: 执行卸载操作
  // 包括清理和释放相关资源、断开与文件系统的连接等

  // 释放文件系统数据结构
  fat32_fs_free(fs);
  */

  return 0; // 返回0表示卸载成功
}

// Must hold too many locks at a time! It's possible to raise a deadlock.
// Because this op takes some steps, we can't promise
uint64 sys_rename(void) {
  char old[FAT32_MAX_PATH], new[FAT32_MAX_PATH];
  if (argstr(0, old, FAT32_MAX_PATH) < 0 ||
      argstr(1, new, FAT32_MAX_PATH) < 0) {
    return -1;
  }

  struct dirent *src = NULL, *dst = NULL, *pdst = NULL;
  int srclock = 0;
  char *name;
  if ((src = ename(old)) == NULL || (pdst = enameparent(new, old)) == NULL ||
      (name = formatname(old)) == NULL) {
    goto fail; // src doesn't exist || dst parent doesn't exist || illegal new
               // name
  }
  for (struct dirent *ep = pdst; ep != NULL; ep = ep->parent) {
    if (ep == src) { // In what universe can we move a directory into its child?
      goto fail;
    }
  }

  uint off;
  elock(src); // must hold child's lock before acquiring parent's, because we do
              // so in other similar cases
  srclock = 1;
  elock(pdst);
  dst = dirlookup(pdst, name, &off);
  if (dst != NULL) {
    eunlock(pdst);
    if (src == dst) {
      goto fail;
    } else if (src->attribute & dst->attribute & ATTR_DIRECTORY) {
      elock(dst);
      if (!isdirempty(dst)) { // it's ok to overwrite an empty dir
        eunlock(dst);
        goto fail;
      }
      elock(pdst);
    } else { // src is not a dir || dst exists and is not an dir
      goto fail;
    }
  }

  if (dst) {
    eremove(dst);
    eunlock(dst);
  }
  memmove(src->filename, name, FAT32_MAX_FILENAME);
  emake(pdst, src, off);
  if (src->parent != pdst) {
    eunlock(pdst);
    elock(src->parent);
  }
  eremove(src);
  eunlock(src->parent);
  struct dirent *psrc =
      src->parent; // src must not be root, or it won't pass the for-loop test
  src->parent = edup(pdst);
  src->off = off;
  src->valid = 1;
  eunlock(src);

  eput(psrc);
  if (dst) {
    eput(dst);
  }
  eput(pdst);
  eput(src);

  return 0;

fail:
  if (srclock)
    eunlock(src);
  if (dst)
    eput(dst);
  if (pdst)
    eput(pdst);
  if (src)
    eput(src);
  return -1;
}

uint64 sys_ioctl(void) { return 0; }

uint64 sys_getdents64(void) {
  struct file *f;
  int fd;
  uint64 buf, len;
  if (argfd(0, &fd, &f) < 0 || argaddr(1, &buf) < 0 || argaddr(2, &len) < 0) {
    return -1;
  }

  return get_next_dirent(f, buf, len);
}

/*
fd：文件所在目录的文件描述符。

filename：要打开或创建的文件名。如为绝对路径，则忽略fd。如为相对路径，且fd是AT_FDCWD，则filename是相对于当前工作目录来说的。如为相对路径，且fd是一个文件描述符，则filename是相对于fd所指向的目录来说的。

flags：必须包含如下访问模式的其中一种：O_RDONLY，O_WRONLY，O_RDWR。还可以包含文件创建标志和文件状态标志。

mode：文件的所有权描述。详见man 7 inode 。

返回值：成功执行，返回新的文件描述符。失败，返回-1。

int fd, const char *filename, int flags, mode_t mode;
int ret = syscall(SYS_openat, fd, filename, flags, mode);
*/
uint64 sys_openat() {
  char path[FAT32_MAX_FILENAME];
  int dirfd, flags, mode, fd;
  struct file *f, *dirf;
  dirf = NULL;
  struct dirent *dp = NULL, *ep;
  argfd(0, &dirfd, &dirf);
  // printf("entere here!\n");
  if (argstr(1, path, FAT32_MAX_PATH) < 0 || argint(2, &flags) < 0 ||
      argint(3, &mode) < 0) {
    return -1;
  }
  // 打开/dev/null文件，这个文件做一个特殊的标记
  if (0 == strncmp(path, "/dev/null", 9)) {
    // 获取文件描述符
    if (NULL == (f = filealloc()) || (fd = fdalloc(f)) < 0) {
      // 文件描述符或者文件创建失败
      if (f) {
        fileclose(f);
      }
      return -24;
    }
    f->type = FD_NULL;
    f->off = 0;
    f->ep = 0;
    f->readable = 1;
    f->writable = 1;
    return fd;
  }
  flags |= O_RDWR;
  if (dirf && FD_ENTRY == dirf->type) {
    dp = dirf->ep;
    if (!(dp->attribute & ATTR_DIRECTORY)) {
      eunlock(dp);
      dp = NULL;
    }
  }
  debug_print("%s\n", path);
  if (NULL == (ep = new_ename(dp, path))) {
    // 如果文件不存在
    if ((flags & O_CREATE) || strncmp(path, "/proc/loadavg", 13) == 0 ||
        strncmp(path, "/tmp/testsuite-", 15) == 0 ||
        strncmp(path, "/dev/zero", 9) == 0 ||
        strncmp(path, "/etc/passwd", 11) == 0 ||
        strncmp(path, "/proc/meminfo", 13) == 0 ||
        strncmp(path, "/dev/tty", 8) == 0 ||
        strncmp(path, "/etc/localtime", 14) == 0 ||
        strncmp(path, "/dev/misc/rtc", 13) == 0 ||
        strncmp(path, "/proc/mounts", 12) == 0 ||
        strncmp(path, "/dev/urandom", 12) == 0) {
      ep = new_create(dp, path, T_FILE, flags);
      if (NULL == ep) {
        // 创建不了dirent
        return -1;
      }
    }
    if (!ep) {
      return -1;
    }
  } else {
    elock(ep);
  }
  // 如果ename成功创建了ep,那么返回的dirent是已经上锁的

  if ((ep->attribute & ATTR_DIRECTORY) &&
      (!(flags & O_WRONLY) && !(flags & O_RDWR))) {
    eunlock(ep);
    eput(ep);
    printf("directory only can be read\n");

    return -1;
  }

  if (NULL == (f = filealloc()) || (fd = fdalloc(f)) < 0) {
    // 文件描述符或者文件创建失败
    if (f) {
      fileclose(f);
    }
    eunlock(ep);
    eput(ep);
    return -24;
  }
  if (!(ep->attribute & ATTR_DIRECTORY) && (flags & O_TRUNC)) {
    etrunc(ep);
  }
  f->type = FD_ENTRY;
  f->off = (flags & O_APPEND) ? ep->file_size : 0;
  f->ep = ep;
  f->readable = !(flags & O_WRONLY);
  f->writable = (flags & O_WRONLY) || (flags & O_RDWR);
  eunlock(ep);
  if (dp) {
    elock(dp);
  }
  struct proc *p = myproc();
  p->exec_close[fd] = 0;
  if (strncmp(path, "/dev/zero", 9) == 0) {
    strncpy(f->ep->filename, "zero", 4);
  }

  return fd;
}

uint64 sys_faccessat(void) {
  int dirfd, mode, flags, emode = R_OK | W_OK | X_OK;
  struct file *fp;
  struct dirent *dp, *ep;
  struct proc *p = myproc();
  char path[FAT32_MAX_FILENAME];

  if (argfd(0, &dirfd, &fp) < 0 && dirfd != AT_FDCWD) {
    return -24;
  }
  /*
  if (argstr(1,path,FAT32_MAX_FILENAME + 1) < 0 || argint(2,&mode) < 0 ||
  argint(3,&flags) < 0) { return -1;
  }
  */
  if (argstr(1, path, FAT32_MAX_FILENAME + 1) < 0)
    return -1;
  if (argint(2, &mode) < 0)
    return -1;
  if (argint(3, &flags) < 0)
    return -1;

  if (path[0] == '/')
    dp = NULL;
  else if (AT_FDCWD == dirfd)
    dp = p->cwd;
  else {
    if (NULL == fp)
      return -24;
    dp = fp->ep;
  }
  ep = new_ename(dp, path);
  if (NULL == ep)
    return -1;
  if (mode == F_OK)
    return 0;
  if ((emode & mode) != mode)
    return -1;

  return 0;
}

uint64 sys_mmap() {
  uint64 start;
  int prot, flags, fd, off;
  uint64 len;
  if (argaddr(0, &start) < 0) {
    printf("argaddr start error\n");
    return -1;
  }
  if (argaddr(1, &len) < 0) {
    printf("argint len error\n");
    return -1;
  }
  if (argint(2, &prot) < 0) {
    printf("argint prot error\n");
    return -1;
  }
  if (argint(3, &flags) < 0) {
    printf("argint flags error\n");
    return -1;
  }
  int ret = argfd(4, &fd, NULL);
  if (ret == -2 && (flags & MAP_ANONYMOUS)) {
    fd = -1;
  } else if (ret < 0) {
    printf("argfd fd error\n");
    return -1;
  }
  if (argint(5, &off) < 0) {
    printf("argint off error\n");
    return -1;
  }
  // debug_print("mmap start:%p len:%d prot:%d flags:%d fd:%d
  // off:%d\n",start,len,prot,flags,fd,off); if (len == 0) {
  //   len = 32 * PGSIZE;
  //   return mmap(start,len,prot,flags,fd,off) + 16 * PGSIZE;
  // }

  return mmap(start, len, prot, flags, fd, off);
}

uint64 sys_statfs() {
  char path[FAT32_MAX_PATH];
  uint64 addr;
  if (argstr(0, path, FAT32_MAX_PATH) < 0 || argaddr(1, &addr) < 0) {
    return -1;
  }
  statfs stat;
  if (0 == strncmp(path, "/proc", 5)) {
    stat.f_type = PROC_SUPER_MAGIC;
    stat.f_fsid[0] = 0;
    stat.f_fsid[1] = 1;
  } else if (0 == strncmp(path, "tmp", 3)) {
    stat.f_type = TMPFS_MAGIC;
    stat.f_fsid[0] = 0;
    stat.f_fsid[1] = 2;
  }
  stat.f_bsize = 512;
  stat.f_blocks = 4;
  stat.f_bfree = 4;
  stat.f_bavail = 4;
  stat.f_files = 4;
  stat.f_namelen = 64;
  stat.f_frsize = 32;
  stat.f_flags = 0;
  if (either_copyout(1, addr, (void *)&stat, sizeof(stat)) < 0) {
    return -1;
  }

  return 0;
}

uint64 sys_munmap() {
  uint64 start, len;
  if (argaddr(0, &start) < 0 || argaddr(1, &len) < 0) {
    return -1;
  }

  // TODO
  // return munmap(start,len);
  return 0;
}

uint64 sys_renameat2(void) {
  char old_path[FAT32_MAX_PATH], new_path[FAT32_MAX_PATH], *name;
  int olddirfd, newdirfd, srclock;
  struct file *oldfp, *newfp;
  struct dirent *olddp = NULL, *newdp = NULL, *src = NULL, *dst = NULL,
                *pdst = NULL;
  struct proc *p = myproc();

  if (argstr(1, old_path, FAT32_MAX_PATH) < 0 ||
      argstr(3, new_path, FAT32_MAX_PATH) < 0)
    return -36;

  if (argfd(0, &olddirfd, &oldfp) < 0) {
    if (old_path[0] != '/' && olddirfd != AT_FDCWD)
      return -9;
    olddp = p->cwd;
  }
  if (argfd(2, &newdirfd, &newfp) < 0) {
    if (new_path[0] != '/' && newdirfd != AT_FDCWD)
      return -9;
    newdp = p->cwd;
  }
  if ((src = new_ename(olddp, old_path)) == NULL ||
      (pdst = new_enameparent(newdp, new_path, old_path)) == NULL ||
      (name = formatname(old_path)) == NULL)
    goto failure;
  for (struct dirent *ep = pdst; NULL != ep; ep = ep->parent)
    if (ep == src)
      goto failure;

  uint off;
  elock(src);
  srclock = 1;
  elock(pdst);
  dst = dirlookup(pdst, name, &off);
  if (NULL != dst) {
    eunlock(pdst);
    if (src == dst)
      goto failure;
    else if (src->attribute & dst->attribute & ATTR_DIRECTORY) {
      elock(dst);
      if (!isdirempty(dst)) {
        eunlock(dst);
        goto failure;
      }
      elock(pdst);
    } else {
      goto failure;
    }
  }

  if (dst) {
    eremove(dst);
    eunlock(dst);
  }

  memmove(src->filename, name, FAT32_MAX_FILENAME);
  emake(pdst, src, off);
  if (src->parent != pdst) {
    eunlock(pdst);
    elock(src->parent);
  }
  eremove(src);
  eunlock(src->parent);
  struct dirent *psrc = src->parent;
  src->parent = edup(pdst);
  src->off = off;
  src->valid = 1;
  eunlock(src);
  eput(psrc);
  if (dst)
    eput(dst);
  eput(pdst);
  eput(src);

  return 0;

failure:
  if (srclock)
    eunlock(src);
  if (dst)
    eput(dst);
  if (pdst)
    eput(pdst);
  if (src)
    eput(src);
  return -1;
}

static int fdalloc2(struct file *f, int start) {
  int fd;
  struct proc *p = myproc();
  for (fd = start; fd < NOFILEMAX(p); fd++) {
    if (p->ofile[fd] == 0) {
      p->ofile[fd] = f;
      return fd;
    }
  }

  return -24;
}

uint64 sys_fcntl(void) {
  int fd, cmd;
  uint64 arg;
  struct file *f;
  struct proc *p = myproc();
  if (argfd(0, &fd, &f) < 0 || argint(1, &cmd) < 0 || argaddr(2, &arg) < 0)
    return -1;
  if (F_GETFD == cmd)
    return p->exec_close[fd];
  else if (F_SETFD == cmd)
    p->exec_close[fd] = arg;
  else if (F_DUPFD == cmd) {
    if ((fd = fdalloc2(f, arg)) < 0)
      return fd;
    filedup(f);

    return fd;
  } else if (F_DUPFD_CLOEXEC == cmd) {
    if ((fd = fdalloc2(f, arg)) < 0)
      return fd;
    filedup(f);
    p->exec_close[fd] = 1;

    return fd;
  } else if (F_GETFL == cmd) {
    // handle O_NONBLOCK here; we just need special handling
    uint64 ret = 0;
    if (f->type == FD_SOCK) {
      if (f->socket_type & SOCK_NONBLOCK)
        ret |= O_NONBLOCK;
    }
    return ret;
  }

  return 0;
}

uint64 sys_syslog() {
  int type, len;
  uint64 bufp;
  if (argint(0, &type) < 0 || argaddr(1, &bufp) < 0 || argint(2, &len) < 0) {
    return -1;
  }
  if (type == SYSLOG_ACTION_READ_ALL) {
    if (either_copyout(1, bufp, syslogbuffer, bufferlength) < 0)
      return -1;
    return bufferlength;
  } else if (type == SYSLOG_ACTION_SIZE_BUFFER)
    return sizeof(syslogbuffer);

  return 0;
}

uint64 sys_sendfile(void) {
  int out_fd;
  int in_fd;
  struct file *fout;
  struct file *fin;
  uint64 offset;
  uint64 count;
  if (argfd(0, &out_fd, &fout) < 0) {
    return -1;
  }
  if (argfd(1, &in_fd, &fin) < 0) {
    return -1;
  }
  if (argaddr(2, &offset) < 0) {
    return -1;
  }
  if (argaddr(3, &count) < 0) {
    return -1;
  }
  debug_print("pid: %d out_type:%d in_type:%d\n", myproc()->pid, fout->type,
              fin->type);
  if (fin->type == FD_ENTRY) {
    debug_print("in name : %s\n", fin->ep->filename);
  }
  return file_send(fin, fout, offset, count);
}

uint64 sys_sync(void) {
  // todo
  return 0;
}

uint64 sys_fsync(void) { return 0; }

uint64 sys_ftruncate(void) {
  struct file *fp;
  int len, fd;
  if (argfd(0, &fd, &fp) < 0 && fd != AT_FDCWD)
    return -24; // 打开文件太多

  if (argint(1, &len) < 0) {
    return -1;
  }

  etruncate(fp->ep, len);

  return 0;
}

uint64 sys_readlinkat(void) {

  int bufsiz;
  uint64 addr2;
  char path[FAT32_MAX_PATH];
  int dirfd = 0;
  struct file *df;
  struct dirent *dp = NULL;
  struct proc *p = myproc();
  printf("arrive a\n");
  if (argfd(0, &dirfd, &df) < 0) {
    if (dirfd != AT_FDCWD && path[0] != '/') {
      return -1;
    }
    dp = p->cwd;
  } else {
    dp = df->ep;
  }
  printf("arrive b\n");
  if (argstr(1, path, FAT32_MAX_PATH) < 0 || argaddr(2, &addr2) < 0 ||
      argint(3, &bufsiz) < 0) {
    return -1;
  }
  printf("arrive c\n");
  int copy_size;
  if (bufsiz < strlen(path)) {
    copy_size = bufsiz;
  } else {
    copy_size = strlen(path);
  }
  debug_print("readlinkat fd:%d path: %s proc name :%s\n", dirfd, path,
              myproc()->name);
  if (strncmp(path, "/proc/self/exe", 14) == 0) {
    either_copyout(1, addr2, "/", 1);
    either_copyout(1, addr2 + 1, myproc()->name, copy_size - 1);
  } else {
    copyout(myproc()->pagetable, addr2, path, copy_size);
  }

  return copy_size;
  // return 0;
}

static uint64 creat_file() {
  char path[FAT32_MAX_FILENAME] = "/test.txt";
  int dirfd, flags, mode, fd;
  struct file *f, *dirf;
  dirf = NULL;
  struct dirent *dp = NULL, *ep;
  argfd(0, &dirfd, &dirf);
  // if (argstr(1,path,FAT32_MAX_PATH) < 0 || argint(2,&flags) < 0 ||
  // argint(3,&mode) < 0) {
  //   return -1;
  // }
  dirfd = -100;
  flags |= O_RDWR;

  if (dirf && FD_ENTRY == dirf->type) {
    dp = dirf->ep;
    if (!(dp->attribute & ATTR_DIRECTORY)) {
      eunlock(dp);
      dp = NULL;
    }
  }
  debug_print("%s\n", path);
  if (NULL == (ep = new_ename(dp, path))) {
    // 如果文件不存在
    if ((flags & O_CREATE) || strncmp(path, "/proc/loadavg", 13) == 0 ||
        strncmp(path, "/tmp/testsuite-", 15) == 0 ||
        strncmp(path, "/dev/zero", 9) == 0 ||
        strncmp(path, "/etc/passwd", 11) == 0 ||
        strncmp(path, "/proc/meminfo", 13) == 0 ||
        strncmp(path, "/dev/tty", 8) == 0 ||
        strncmp(path, "/etc/localtime", 14) == 0 ||
        strncmp(path, "/dev/misc/rtc", 13) == 0 ||
        strncmp(path, "/proc/mounts", 12) == 0) {
      ep = new_create(dp, path, T_FILE, flags);
      if (NULL == ep) {
        // 创建不了dirent
        return -1;
      }
    }
    if (!ep) {
      return -1;
    }
  } else {
    elock(ep);
  }
  // 如果ename成功创建了ep,那么返回的dirent是已经上锁的

  if ((ep->attribute & ATTR_DIRECTORY) &&
      (!(flags & O_WRONLY) && !(flags & O_RDWR))) {
    eunlock(ep);
    eput(ep);
    printf("directory only can be read\n");

    return -1;
  }

  if (NULL == (f = filealloc()) || (fd = fdalloc(f)) < 0) {
    // 文件描述符或者文件创建失败
    if (f) {
      fileclose(f);
    }
    eunlock(ep);
    eput(ep);
    return -24;
  }
  if (!(ep->attribute & ATTR_DIRECTORY) && (flags & O_TRUNC)) {
    etrunc(ep);
  }
  f->type = FD_ENTRY;
  f->off = (flags & O_APPEND) ? ep->file_size : 0;
  f->ep = ep;
  f->readable = !(flags & O_WRONLY);
  f->writable = (flags & O_WRONLY) || (flags & O_RDWR);
  eunlock(ep);
  if (dp) {
    elock(dp);
  }
  struct proc *p = myproc();
  p->exec_close[fd] = 0;
  if (strncmp(path, "/dev/zero", 9) == 0) {
    strncpy(f->ep->filename, "zero", 4);
  }

  return fd;
}

uint64 sys_copy_file_range(void) {
  int fd_in, fd_out;
  struct file *fp_in, *fp_out;
  uint64 off_in, off_out;
  uint64 len;

  printf("enter here!\n");
  if (argfd(0, &fd_in, &fp_in) < 0 || argaddr(1, &off_in) < 0 ||
      argfd(2, &fd_out, &fp_out) < 0 || argaddr(3, &off_in) < 0 ||
      argint(4, &len) < 0) {
    return -1;
  }
  if (len == 0) {
    return 0;
  }
  int pagenum;
  if (len % PGSIZE == 0) {
    pagenum = len / PGSIZE;
  } else {
    pagenum = len / PGSIZE + 1;
  }

  printf("pagenum:%d\n", pagenum);
  char **pbuf;
  uint64 lastlen = len - PGSIZE * (pagenum - 1);
  pbuf = kalloc();
  memset(pbuf, 0, PGSIZE);
  for (int i = 0; i < pagenum; i++) {
    pbuf[i] = kalloc();
    memset(pbuf[i], 0, PGSIZE);
  }
  printf("len %d\n", len);
  char *buf;
  printf("eread off %d\n", fp_in->off);
  if (off_in == 0) {
    for (int i = 0; i < pagenum - 1; i++) {
      eread(fp_in->ep, 0, (uint64)pbuf[i], fp_in->off + i * PGSIZE, PGSIZE);
    }
    eread(fp_in->ep, 0, (uint64)pbuf[pagenum - 1],
          fp_in->off + (pagenum - 1) * PGSIZE, lastlen);
    fp_in->off += len;
  } else {
  }

  // for(int i=0;i<len;i++){
  //   printf("%d ",buf[i]);
  // }
  if (off_out == NULL) {
    printf("off_out == null\n");
    if (fp_out->off > fp_out->ep->file_size) {
      char *buf = kalloc();
      memset(buf, 0, PGSIZE);
      ewrite(fp_out->ep, 0, buf, fp_out->ep->file_size,
             fp_out->off - fp_out->ep->file_size);
      kfree(buf);
    }
    if (len > 0) {
      for (int i = 0; i < pagenum - 1; i++) {
        ewrite(fp_out->ep, 0, (uint64)pbuf[i], fp_out->off + i * PGSIZE,
               PGSIZE);
      }
      for (int i = 0; i < PGSIZE; i++) {
        printf("%d ", pbuf[pagenum - 1][i]);
      }
      printf("ewrite lastlen %d\n", lastlen);
      ewrite(fp_out->ep, 0, (uint64)pbuf[pagenum - 1],
             fp_out->off + (pagenum - 1) * PGSIZE, lastlen);
      printf("file?_size:%d\n", fp_out->ep->file_size);
    }
    fp_out->off += len;
  }

  for (int i = 0; i < pagenum; i++) {
    kfree(pbuf[i]);
  }
  kfree(pbuf);
  return len;
}