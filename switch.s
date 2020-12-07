	.section .text
	.globl   switch2
switch2:
	subq $21, %rdx
	movq $0, %rax
	cmpq $10, %rdx
	ja .L1
	jmp *.L32(,%rdx,8)
.L6:
	movq (%rdi), %rbx
	movq %rbx, (%rsi)
	movq (%rsi), %rax
	ret
.L4:
	movq (%rsi), %rbx
	addq %rbx, (%rdi)
	movq (%rdi), %rax
	ret
.L8:
.L9:
	subq $59, (%rdi)
	movq (%rdi), %rbx
	subq %rbx, (%rsi)
	ret
.L2:
	addq $60, (%rdi)
.L0:
	movq (%rsi), %rax
	imulq %rax, %rax
	ret
.L10:
	movq (%rsi), %rax
	movq (%rdi), %rcx
	sarq %cl, %rax
	ret
.L1:
	movq $12, %rax
	movq $3, %rcx
	salq %cl, %rax
	ret

.section .rodata
	.align 8
.L32:
	.quad	.L0
	.quad	.L1
	.quad	.L2
	.quad	.L1
	.quad	.L4
	.quad	.L1
	.quad	.L6
	.quad	.L1
	.quad	.L8
	.quad	.L9
	.quad	.L10
