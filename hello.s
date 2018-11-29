
.section .data

hello:
	.string "hello world\n"

.section .text

.global _start

_start:

	movq $1, %rax		# syscall write
	movq $1, %rdi		# stdout
	movq $hello, %rsi	# output string
	movq $0x0c, %rdx	# string length
	syscall

	movq $0x3c, %rax	# syscall exit
	movq $0, %rdi		# exit code
	syscall
