#include "../include/arch/string.h"

int strcmp(const char *p, const char *q) {
  while (*p && *p == *q)
    p++, q++;
  return (unsigned char)*p - (unsigned char)*q;
}