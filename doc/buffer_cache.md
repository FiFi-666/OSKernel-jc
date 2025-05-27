### buffer cache

在文件系统中，对磁盘的直接读写非常缓慢，因此有必要添加buffer cache层，将经常用到的扇区缓存在内存中，并尽可能延迟写操作。

#### buffer cache结构体

用来做buffer cache的结构体`struct buf`大体上以双向链表来组织。

```c
struct buf {
  int valid;    // ready to read?
  int disk;		// does disk "own" buf? 
  int dirty;    // data modified?
  uint dev;     // dev number
  uint sectorno;	// sector number 
  struct sleeplock lock;
  uint refcnt;
  struct buf *prev;
  struct buf *next;
  uchar data[BSIZE]; // block data
};
```

注意到 buf 结构体中有一个睡眠锁，用来确保每时每刻只有一个进程能够读写buf对应的扇区。

另外，我们还有一个结构体`bcache`，用来管理所有buffer cache缓冲区。

```c
struct {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct buf head;
} bcache;
```

bcache内含一个由`struct buf` 构成的数组和一个 buffer cache 队列，队列头为head。这一队列遵循类似于最近最少使用(LRU)的替换方法，刚刚使用的 buffer cache 会排到队头。

bcache中的自旋锁用来控制多进程条件下对bcache中buf队列的修改。

#### 重要函数

```c
void        binit(void);
```

用于初始化 bcache，它初始化 buf 数组中的内容，并且使用 buf 数组构建 buffer cache 队列。

```c
struct buf* bget(uint dev, uint sectorno);
```

获取对应设备和缓冲区的buffer cache。首先，它会从 bcache 中的LRU队列从前往后查找 buffer cache，找到则立即返回；如果没有找到，则从后往前查找一个`refcnt == 0`的空闲 buffer cache来使用，并把 valid 位标记为0。如果空闲 buffer cache 被写过(即`dirty == 1`)，那就在更改了buffer cache的元数据之前写回对应扇区，尽量延迟磁盘写。

```c
struct buf* bread(uint devno, uint sectorno);
```

通过bget获取一个可用的缓冲区，如果`valid == 0`，则进行磁盘读获取数据。

```c
void        brelse(struct buf *b);
```

让该 buffer cache 的refcnt减一，如果为0，则将其移到LRU队列的队尾。

```c
void        bwrite(struct buf *b);
```

将dirty位置1，表明这一块 buffer cache 已经写过。