platform	:= visionfive
#platform	:= qemu
 mode := debug
# mode := release
exam := no
K=kernel
U=xv6-user
T=target

OBJS =
ifeq ($(platform), visionfive)
OBJS += $K/entry_visionfive.o
else
OBJS += $K/entry_qemu.o
endif

OBJS += \
  $K/printf.o \
  $K/uart.o \
  $K/kalloc.o \
  $K/intr.o \
  $K/spinlock.o \
  $K/string.o \
  $K/main.o \
  $K/proc.o \
  $K/vm.o \
  $K/swtch.o \
  $K/trampoline.o \
  $K/trap.o \
  $K/syscall.o \
  $K/sysproc.o \
  $K/bio.o \
  $K/sleeplock.o \
  $K/file.o \
  $K/pipe.o \
  $K/exec.o \
  $K/sysfile.o \
  $K/systime.o \
  $K/kernelvec.o \
  $K/timer.o \
  $K/disk.o \
  $K/fat32.o \
  $K/plic.o \
  $K/console.o \
  $K/mmap.o \
  $K/fs.o \
  $K/vma.o \
  $K/signal.o \
  $K/syssig.o \
  $K/bin.o \
  $K/ring_buffer.o \
  $K/socket_new.o \
  $K/sem.o \
  $K/syssocket.o \
  $K/SignalTrampoline.o \
  $K/thread.o \
  $K/futex.o \
  $K/pselect.o \
  $K/uart8250.o \


ifeq ($(platform), qemu)
OBJS += \
  $K/virtio_disk.o \
  #$K/uart.o \

else
OBJS += \
	$K/sd_final.o
#   $K/spi.o \
#   $K/gpiohs.o \
#   $K/fpioa.o \
#   $K/utils.o \
#   $K/sdcard.o \
#   $K/dmac.o \
#   $K/sysctl.o 
  

endif

LWIP_INCLUDES := \
	-Ikernel/lwip/include \
	-Ikernel/lwip/include/ipv4 \
	-Ikernel/lwip/include/arch \
	-Ikernel/lwip \
	-Ikernel/include \
	-I.

LWIP_SRCFILES += \
	kernel/lwip/api/api_lib.c \
	kernel/lwip/api/api_msg.c \
	kernel/lwip/api/err.c \
	kernel/lwip/api/if_api.c \
	kernel/lwip/api/netdb.c \
	kernel/lwip/api/netifapi.c \
	kernel/lwip/api/sockets.c \
	kernel/lwip/api/tcpip.c \
	kernel/lwip/api/netbuf.c \
	kernel/lwip/core/init.c \
	kernel/lwip/core/tcp_in.c \
	kernel/lwip/core/dns.c \
	kernel/lwip/core/def.c \
	kernel/lwip/core/mem.c \
	kernel/lwip/core/memp.c \
	kernel/lwip/core/netif.c \
	kernel/lwip/core/pbuf.c \
	kernel/lwip/core/raw.c \
	kernel/lwip/core/stats.c \
	kernel/lwip/core/sys.c \
	kernel/lwip/core/tcp.c \
	kernel/lwip/core/inet_chksum.c \
	kernel/lwip/core/ip.c \
	kernel/lwip/core/timeouts.c \
	kernel/lwip/core/ipv4/autoip.c \
	kernel/lwip/core/ipv4/dhcp.c \
	kernel/lwip/core/ipv4/ip4_addr.c \
	kernel/lwip/core/ipv4/icmp.c \
	kernel/lwip/core/ipv4/ip4.c \
	kernel/lwip/core/ipv4/ip4_frag.c \
	kernel/lwip/core/ipv4/etharp.c \
	kernel/lwip/core/tcp_out.c \
	kernel/lwip/core/udp.c \
	kernel/lwip/netif/ethernet.c \
	kernel/lwip/netif/bridgeif.c \
	kernel/lwip/netif/bridgeif_fdb.c \
	kernel/lwip/arch/sys_arch.c \
	kernel/lwip/arch/atoi.c \
	kernel/lwip/arch/errno.c \
	kernel/lwip/arch/string.c \
	kernel/lwip/arch/rand.c \

LWIP_OBJFILES := $(patsubst %.c, %.o, $(LWIP_SRCFILES))
LWIP_OBJFILES := $(patsubst %.S, %.o, $(LWIP_OBJFILES))

OBJS += $(LWIP_OBJFILES)

TOOLPREFIX	:= riscv64-unknown-elf-
# TOOLPREFIX	:= riscv64-linux-gnu-
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

CFLAGS = -Wall -O -fno-omit-frame-pointer -ggdb -g
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
CFLAGS += -Ikernel/lwip/include
CFLAGS += -Ikernel/lwip/include/arch
CFLAGS += -Ikernel/lwip
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

ifeq ($(mode), debug) 
CFLAGS += -DDEBUG 
endif 

ifeq ($(exam), yes) 
CFLAGS += -DEXAM 
endif 

ifeq ($(platform), qemu)
CFLAGS += -D QEMU
else ifeq ($(platform), k210)
CFLAGS += -D k210
else ifeq ($(platform), visionfive)
CFLAGS += -D visionfive
OBJS += $K/sddata.o $K/ramdisk.o
endif

LDFLAGS = -z max-page-size=4096

ifeq ($(platform), visionfive)
linker = ./linker/visionfive.ld
endif

ifeq ($(platform), qemu)
linker = ./linker/qemu.ld
endif

# $K/liblwip.a:
# 	@cd kernel && make -f net.mk liblwip.a

# Compile Kernel
$T/kernel: $(OBJS) $(linker) $U/initcode
	@if [ ! -d "./target" ]; then mkdir target; fi
	@$(LD) $(LDFLAGS) -T $(linker) -o $T/kernel $(OBJS)
	@$(OBJDUMP) -S $T/kernel > $T/kernel.asm
	@$(OBJDUMP) -t $T/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $T/kernel.sym

