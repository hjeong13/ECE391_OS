#include "paging.h"
#include "types.h"
#include "lib.h"


//make page directory
uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned (PAGE_SIZE)));

//make page table
uint32_t page_table[PAGE_TABLE_SIZE] __attribute__((aligned (PAGE_SIZE)));
//user table
uint32_t user_table[PAGE_TABLE_SIZE] __attribute__((aligned(PAGE_SIZE)));

/*
	void init_paging()
		Input : none. Using page_table and page_directory that was already made.
		Return value: none.
		Function: initializing page_directory and page_table with read write enable bit high.
				  Assign page_table to page_directory.
				  map Video memory(4kb) to page_table and is within range page_directory[0] to page_directory[1]. 
				  map Kernel to page_directory[1](4-8MB)
*/
void init_paging(){
	/*iterator variable*/
	int i;
	/*set initial page_directory and page_table to 0*/
	for(i = 0; i < PAGE_DIRECTORY_SIZE; i++){
		page_directory[i] = rw_enable_nopresent;
		/*allocating 4096 size page into each page_table elements*/
		page_table[i] = (PAGE_SIZE * i) | rw_enable_nopresent;
	}
	/*assigning page_talbe to page_directory*/
	page_directory[0] = ((unsigned int)page_table) | rw_enable_present;
	/*if the page_size bit is set, then pages are 4 MB in size, otherwise they are 4KB*/
	page_directory[1] = kernel_physical_addr | page_size | rw_enable_present;
	/*video memory is 4kb, so we only need one paging needed.*/
	
	//enable rw_enable_present
	// B9 no need user
	page_table[VIDEO_MEMORY] |= rw_enable_present;
	page_table[VIDEO_MEMORY + 1] |= 0xB9000 | USER_ENABLE | rw_enable_present;
	page_table[VIDEO_MEMORY + 2] |= 0xBA000 | USER_ENABLE | rw_enable_present;
	page_table[VIDEO_MEMORY + 3] |= 0xBB000 | USER_ENABLE | rw_enable_present;
	
	//extended assembly code for enabling paging
	//cr3 : address of page directory
	//cr4 : PSE(page size enable) makes 4MiB page mapping
	//cr0 : PG, PE enabled
	asm volatile("								\n\
				 movl %0, %%eax					\n\
				 movl %%eax, %%cr3				\n\
				 movl %%cr4, %%eax				\n\
				 orl $0x00000010, %%eax			\n\
				 movl %%eax, %%cr4				\n\
				 movl %%cr0, %%eax				\n\
				 orl $0x80000001, %%eax			\n\
				 movl %%eax, %%cr0				\n\
				 "								\
				 :								\
				 : "r"(page_directory)			\
				 :"eax"							\
				 );
}


/*
	check_1_paging(int result)
		Input : result
					- PASS
					- FAIL
		Return value : result.
		Function : test out various range of address of page.
				   1) print out page_directory[0] (0-4 MB)
				   2) print out page_directory[1] (4-8 MB)
				   3) print out video memory, page_talbe[video_memory]
				   4) print out invalid range, NULL pointer. testing page fault interrupt.
*/
int check_1_paging(int result){
	
	int * a = 0;
	
	printf("page_directory for first 0 to 4 MB : %x\n", page_directory[0]);
	printf("page_directory for kernel 4 to 8 MB : %x\n", page_directory[1]);
	printf("page table including video memory : %x\n", page_table[VIDEO_MEMORY]);
	printf("NULL pointer : %x\n", *(a));

	return result;
}

/*
	void page_process(uint32_t virtual_addr, uint32_t physical)
		Input : uint32_t virtual_addr, uint32_t physical
		Return value: none.
		Function: It takes virtual and physical addresses and maps the meory beginning to the physical address
*/
void page_process(uint32_t virtual_addr, uint32_t physical){

	uint32_t page_directory_entry = virtual_addr / PHYSICAL_OFFSET;
	//  virtual address : 128MB / 4MB
	
	//page_directory[page_directory_entry] = physical | USER_ENABLE | rw_enable_present | page_size;
	//page_directory[page_directory_entry] = physical | (unsigned_int)user_table | USER_ENABLE | rw_enable_present;
	page_directory[page_directory_entry] = physical | 0X87;
	//0x87 is USER_ENABLE+ PRESENT+ Read_Write + SIZE (4MB) 
	
	//user_table[0] = VIDEO | USER_ENABLE | rw_enable_present | page_size;	//make it 4MB

	flush_tlb();

}

/*
	void page_process_video(uint32_t virtual_addr, uint32_t physical)
		Input : uint32_t virtual_addr, uint32_t physical
		Return value: none.
		Function: It maps the 4MB memory at the given virtual address as an input to
				user_table[0].
*/
void page_process_video(uint32_t virtual_addr, uint32_t physical){

	uint32_t page_directory_entry = virtual_addr / PHYSICAL_OFFSET; // 0
	uint32_t page_table_entry = (virtual_addr % PHYSICAL_OFFSET) / 4096; //B8
	
	if(page_directory_entry == 0)
	{
		page_table[page_table_entry] = physical | USER_ENABLE | rw_enable_present;
	}
	else
	{
		page_directory[page_directory_entry] = (uint32_t)user_table | USER_ENABLE | rw_enable_present;
		user_table[page_table_entry] = physical | USER_ENABLE | rw_enable_present;
	}
	
	flush_tlb();

}

/*
	void remap_video(uint32_t process_number)
		Input : uint32_t process_number
		Return value: none.
		Function: remap virtual memories and physical memories 0xB8000, 0xB9000, 
		0xBA000, 0xBB000, so that it is 1:1 mapped
		
*/
void remap_video(uint32_t process_number){
	uint32_t virtual_addr[4] = {0xB8000, 0xB9000, 0xBA000, 0xBB000};
	uint32_t page_table_entry = (virtual_addr[process_number + 1] % PHYSICAL_OFFSET) / 4096;
	page_table[0xB8] = (PAGE_SIZE * ((0xB8) + process_number + 1)) | rw_enable_present;
	//if already processed
	if((user_table[page_table_entry] & rw_enable_present) == rw_enable_present){
		user_table[page_table_entry] = (PAGE_SIZE * ((0xB8) + process_number + 1)) | USER_ENABLE | rw_enable_present;
	}
	flush_tlb();
}


/*
	void flush_tlb()
		Input : none. 
		Return value: none.
		Function: reset tlb
*/
void flush_tlb() {
	asm volatile("								\n\
				 movl %%cr3, %%eax				\n\
				 movl %%eax, %%cr3				\n\
				 "
				 :
				 :
				 : "eax"
				);
}


