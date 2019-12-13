#include "file_system.h"
#include "systemcalls.h"

static int32_t starting;		//starting address for bootblock
static int32_t starting_inode;	//starting address for inode
static uint8_t * starting_data;	//starting address for datablocks
int directory_idx=0; 			//first initialize this to 0


boot_block_table * boot_block;		//bootblock structure
inode_block_table * inode_block;	//inodeblock structure
dentry_t directory_dentry;			//dentry structure for directory
dentry_t file_dentry;				//dentry structure for file 


/*
	int32_t file_system_init(int32_t start)
		Input : int32_t start, this is the starting address of (module_t*)mbi->mods_addr)->mod_start
		Return value: 0
		Function: Initializing file_system and it assigns bootblock and inode_block for file system.
					Inode block starts 4KB later than boot_block
*/
int32_t file_system_init(int32_t start){
	
	boot_block = (boot_block_table*)(start);			//assign boot block
	inode_block = (inode_block_table*)(start+BLOCK_SIZE);		//assign inode block
	

	///starting_data = (((uint8_t*)inode_block) + (boot_block->inodes_num)*4096);
	starting_data = (uint8_t*)(inode_block + (boot_block->inodes_num));	//get base block data
	
	printf("inode_block is : %d\n", (uint8_t*)inode_block);
	printf("starting_data is : %d\n", starting_data);
	printf("inodes numb is : %d\n", (uint8_t*)boot_block->inodes_num);
	
	starting = start;				//starting address
	starting_inode = start+BLOCK_SIZE;			//starting address of inode
	
	return 0;
	 
}

/*
	int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
		Input : uint32_t index, dentry_t* dentry
		Return value: -1 if fail and 0 for success
		Function: This function returns -1 for invalid input index number. It finds directory entries with given index and assign file name , file type and information to the input	
					dentry.
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	
	//invalid case
	if( index < 0 || ( index > ( boot_block->inodes_num-1 ) ) )
		return -1;	//fail
	
	int i;
	
	//copy file name
	//strcpy(dentry->file_name, (boot_block->dir_entry[index]).file_name);
		
		for (i=0; i< FILE_NAME_CHARS; i++) {
			dentry->file_name[i] = (boot_block->dir_entry[index]).file_name[i];
		}
		
	//copy file type
	dentry->file_type = (boot_block->dir_entry[index]).file_type;
	//copy inode_index
	dentry->inode_index = (boot_block->dir_entry[index]).inode_index;

	return 0; //success
	
}

/*
	int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
		Input : const uint8_t* fname, dentry_t* dentry
		Return value: -1 if fail and 0 for success
		Function: It gets filename as an input and dentry for input as well. It fills out dentry if the input fname is in the directory entries
					It iterates through directory entries to find the same file name and fill out dentry with inode index
					file type is 2 for normal files and 0 for rtc
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
	
	int i,j;	//iteration variables
	int a;
	int existing_len;
	int fname_len;
	int really_found =0;
	
	fname_len = strlen((int8_t*)fname);
	//loop through the entire directory
	//printf("reach point1\n");
	//loop through directory numbers
	// max f name length as 33
	if(fname_len < 33)
	{
		for(i=0; i< boot_block->dir_num; i++){
			existing_len = strlen((int8_t*)(boot_block->dir_entry[i]).file_name);
			for(j = 0; j <= fname_len; j++)
			{
				if(fname[j] != boot_block->dir_entry[i].file_name[j])
				{
					break;
				}
				if((j == 31) || (fname[j] == '\0'))
				{
					really_found = 1;
				}
			}
			if(really_found)
			{
				break;
			}
		}
	}
	//printf("reach point2\n");
	//printf("really found  = %d\n", really_found);
	if(really_found == 1 ){
		//copy file name
		for (a=0; a<FILE_NAME_CHARS; a++) {
			dentry->file_name[a] = (boot_block->dir_entry[i]).file_name[a];
		}
		//copy file type
		//printf("file type is : %d\n", (boot_block->dir_entry[found_index]).file_type);
		dentry->file_type = (boot_block->dir_entry[i]).file_type;
		//copy inode_index
		dentry->inode_index = (boot_block->dir_entry[i]).inode_index;
		return 0;
	}
	//printf("reach point3\n");
	return -1;
}



// inode : the inode number of the file to read from
// offset : the position in the file (in bytes) from which to start the read from
// length : the number of bytes to read
//reading up to length bytes starting from position offset in the file with inode number inode 
//and returning the number of bytes read and placed in the buffer. 
//A return value of 0 thus indicates that the end of the file has been reached.

/*
	int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
		Input : uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length
		Return value: -1 for invalid cases and length of data in bytes for other cases
		Function: It copies from given offset for length and then put it to buffer. There are invalid
					cases that have to be handled. 
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){

	//return value
	int ret;
 	//printf("inode_block[inode].length_B is %d\n",inode_block[inode].length_B);
	//invalid inode index
	if (inode < 0 || inode >= boot_block->inodes_num ){
			return -1; 
	}
	//if file length in bytes are zero	
	if(inode_block[inode].length_B == 0){	
			return -1;
	}			
	//nothing to read, reach end
	if(offset>=inode_block[inode].length_B) return 0;
	//offset for which block	
	uint32_t data_blocks_which_block = offset/BLOCK_SIZE;
	//offset for where to start in the block
	uint32_t data_blocks_index = offset%BLOCK_SIZE;	
	//copy length 
	uint32_t copy_length;
	//if offset+length are smaller than length
	if(offset+length <= (inode_block[inode]).length_B){
		
		copy_length = length;
		ret= length;
	}
	else{
		copy_length = (inode_block[inode]).length_B - offset;
		ret = copy_length;
	}
	
	uint32_t data_index;
	int i=0;
	int j;
	//iteration for copying into buffer
	while(copy_length!=0){
		
		data_index = (inode_block[inode]).data_block[data_blocks_which_block];
		//invalid case
		if(data_index <0 || data_index >= boot_block->data_num ){
			return -1;
		}
		for(j=data_blocks_index; j<BLOCK_SIZE; j++){
			//put it into buffer
			buf[i]=*(starting_data+data_index*BLOCK_SIZE+j);
			copy_length--;	//copy length decrement
			i++;
			
			if(copy_length==0) break;	//stop iteration
		}
		data_blocks_index=0;
		data_blocks_which_block++;
		
	}
	
	return ret;	

}



/*
		int32_t file_write(int32_t fd, const void * buf, int32_t nbytes)
		Input : int32_t fd, const void * buf, int32_t nbytes
		Return value: -1 
		Function: NONE
*/
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes){
		return -1;
}

