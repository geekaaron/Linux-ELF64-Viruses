
	.section	.text
	.globl	_start

	_start:

		# Create a file called Hack.txt in path
		# /home/aaron/Desktop. Ok, if you wanna
		# use this program you need to change
		# the path below.

		pushq	%rax
		pushq	%rbx
		pushq	%rcx
		pushq	%rdx
		pushq	%rsi
		pushq	%rdi

		movq	$0x000000007478742e,	%rax
		movq	$0x6b6361482f706f74,	%rbx
		movq	$0x6b7365442f6e6f72,	%rcx
		movq	$0x61612f656d6f682f,	%rdx
		pushq	%rax
		pushq	%rbx
		pushq	%rcx
		pushq	%rdx

		movq	$2,	%rax
		leaq	(%rsp),	%rdi
		movq	$0100,	%rsi
		movq	$0444,	%rdx
		syscall

		movq	%rax,	%rdi
		movq	$3,	%rax
		syscall

		addq	$32,	%rsp

		popq	%rdi
		popq	%rsi
		popq	%rdx
		popq	%rcx
		popq	%rbx
		popq	%rax
