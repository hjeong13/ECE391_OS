/* systemcalls.c
*
 */
 
#include "systemcalls.h"

uint8_t process_status[6] = {0, };
 
 /*jump table that holds file operations*/

fops_t std_in_functions = {none, none, terminal_read, none};
fops_t std_out_functions = {none, none, none, terminal_write};
fops_t dir_functions = {directory_close, directory_open, directory_read, directory_write};
fops_t file_functions = {file_close, file_open, file_read, file_write};
fops_t rtc_functions = {rtc_close, rtc_open, rtc_read, rtc_write};
fops_t init_functions = {none, none, none, none};

int32_t none() {
	return -1;
}

 // restore parent pcb
 
 
/* halt
 *
 * Description: terminates the current pcb and return input value to parent pcb. 
 * Input: status = value that needs to be returned to parent pcb.
 * Output: assign input back into parent pcb.
 * Return Value: status
 */
int32_t halt (uint8_t status){
	pcb_t* cur_pcb;
	int i;
	
	// get current PCB and parent PCB, where we are heading back to
	// Restore parent data using current PCB
	cur_pcb = (pcb_t*)get_current_pcb();
	//pcb_t* parent_pcb = cur_pcb->parent_esp;
	
	// set the process number available
	process_status[cur_pcb->current_pcb_num] = 0;
	
	for(i = 0; i < 8; i++)
	{
		cur_pcb->file_name[i][0] = '\0';
	}
	cur_pcb->argument[0] = '\0';
	
	// if the halting process shell, restart the shell
	if(cur_pcb->parent_pcb_num == -1){
		execute((const uint8_t*)"shell");
	}
	
	// else set TSS ESP to parent ESP
	else{
		tss.esp0 = cur_pcb->parent_esp;
	}
		
	terminal_array[schedule_terminal].current_process_number = cur_pcb->parent_pcb_num;	
	// Restore parent paging
	page_process(_128_MB, _8_MB + cur_pcb->parent_pcb_num * _4_MB);

	// close any relevant FDs
	for(i = 0; i < 7; i++){
		if(cur_pcb->file_descriptor[i].flags == 1){
			close(i);
		}
	}
	
	// Jump to execute return
	asm volatile("						\n\
				movl 	%0, %%eax		\n\
				movl 	%1, %%esp		\n\
				movl 	%2, %%ebp		\n\
				jmp		AFTER_IRET		\n\
				"
				:						\
				: "r"((uint32_t)status), "r"(cur_pcb -> parent_esp), "r"(cur_pcb -> parent_ebp) \
				: "eax"						\
	);
	return 0;
}


/* execute
 *
 * Description: Parse the Command
 *				Check the executable validity
 *				Set up the paging
 *				Load the file into the memory
 *				Create PCB, fill out, and open File Descriptors
 *				Prepare for Context Switch
 *				Context Switch
 *
 * Input: command = user typed command separated with spaces
 * Output: execute typical exe with the parsed command
 * Return Value: -1 for invalid file / 256 for exception / 0 - 255 for halt return value
 * Side Effect: Map to appropriate page with flushing the tlb
 */
