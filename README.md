# UCAS-CA-Lab

This project implements an Operating System, for the UCAS course, Operating System Laboratory. This kernel is based on RISC-V64GC ISA

## Supported Features

- Process and Thread Management, Task Scheduling
- Multicore CPU Support
- Inter-Process Communication (IPC): Mutex, Barriers, Semaphores, Message Boxes
- Virtual Memory Management: Paging, Demand paging, Swapping, Protection, provides isolation and protection between processes
- Networking (via E1000): Packet transmission, Reception, Basic protocol handling
- Filesystem with cache: Basic file operations, Cache policy manual configuration


## Directory Structure

- `bootloader/` - Contains a bootloader
- `src/` - Contains the kernel
- `test/` - Contains user level test programs
- `tiny_libc/` - Contains basic runtime library for user programs
- `tools/` - Contains a tool to pack elf files to a single image for run

## Dependencies

- [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain)
- QEMU
- UBOOT modified by UCAS-OS-Lab course

### Usage

Use make to build the kernel and user-programs. It will generate a image file under build directory.

```bash
make
```

Simulate in QEMU:

```bash
make run-smp #smp stands for multicore
```

Debug using QEMU:

```bash
make debug-smp # start QEMU in debug mode
# in another terminal
make gdb # start GDB and connect to QEMU
```

Run on board provided by the course:

```bash
make floppy # burn image to sdcard
# plugin sdcard into the board, then start the board and connect the board to main computer
make minicom # interact with the board
```

