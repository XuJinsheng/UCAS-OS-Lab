# -----------------------------------------------------------------------
# Project Information
# -----------------------------------------------------------------------

PROJECT_IDX	= 2

# -----------------------------------------------------------------------
# Host Linux Variables
# -----------------------------------------------------------------------

SHELL       = /bin/sh
DISK        = /dev/sdb
TTYUSB1     = /dev/ttyUSB1
DIR_OSLAB   = $(HOME)/OSLab-RISC-V
DIR_QEMU    = $(DIR_OSLAB)/qemu
DIR_UBOOT   = $(DIR_OSLAB)/u-boot

# -----------------------------------------------------------------------
# Build and Debug Tools
# -----------------------------------------------------------------------

HOST_CC         = gcc
CROSS_PREFIX    = /opt/riscv/bin/riscv64-unknown-elf-
CC              = $(CROSS_PREFIX)gcc
CXX             = $(CROSS_PREFIX)g++
AR              = $(CROSS_PREFIX)ar
OBJDUMP         = $(CROSS_PREFIX)objdump
GDB             = $(CROSS_PREFIX)gdb
QEMU            = $(DIR_QEMU)/riscv64-softmmu/qemu-system-riscv64
UBOOT           = $(DIR_UBOOT)/u-boot
MINICOM         = minicom

# -----------------------------------------------------------------------
# Build/Debug Flags and Variables
# -----------------------------------------------------------------------

CFLAGS          = -O2 -fno-builtin -nostdlib -nostdinc -Wall -mcmodel=medany -ggdb3
CXXFLAGS        = -O2 -std=c++20 -Wall -mcmodel=medany -ggdb3 -fno-builtin -nostdlib 
CXXFLAGS       += -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics

BOOT_INCLUDE    = -I$(DIR_KERNEL)/arch
BOOT_CFLAGS     = $(BOOT_INCLUDE) -Wl,--defsym=TEXT_START=$(BOOTLOADER_ENTRYPOINT) -T riscv.lds

KERNEL_INCLUDE  = -I$(DIR_KERNEL)
KERNEL_CFLAGS   = $(KERNEL_INCLUDE) -Wl,--defsym=TEXT_START=$(KERNEL_ENTRYPOINT) -T riscv.lds

USER_INCLUDE    = -I$(DIR_TINYLIBC)/include
USER_CFLAGS     = $(USER_INCLUDE)
USER_LDFLAGS    = -L$(DIR_BUILD) -ltinyc

QEMU_LOG_FILE   = $(DIR_OSLAB)/oslab-log.txt
QEMU_OPTS       = -nographic -machine virt -m 256M -kernel $(UBOOT) -bios none \
                     -drive if=none,format=raw,id=image,file=${ELF_IMAGE} \
                     -device virtio-blk-device,drive=image \
                     -monitor telnet::45454,server,nowait -serial mon:stdio \
                     -D $(QEMU_LOG_FILE) -d oslab
QEMU_DEBUG_OPT  = -s -S

# -----------------------------------------------------------------------
# UCAS-OS Entrypoints and Variables
# -----------------------------------------------------------------------

DIR_BUILD       = ./build
DIR_BOOTLOADER 	= ./bootloader
DIR_KERNEL      = ./kernel
DIR_TINYLIBC    = ./tiny_libc
DIR_TEST        = ./test
DIR_TEST_PROJ   = $(DIR_TEST)/test_project$(PROJECT_IDX)

BOOTLOADER_ENTRYPOINT   = 0x50200000
KERNEL_ENTRYPOINT       = 0x50201000
USER_ENTRYPOINT         = 0x52000000

# -----------------------------------------------------------------------
# UCAS-OS Kernel Source Files
# -----------------------------------------------------------------------

SRC_BOOT  =  $(wildcard $(DIR_BOOTLOADER)/*.S)
SRC_MAIN  += $(wildcard $(DIR_KERNEL)/*.c)
SRC_MAIN  += $(wildcard $(DIR_KERNEL)/*.cpp)
SRC_MAIN  += $(wildcard $(DIR_KERNEL)/*/*.S)
SRC_MAIN  += $(wildcard $(DIR_KERNEL)/*/*.c)
SRC_MAIN  += $(wildcard $(DIR_KERNEL)/*/*.cpp)


ELF_BOOT    = $(DIR_BUILD)/bootblock
ELF_MAIN    = $(DIR_BUILD)/main
ELF_IMAGE   = $(DIR_BUILD)/image

# -----------------------------------------------------------------------
# UCAS-OS User Source Files
# -----------------------------------------------------------------------

