/* idt.h - 
 */

#ifndef _IDT_H
#define _IDT_H

#include "types.h"
#include "x86_desc.h"

/* Exception Handler */

/*printing divide error exception on screen*/
void divide_error_exception();

/*printing debug exception on screen*/
void debug_exception();

/*printing nmi interrupt on screen*/
void nmi_interrupt();

/*printing breakpoint exception on screen*/
void breakpoint_exception();

/*printing overflow exception on screen*/
void overflow_exception();

/*printing bound range exceed exception on screen*/
void bound_range_exceeded_exception();

/*printing invalid code exception on screen*/
void invalid_opcode_exception();

/*printing device not available exception on screen*/
void device_not_available_exception();

/*printing double fault exception on screen*/
void double_fault_exception();

/*printing coprocessor segment exception on screen*/
void coprocessor_segment_overrun();

/*printing invalid tss exception on screen*/
void invalid_tss_exception();

/*printing segment not present exception on screen*/
void segment_not_present();

/*printing stack fault exception on screen*/
void stack_fault_exception();

/*printing general protection exception on screen*/
void general_protection_exception();

/*printing page fault exception on screen*/
void page_fault_exception();

/*printing x87 fpu floating point exception on screen*/
void x87_fpu_floating_point_error();

/*printing alignment check exception on screen*/
void alignment_check_exception();

/*printing machine check exception on screen*/
void machine_check_exception();

/*printing simd floating point exception on screen*/
void simd_floating_point_exception();

/*printing real time clock interrupt on screen*/
void RTC_read();

/*printing keyboard interrupt on screen*/
void KB_read();

/* Initialize IDT */
void idt_init(void);
#endif /* _IDT_H */

