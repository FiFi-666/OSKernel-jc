#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"
#include "spinlock.h"
#include "param.h"

#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2

#define NTIMERS 10

extern struct spinlock tickslock;
extern uint ticks;

typedef struct TimeSpec {
    uint64 second;
    uint64 microSecond;    
} TimeSpec;

struct tms              
{                     
	long tms_utime;  
	long tms_stime;  
	long tms_cutime; 
	long tms_cstime; 
};

struct timespec2 {
	long tv_sec;	// seconds
	long tv_nsec;	// nanoseconds
};

struct timeval {
    time_t      tv_sec;         // 秒数
    suseconds_t tv_usec;        // 微秒数
};

typedef struct itimerval {
    struct timeval it_interval;  // 定时器的间隔时间
    struct timeval it_value;     // 定时器的初始值
}itimerval;

typedef struct timer{
    itimerval itimer;
    int which;
    int pid;
    int ticks;
}timer;

static inline uint64 convert_from_timespec2(const struct timespec2 *ts)
{
	uint64 time = ts->tv_sec * CLK_FREQ
					+ ts->tv_nsec * (CLK_FREQ / 1000 / 100) / 10 / 1000;
	return time;
}

typedef struct timespec2 TimeSpec2;

void timerinit();
void set_next_timeout();
void timer_tick();
uint64 setitimer(int which, const struct itimerval *value, struct itimerval *ovalue);
struct timeval get_timeval();
#endif