kernel-qemu: $(OBJS) $(linker) $U/initcode $U/init_for_test
	@$(LD) $(LDFLAGS) -T $(linker) -so kernel-qemu $(OBJS) -L.
	@$(OBJDUMP) -S kernel-qemu > kernel-qemu.asm
	@$(OBJDUMP) -t kernel-qemu | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel-qemu.sym

build-grading: kernel-qemu
	@$(OBJCOPY) kernel-qemu --strip-all -O binary kernel-qemu.bin

build: userprogs $T/kernel
	@$(OBJCOPY) $T/kernel --strip-all -O binary $(image)

image = $T/kernel.bin

QEMU = qemu-system-riscv64
CPUS := 2

QEMUOPTS = -machine virt -m 128M -nographic -kernel target/kernel 
# use multi-core 
QEMUOPTS += -smp $(CPUS)
QEMUOPTS += -bios default
# import virtual disk image
QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0 
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

gdb-server: 
	@make build platform=qemu
#	@make fs
	@$(QEMU) $(QEMUOPTS) -s -S

gdb-client:
	riscv64-unknown-elf-gdb -quiet -ex "set architecture riscv:rv64" -ex "target remote localhost:1234" target/kernel

all:
	@gunzip -k sdcard.img.gz
	@make build platform=visionfive mode=release exam=no
	@cp target/kernel.bin os.bin

qemu-run:
	@make build platform=qemu mode=debug
#	@make fs
	@$(QEMU) $(QEMUOPTS)

$K/bin.S:$U/initcode $U/init-for-test

$U/initcode: $U/initcode.S
	$(CC) $(CFLAGS) -march=rv64g -nostdinc -I. -Ikernel -c $U/initcode.S -o $U/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $U/initcode.out $U/initcode.o
	$(OBJCOPY) -S -O binary $U/initcode.out $U/initcode
	$(OBJDUMP) -S $U/initcode.o > $U/initcode.asm

$U/init-for-test: $U/init-for-test.S
	$(CC) $(CFLAGS) -march=rv64g -nostdinc -I. -Ikernel -c $U/init-for-test.S -o $U/init-for-test.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $U/init-for-test.out $U/init-for-test.o
	$(OBJCOPY) -S -O binary $U/init-for-test.out $U/init-for-test
	$(OBJDUMP) -S $U/init-for-test.o > $U/init-for-test.asm

tags: $(OBJS) _init
	@etags *.S *.c

ULIB = $U/ulib.o $U/usys.o $U/printf.o $U/umalloc.o

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

$U/usys.S : $U/usys.pl
	@perl $U/usys.pl > $U/usys.S

$U/usys.o : $U/usys.S
	$(CC) $(CFLAGS) -c -o $U/usys.o $U/usys.S

$U/_forktest: $U/forktest.o $(ULIB)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/_forktest $U/forktest.o $U/ulib.o $U/usys.o
	$(OBJDUMP) -S $U/_forktest > $U/forktest.asm

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS=\
	$U/_init\
	$U/_sh\
	$U/_cat\
	$U/_echo\
	$U/_grep\
	$U/_ls\
	$U/_kill\
	$U/_mkdir\
	$U/_xargs\
	$U/_sleep\
	$U/_find\
	$U/_rm\
	$U/_wc\
	$U/_test\
	$U/_usertests\
	$U/_strace\
	$U/_mv\
	$U/_busybox_test\
	# $U/_myDup3\

	# $U/_forktest\
	# $U/_ln\
	# $U/_stressfs\
	# $U/_grind\
	# $U/_zombie\

userprogs: $(UPROGS)
	@$(OBJCOPY) -S -O binary $U/_busybox_test $U/busybox_test.bin

dst=/mnt

# @sudo cp $U/_init $(dst)/init
# @sudo cp $U/_sh $(dst)/sh
# Make fs image
fs: $(UPROGS)
#	@echo a
	@if [ ! -f "fs.img" ]; then \
		echo "making fs image..."; \
		dd if=/dev/zero of=fs.img bs=512k count=512; \
		mkfs.vfat -F 32 fs.img; fi
	@sudo mount fs.img $(dst)
	@if [ ! -d "$(dst)/bin" ]; then sudo mkdir $(dst)/bin; fi
	@sudo cp README $(dst)/README
	@for file in $$( ls $U/_* ); do \
		sudo cp $$file $(dst)/$${file#$U/_};\
		sudo cp $$file $(dst)/bin/$${file#$U/_}; done
	@sudo umount $(dst)

# Write mounted sdcard
sdcard: userprogs
	@if [ ! -d "$(dst)/bin" ]; then sudo mkdir $(dst)/bin; fi
	@for file in $$( ls $U/_* ); do \
		sudo cp $$file $(dst)/bin/$${file#$U/_}; done
	@sudo cp $U/_init $(dst)/init
	@sudo cp $U/_sh $(dst)/sh
	@sudo cp README $(dst)/README

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*/*.o */*.d */*.asm */*.sym kernel-qemu kernel-qemu.asm kernel-qemu.bin kernel-qemu.sym \
	$T/* \
	$U/initcode $U/initcode.out \
	$U/init-for-test $U/init-for-test.out \
	$U/busybox_test.bin \
	$U/*.bin \
	$K/kernel \
	.gdbinit \
	$U/usys.S \
	$(UPROGS) \
	os.bin \
	sdcard.img \
	$K/lwip/*/*.o \
	$K/lwip/*/*.d \
	$K/lwip/*/*/*.o \
	$K/lwip/*/*/*.d \
	$K/*.a \

