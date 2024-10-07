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
	csrrw x31,sscratch,x31
	// x31 = current_running
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

	mv tp, x31//current_running
	ld sp, (USER_BASE+280)(x31)//kernel_stack_top

	la t1, kernel_trap_entry
	csrw stvec, t1

	call user_trap_handler

user_trap_return:# return to user space, kernel stack is clear
	mv x31, tp
	csrw sscratch, tp
	ld t0, (USER_BASE+256)(x31)
	csrw sepc, t0
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

# void switch_context_entry(Thread* to);
switch_context_entry:
	addi sp, sp, -13*8
	sd ra, 0(sp)
	sd s0, 8(sp)
	sd s1, 16(sp)
	sd s2, 24(sp)
	sd s3, 32(sp)
	sd s4, 40(sp)
	sd s5, 48(sp)
	sd s6, 56(sp)
	sd s7, 64(sp)
	sd s8, 72(sp)
	sd s9, 80(sp)
	sd s10, 88(sp)
	sd s11, 96(sp)
	sd sp,(USER_BASE+280)(tp)
	mv tp,a0
	ld sp, (USER_BASE+280)(tp)
	ld ra, 0(sp)
	ld s0, 8(sp)
	ld s1, 16(sp)
	ld s2, 24(sp)
	ld s3, 32(sp)
	ld s4, 40(sp)
	ld s5, 48(sp)
	ld s6, 56(sp)
	ld s7, 64(sp)
	ld s8, 72(sp)
	ld s9, 80(sp)
	ld s10, 88(sp)
	ld s11, 96(sp)
	addi sp, sp, 13*8
	ret



