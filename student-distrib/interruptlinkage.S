# interruptlinkage.S - save & pop

.global keyboard_link
.global rtc_link
.global systemcall_link
.global pit_link

# handle keyboard interrupt
keyboard_link:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	pushl	%eax
	pushfl
	call 	keyboard_handler
	popfl
	popl	%eax
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	iret
	
# handle RTC interrupt 	
rtc_link:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	pushl	%eax
	pushfl
	call 	rtc_handler
	popfl
	popl	%eax
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	iret

# handle PIT interrupt
pit_link:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	pushl	%eax
	pushfl
	call 	pit_handler
	popfl
	popl	%eax
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	iret
	
systemcall_link:
	cmpl	$1, %eax
	jl		invalid
	cmpl	$8, %eax
	jg		invalid
		
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi
	pushl	%ebp
	
	pushl	%edx		# 3rd argument
	pushl	%ecx		# 2nd argument
	pushl	%ebx		# 1st argument
	
	addl	$-1, %eax
	sti
	call 	*system_call_table(, %eax, 4)
	cli
	addl	$12, %esp	# pop stack
	
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx
	iret

invalid:
	movl	$-1, %eax
	iret

# system call jump table
system_call_table:
	.long	halt, execute, read, write, open, close, getargs, vidmap	#done

