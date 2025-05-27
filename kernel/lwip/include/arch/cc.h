#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#define LWIP 1

#define LWIP_NO_INTTYPES_H 1
#define LWIP_NO_CTYPE_H 1
#define LWIP_NO_LIMITS_H 1
#define LWIP_NO_STDDEF_H 1
#define LWIP_NO_STDINT_H 1
#define LWIP_NO_UNISTD_H 1
#define LWIP_NO_STDIO_H 1
#define LWIP_NO_STRING_H 1
#define LWIP_NO_STDLIB_H 1

#define LWIP_TIMEVAL_PRIVATE 0

#include "../../../include/types.h"
#include "../../../include/printf.h"
#include "../../../include/timer.h"


void            panic(char*) __attribute__((noreturn));
void            printf(char*, ...);

typedef uint32 u32_t;
typedef int s32_t;

typedef uint64 u64_t;
typedef int64 s64_t;

typedef uint16 u16_t;
typedef short s16_t;

typedef uint8 u8_t;
typedef char s8_t;

typedef uint64 mem_ptr_t;

typedef int64 ptrdiff_t;

#define PACK_STRUCT_FIELD(x)	x __attribute__((packed))
#define PACK_STRUCT_STRUCT  __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define S16_F	"d"
#define U16_F	"d"
#define X16_F	"x"

#define S32_F	"d"
#define U32_F	"d"
#define X32_F	"x"

#define S64_F	"lld"
#define U64_F	"llu"
#define X64_F	"llx"

#define X8_F   "02" "x"

#define SZT_F   "d"

#define SSIZE_MAX  0x7fffffffL
#define INT_MAX    0x7fffffffL

#define LWIP_PLATFORM_DIAG(x)	printf x
#define LWIP_PLATFORM_ASSERT(x)	panic(x)

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif


static inline u16_t
LWIP_PLATFORM_HTONS(u16_t n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

static inline u32_t
LWIP_PLATFORM_HTONL(u32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

#define LWIP_PROVIDE_ERRNO 1
#define LWIP_ERR_T s16_t
#define LWIP_ERRNO_INCLUDE "../include/errno.h"

#include "rand.h"
#define LWIP_RAND() (u32_t)rand()

#endif