/*
		int32_t file_open(const uint8_t* filename)
		Input : const uint8_t* filename
		Return value: ret
		Function: call read_dentry_by_name and return the length
*/
int32_t file_open(const uint8_t* filename){
	//fill out file_dentry
	return read_dentry_by_name(filename, &file_dentry);
}

/*
	int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
		Input : int32_t fd, void* buf, int32_t nbytes
		Return value: length
		Function: call read_data function with index, buff, offset and bytes and return the length
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
	
	pcb_t* pcb_open = (pcb_t *)get_current_pcb();
	
	int length;
	length = read_data(pcb_open->file_descriptor[fd].inode, pcb_open->file_descriptor[fd].file_position, buf, nbytes);
	pcb_open->file_descriptor[fd].file_position += length;
	return length;
	
}

/*
	int32_t file_close(int32_t fd)
		Input : int32_t fd
		Return value: 0
		Function: NONE
*/
int32_t file_close(int32_t fd){
	return 0;
}

/*
	int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes)
		Input : int32_t fd, const void * buf, int32_t nbytes
		Return value: -1
		Function: NONE
*/
int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes){
	return -1;
}
/*
		int32_t directory_read(int32_t fd, void* buf, int32_t nbytes)
		Input : int32_t fd, void* buf, int32_t nbytes
		Return value: -1 for invalid and file length
		Function: This function calls read dentry by index function from index 0 till index become same as the whole directory numbers
					It fills file name to the buffer 
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
	
	int i = 0;
	int file_length;

	read_dentry_by_index(directory_idx,&directory_dentry);
	directory_idx++;

	if(directory_idx == boot_block->dir_num){
		directory_idx = 0;
	return 0;}

	file_length = strlen((int8_t*)(directory_dentry.file_name));

	for(i = 0; i<file_length; i++){
		if(i == FILE_NAME_CHARS)
			break;
		*(uint8_t*)(buf+i) = directory_dentry.file_name[i];
	}
	
	*(uint8_t*)(buf+file_length) =(uint8_t)directory_dentry.file_type;
	//int temp = (int)directory_dentry.inode_index
	//*(uint8_t*)(buf+file_length+1) = (uint8_t)inode_block[temp].length_B;
	return i;
	
}
/*
	int32_t directory_close(int32_t fd)
		Input : int32_t fd
		Return value: 0
		Function: NONE
*/
int32_t directory_close(int32_t fd){
	return 0;
}
/*
	int32_t directory_open(const uint8_t* filename)
		Input : const uint8_t* filename
		Return value: -1 if file type is not directory type
		Function: This function calls read dentry by name function with filename given and directory dentry
*/
int32_t directory_open(const uint8_t* filename){
	
	read_dentry_by_name(filename, &directory_dentry);	//fill out directory_entry
	//printf("file type is %d",directory_dentry.file_type); 
	//file type for directory should be 1
	if(directory_dentry.file_type!=1)
		return -1;
	
	return 0;
}



