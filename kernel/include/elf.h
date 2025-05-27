#ifndef __ELF_H
#define __ELF_H
#include "types.h"
// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
  uint magic;  // must equal ELF_MAGIC
  uchar elf[12];
  ushort type;
  ushort machine;
  uint version;
  uint64 entry;
  uint64 phoff;
  uint64 shoff;
  uint flags;
  ushort ehsize;
  ushort phentsize;
  ushort phnum;
  ushort shentsize;
  ushort shnum;
  ushort shstrndx;
};

// Program section header
struct proghdr {
  uint32 type;
  uint32 flags;
  uint64 off;
  uint64 vaddr;
  uint64 paddr;
  uint64 filesz;
  uint64 memsz;
  uint64 align;
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1
#define ELF_PROG_DYNAMIC        2
#define ELF_PROG_INTERP         3
#define ELF_PROG_PHDR           6
#define ELF_PROG_TLS            7
#define ELF_GNU_STACK           0x6474e551
#define ELF_GNU_RELRO           0x6474e552

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

#define AT_NULL		0
#define AT_IGNORE	1
#define AT_EXECFD	2
#define AT_PHDR		3
#define AT_PHENT	4
#define AT_PHNUM	5
#define AT_PAGESZ	6
#define AT_BASE		7
#define AT_FLAGS	8
#define AT_ENTRY	9
#define AT_NOTELF	10
#define AT_UID		11
#define AT_EUID		12
#define AT_GID		13
#define AT_EGID		14
#define AT_PLATFORM	15
#define AT_HWCAP	16
#define AT_CLKTCK	17
#define AT_FPUCW	18
#define AT_DCACHEBSIZE	19
#define AT_ICACHEBSIZE	20
#define AT_UCACHEBSIZE	21
#define AT_IGNOREPPC	22
#define	AT_SECURE	23
#define AT_BASE_PLATFORM 24
#define AT_RANDOM	25
#define AT_HWCAP2	26
#define AT_EXECFN	31
#define AT_SYSINFO	32
#define AT_SYSINFO_EHDR	33
#endif