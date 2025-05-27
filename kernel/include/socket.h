#ifndef	_SYS_SOCKET_H
#define	_SYS_SOCKET_H

// #define LWIP 0

#include "ring_buffer.h"
#include "file.h"
#include "proc.h"
#include "error.h"

#include "lwip/sockets.h"


// #define AF_UNSPEC 0
// #define PF_INET 2
// // 套接字的通信域(domain)的常用宏定义
// #define AF_INET PF_INET  // IPv4 网络协议
// #define AF_INET6    10   // IPv6 网络协议
// #define AF_UNIX     1    // Unix 域套接字，用于本地进程间通信
// // 套接字的类型(type)的常用宏定义
// #define SOCK_STREAM 1    // 面向连接的流套接字，如TCP套接字
// #define SOCK_DGRAM  2    // 无连接的数据报套接字，如UDP套接字

// #define SOL_SOCKET      1

// // 套接字的协议(protocol)的常用宏定义
// #define IPPROTO_TCP 6    // TCP协议
// #define IPPROTO_UDP 17   // UDP协议
// #define IPPROTO_SCTP 132 // SCTP协议
// #define IPPROTO_ICMP 1   // ICMP协议

#define SOCK_CLOEXEC   02000000
#define SOCK_NONBLOCK  04000
#define FD_CLOEXEC 1
#define O_NONBLOCK (SOCK_NONBLOCK)
// #define SO_RCVTIMEO	20

// #define MAX_SOCK_NUM 16
// #define MAX_WAIT_LIST 1024

// #define SOCK_CLOSED 0
// #define SOCK_LISTEN 1
// #define SOCK_ACCEPTED 2
// #define SOCK_ESTABLISHED 3

// struct in_addr { in_addr_t s_addr; };
// struct sockaddr {
// 	sa_family_t sin_family;
// 	in_port_t sin_port;
// 	struct in_addr sin_addr;
// 	uint8 sin_zero[8];
// };
// struct sockaddr {
// 	sa_family_t sa_family;
// 	char sa_data[14];
// };
// struct socket {
// 	int domain;
// 	int type;
// 	int protocol;
// 	int backlog;
// 	int socknum;
// 	uint8 wait_list[MAX_WAIT_LIST];
// 	uint8 used;
// 	uint8 status;
// 	struct sockaddr addr;
// 	struct ring_buffer data;
// };

// uint16 htons(uint16 hostshort);
// uint32 htonl(uint32 hostlong);
// uint16 ntohs(uint16 netshort);
// uint32 ntohl(uint32 netlong);


// struct sockaddr_in_compat
struct sockaddr_in_compat {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
	uint8 sin_zero[8];
};

// int init_socket();
void tcpip_init_with_loopback(void);
int do_socket(int domain, int type, int protocol);
int do_bind(int sockfd,  struct sockaddr *addr, socklen_t addrlen);
int do_listen(int sockfd, int backlog);
int do_connect(int sockfd,  struct sockaddr *addr,socklen_t addrlen);
int do_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t do_sendto(int sockfd,  void *buf, size_t len, int flags, struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t do_recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
int close_socket(uint32 sock_num);

int do_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int do_setsockopt(int sockfd, int level, int optname,  void *optval, socklen_t optlen);
int do_lwip_select(int socknum, struct timeval * timeout);
#endif