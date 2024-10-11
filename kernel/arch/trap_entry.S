.equ USER_BASE, 0
.equ KERNEL_STACK_IN_THREAD, 280

.section .text
.globl init_trap
.globl user_trap_entry
.globl user_trap_return
.globl kernel_trap_entry
.globl switch_context_entry


.balign 4
user_trap_entry:
	csrrw sp,sscratch,sp
	// sp = current_running
	sd x1, (USER_BASE+0)(sp)
	sd x3, (USER_BASE+16)(sp)
	sd x4, (USER_BASE+24)(sp)
	sd x5, (USER_BASE+32)(sp)
	sd x6, (USER_BASE+40)(sp)
	sd x7, (USER_BASE+48)(sp)
	sd x8, (USER_BASE+56)(sp)
	sd x9, (USER_BASE+64)(sp)
	sd x10, (USER_BASE+72)(sp)
	sd x11, (USER_BASE+80)(sp)
	sd x12, (USER_BASE+88)(sp)
	sd x13, (USER_BASE+96)(sp)
	sd x14, (USER_BASE+104)(sp)
	sd x15, (USER_BASE+112)(sp)
	sd x16, (USER_BASE+120)(sp)
	sd x17, (USER_BASE+128)(sp)
	sd x18, (USER_BASE+136)(sp)
	sd x19, (USER_BASE+144)(sp)
	sd x20, (USER_BASE+152)(sp)
	sd x21, (USER_BASE+160)(sp)
	sd x22, (USER_BASE+168)(sp)
	sd x23, (USER_BASE+176)(sp)
	sd x24, (USER_BASE+184)(sp)
	sd x25, (USER_BASE+192)(sp)
	sd x26, (USER_BASE+200)(sp)
	sd x27, (USER_BASE+208)(sp)
	sd x28, (USER_BASE+216)(sp)
	sd x29, (USER_BASE+224)(sp)
	sd x30, (USER_BASE+232)(sp)
	sd x31, (USER_BASE+240)(sp)
	csrr a0, sscratch
	sd a0, (USER_BASE+8)(sp)
	csrr a0, sstatus
	sd a0, (USER_BASE+248)(sp)
	csrr a0, sepc
	sd a0, (USER_BASE+256)(sp)
	csrr a0, scause
	sd a0, (USER_BASE+264)(sp)
	csrr a0, stval
	sd a0, (USER_BASE+272)(sp)

	mv tp, sp//current_running
	ld sp, KERNEL_STACK_IN_THREAD(sp)//kernel_stack_top

	la a0, kernel_trap_entry
	csrw stvec, a0

	la a0, 0// from_kernel=false
	csrr a1, scause
	csrr a2, stval
	call trap_handler

user_trap_return:# return to user space, kernel stack is clear
	mv sp, tp
	csrw sscratch, tp
	li a0, 1<<8// set sstatus.SPP=user mode
	csrc sstatus, a0
	ld a0, (USER_BASE+256)(sp)
	csrw sepc, a0
	la a0, user_trap_entry
	csrw stvec, a0

	ld x1, (USER_BASE+0)(sp)
	ld x3, (USER_BASE+16)(sp)
	ld x4, (USER_BASE+24)(sp)
	ld x5, (USER_BASE+32)(sp)
	ld x6, (USER_BASE+40)(sp)
	ld x7, (USER_BASE+48)(sp)
	ld x8, (USER_BASE+56)(sp)
	ld x9, (USER_BASE+64)(sp)
	ld x10, (USER_BASE+72)(sp)
	ld x11, (USER_BASE+80)(sp)
	ld x12, (USER_BASE+88)(sp)
	ld x13, (USER_BASE+96)(sp)
	ld x14, (USER_BASE+104)(sp)
	ld x15, (USER_BASE+112)(sp)
	ld x16, (USER_BASE+120)(sp)
	ld x17, (USER_BASE+128)(sp)
	ld x18, (USER_BASE+136)(sp)
	ld x19, (USER_BASE+144)(sp)
	ld x20, (USER_BASE+152)(sp)
	ld x21, (USER_BASE+160)(sp)
	ld x22, (USER_BASE+168)(sp)
	ld x23, (USER_BASE+176)(sp)
	ld x24, (USER_BASE+184)(sp)
	ld x25, (USER_BASE+192)(sp)
	ld x26, (USER_BASE+200)(sp)
	ld x27, (USER_BASE+208)(sp)
	ld x28, (USER_BASE+216)(sp)
	ld x29, (USER_BASE+224)(sp)
	ld x30, (USER_BASE+232)(sp)
	ld x31, (USER_BASE+240)(sp)
	ld x2, (USER_BASE+8)(sp)
	sret

.balign 4
kernel_trap_entry:
	addi sp, sp, -18*8
	sd ra, 0(sp)
	sd t0, 8(sp)
	sd t1, 16(sp)
	sd t2, 24(sp)
	sd t3, 32(sp)
	sd t4, 40(sp)
	sd t5, 48(sp)
	sd t6, 56(sp)
	sd a0, 64(sp)
	sd a1, 72(sp)
	sd a2, 80(sp)
	sd a3, 88(sp)
	sd a4, 96(sp)
	sd a5, 104(sp)
	sd a6, 112(sp)
	sd a7, 120(sp)
	csrr a0, sepc
	sd a0, 128(sp)
	csrr a0, sstatus
	sd a0, 136(sp)


	li a0,1//from_kernel=true
	csrr a1,scause
	csrr a2,stval
	call trap_handler

	li a0, 1<<8	// set sstatus.SPP=kernel mode
	csrc sstatus, a0
	ld a0, 128(sp)
	csrw sepc, a0
	ld a0, 136(sp)
	csrw sstatus, a0

	ld ra, 0(sp)
	ld t0, 8(sp)
	ld t1, 16(sp)
	ld t2, 24(sp)
	ld t3, 32(sp)
	ld t4, 40(sp)
	ld t5, 48(sp)
	ld t6, 56(sp)
	ld a0, 64(sp)
	ld a1, 72(sp)
	ld a2, 80(sp)
	ld a3, 88(sp)
	ld a4, 96(sp)
	ld a5, 104(sp)
	ld a6, 112(sp)
	ld a7, 120(sp)
	addi sp, sp, 18*8
	sret

# void switch_context_entry(Thread* to);
.balign 4
switch_context_entry:
	addi sp, sp, -14*8	// 14 is related to Thread construction
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
	ld s0, KERNEL_STACK_IN_THREAD(tp)
	sd s0, 104(sp)
	sd sp, KERNEL_STACK_IN_THREAD(tp)

	mv tp,a0
	ld sp, KERNEL_STACK_IN_THREAD(tp)
	ld s0, 104(sp)
	sd s0, KERNEL_STACK_IN_THREAD(tp)
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
	addi sp, sp, 14*8
	ret



