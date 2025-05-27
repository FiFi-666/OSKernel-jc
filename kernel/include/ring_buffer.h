#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "types.h"
#include "printf.h"
#include "spinlock.h"
#include "string.h"
#include "vm.h"

#define RING_BUFFER_SIZE 4095
#pragma pack(8)
struct ring_buffer {
	size_t size;		// for future use
	int head;		// read from head
	int tail;		// write from tail
	// char buf[RING_BUFFER_SIZE + 1]; // left 1 byte
	char buf[RING_BUFFER_SIZE + 1];
};
#pragma pack()


int wait_ring_buffer_read(struct ring_buffer *rbuf, time_t final_ticks);
int wait_ring_buffer_write(struct ring_buffer *rbuf, time_t final_ticks);
void init_ring_buffer(struct ring_buffer *rbuf);
int ring_buffer_used(struct ring_buffer *rbuf);
int ring_buffer_free(struct ring_buffer *rbuf);
int ring_buffer_empty(struct ring_buffer *rbuf);
int ring_buffer_full(struct ring_buffer *rbuf);
size_t read_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size);
size_t write_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size);
#endif
