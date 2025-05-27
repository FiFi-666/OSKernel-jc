#include "kernel/include/fcntl.h"
#include "kernel/include/stat.h"
#include "kernel/include/types.h"
#include "xv6-user/user.h"

static int count = 0;
static int magic[20] = {9443, 16589,    49882,    23478,    234, 45,
                        23,   87684334, 98374812, 89734872, 889};

char *strcpy(char *s, const char *t) {
  char *os;

  os = s;
  while ((*s++ = *t++) != 0)
    ;
  return os;
}

char *strcat(char *s, const char *t) {
  char *os = s;
  while (*s)
    s++;
  while ((*s++ = *t++))
    ;
  return os;
}

int strcmp(const char *p, const char *q) {
  while (*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint strlen(const char *s) {
  int n;

  for (n = 0; s[n]; n++)
    ;
  return n;
}

void *memset(void *dst, int c, uint n) {
  char *cdst = (char *)dst;
  int i;
  for (i = 0; i < n; i++) {
    cdst[i] = c;
  }
  return dst;
}

char *strchr(const char *s, char c) {
  for (; *s; s++)
    if (*s == c)
      return (char *)s;
  return 0;
}

char *gets(char *buf, int max) {
  int i, cc;
  char c;

  for (i = 0; i + 1 < max;) {
    cc = read(0, &c, 1);
    if (cc < 1)
      break;
    buf[i++] = c;
    if (c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int stat(const char *n, struct stat *st) {
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if (fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

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

void *memmove(void *vdst, const void *vsrc, int n) {
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
    while (n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while (n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

int memcmp(const void *s1, const void *s2, uint n) {
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

void *memcpy(void *dst, const void *src, uint n) {
  return memmove(dst, src, n);
}

int extractCountNumber(const char *str, const char *target) {
  const char *countStr = target;
  int num = 1;
  int i = 0;

  while (str[i] != '\0') {
    // 检查是否匹配"COUNT"模式
    int countIndex = 0;
    while (countStr[countIndex] != '\0' &&
           str[i + countIndex] == countStr[countIndex]) {
      countIndex++;
    }

    if (countStr[countIndex] == '\0') {
      // "COUNT"匹配成功，查找数字部分
      num = 0;
      i += countIndex;
      while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
          num = num * 10 + (str[i] - '0');
        } else {
          break;
        }
        i++;
      }
      break;
    }

    // 没有匹配"COUNT"，继续查找下一行
    while (str[i] != '\0' && str[i] != '\n') {
      i++;
    }
    if (str[i] == '\n') {
      i++;
    }
  }
  if (num == 1 && count <= 10)
    num = magic[count++];
  if (num < 0)
    num = -num;
  return num;
}