#include "pit.h"

uint32_t my_array[3] = {0x8801000, 0x8802000, 0x8803000};
uint32_t my_second[3] = {0xB9000, 0xBA000, 0xBB000};

void pit_init(void){
	outb(PIT_SQUARE_WAVE, PIT_CMD_REG);
	outb(_20HZ & FREQUENCY_MASK, PIT_CHANNEL_0);
	outb(_20HZ >> 8, PIT_CHANNEL_0);
	enable_irq(0);
}

// triple fault because of esp
void pit_handler(void){
	
	cli();
	
	uint8_t next_running_process;
	uint8_t next_schedule_terminal;
	uint8_t i;
	uint8_t * temp;
	send_eoi(0);
	
	if(terminal_array[1].running + terminal_array[2].running == 0){
		sti();
		return;
	}
	
	for(i = 0; i < 2; i++){
		if(terminal_array[(schedule_terminal + 1 + i) % 3].running == 1){
			next_schedule_terminal = (schedule_terminal + 1 + i) % 3; //1
			next_running_process = terminal_array[next_schedule_terminal].current_process_number; //205?
			break;
		}
	} 
	
	
	page_process(_128_MB, _8_MB + next_running_process * _4_MB);
	
	
	if(next_schedule_terminal == current_terminal){
		remap_video(-1);
		vidmap_terminal(temp, 0xB8000);
	}
	else{
		remap_video(next_schedule_terminal);
		vidmap_terminal(temp, (uint32_t)terminal_array[next_schedule_terminal].video_memory);
	}
	
	tss.ss0=KERNEL_DS;
	tss.esp0= _8_MB - PCB_SIZE * (next_running_process) - 4;

	pcb_t* running_pcb = (pcb_t*)(_8_MB - (terminal_array[schedule_terminal].current_process_number + 1) * PCB_SIZE);	
	
	//dbug point
	asm volatile("movl %%esp, %0" : "=g" (running_pcb->current_esp));
	asm volatile("movl %%ebp, %0" : "=g" (running_pcb->current_ebp));
	
	schedule_terminal = next_schedule_terminal;
	pcb_t* next_running_pcb = (pcb_t*)(_8_MB - (terminal_array[schedule_terminal].current_process_number + 1) * PCB_SIZE);
	asm volatile("movl %0, %%esp" : : "r" (next_running_pcb->current_esp));
	asm volatile("movl %0, %%ebp" : : "r" (next_running_pcb->current_ebp));
	
	sti();
	return;
	
	
	/*
	uint8_t pit_next_terminal = (get_pit_terminal() + 1) % NUM_TERMINAL;
	int i;
	send_eoi(0);

	else if(terminal_array[0].running && terminal_array[1].running && terminal_array[2].running){
		for(i = 0; i < 3; i++)
		{
			if(i == terminal_displayed)
			{
				page_process_video(my_array[i], 0xB8000);
			}
			else
			{
				page_process_video(my_array[i], my_second[i]);
			}
		}
		
		page_process(_128_MB, _8_MB + terminal_array[pit_next_terminal].current_process_number * _4_MB);
		
		tss.ss0=KERNEL_DS;
		tss.esp0= _8_MB - PCB_SIZE * (terminal_array[pit_next_terminal].current_process_number) - 4;
		
		pcb_t * pcb = (pcb_t *)(_8_MB - (terminal_array[get_pit_terminal()].current_process_number + 1) * PCB_SIZE);
		set_pit_terminal(pit_next_terminal);
		asm volatile("movl %%esp, %0" : "=g" (pcb->current_esp));
		asm volatile("movl %%ebp, %0" : "=g" (pcb->current_ebp));
		
		pcb = (pcb_t *)(_8_MB - (terminal_array[pit_next_terminal].current_process_number + 1) * PCB_SIZE);
		asm volatile("movl %0, %%esp" : : "r" (pcb->current_esp));
		asm volatile("movl %0, %%ebp" : : "r" (pcb->current_ebp));
		
	}*/
	send_eoi(0);
	sti();
	return;
}
