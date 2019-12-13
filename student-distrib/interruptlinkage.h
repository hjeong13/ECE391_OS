/* interruptlinkage.h - links to interrupt handler
 * vim:ts=4 noexpandtab
 */
 
#ifndef _INTERRUPTLINKAGE_H
#define _INTERRUPTLINKAGE_H
 
extern void keyboard_link();
extern void rtc_link();
extern void systemcall_link();
extern void pit_link();
 
#endif
