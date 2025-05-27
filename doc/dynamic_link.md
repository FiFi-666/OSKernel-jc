# 本部分为系统动态链接

## 动态链接标志
当一个文件需要动态链接，它的elf文件中会显示有INTERP段。


```
$ readlef -aW entry-dynamic.exe

ELF 头：
  Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              EXEC (可执行文件)
  系统架构:                          RISC-V
  版本:                              0x1
  入口点地址：               0x2f320
  程序头起点：          64 (bytes into file)
  Start of section headers:          347968 (bytes into file)
  标志：             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         8
  Size of section headers:           64 (bytes)
  Number of section headers:         35
  Section header string table index: 34

  ······
  ······
  ······

  程序头：
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x0001c0 0x0001c0 R   0x8
  INTERP         0x000200 0x0000000000010200 0x0000000000010200 0x00001d 0x00001d R   0x1
      [Requesting program interpreter: /lib/ld-musl-riscv64-sf.so.1]
  LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x048db4 0x048db4 R E 0x1000
  LOAD           0x049000 0x000000000005a000 0x000000000005a000 0x004d80 0x008520 RW  0x1000
  DYNAMIC        0x04b020 0x000000000005c020 0x000000000005c020 0x0001b0 0x0001b0 RW  0x8
  TLS            0x049000 0x000000000005a000 0x000000000005a000 0x001041 0x003041 R   0x1000
  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x10
  GNU_RELRO      0x049000 0x000000000005a000 0x000000000005a000 0x003000 0x003000 R   0x1

```

程序头（program header）中有的INTERP段，代表着程序需要动态链接。

## 动态链接器

程序头中的INTERP段实际内容为一个链接器的字符串。本次测试程序依赖的链接器为/lib/ld-musl-riscv64-sf.so.1，但是这个字符串只是一个符号链接，我们实际需要处理的内容是/libc.so

## 动态链接器在内存中的位置

在linux操作系统中，动态链接器的地址应该位于用户虚拟空间的中间部位。

读取libc.so的elf属性：

```
$ readelf -aW libc.so

ELF 头：
  Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              DYN (共享目标文件)
  系统架构:                          RISC-V
  版本:                              0x1
  入口点地址：               0x65b08
  程序头起点：          64 (bytes into file)
  Start of section headers:          933712 (bytes into file)
  标志：             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         5
  Size of section headers:           64 (bytes)
  Number of section headers:         27
  Section header string table index: 26


  ······
  ······
  ······

  程序头：
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000000000000 0x0000000000000000 0x0a22a4 0x0a22a4 R E 0x1000
  LOAD           0x0a2cf0 0x00000000000a3cf0 0x00000000000a3cf0 0x0007f8 0x003528 RW  0x1000
  DYNAMIC        0x0a2eb0 0x00000000000a3eb0 0x00000000000a3eb0 0x000150 0x000150 RW  0x8
  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x10
  GNU_RELRO      0x0a2cf0 0x00000000000a3cf0 0x00000000000a3cf0 0x000310 0x000310 R   0x1
```

注意到，程序头（program header）中的LOAD段，执行的虚拟地址依然是从0开始的。

经过查证相关资料后，我们可以得知，动态链接器在内存中的位置没有固定的约定，只需将其加载到用户程序的空间中，并通过辅助数组来传递对应的信息即可。

故而内核对libc.so文件的加载位置是使用mmap同样的原理。首先获取动态链接器所需要的内存区域大小，然后调用对应的mmap接口，将其声明为匿名映射。

## 参数的传递---让动态链接器知道可执行文件和自己的位置

内核在处理动态链接时，需要将动态链接文件和可执行文件加载到同一个虚拟地址空间中去，那么自然产生了一系列的疑问，程序的入口点在哪里？动态链接器如何知道它自己在内存中的位置？动态链接器如何知道可执行文件的位置？下面我们来解释这个问题。

### 程序入口点
当应用程序是动态链接时，程序的入口点是动态链接器的入口点。这一点是约定好了的

### 辅助数组---通过栈传递信息
![user_stack](./pic/user_stack.png)

如图所示，当内核将控制权传递给一个刚执行完exec的进程后，其栈布局表现为上方所示。

辅助数组的定义和含义如下：
![aux1](./pic/aux1.png)
![aux2](./pic/aux2.png)


所以，我们只需要根据定义，将指定的数据传入到辅助数组中便可完成系列操作