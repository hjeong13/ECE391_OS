/* idt.c
*
 */

#include "idt.h"
#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "rtc.h"
#include "interruptlinkage.h"
#include "systemcalls.h"
#include "pit.h"

#define KB_port		0x21			//keyboard port value
#define RTC_port	0x28			// real time clock port value
#define SYSYEM_CALL_port 0x80
#define PIT_port 	0x20

/*
	void divide_error_exception()
		Input: none
		Return value: none
		Function: handling divide error exception. Print out Divided error exception on screen if interrupt occur
*/
void divide_error_exception(){
	printf("Divided Error Exception! \n");
	cli();
	while(1);
	sti();
}
/*
	void debug_exception()
		Input: none
		Return value: none
		Function: handling debug exception. Print out Debug Exception on screen if interrupt occur
*/
void debug_exception(){
	printf("Debug Exception!");
	cli();
	while(1);
	sti();
}
/*
	void nmi_interrupt()
		Input: none
		Return value: none
		Function: handling NMI Interrupt. Print out NMI interrupt on screen if interrupt occur
*/
void nmi_interrupt(){
	printf("NMI Interrupt!");
	cli();
	while(1);
	sti();
}
/*
	void breakpoint_exception()
		Input: none
		Return value: none
		Function: handling Breakpoint Exception. Print out Breakpoint Exception on screen if interrupt occur
*/
void breakpoint_exception(){
	printf("Breakpoint Exception!");
	cli();
	while(1);
	sti();
}

/*
	void overflow_exception()
		Input: none
		Return value: none
		Function: handling overflow_exception. Print out overflow_exception on screen if interrupt occur
*/
void overflow_exception(){
	printf("Overflow Exception!");
	cli();
	while(1);
	sti();
}
/*
	void bound_range_exceeded_exception()
		Input: none
		Return value: none
		Function: handling Bound Range Exceeded Exception. Print out Bound Range Exceeded Exception on screen if interrupt occur
*/
void bound_range_exceeded_exception(){
	printf("Bound Range Exceeded Exception!");
	cli();
	while(1);
	sti();
}
/*
	void invalid_opcode_exception()
		Input: none
		Return value: none
		Function: handling Invalid Opcode Exception. Print out Invalid Opcode Exception on screen if interrupt occur
*/
void invalid_opcode_exception(){
	printf("Invalid Opcode Exception!");
	cli();
	while(1);
	sti();
}

/*
	void device_not_available_exception()
		Input: none
		Return value: none
		Function: handling Device Not Available Exception. Print out Device Not Available Exception on screen if interrupt occur
*/
void device_not_available_exception(){
	printf("Device Not Available Exception!");
	cli();
	while(1);
	sti();
}

/*
	void double_fault_exception()
		Input: none
		Return value: none
		Function: handling double_fault_exception. Print out Double Fault Exception on screen if interrupt occur
*/
void double_fault_exception(){
	printf("Double Fault Exception!");
	cli();
	while(1);
	sti();
}
/*
	void coprocessor_segment_overrun()
		Input: none
		Return value: none
		Function: handling Coprocessor Segment Overrun. Print out Coprocessor Segment Overrun on screen if interrupt occur
*/
void coprocessor_segment_overrun(){
	printf("Coprocessor Segment Overrun!");
	cli();
	while(1);
	sti();
}
/*
	void invalid_tss_exception()
		Input: none
		Return value: none
		Function: handling Invalid TSS Exception. Print out Invalid TSS Exception on screen if interrupt occur
*/
void invalid_tss_exception(){
	printf("Invalid TSS Exception!");
	cli();
	while(1);
	sti();
}

/*
	void segment_not_present()
		Input: none
		Return value: none
		Function: handling Segment Not Present. Print out Segment Not Present on screen if interrupt occur
*/
void segment_not_present(){
	printf("Segment Not Present!");
	cli();
	while(1);
	sti();
}

/*
	void stack_fault_exception()
		Input: none
		Return value: none
		Function: handling Stack Fault Exception. Print out Stack Fault Exception on screen if interrupt occur
*/
void stack_fault_exception(){
	printf("Stack Fault Exception!");
	cli();
	while(1);
	sti();
}

/*
	void general_protection_exception()
		Input: none
		Return value: none
		Function: handling General Protection Exception. Print out General Protection Exception on screen if interrupt occur
*/
void general_protection_exception(){
	printf("General Protection Exception!");
	cli();
	while(1);
	sti();
}

/*
	void page_fault_exception()
		Input: none
		Return value: none
		Function: handling Page Fault Exception. Print out Page Fault Exception on screen if interrupt occur
*/
void page_fault_exception(){
	printf("Page Fault Exception!");
	cli();
	while(1);
	sti();
}

/*
	void x87_fpu_floating_point_error()
		Input: none
		Return value: none
		Function: handling x87_FPU Floating Point Error. Print out x87_FPU Floating Point Error on screen if interrupt occur
*/
void x87_fpu_floating_point_error(){
	printf("x87_FPU Floating Point Error!");
	cli();
	while(1);
	sti();
}

/*
	void alignment_check_exception()
		Input: none
		Return value: none
		Function: handling Alignment Check Exception. Print out Alignment Check Exception on screen if interrupt occur
*/
void alignment_check_exception(){
	printf("Alignment Check Exception!");
	cli();
	while(1);
	sti();
}

