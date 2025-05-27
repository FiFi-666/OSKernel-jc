#ifndef __DEFS_H
#define __DEFS_H

#include "types.h"

struct buf;
struct context;
struct dirent;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;

// exec.c
int             exec(char*, char**);

// swtch.S
void            swtch(struct context*, struct context*);


// trap.c
extern uint     ticks;
void            trapinit(void);
void            trapinithart(void);
extern struct spinlock tickslock;
void            usertrapret(void);
void            device_init(unsigned long, uint64);

// uart.c
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);
void            uartputc_sync(int);
int             uartgetc(void);

// virtio_disk.c
void            virtio_disk_init(void);
void            virtio_disk_rw(struct buf *b, int write);
void            virtio_disk_intr(void);

// plic.c
void            plicinit(void);
void            plicinithart(void);
int             plic_claim(void);
void            plic_complete(int);

// logo.c
void            print_logo(void);

// test.c
void            test_kalloc(void);
void            test_vm(unsigned long);
void            test_sdcard(void);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
#endif