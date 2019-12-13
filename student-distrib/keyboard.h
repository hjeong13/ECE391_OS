/* keyboard.h -
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"

uint8_t is_enter(void);

/* Initialize IDT */
/* void keyboard_init(void)
 * Inputs: NONE
 * Return Value: NONE
 * Function: This function initializes keyboard by enable KB_IRQ port which is 0x01 (IRQ1) by calling
 *			 enalbe_irq() function in i8259.c
 */
void keyboard_init(void);
void vidmap_terminal (uint8_t* screen_start, uint32_t term_vid_mem);

/* void keyboard_handler(void)
 * Inputs: NONE
 * Return Value: NONE
 * Function: Gets the input from keyboard. inb(DATA_PORT) enables to get input from keyboard
 *			If the valid inputs are entered, print out to the screen. Call send_eoi once it is done
 *			Also it is important to clear and set flag using cli and sti
 */
void keyboard_handler(void);

//this function gives a pointer to the keyboard buffer
char * get_keyboard_buffer(void);
//this function resets the keyboard buffer to 0 and sets the index to 0
void reset_keyboard_buffer(void);
void terminal_run(uint8_t prev_term);

#endif /* _IDT_H */

