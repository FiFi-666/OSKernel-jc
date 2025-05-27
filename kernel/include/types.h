#ifndef __TYPES_H
#define __TYPES_H

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned short wchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long uint64;
typedef long int64;
typedef unsigned long uintptr_t;
typedef uint64 pde_t;

// #define NULL ((void *)0)
#define NULL 0


#define writed(v, addr)                       \
    {                                         \
        (*(volatile uint32 *)(addr)) = (v); \
    }

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))


// for socket use
typedef uint size_t;
typedef int ssize_t;
// typedef int in_addr_t;
// typedef uint16 in_port_t;
// typedef uint32 socklen_t;
// typedef uint16 sa_family_t;
typedef int64 time_t;
typedef int64 suseconds_t;
#endif
