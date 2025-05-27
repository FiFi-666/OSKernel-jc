#include "../include/arch/atoi.h"

int atoi(const char *s) {
  int n;
  int neg = 1;
  if (*s == '-') {
    s++;
    neg = -1;
  }
  n = 0;
  while ('0' <= *s && *s <= '9')
    n = n * 10 + *s++ - '0';
  return n * neg;
}