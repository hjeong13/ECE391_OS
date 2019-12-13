#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGE_DIRECTORY_SIZE 	1024			//size of directory page
#define PAGE_TABLE_SIZE			1024			//size of table page
#define PAGE_SIZE				4096			//size of page
#define rw_enable_present		3				// read right enable with present bit set to high
#define rw_enable_nopresent		2				// read right enable bit high with present bit set to low
#define VIDEO_MEMORY			0xB8			// location of video memory
#define VIDEO                   0xB8000
#define kernel_physical_addr	0x400000		// Physical address of kernel. Used to make offset calculated.
#define page_size				0x80			// page_size bit used to indicate whether we are assigning 4Mib or 4Kib
#define USER_ENABLE				4				// USER privilege bit used to grant video memory.
#define PHYSICAL_OFFSET			0x400000


/*initializing paging and mapping to correct location for each element*/
void init_paging();

/*test cases for checking paging*/
int check_1_paging(int result);
void page_process(uint32_t virtual_addr, uint32_t physical);
void page_process_video(uint32_t virtual_addr, uint32_t physical);
void remap_video(uint32_t terminal_number);
void flush_tlb();
#endif
