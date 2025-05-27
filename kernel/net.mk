HAVE_LWIP = $(shell (cd lwip 2> /dev/null && echo y) || echo n)

ifeq ($(HAVE_LWIP),y)

-include lwip/api/*.d
-include lwip/netif/*.d
-include lwip/core/*.d
-include lwip/core/ipv4/*.d
-include lwip/arch/*.d

OBJS += liblwip.a

CFLAGS += -Ilwip/include  -Ilwip/include/ipv4 -I. -DLWIP

LWIP_CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O -Wall -MD -ggdb \
	 -Werror -std=gnu99 -mcmodel=medany -march=rv64g -nostdlib \
	 -fno-omit-frame-pointer -ffreestanding -fno-common -mno-relax \
	 -Wno-attributes -Wno-address -Wno-char-subscripts -Wno-unused-but-set-variable
LWIP_CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

LDFLAGS = -T default.lds -nostartfiles -nostdlib -nostdinc -static -lgcc \
                     -Wl,--nmagic -Wl,--gc-sections

LWIP_INCLUDES := \
	-Ilwip/include \
	-Ilwip/include/ipv4 \
	-Ilwip/include/arch \
	-Ilwip \
	-Iinclude \
	-I.

LWIP_SRCFILES += \
	lwip/api/api_lib.c \
	lwip/api/api_msg.c \
	lwip/api/err.c \
	lwip/api/sockets.c \
	lwip/api/tcpip.c \
	lwip/api/netbuf.c \
	lwip/core/init.c \
	lwip/core/tcp_in.c \
	lwip/core/def.c \
	lwip/core/mem.c \
	lwip/core/memp.c \
	lwip/core/netif.c \
	lwip/core/pbuf.c \
	lwip/core/raw.c \
	lwip/core/stats.c \
	lwip/core/sys.c \
	lwip/core/tcp.c \
	lwip/core/inet_chksum.c \
	lwip/core/ipv4/dhcp.c \
	lwip/core/ipv4/ip4_addr.c \
	lwip/core/ipv4/icmp.c \
	lwip/core/ipv4/ip4.c \
	lwip/core/ipv4/ip4_frag.c \
	lwip/core/ipv4/etharp.c \
	lwip/core/tcp_out.c \
	lwip/core/udp.c \
	lwip/netif/ethernet.c \
	lwip/netif/bridgeif.c \
	lwip/netif/bridgeif_fdb.c \
	lwip/netif/slipif.c \
	lwip/arch/sys_arch.c \
	lwip/arch/atoi.c \


LWIP_OBJFILES := $(patsubst %.c, %.o, $(LWIP_SRCFILES))
LWIP_OBJFILES := $(patsubst %.S, %.o, $(LWIP_OBJFILES))

TOOLPREFIX	:= riscv64-unknown-elf-
# TOOLPREFIX	:= riscv64-linux-gnu-
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld

lwip/%.o: lwip/%.c
	@echo "  CC     $@"
	mkdir -p $(@D)
	$(CC) $(LWIP_CFLAGS) $(LWIP_INCLUDES) -c -o $@ $<

liblwip.a: $(LWIP_OBJFILES)
	@echo "  AR     $@"
	mkdir -p $(@D)
	$(AR) r $@ $(LWIP_OBJFILES)

.PHONY: clean
clean:
	rm -rf lwip/*.o lwip/*.d lwip/*/*.o lwip/*/*.d lwip/*/*/*.o lwip/*/*/*.d liblwip.a

endif
