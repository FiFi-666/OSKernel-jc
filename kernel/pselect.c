#include "include/pselect.h"
#include "include/file.h"
#include "include/param.h"
#include "include/pipe.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/signal.h"
#include "include/socket.h"
#include "include/string.h"
#include "include/syscall.h"
#include "include/timer.h"
#include "include/types.h"
// 后续可移动到util中

/* flags数组每个元素为64bit，可以表示的flag数量为：数组大小*64 */
#define BITS_PER_LONG 64
/* nr位于的，flags数组的元素下标 */
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
/* 那个元素的哪一位 */
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))

static inline int test_bit(unsigned int nr, const volatile uint64 *addr) {
  return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG - 1)));
}

static inline void clear_bit(unsigned int nr, uint64 *addr) {
  // printf("%p", addr[BIT_WORD(nr)]);
  addr[BIT_WORD(nr)] &= (0xffffffff ^ (1 << nr));
  // printf("%p", addr[BIT_WORD(nr)]);
}

uint64 sys_ppoll(void) { return 0; }

uint64 sys_pselect6(void) {
  int nfds;
  uint64 urfds, uwfds, uexfds;
  uint64 utoaddr, usmaddr;
  struct proc *p = myproc();

  argint(0, &nfds);
  argaddr(1, &urfds);
  argaddr(2, &uwfds);
  argaddr(3, &uexfds);
  argaddr(4, &utoaddr);
  argaddr(5, &usmaddr);

  struct fdset rfds, wfds, exfds;
  struct timespec2 timeout;
  __sigset_t sigmask;

  if (urfds && either_copyin((void *)&rfds, 1, urfds, sizeof(struct fdset)) < 0)
    return -1;
  if (uwfds && either_copyin((void *)&wfds, 1, uwfds, sizeof(struct fdset)) < 0)
    return -1;
  if (uexfds &&
      either_copyin((void *)&exfds, 1, uexfds, sizeof(struct fdset)) < 0)
    return -1;
  if (utoaddr &&
      either_copyin((void *)&timeout, 1, utoaddr, sizeof(timeout)) < 0)
    return -1;
  if (usmaddr &&
      either_copyin((void *)&sigmask, 1, usmaddr, sizeof(sigmask)) < 0)
    return -1;

  // int ret = pselect(nfds,
  // 			urfds ? &rfds: NULL,
  // 			uwfds ? &wfds: NULL,
  // 			uexfds ? &exfds: NULL,
  // 			utoaddr ? &timeout : NULL,
  // 			usmaddr ? &sigmask : NULL
  // 		);
  // printf("out pselect!\n");
  int ret = 0;
  if (nfds > FDSET_SIZE) {
    return -1;
  }
  int fds_bytes = ((nfds) + (8) - 1) / (8);

  uint64 expire;
  if (utoaddr) {
    expire = convert_from_timespec2(&timeout);
  } else
    expire = -1;
  uint64 start_time = r_time();
  while (1) {
    // struct timespec2 tt;
    // int flag1 = 0;
    // convert_to_timespec2(r_time(), &tt);
    // if (tt.tv_sec > 0x10c)
    // {
    // 	rfds.fd_bits[0] = 0x10;
    // }

    for (int i = 0; i < nfds && urfds; i++) {
      if (!test_bit(i, &rfds)) {
        continue;
      }
      struct file *f = p->ofile[i];
      // printf("enter read: fd %d, ftype :%d\n", i, f->type);
      switch (f->type) {
      case FD_PIPE:
        if (!f->readable || pipe_empty(f->pipe)) {
          clear_bit(i, &rfds);
          break;
        }
        // printf("arrive here!\n");
      case FD_DEVICE:
      case FD_ENTRY:
      case FD_SOCK:
        if (do_lwip_select(f->socketnum, (struct timeval *)&timeout) > 0) {
          ret++;
        }
        break;

      default:
        break;
      }
    }

    for (int i = 0; i < nfds && uwfds; i++) {
      if (!test_bit(i, &wfds)) {
        continue;
      }
      // printf("enter write: fd %d\n", i);
      struct file *f = p->ofile[i];
      switch (f->type) {
      case FD_PIPE:
        if (!f->writable || pipe_full(f->pipe)) {
          clear_bit(i, &wfds);
          break;
        }
      case FD_DEVICE:
      case FD_ENTRY:
      case FD_SOCK:
        if (do_lwip_select(f->socketnum, (struct timeval *)&timeout) > 0) {
          ret++;
        }
        break;

      default:
        break;
      }
    }

    if (ret) {
      break;
    }

    if (expire == -1) {
      break;
    } else {
      if (r_time() - start_time >= expire) {
        break;
      }
    }
  }

  memset((void *)&exfds, 0, sizeof(struct fdset));

  if (urfds)
    either_copyout(1, urfds, (char *)&rfds, sizeof(struct fdset));
  if (uwfds)
    either_copyout(1, uwfds, (char *)&wfds, sizeof(struct fdset));
  if (uexfds)
    either_copyout(1, uexfds, (char *)&exfds, sizeof(struct fdset));

  //__debug_info("[sys_pselect6] ret = %d\n", ret);

  return ret;
}