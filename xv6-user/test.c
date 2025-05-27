#include "kernel/include/stat.h"
#include "kernel/include/sysinfo.h"
#include "kernel/include/types.h"
#include "xv6-user/user.h"

int main() {
  // struct sysinfo info;
  // if (sysinfo(&info) < 0) {
  //     printf("sysinfo fail!\n");
  // } else {
  //     printf("memory left: %d KB\n", info.freemem >> 10);
  //     printf("process amount: %d\n", info.nproc);
  // }
  // int fd_in = openat(-100, "/filein.txt", 0x42, 0);
  // int fd_out = openat(-100, "/fileout.txt", 0x42, 0);
  // write(fd_in, "Hello world\n", 12);
  // copy_file_range(fd_in, NULL, fd_out, NULL, 5, 0);
  int fd_in = openat(-100, "/testfile", 0x42, 0);
  write(fd_in, "hello_world\n", 12);
  lseek(fd_in, 5, 0);
  write(fd_in, "wwww", 4);
  char buf[10];
  int len = 2;
  read(fd_in, buf, len);
  for (int i = 0; i < len; i++) {
    printf("%d ", buf[i]);
  }

  // ftruncate(fd_in, 5);
  exit(0);
}