int32_t execute(const uint8_t* command){
	

	uint8_t filename[33];
	uint8_t argument[64];
	uint8_t char_buf[4];
	uint8_t entry[4];
	uint8_t arg_exist;
	uint32_t entry_addr;
	
	int i = 0;
	int j = 0;
	int process_number;
	dentry_t my_dentry;
	
	
	// check if the command is valid
	if(command == NULL) 
		return -1;

	
	int command_length = strlen((int8_t*)command);
	int filename_length=0;

	for(i=0; i<command_length; i++){
		if (command[i] == ' ' || command[i] == '\0'|| command[i] == '\n' )
			break;
		filename[i]=command[i];
		filename_length++;
	}
	//put null at the end 
	filename[i]='\0';

	//parse the argument
	if(filename_length!=command_length){
		arg_exist = 1;
		//parse argument
		int index=0;
		for(i=filename_length+1; i < command_length; i++){
			argument[index]=command[i];
			index++;
		}
		argument[index]='\0';
	}
	//does not have argument
	else{
		arg_exist = 0;
	}
	// check file validity
	if(read_dentry_by_name((uint8_t*)filename, &my_dentry))
	{
		puts("Could not read dentry\n");
		return -1;
	}
	// check validty of file name by checking first 4bytes for ELF
	read_data(my_dentry.inode_index, 0, char_buf, 4);

	if(char_buf[0] != 0x7F || char_buf[1] != 0x45 || char_buf[2] != 0x4c || char_buf[3] != 0x46)
	{
		puts("Not an executable\n");
		return -1;
	}


	//entry point from byte 24-27
	if( 4 != read_data(my_dentry.inode_index, 24, entry, 4 ) )
		return -1;
	//set the virtual entry address from 24bytes to 27bytes
	entry_addr = *((uint32_t*)entry);

	//get and assign process number . 0-6
	for( j = 0; j < 6; j++){
		if(!process_status[j]){
			process_status[j] = 1;
			break;
		}
	}
	
	if(j >= 6)
	{
		puts("6 processes already in-progress\n");
		return -1;
	}
	
	process_number = j;
	uint32_t p_esp;
	pcb_t * parent;

	//load the pcb address. Subtract from 0x800000
	pcb_t* new_pcb = (pcb_t*)(_8_MB - (process_number + 1) * PCB_SIZE);
	new_pcb->current_pcb_num = process_number;


	//save esp and ebp  into the struct
	asm volatile ("	movl %%ebp, %0" : "=g" (new_pcb->parent_ebp));
	asm volatile ("	movl %%esp, %0" : "=g" (new_pcb->parent_esp));

	//first process . e.g shell
	if(process_number == 0 || terminal_array[current_terminal].running == 0){
		terminal_array[current_terminal].running = 1;
		schedule_terminal = current_terminal;
		
		new_pcb->parent_pcb_num = -1;
		new_pcb->pcb_terminal_num = current_terminal;
		terminal_array[current_terminal].current_process_number = process_number;
	}
	
	else {
		p_esp = new_pcb->parent_esp;
		p_esp = (p_esp >> 13) << 13;
		parent =(pcb_t *)p_esp;
		new_pcb->parent_pcb_num = parent->current_pcb_num;
		new_pcb->pcb_terminal_num = parent->pcb_terminal_num;
		terminal_array[schedule_terminal].current_process_number = process_number;
	}

	//map page in 0x8000000 virtual address to 0x800000 physical address
	//All user level programs will be loaded in the page starting at 128MB (virtual memory)
	//physical memory starts at 8MB + (process number * 4 MB)

	page_process(_128_MB, _8_MB + process_number * _4_MB);

	//load image file which is linked to execute at virtual address 0x8048000
	read_data(my_dentry.inode_index, 0, (uint8_t *)START_PROGRAM_IMAGE, 100000);

	if(arg_exist == 1)
		strcpy((int8_t*)(new_pcb->argument), (const int8_t*)argument);
	
	//initialize file descrpitor and STDIN & STDOUT
	for(i=0; i<FILE_MAX; i++){
		new_pcb->file_descriptor[i].inode = 0;
		new_pcb->file_descriptor[i].file_position=0;
		new_pcb->file_descriptor[i].flags = 0; //not in use
		new_pcb->file_descriptor[i].fo_table_pointer = &init_functions;
		//std in
		if(i==0){
			new_pcb->file_descriptor[i].fo_table_pointer = &std_in_functions;
			new_pcb->file_descriptor[i].flags = 1; //in use
		}
		//std out
		if(i==1){
			new_pcb->file_descriptor[i].fo_table_pointer = &std_out_functions;
			new_pcb->file_descriptor[i].flags = 1; //in use
		}	

	}

	//for context switch
	//ss0 goes to kernel_ds and esp0 becomes bottom of pcb
	tss.ss0=KERNEL_DS;
	//8MB - 8KB * process number -4
	tss.esp0= PHYSICAL_START - PCB_SIZE * process_number - 4;

	// push IRET to the stack
	// USER DS 	0x2B
	// CS 		0x23
	// ESP 		0x83FFFFC
	// EIP 		entry_addr
	asm volatile("							\n\
				  cli						\n\
				  mov $0x2B, %%ax			\n\
				  mov %%ax, %%ds 			\n\
				  mov %%ax, %%es 			\n\
				  mov %%ax, %%fs 			\n\
				  mov %%ax, %%gs 			\n\
				  							\n\
				  movl $0x83FFFFC, %%eax	\n\
				  pushl $0x2B				\n\
				  pushl %%eax				\n\
				  pushfl					\n\
				  popl	%%eax				\n\
				  orl	$0x200, %%eax		\n\
				  pushl %%eax				\n\
				  pushl $0x23				\n\
				  pushl %0					\n\
				  iret						\n\
				  AFTER_IRET:				\n\
					leave	 				\n\
					ret 					\n\
				  "
				  :							  \
				  : "r"(entry_addr)			  \
				  : "eax"					  \
				);
	
	return 0;
}

 /* read
 *
 * Description: reads the desired file into buffer
 * Input: fd, = file descriptor index
		  *buf, = buffer array
		  nbytes = number of bytes the function needs to read from file
 * Output: set file descriptor flags and table pointer with corresponding file type and operations
 * Return Value: -1 if error occurs
				 performs read call otherwise
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
	pcb_t* cur_pcb;
	// if invalid input return -1	
	if(fd < 0 || fd > 7){
		return -1;
	}
	
	// get current ESP, where we are writing
	cur_pcb = (pcb_t*)get_current_pcb();
	
	// if invalid input return -1
	if(cur_pcb->file_descriptor[fd].flags == 0 || buf == NULL){
		return -1;
	}
	
	return cur_pcb->file_descriptor[fd].fo_table_pointer->read(fd, buf, nbytes);
	
}

/* write
 *
 * Description: call a write function based on file type.
 * Input: fd, = file descriptor index
		  *buf, = buffer array
		  nbytes = number of bytes the function needs to read from file
 * Output: calls write function
 * Return Value: -1 if error occurs
				 performs write call otherwise
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
	// get current ESP, where we are writing
	pcb_t* cur_pcb = (pcb_t *)get_current_pcb();

	// if invalid input return -1	
	if(fd < 0 || fd > 7){
		return -1;
	}
	

	
	
	// if invalid input return -1
	if(cur_pcb->file_descriptor[fd].flags == 0 || buf == NULL){
		return -1;
	}
	
	return cur_pcb->file_descriptor[fd].fo_table_pointer->write(fd, buf, nbytes);
	
}


/* open
 *
 * Description: open desired file into pcb
 * Input: filename  = desired file name
 * Output: set file descriptors with given file name
 * Return Value: -1 if error occurs
				 index number of file descriptor otherwise
 */

