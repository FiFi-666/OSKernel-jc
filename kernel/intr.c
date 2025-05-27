#include "include/intr.h"
#include "include/console.h"
#include "include/param.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/types.h"

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void push_off(void) {
  int old = intr_get();

  intr_off();
  // printf("\e[32mpush_off()\e[0m: cpuid(): %d\n", cpuid());
  if (mycpu()->noff == 0)
    mycpu()->intena = old;
  // printstring("push_off noff++ noff:\n");
  // printint(mycpu()->noff, 10, 1);
  // consputc('\n');
  mycpu()->noff += 1;
}

void pop_off(void) {
  struct cpu *c = mycpu();

  // printf("\e[31mpop_off()\e[0m: cpuid(): %d\n", cpuid());
  if (intr_get())
    panic("pop_off - interruptible");
  if (c->noff < 1) {
    // printf("c->noff = %d\n", c->noff);
    // consputc('P');
    panic("pop_off");
  }
  // printf("c->noff: %d\n", c->noff);
  // printf("c: %x\n", c);
  // printstring("pop_off noff--\n");
  // printstring("pop_off noff-- noff:\n");
  // printint(mycpu()->noff, 10, 1);
  // consputc('\n');
  c->noff -= 1;
  if (c->noff == 0 && c->intena)
    intr_on();
}