/*
	void machine_check_exception()
		Input: none
		Return value: none
		Function: handling Machine Check Exception. Print out Machine Check Exception on screen if interrupt occur
*/
void machine_check_exception(){
	printf("Machine Check Exception");
	cli();
	while(1);
	sti();
}

/*
	void simd_floating_point_exception()
		Input: none
		Return value: none
		Function: handling SIMD Floating Point Exception. Print out SIMD Floating Point Exception on screen if interrupt occur
*/
void simd_floating_point_exception(){
	printf("SIMD Floating Point Exception! \n");
	cli();
	while(1);
	sti();
}

/*
	void keyboard_read()
		Input: none
		Return value: none
		Function: handling Keyboard Exception. Print out keyboard Exception on screen if interrupt occur
*/
void keyboard_read(){
	char c;
	printf("Keyboard Input Recognized? ");
	cli();
	c = inb(0x60);
	send_eoi(0x01);
	sti();
}
/*
	void RTC_interrupt()
		Input: none
		Return value: none
		Function: handling RTC Exception. Print out RTC functioning well? on screen if interrupt occur
*/
void RTC_interrupt(){
	printf("RTC functioning well? \n");
	while(1);
}

/*
	void idt_init()
		Input: none
		Return value: none
		Function: Initializing interrupt data table.
				  Using the struct that was made in Kernel.c, assigned each bit with corresponding value.
				  Starting from bottom is the corresponding bit for interrupt gate and trap gate.
				  15th	   bit - present
				  14,13th  bit - DPL
				  12th	   bit - reserved 0
				  11th 	   bit - size
				  10th	   bit - reserved 1
				  9th 	   bit - reserved 2
				  8th 	   bit - reserved 3
				  7,6,5th  bit - reserved 4
*/
/* Initialize the idt */
void idt_init(void) {
	int i;
	/* 32 exceptions */
	/* TRAP Gate / Software interrupt / EXCEPTION */
	for( i = 0; i < 32; i++){
		idt[i].seg_selector = KERNEL_CS; 	/* Kernel code segment*/
		idt[i].reserved4 = 0;
		idt[i].reserved3 = 1;				/* interrupt gate = 0 / trap gate = 1 */
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;						/* KERNEL LEVEL:0 / USER LEVEL: 3 */
		if( i < 20 ){
			idt[i].present = 1;
		}
		else{
			idt[i].present = 0;
		}
	}
	
	/* INTERRUPT Gate / Hardware interrupt */
	for ( i = 32; i < 256; i++){
		idt[i].seg_selector = KERNEL_CS; 	/* Kernel code segment*/
		idt[i].reserved4 = 0;
		idt[i].reserved3 = 0;				/* interrupt gate = 0 / trap gate = 1 */
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;						/* KERNEL LEVEL:0 / USER LEVEL: 3 */
		idt[i].present = 0;
	}
	/*assigning each exceptions with function that we implemented to distinguish which interrupt has occurred by printing out on screen*/
	SET_IDT_ENTRY(idt[0], divide_error_exception);
	SET_IDT_ENTRY(idt[1], debug_exception);
	SET_IDT_ENTRY(idt[2], nmi_interrupt);
	SET_IDT_ENTRY(idt[3], breakpoint_exception);
	SET_IDT_ENTRY(idt[4], overflow_exception);
	SET_IDT_ENTRY(idt[5], bound_range_exceeded_exception);
	SET_IDT_ENTRY(idt[6], invalid_opcode_exception);
	SET_IDT_ENTRY(idt[7], device_not_available_exception);
	SET_IDT_ENTRY(idt[8], double_fault_exception);
	SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun);
	SET_IDT_ENTRY(idt[10], invalid_tss_exception);
	SET_IDT_ENTRY(idt[11], segment_not_present);
	SET_IDT_ENTRY(idt[12], stack_fault_exception);
	SET_IDT_ENTRY(idt[13], general_protection_exception);
	SET_IDT_ENTRY(idt[14], page_fault_exception);
	//SET_IDT_ENTRY(idt[15], divide_error_exception);
	SET_IDT_ENTRY(idt[16], x87_fpu_floating_point_error);
	SET_IDT_ENTRY(idt[17], alignment_check_exception);
	SET_IDT_ENTRY(idt[18], machine_check_exception);
	SET_IDT_ENTRY(idt[19], simd_floating_point_exception);        
	
	/* Set Keyboard */
	idt[KB_port].present = 1;
	SET_IDT_ENTRY(idt[KB_port], keyboard_link);
	
	
	/* Set RTC */
	idt[RTC_port].present = 1;
	SET_IDT_ENTRY(idt[RTC_port], rtc_link);

	/* Set System Calls */
	idt[SYSYEM_CALL_port].present = 1;
	idt[SYSYEM_CALL_port].dpl = 3;			// System Call User-level
	SET_IDT_ENTRY(idt[SYSYEM_CALL_port], systemcall_link);
	
	/* Set PIT */
	idt[PIT_port].present = 1;
	SET_IDT_ENTRY(idt[PIT_port], pit_link);
	
};



