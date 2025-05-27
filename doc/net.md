# socket网络接口

注意到测试程序中只存在本机回环，我们的socket接口采取了简化的实现方法，不经过qemu的网卡，直接通过本机ring buffer进行信息传递，在支持socket接口基本功能的同时提升传输效率。

### 与socket相关的系统调用

#### 相关结构体

```rust
// socket套接字的IP地址和端口信息
struct in_addr { in_addr_t s_addr; };
struct sockaddr {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
	uint8 sin_zero[8];
};

// socket套接字
struct socket {
	int domain;                      // 协议簇
	int type;                        // socket类型
	int protocol;                    // socket所用协议
	int backlog;                     // 最大等待连接数
	int socknum;
	uint8 wait_list[MAX_WAIT_LIST];  // 监听等待名单
	uint8 used;                      // 是否被某个文件使用
	uint8 status;                    // socket当前状态
	struct sockaddr addr;            // IP地址和端口信息
	struct ring_buffer data;         // 用于收发信息的环形队列ring_buffer
};

```

#### socket系统调用接口

```c

// 创建一个未绑定的socket套接字。
// 
// domain: 指明套接字被创建的协议簇
// type: 指明被创建的socket的类型
// protocol: 指明该socket的协议
// 返回: 创建成功则返回一个socket文件描述符，否则返回错误信息。
int sys_socket(int domain, int type, int protocol)；

    

// 将地址和端口绑定到socket上。
// 
// sockfd: 待操作socket的文件描述符
// addr: 指明存储的有关绑定信息（sockaddr结构）的地址（sockaddr结构包括地址组信息address_family和要绑定的地址信息socket_address）
// addrlen: addr（即sockaddr结构）的长度。
// 返回: 执行成功则返回0，否则返回错误信息
int sys_bind(int sockfd, struct sockaddr *addr, socklen_t addrlen);



// 监听客户端连接请求。调用后客户端通过connect对其进行连接。
//
// sockfd: socket文件描述符
// backlog: 指明套接字侦听队列中正在处于半连接状态（等待accept）的请求数最大值。如果该值小于等于0，则自动调为0，同时也有最大值上限。
// 返回: 执行成功返回0，否则返回错误信息
int sys_listen(int sockfd, int backlog)




// 用于取出本监听队列中的第一个连接，创建一个与指定套接字具有相同套接字类型的地址族的新套接字
// 新套接字用于传递数据，原套接字继续处理侦听队列中的连接请求。如果侦听队列中无请求，accept()将阻塞。
// 
// sockfd: socket文件描述符，需先绑定ip、端口并开始监听
// addr: 要么为空，要么指明连接的客户端相关信息（sockaddr结构）的保存地址
// addrlen: 保存连接的客户端相关信息长度的地址。
// 返回: 执行成功则返回新的套接字的文件描述符，否则返回错误信息
int sys_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);




// 用于客户端请求在一个套接字上建立连接
// 
// sockfd: socket文件描述符
// addr: 指明包含服务器地址和端口号的数据结构（sockaddr结构）的地址
// addrlen: addr（即sockaddr结构）的长度。
// 返回: 执行成功则返回0，否则返回错误信息
int sys_connect(int sockfd,  struct sockaddr *addr,socklen_t addrlen);




// 查询一个套接字的addr结构体，即其绑定的IP地址和端口。
//
// socket: 指明要操作socket的文件描述符id
// address: 指明相关信息（sockaddr结构）的保存地址
// address_len: 保存address长度的地址。
// 返回: 执行成功则返回0，否则返回错误信息
int sys_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);




// 发送消息。
//
// sockfd: socket文件描述符
// buf: 待发送信息的首地址
// length: 待发送信息的长度
// flags: 信息发送类型
// dest_addr: 目标socket sockaddr结构的指针
// addrlen: 指明dest_addr的结构体的大小
// 返回: 如果发送成功，返回发送的字节数；否则返回错误信息
ssize_t sys_sendto(int sockfd,  void *buf, size_t len, int flags, struct sockaddr *dest_addr, socklen_t addrlen);



// 接收消息。
//
// sockfd: socket文件描述符
// buf: 待接收信息的首地址
// length: 待接收信息的最大长度
// flags: 信息接收类型
// src_addr: 发送者socket sockaddr结构的指针
// addrlen: 指明src_addr的（sockaddr）结构体的长度的地址
// 返回: 如果发送成功，返回接收message的字节数；否则返回错误信息
ssize_t sys_recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);

```

### ring buffer实现

socket本地回环的信息传输由ring buffer实现，sendto将信息写入到目的socket的ring buffer中，recvfrom从自有socket的ring buffer中读取信息。ring buffer本质上是一个环形队列，能够有效避免假写满的问题，只要不一次写入超过整个缓冲区空闲大小的信息就不会发生越界问题。

#### ring buffer结构体

```c
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
```

#### ring buffer操作接口

```c
// 初始化ring buffer
void init_ring_buffer(struct ring_buffer *rbuf);

// 从rbuf中读取信息到buf
size_t read_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size);

// 向rbuf写入buf中的信息
size_t write_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size);
```



## 参考资料

[socket.h](https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html)