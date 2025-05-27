#include "include/buf.h"
#include "include/printf.h"
#include "include/spinlock.h"
#include "include/string.h"
#include "include/types.h"

#define NRAMDISKPAGES (sddata_size * BSIZE / PGSIZE)

struct spinlock ramdisklock;
extern uchar sddata_start[];
extern uchar sddata_end[];
char *ramdisk;

void ramdisk_init(void) {
  initlock(&ramdisklock, "ramdisk lock");
  ramdisk = (char *)sddata_start;
  debug_print("ramdiskinit ram start:%p\n", ramdisk);
}

void ramdisk_read(struct buf *b) {
  acquire(&ramdisklock);
  uint sectorno = b->sectorno;

  char *addr = ramdisk + sectorno * BSIZE;
  memmove(b->data, (void *)addr, BSIZE);
  release(&ramdisklock);
}

void ramdisk_write(struct buf *b) {
  acquire(&ramdisklock);
  uint sectorno = b->sectorno;

  char *addr = ramdisk + sectorno * BSIZE;
  memmove((void *)addr, b->data, BSIZE);
  release(&ramdisklock);
}