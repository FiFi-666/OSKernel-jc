#ifndef __PRINTF_H
#define __PRINTF_H

#include "proc.h"

void debug_print(char *fmt, ...);

void serious_print(char *fmt, ...);

void printfinit(void);

void printf(char *fmt, ...);

void panic(char *s) __attribute__((noreturn));

void backtrace();

void print_logo();

void printstring(const char* s);

void printint(int xx, int base, int sign);

void checkup1(struct proc *p);

#endif 