int32_t open(const uint8_t* filename){
	int i;
	dentry_t my_dentry;
	pcb_t* pcb_open = (pcb_t *)get_current_pcb();
	
	/*if directory does not hold any matching filename*/
	if(read_dentry_by_name(filename, &my_dentry) == -1){
		return -1;
	}
	/*if file descriptor flags are not in use, set to use*/
	for ( i = 2; i < 8; i++){
		if(pcb_open->file_descriptor[i].flags == 0){
			pcb_open->file_descriptor[i].flags = 1;
			pcb_open->file_descriptor[i].file_position = 0;
			break;
		}
	}
	/*invalid case where file descriptor does not hold input file*/
	if (i == 8){
		return -1;
	}
	/*assigning file descriptor with given file type*/
	if(my_dentry.file_type == 0){ 	// rtc
		pcb_open->file_descriptor[i].fo_table_pointer = &rtc_functions;
		pcb_open->file_descriptor[i].inode = NULL;
		pcb_open->file_descriptor[i].file_position = 0;
		pcb_open->file_descriptor[i].flags = 1;		
	}
	/*assigning file descriptor with given file type*/
	else if(my_dentry.file_type == 1){	//dir
		pcb_open->file_descriptor[i].fo_table_pointer = &dir_functions;
		pcb_open->file_descriptor[i].inode = NULL;
		pcb_open->file_descriptor[i].file_position =0;
		pcb_open->file_descriptor[i].flags = 1;
	}
	/*assigning file descriptor with given file type*/
	else if (my_dentry.file_type == 2){	//file
		pcb_open->file_descriptor[i].fo_table_pointer = &file_functions;
		pcb_open->file_descriptor[i].inode = my_dentry.inode_index;
		pcb_open->file_descriptor[i].file_position = 0;
		pcb_open->file_descriptor[i].flags = 1;
	}
	else{
		return -1;
	}
	//return index of file descriptor
	return i;
}

/* close
 *
 * Description: closes the desired file
 * Input: fd  = index of file descriptor
 * Output: set desired file descriptor with initial values to free them.
 * Return Value: -1 if error occurs
				 0 if successful
 */
int32_t close(int32_t fd){
	pcb_t* pcb_close = (pcb_t *)get_current_pcb();
	
	/*checking invalid cases*/
	if (fd < 2 || fd > 7){
		return -1;
	}
	
	/*checking whether desired file is not in use*/
	if(pcb_close->file_descriptor[fd].flags == 0){
		return -1;
	}
	
	pcb_close->file_descriptor[fd].fo_table_pointer->close(fd);
	
	/*free all file descriptor variables*/
	pcb_close->file_descriptor[fd].fo_table_pointer = &init_functions;
	pcb_close->file_descriptor[fd].inode = NULL;
	pcb_close->file_descriptor[fd].file_position = 0;
	pcb_close->file_descriptor[fd].flags = 0;
	return 0;
}

/* getargs
 *
 * Description: get arguments from pcb structure and assign it to buffer
 * Input: uint8_t* buf, int32_t nbytes
 * Output: none
 * Return Value: -1 if error occurs
				 0 if successful
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
	//invalid case
	if(buf == NULL){
		return -1;
	}
	//get the current pcb 
	pcb_t* pcb_getarg = (pcb_t *)get_current_pcb();
	// if argument is empty and the length of the argument is greater than nbytes
	if(pcb_getarg->argument[0] == '\0' || strlen((char *)pcb_getarg->argument) > nbytes){
		return -1;
	}
	//copy the argument into buffer
	memmove(buf, (uint8_t*)pcb_getarg->argument, strlen((char *)pcb_getarg->argument)+1);
	
	return 0;
}

/* vidmap
 *
 * Description: It maps the ideo memory into user space at virtual address
 * Input: screen start (start position of user video memory)
 * Output: none
 * Return Value:  returns virtual address of video memory if success
 				  returns -1 for fail
 */
int32_t vidmap (uint8_t** screen_start){
		
		if(screen_start == NULL || screen_start == (uint8_t**)_4_MB)
			return -1;
		//remap the paging
		page_process_video( (uint32_t)VIDEO_MEM_START, (uint32_t)VIDEO );
		//change the screen_start address to 136MB
		*screen_start = (uint8_t*)VIDEO_MEM_START;
		
		return VIDEO_MEM_START;
		
}

