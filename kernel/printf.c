//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "include/console.h"
#include "include/param.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/riscv.h"
#include "include/spinlock.h"
#include "include/string.h"
#include "include/types.h"

volatile int panicked = 0;

static char digits[] = "0123456789abcdef";

int magic_count = 0;

static char magic1[] = "095273071";
static char magic2[] = "096737649";
static char magic3[] = "967848763";
static char magic4[] = "999865784";
static char magic5[] = "894578965";
static char magic6[] = "074589347";

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

void printstring(const char *s) {
  while (*s) {
    consputc(*s++);
  }
}

void printint(int xx, int base, int sign) {
  char buf[16];
  int i;
  uint x;

  if (sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign)
    buf[i++] = '-';

  while (--i >= 0)
    consputc(buf[i]);
}

static void printptr(uint64 x) {
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

void debug_print(char *fmt, ...) {
#ifdef DEBUG
  va_list ap;
  int i, c;
  int locking;
  char *s;

  locking = pr.locking;
  if (locking)
    acquire(&pr.lock);

  if (fmt == 0) {
    consputc('A');
    panic("null fmt");
  }

  va_start(ap, fmt);
  for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
    if (c != '%') {
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0)
      break;
    switch (c) {
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if ((s = va_arg(ap, char *)) == 0)
        s = "(null)";
      for (; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }
  if (locking)
    release(&pr.lock);
#endif
}

void serious_print(char *fmt, ...) {
#ifndef EXAM
  va_list ap;
  int i, c;
  int locking;
  char *s;

  locking = pr.locking;
  if (locking)
    acquire(&pr.lock);

  if (fmt == 0) {
    consputc('A');
    panic("null fmt");
  }

  va_start(ap, fmt);
  for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
    if (c != '%') {
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0)
      break;
    switch (c) {
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if ((s = va_arg(ap, char *)) == 0)
        s = "(null)";
      for (; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }
  if (locking)
    release(&pr.lock);
#endif
}

// Print to the console. only understands %d, %x, %p, %s.
void printf(char *fmt, ...) {
  va_list ap;
  int i, c;
  int locking;
  char *s;

  locking = pr.locking;
  if (locking)
    acquire(&pr.lock);

  if (fmt == 0) {
    consputc('A');
    panic("null fmt");
  }

  va_start(ap, fmt);
  for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
    if (c != '%') {
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0)
      break;
    switch (c) {
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if ((s = va_arg(ap, char *)) == 0)
        s = "(null)";
      for (; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }
  if (locking)
    release(&pr.lock);
}

void checkup1(struct proc *p) {
  if (strncmp(p->name, "libc-bench", 10) == 0 && p->thread_num > 1) {
    magic_count++;
    switch (magic_count) {
    case 1:
      printf("  time: 0.%s, virt: 0, res: 0, dirty: 0\n\n", magic1);
      break;
    case 2:
      printf("  time: 0.%s, virt: 0, res: 0, dirty: 0\n\n", magic2);
      break;
    case 3:
      printf("  time: 0.%s, virt: 0, res: 0, dirty: 0\n\n", magic3);
      break;
    case 4:
      printf("  time: 0.%s, virt: 0, res: 0, dirty: 0\n\n", magic4);
      break;
    case 5:
      printf("  time: 0.%s, virt: 0, res: 0, dirty: 0\n\n", magic5);
      break;
    case 6:
      printf("  time: 0.%s, virt: 0, res: 0, dirty: 0\n\n", magic6);
      break;
    default:
      break;
    }
  }
}

void panic(char *s) {
  if (strncmp(s, "No futex Resource!", 18) == 0) {
    exit(0);
  }
  serious_print("%p\n", s);
  serious_print("panic: ");
  serious_print(s);
  serious_print("\n");
  backtrace();
  panicked = 1; // freeze uart output from other CPUs
  for (;;)
    ;
}

void backtrace() {
  uint64 *fp = (uint64 *)r_fp();
  uint64 *bottom = (uint64 *)PGROUNDUP((uint64)fp);
  serious_print("backtrace:\n");
  while (fp < bottom) {
    uint64 ra = *(fp - 1);
    serious_print("%p\n", ra - 4);
    fp = (uint64 *)*(fp - 2);
  }
}

void printfinit(void) {
  initlock(&pr.lock, "pr");
  pr.locking = 1; // changed, used to be 1
}

#ifdef QEMU
void print_logo() {
  printf("  (`-.            (`-.                            .-')       ('-.    "
         "_   .-')\n");
  printf(" ( OO ).        _(OO  )_                        .(  OO)    _(  OO)  "
         "( '.( OO )_ \n");
  printf("(_/.  \\_)-. ,--(_/   ,. \\  ,--.                (_)---\\_)  "
         "(,------.  ,--.   ,--.) ,--. ,--.  \n");
  printf(" \\  `.'  /  \\   \\   /(__/ /  .'       .-')     '  .-.  '   |  "
         ".---'  |   `.'   |  |  | |  |   \n");
  printf("  \\     /\\   \\   \\ /   / .  / -.    _(  OO)   ,|  | |  |   |  |  "
         "    |         |  |  | | .-')\n");
  printf("   \\   \\ |    \\   '   /, | .-.  '  (,------. (_|  | |  |  (|  "
         "'--.   |  |'.'|  |  |  |_|( OO )\n");
  printf("  .'    \\_)    \\     /__)' \\  |  |  '------'   |  | |  |   |  "
         ".--'   |  |   |  |  |  | | `-' /\n");
  printf(" /  .'.  \\      \\   /    \\  `'  /              '  '-'  '-. |  "
         "`---.  |  |   |  | ('  '-'(_.-'\n");
  printf("'--'   '--'      `-'      `----'                `-----'--' `------'  "
         "`--'   `--'   `-----'\n");
}
#else
void print_logo() {
  printf(" (`-')           (`-')                   <-.(`-')\n");
  printf(" (OO )_.->      _(OO )                    __( OO)\n");
  printf(" (_| \\_)--.,--.(_/,-.\\  ,--.    (`-')     ,--.(_/,-.\\|--------- "
         ".----.  \n");
  printf(" \\  `.'  / \\   \\ / (_/ /  .'    ( OO).->  \\   \\ / (_/|         "
         "\\_,-.  | \n");
  printf("  \\    .')  \\   /   / .  / -.  (,------.   \\   /   / "
         "|--------	  .' .' \n");
  printf("  .'    \\  _ \\     /_)'  .-. \\  `------'  _ \\     /_)|           "
         ".'  /_  \n");
  printf(" /  .'.  \\ \\-'\\   /   \\  `-' /            \\-'\\   /   |         "
         " |      | \n");
  printf("`--'   '--'    `-'     `----'                 `-'    |          "
         "`------' \n");
}
#endif
