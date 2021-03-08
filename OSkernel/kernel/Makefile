NAME           := kernel
SRCS           := framework/main.c $(shell find -L ./src/ -name "*.c" -o -name "*.S")
INC_DIR        := include/ framework/
export MODULE  := L2
export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
#	export ARCH := native
	export ARCH := x86_64-qemu
endif

include $(AM_HOME)/Makefile.app
include ../Makefile.lab
image: git
QEMU32:= qemu-system-i386
IMG32:= build/kernel-x86-qemu
BIN32:= build/kernel-x86-qemu.o
QEMU64:= qemu-system-x86_64
IMG64:= build/kernel-x86_64-qemu
BIN64:= build/kernel-x86_64-qemu.o
debug32:$(IMG32)
	qemu-system-i386 -smp 1 -S -s -serial stdio -monitor telnet:127.0.0.1:1111,server,nowait build/kernel-x86-qemu
gdb32:$(IMG32)
	@gdb -ex "target remote 127.0.0.1:1234" -ex "symbol $(BIN32)"

debug64:$(IMG64)
	qemu-system-x86_64 -smp 1 -S -s -serial stdio -monitor telnet:127.0.0.1:1111,server,nowait build/kernel-x86_64-qemu
gdb64:$(IMG64)
	@gdb -ex "target remote 127.0.0.1:1234" -ex "symbol $(BIN64)"
run64:$(IMG64)
	qemu-system-x86_64 -smp 4 -s -serial stdio $(IMG64)


log:$(IMG64)
	qemu-system-x86_64 -nographic -smp 1 -d int,cpu_reset -D log.txt -accel tcg,thread=multi -serial mon:stdio build/kernel-x86_64-qemu
