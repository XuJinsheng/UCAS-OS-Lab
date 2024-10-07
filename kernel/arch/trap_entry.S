#include <asm.h>

.equ USER_BASE, 0

.section .text
.globl init_trap
.globl user_trap_entry
.globl user_trap_return
.globl kernel_trap_entry
.globl switch_context_entry


.balign 4

user_trap_entry:
	csrw sscratch, x31
	la x31, current_running
	ld x31, 0(x31)
	sd x1, (USER_BASE+0)(x31)
	sd x2, (USER_BASE+8)(x31)
	sd x3, (USER_BASE+16)(x31)
	sd x4, (USER_BASE+24)(x31)
	sd x5, (USER_BASE+32)(x31)
	sd x6, (USER_BASE+40)(x31)
	sd x7, (USER_BASE+48)(x31)
	sd x8, (USER_BASE+56)(x31)
	sd x9, (USER_BASE+64)(x31)
	sd x10, (USER_BASE+72)(x31)
	sd x11, (USER_BASE+80)(x31)
	sd x12, (USER_BASE+88)(x31)
	sd x13, (USER_BASE+96)(x31)
	sd x14, (USER_BASE+104)(x31)
	sd x15, (USER_BASE+112)(x31)
	sd x16, (USER_BASE+120)(x31)
	sd x17, (USER_BASE+128)(x31)
	sd x18, (USER_BASE+136)(x31)
	sd x19, (USER_BASE+144)(x31)
	sd x20, (USER_BASE+152)(x31)
	sd x21, (USER_BASE+160)(x31)
	sd x22, (USER_BASE+168)(x31)
	sd x23, (USER_BASE+176)(x31)
	sd x24, (USER_BASE+184)(x31)
	sd x25, (USER_BASE+192)(x31)
	sd x26, (USER_BASE+200)(x31)
	sd x27, (USER_BASE+208)(x31)
	sd x28, (USER_BASE+216)(x31)
	sd x29, (USER_BASE+224)(x31)
	sd x30, (USER_BASE+232)(x31)
	csrr t0, sscratch
	sd t0, (USER_BASE+240)(x31)
	csrr t0, sstatus
	sd t0, (USER_BASE+248)(x31)
	csrr t0, sepc
	sd t0, (USER_BASE+256)(x31)
	csrr t0, scause
	sd t0, (USER_BASE+264)(x31)
	csrr t0, stval
	sd t0, (USER_BASE+272)(x31)

	ld sp, (USER_BASE+392)(x31)

	la t1, kernel_trap_entry
	csrw stvec, t1

	call user_trap_handler

user_trap_return:# return to user space, kernel stack is clear
	la x31, current_running
	ld x31, 0(x31)
	ld x1, (USER_BASE+256)(x31)
	csrw sepc, x1
	la t0, user_trap_entry
	csrw stvec, t0

	ld x1, (USER_BASE+0)(x31)
	ld x2, (USER_BASE+8)(x31)
	ld x3, (USER_BASE+16)(x31)
	ld x4, (USER_BASE+24)(x31)
	ld x5, (USER_BASE+32)(x31)
	ld x6, (USER_BASE+40)(x31)
	ld x7, (USER_BASE+48)(x31)
	ld x8, (USER_BASE+56)(x31)
	ld x9, (USER_BASE+64)(x31)
	ld x10, (USER_BASE+72)(x31)
	ld x11, (USER_BASE+80)(x31)
	ld x12, (USER_BASE+88)(x31)
	ld x13, (USER_BASE+96)(x31)
	ld x14, (USER_BASE+104)(x31)
	ld x15, (USER_BASE+112)(x31)
	ld x16, (USER_BASE+120)(x31)
	ld x17, (USER_BASE+128)(x31)
	ld x18, (USER_BASE+136)(x31)
	ld x19, (USER_BASE+144)(x31)
	ld x20, (USER_BASE+152)(x31)
	ld x21, (USER_BASE+160)(x31)
	ld x22, (USER_BASE+168)(x31)
	ld x23, (USER_BASE+176)(x31)
	ld x24, (USER_BASE+184)(x31)
	ld x25, (USER_BASE+192)(x31)
	ld x26, (USER_BASE+200)(x31)
	ld x27, (USER_BASE+208)(x31)
	ld x28, (USER_BASE+216)(x31)
	ld x29, (USER_BASE+224)(x31)
	ld x30, (USER_BASE+232)(x31)
	ld x31, (USER_BASE+240)(x31)
	sret


kernel_trap_entry:
	.word 0
	sret

# void switch_context_entry(kernel_context_reg_t *from_reg,kernel_context_reg_t *to_reg);
switch_context_entry:
	sd ra, 0(a0)
	sd sp, 8(a0)
	sd s0, 16(a0)
	sd s1, 24(a0)
	sd s2, 32(a0)
	sd s3, 40(a0)
	sd s4, 48(a0)
	sd s5, 56(a0)
	sd s6, 64(a0)
	sd s7, 72(a0)
	sd s8, 80(a0)
	sd s9, 88(a0)
	sd s10, 96(a0)
	sd s11, 104(a0)
	ld ra, 0(a1)
	ld sp, 8(a1)
	ld s0, 16(a1)
	ld s1, 24(a1)
	ld s2, 32(a1)
	ld s3, 40(a1)
	ld s4, 48(a1)
	ld s5, 56(a1)
	ld s6, 64(a1)
	ld s7, 72(a1)
	ld s8, 80(a1)
	ld s9, 88(a1)
	ld s10, 96(a1)
	ld s11, 104(a1)
	ret



