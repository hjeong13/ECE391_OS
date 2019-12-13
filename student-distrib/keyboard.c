/* keyboard.c
*
 */

#include "idt.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"
#include "systemcalls.h"

#define KB_IRQ		0x01			//IRQ1
#define DATA_PORT	0x60			//DATA port for keyboard
#define BUFFER_SIZE 128

//0x64 is command port of keyboard
static char keyboard_buffer[BUFFER_SIZE];
static uint8_t keyboard_buffer_index;
uint8_t caps_lock; //this variable is 0 if capslock is off and 1 if on
uint8_t shift; //this variable is 0 if shift is off and 1 if on
uint8_t ctrl; //this variable is 0 if shift is off and 1 if on
uint8_t alt; //this varialbe is 0 if alt is unpressed and 1 if 1 pressed
uint8_t enter[3];


/*
	is_enter: this function tells us if enter was pushed
	INPUTS: NONE
	OUTPUTS: 1 if it was pushed, 0 if it wasn't
*/
uint8_t is_enter()
{
	return enter[schedule_terminal];
}

void vidmap_terminal (uint8_t* screen_start, uint32_t term_vid_mem){
		
		if(screen_start == NULL || screen_start == (uint8_t*)_4_MB){
			return;
		}
		
		//remap the paging 0x8800000
		page_process_video( (uint32_t)VIDEO_MEM_START, (uint32_t)term_vid_mem );
		//change the screen_start address to 136MB
		screen_start = (uint8_t*)VIDEO_MEM_START;

		//return VIDEO_MEM_START;
		
}

void terminal_change(uint8_t next_terminal){
	remap_video(-1);
	
	memcpy((void*)terminal_array[current_terminal].video_memory, (void*)0xB8000, 4096);
	memcpy((void*)0xB8000, (void*)terminal_array[next_terminal].video_memory, 4096);
	
	terminal_array[current_terminal].x_position = get_cursor_x();
	terminal_array[current_terminal].y_position = get_cursor_y();
	
	memcpy((void*)terminal_array[current_terminal].keyboard_buffer, (void*)keyboard_buffer, 128);
	memcpy((void*)keyboard_buffer, (void*)terminal_array[next_terminal].keyboard_buffer, 128);
	
	terminal_array[current_terminal].keyboard_buffer_index = keyboard_buffer_index;
	keyboard_buffer_index = terminal_array[next_terminal].keyboard_buffer_index;	
	
	
	uint8_t* screen_start;
	vidmap(&screen_start);
	
	//vidmap_terminal(screen_start, (uint32_t)terminal_array[next_terminal].video_memory);
	//remap_video(-1);
	
	current_terminal = next_terminal;

	set_cursor_terminal(terminal_array[current_terminal].x_position, terminal_array[current_terminal].y_position);
	
}

/*
get_keyboard_buffer: this function provides a pointer to the keyboard buffer
INPUTS: NONE
OUTPUTS: keyboard_buffer
*/
char * get_keyboard_buffer(void)
{
	if(schedule_terminal == current_terminal)
	{
		return keyboard_buffer;
	}
	else
	{
		return (char *)terminal_array[schedule_terminal].keyboard_buffer;
	}
}

/*
reset_keyboard_buffer: this function resets to keyboard buffer to all 0s and resets the keyboard buffer index to 0
INPUTS: NONE
OUTPUTS: NONE
*/
void reset_keyboard_buffer(void)
{
	int i = 0;
	while(i < BUFFER_SIZE)
	{
		keyboard_buffer[i] = 0;
		i++;
	}
	keyboard_buffer_index = 0;
	enter[1] = 0;
	enter[2] = 0;
	enter[3] = 0;
}

/* void keyboard_init(void)
 * Inputs: NONE
 * Return Value: NONE
 * Function: This function initializes keyboard by enable KB_IRQ port which is 0x01 (IRQ1) by calling
 *			 enable_irq() function in i8259.c
 */
void keyboard_init(void){
	int i = 0;
	while(i < BUFFER_SIZE) //initialize keyboard buffer to 0
	{
		keyboard_buffer[i] = 0;

		i++;
	}
	
	caps_lock = 0; //we have to initialize all of the variables for the keyboard
	shift = 0;
	ctrl = 0;
	keyboard_buffer_index = 0;
	enter[schedule_terminal] = 0;
	clear();	//clear the screen
	set_cursor_init(); //puts us back at the top of the screen
	enable_irq(KB_IRQ);	//enable irq1 which is keyboard
}

void terminal_run(uint8_t prev_term){
		pcb_t* pcb = (pcb_t*)(_8_MB - PCB_SIZE*(terminal_array[prev_term].current_process_number + 1));	
		
		
		asm volatile( "movl %%esp, %0" : "=g" (pcb->current_esp));
		asm volatile( "movl %%ebp, %0" : "=g" (pcb->current_ebp));

		//terminal_array[current_terminal].running=1;
		execute((const uint8_t *)"shell");
}


