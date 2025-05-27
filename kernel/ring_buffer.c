#include "include/ring_buffer.h"
#include "include/proc.h"
#include "include/string.h"

struct spinlock ring_buffer_lock;

/* wait until ring buffer is not empty or timeout */
/* wait success return 0, timeout return 1, error return -1 */
int wait_ring_buffer_read(struct ring_buffer *rbuf, time_t final_ticks) {
  while (ring_buffer_empty(rbuf)) {
    time_t now_ticks = r_time();
    if (final_ticks < now_ticks)
      return 1;
    yield(); // give up CPU
  }
  return 0;
}

/* wait until ring buffer is not full or timeout */
/* wait success return 0, timeout return 1, error return -1 */
int wait_ring_buffer_write(struct ring_buffer *rbuf, time_t final_ticks) {
  while (ring_buffer_full(rbuf)) {
    time_t now_ticks = r_time();
    if (final_ticks < now_ticks)
      return 1;
    yield(); // give up CPU
  }
  return 0;
}

void init_ring_buffer(struct ring_buffer *rbuf) {
  // there is always one byte which should not be read or written
  // memset(rbuf, 0, sizeof(struct ring_buffer)); /* head = tail = 0 */
  memset(rbuf, 0, RING_BUFFER_SIZE); /* head = tail = 0 */
  rbuf->size = RING_BUFFER_SIZE;
  // WH ADD
  // rbuf->buf = kmalloc(PAGE_SIZE);
  initlock(&ring_buffer_lock, "ring_buffer_lock");
  return;
}

int ring_buffer_used(struct ring_buffer *rbuf) {
  return (rbuf->tail - rbuf->head + rbuf->size) % (rbuf->size);
}

int ring_buffer_free(struct ring_buffer *rbuf) {
  // let 1 byte to distinguish empty buffer and full buffer
  return rbuf->size - ring_buffer_used(rbuf) - 1;
}

int ring_buffer_empty(struct ring_buffer *rbuf) {
  return ring_buffer_used(rbuf) == 0;
}

int ring_buffer_full(struct ring_buffer *rbuf) {
  return ring_buffer_free(rbuf) == 0;
}

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

// buf是用户空间地址
size_t read_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size) {
  acquire(&ring_buffer_lock);
  int len = min(ring_buffer_used(rbuf), size);
  if (len > 0) {
    if (rbuf->head + len > rbuf->size) {
      int right = rbuf->size - rbuf->head, left = len - right;
      // memcpy(buf, rbuf->buf + rbuf->head, right);
      copyout(myproc()->pagetable, (uint64)buf, rbuf->buf + rbuf->head, right);
      // memcpy(buf + right, rbuf->buf, left);
      copyout(myproc()->pagetable, (uint64)buf + right, rbuf->buf, left);
    } else {
      // memcpy(buf, rbuf->buf + rbuf->head, len);
      copyout(myproc()->pagetable, (uint64)buf, rbuf->buf + rbuf->head, len);
    }

    rbuf->head = (rbuf->head + len) % (rbuf->size);
  } else if (len < 0)
    panic("read_ring_buffer: len < 0");
  release(&ring_buffer_lock);
  return len;
}

// rbuf should have enough space for buf
// buf是用户空间地址
size_t write_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size) {
  acquire(&ring_buffer_lock);
  int len = min(ring_buffer_free(rbuf), size);
  if (len > 0) {
    // char tmp[RING_BUFFER_SIZE + 1];
    // memset(tmp, 0, RING_BUFFER_SIZE + 1);
    // copyin(myproc()->pagetable, tmp, (uint64)buf, len);
    // memcpy(tmp, buf, len);
    if (rbuf->tail + len > rbuf->size) {
      int right = rbuf->size - rbuf->tail, left = len - right;
      // memcpy(rbuf->buf + rbuf->tail, tmp, right);
      copyin(myproc()->pagetable, rbuf->buf + rbuf->tail, (uint64)buf, right);
      if (left > 0)
        // memcpy(rbuf->buf, tmp + right, left);
        copyin(myproc()->pagetable, rbuf->buf, (uint64)buf + right, left);
    } else {
      // memcpy(rbuf->buf + rbuf->tail, tmp, len);
      copyin(myproc()->pagetable, rbuf->buf + rbuf->tail, (uint64)buf, len);
    }

    rbuf->tail = (rbuf->tail + len) % (rbuf->size);
  } else if (len < 0)
    panic("read_ring_buffer: len < 0");
  release(&ring_buffer_lock);
  return len;
}