SRC_CRT0    = $(wildcard $(DIR_TINYLIBC)/crt0/*.S)
OBJ_CRT0    = $(DIR_BUILD)/$(notdir $(SRC_CRT0:.S=.o))

SRC_LIBC    = $(wildcard ./tiny_libc/*.c)
OBJ_LIBC    = $(patsubst %.c, %.o, $(foreach file, $(SRC_LIBC), $(DIR_BUILD)/$(notdir $(file))))
SRC_LIBS    = $(wildcard ./tiny_libc/*.S)
OBJ_LIBS    = $(patsubst %.S, %.o, $(foreach file, $(SRC_LIBS), $(DIR_BUILD)/$(notdir $(file))))
LIB_TINYC   = $(DIR_BUILD)/libtinyc.a

SRC_USER    = $(wildcard $(DIR_TEST_PROJ)/*.c)
ELF_USER    = $(patsubst %.c, %, $(foreach file, $(SRC_USER), $(DIR_BUILD)/$(notdir $(file))))

# -----------------------------------------------------------------------
# Host Linux Tools Source Files
# -----------------------------------------------------------------------

SRC_CREATEIMAGE = ./tools/createimage.c
ELF_CREATEIMAGE = $(DIR_BUILD)/$(notdir $(SRC_CREATEIMAGE:.c=))

# -----------------------------------------------------------------------
# Top-level Rules
# -----------------------------------------------------------------------

all: dirs elf image asm # floppy

dirs:
	@mkdir -p $(DIR_BUILD)

clean:
	rm -rf $(DIR_BUILD)

floppy:
	sudo fdisk -l $(DISK)
	sudo dd if=$(DIR_BUILD)/image of=$(DISK)3 conv=notrunc

asm: $(ELF_BOOT) $(ELF_MAIN) $(ELF_USER)
	for elffile in $^; do $(OBJDUMP) -d $$elffile > $(notdir $$elffile).s; done

gdb:
	$(GDB) $(ELF_MAIN) -ex "target remote:1234"

run:
	$(QEMU) $(QEMU_OPTS)

debug:
	$(QEMU) $(QEMU_OPTS) $(QEMU_DEBUG_OPT)

minicom:
	sudo $(MINICOM) -D $(TTYUSB1)

bear:
	bear -- make
	mv compile_commands.json .vscode/compile_commands.json

.PHONY: all dirs clean floppy asm gdb run debug minicom bear

# -----------------------------------------------------------------------
# UCAS-OS Rules
# -----------------------------------------------------------------------

$(ELF_BOOT): $(SRC_BOOT) riscv.lds
	$(CC) $(CFLAGS) $(BOOT_CFLAGS) -o $@ $(SRC_BOOT) -e main

$(ELF_MAIN): $(SRC_MAIN) riscv.lds
	$(CXX) $(CXXFLAGS) $(KERNEL_CFLAGS) -o $@ $(SRC_MAIN)

$(OBJ_CRT0): $(SRC_CRT0)
	$(CC) $(CFLAGS) $(USER_CFLAGS) -I$(DIR_ARCH)/include -c $< -o $@

$(LIB_TINYC): $(OBJ_LIBC) $(OBJ_LIBS)
	$(AR) rcs $@ $^

$(DIR_BUILD)/%.o: $(DIR_TINYLIBC)/%.c
	$(CC) $(CFLAGS) $(USER_CFLAGS) -c $< -o $@

$(DIR_BUILD)/%.o: $(DIR_TINYLIBC)/%.S
	$(CC) $(CFLAGS) $(USER_CFLAGS) -c $< -o $@

$(DIR_BUILD)/%: $(DIR_TEST_PROJ)/%.c $(OBJ_CRT0) $(LIB_TINYC) riscv.lds
	$(CC) $(CFLAGS) $(USER_CFLAGS) -o $@ $(OBJ_CRT0) $< $(USER_LDFLAGS) -Wl,--defsym=TEXT_START=$(USER_ENTRYPOINT) -T riscv.lds
	$(eval USER_ENTRYPOINT := $(shell python3 -c "print(hex(int('$(USER_ENTRYPOINT)', 16) + int('0x10000', 16)))"))

elf: $(ELF_BOOT) $(ELF_MAIN) $(LIB_TINYC) $(ELF_USER)

.PHONY: elf

# -----------------------------------------------------------------------
# Host Linux Rules
# -----------------------------------------------------------------------

$(ELF_CREATEIMAGE): $(SRC_CREATEIMAGE)
	$(HOST_CC) $(SRC_CREATEIMAGE) -o $@ -ggdb -Wall

image: $(ELF_CREATEIMAGE) $(ELF_BOOT) $(ELF_MAIN) $(ELF_USER)
	cd $(DIR_BUILD) && ./$(<F) --extended $(filter-out $(<F), $(^F))

.PHONY: image