//this array is for when shift and caps lock are off
static uint8_t input_array_lower[58] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 		// 12
								  '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 	// 14
								  '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',		// 13
								  ';', 0x27, 0x60, 0, 0x5C, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 0x20};
//this array is for when shift is on
static uint8_t input_array_upper[58] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
									'_', '+', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 	// 14
								  '{', '}', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',		// 13
								  ':', 0x22, 0x7E, 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, 0x20};
//this array is for when caps lock is on
static uint8_t input_array_lock[58] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
									'-', '=', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 	// 14
								  '[', ']', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',		// 13
								  ';', 0x27, 0x60, 0, 0x5C, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, 0, 0, 0x20};
//this array is for when shift and caps lock are both on
static uint8_t input_array_shift_lock[58] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
									'_', '+', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 	// 14
								  '{', '}', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',		// 13
								  ':', 0x22, 0x7E, 0, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0, 0, 0, 0x20};



/* void keyboard_handler(void)
 * Inputs: NONE
 * Return Value: NONE
 * Function: Gets the input from keyboard. inb(DATA_PORT) enables to get input from keyboard
 *			If the valid inputs are entered, print out to the screen. Call send_eoi once it is done
 *			Also it is important to clear and set flag using cli and sti
 */
void keyboard_handler(void){
	cli();				//clear flag
	uint8_t k_input = 0;  //variable that contains input
	uint8_t k_print = 0;  //variable to print
	uint8_t temp = 0;
	send_eoi(KB_IRQ);
	k_input = inb(DATA_PORT);		//get the input from keyboard
	//printf("[Number : %d]", k_input);
	switch(k_input)
	{

		case(56): // this is the case where alt is pressed
		alt = 1;
		break;
		case(184): // this is the case where alt is unpressed
		alt = 0;
		break;
		case(28): //this is the case where enter is pressed
		keyboard_buffer[keyboard_buffer_index] ='\n';
		enter[current_terminal] = 1;
		keyboard_buffer_index++;
		new_line();
		break;
		case(14): //this is the case where backspace is pressed
		if(keyboard_buffer_index > 0)
		{
			keyboard_buffer_index--;
			keyboard_buffer[keyboard_buffer_index] = 0;
			backspace();
		}
		break;
		case(29): //this is the case where CTRL is pressed
		ctrl = 1;
		break;
		case(157): //this is the case where CTRL is released
		ctrl = 0;
		break;
		case(42): //this is the case where shift is pressed
		case(54):
		shift = 1;
		break;
		case(170): //this is the case where shift is released
		shift = 0;
		break;
		case(58): //this is the case where capslock is pressed
		if(caps_lock == 0)
		{
			caps_lock = 1;
		}
		else
		{
			caps_lock = 0;
		}
		break;
		//f1
		case(0x3B):
		if(alt)
		{
			if(current_terminal!=0) {
				terminal_change(0);
			}
		}
		break;
		//f2
		case(0x3C):
		if(alt)
		{
			if(current_terminal!=1){
				temp = current_terminal;
				terminal_change(1);
				if( ! terminal_array[1].running	)
					terminal_run(temp);
			}
		}
		break;
		//f3
		case(0x3D):
		if(alt)
		{
			if(current_terminal!=2){
				temp = current_terminal;
				terminal_change(2);
				if( ! terminal_array[2].running	)
					terminal_run(temp);
			}
		}
		break;
		default:
			if((k_input == 38) && (ctrl == 1)) //if ctrl and l are pressed then we clear the screen, set the cursor, and reset the keyboard buffer
			{
				clear();
				set_cursor_init();
				reset_keyboard_buffer();
			}
			else
			{
				if(keyboard_buffer_index < BUFFER_SIZE - 1) //we have to see if we can add the character to the keyboard buffer
				{
					if(k_input > 1 && k_input <= 57){ //this is if the character is a printable character
						if(shift)
						{
							if(caps_lock)
							{
								k_print = input_array_shift_lock[k_input]; //this is the case for caps lock and shift being enabled
							}
							else
							{
								k_print = input_array_upper[k_input]; //this is if shift is only enabled
							}
						}
						else
						{
							if(caps_lock) //this is if caps lock is only enabled
							{
								k_print = input_array_lock[k_input];
							}
							else
							{
								k_print = input_array_lower[k_input]; //this is if nothing is enabled
							}
						}
						keyboard_buffer[keyboard_buffer_index] = k_print; //we add the character to the keyboard buffer
						keyboard_buffer_index++; //we increment the keyboard buffer index
						putc_current(k_print); //we also echo it to the screen
					}
				}
			}
		break;
	}

	//check the validity of the input


	sti();			//set flag
			//send eoi so we could get another interrupts later
}
