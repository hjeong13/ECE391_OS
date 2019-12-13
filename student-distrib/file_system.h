#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include "types.h"
#include "lib.h"

#define BLOCK_SIZE		4096		//size of 4kB
#define BOOT_BLOCK_SIZE	64			// size of boot block
#define BOOT_BLOCK_RESERVED	52		// reserve space
#define DATA_BLOCK_IN_INODES 1023   //1024-1 because length in B not included
#define FILE_NAME_CHARS	32			// maximum number of characters available for file name
#define NUM_DIR_ENTRIES 63			//maximum number of directory entries.


/*initializing file_system. assign starting addresses*/
int32_t file_system_init(int32_t start);
/*read the directory entry by comparing name*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
/*read the directory entry by comparing indexes*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
/*copy file or directory data into buf with given offset and length of bytes to be written*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/*directory helper function that helps directory to open, read, write, and close*/
int32_t directory_write(int32_t fd, const void * buf, int32_t nbytes);
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_close(int32_t fd);
int32_t directory_open(const uint8_t* filename);

/*file helper function that helps file to open, read, write, and close*/
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_close(int32_t fd);
int32_t file_open(const uint8_t* filename);



#endif
