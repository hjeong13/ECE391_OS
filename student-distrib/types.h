/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0
#define FILE_NAME_CHARS	32			// maximum number of characters available for file name
#define BOOT_BLOCK_RESERVED	52		// reserve space
#define DATA_BLOCK_IN_INODES 1023   //1024-1 because length in B not included

#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

/*terminal process struct*/
typedef struct terminal_t{
	uint8_t running;
	uint32_t x_position;
	uint32_t y_position;
	uint8_t keyboard_buffer_index;
	uint8_t keyboard_buffer[128];
	uint8_t* video_memory;
	
	uint8_t current_process_number;

} terminal_t;

/*file operation table structure*/
typedef struct fops_t {
	int32_t (*close)(int32_t fd);
	int32_t (*open)(const uint8_t * filename);
	int32_t	(*read)(int32_t fd, void * buf, int32_t nbytes); 
	int32_t (*write)(int32_t fd, const void * buf, int32_t nbytes);
} fops_t;

/*file descriptor structure*/
typedef struct fd_t {
	fops_t * fo_table_pointer;
	uint32_t inode;
	uint32_t file_position;
	uint32_t flags;
} fd_t;

/*process control block structure*/
typedef struct pcb_t {
	fd_t file_descriptor[8];
	char file_name[8][32];
	uint8_t argument[100];
	uint8_t pcb_terminal_num;
	uint32_t parent_pcb_num;
	uint32_t current_pcb_num;
	uint32_t parent_esp;
	uint32_t parent_ebp;
	uint32_t current_esp;
	uint32_t current_ebp;

} pcb_t;

/* structure of directory entries*/
typedef struct dentry_t {
	
	uint8_t file_name[FILE_NAME_CHARS];
	int32_t file_type;
	int32_t inode_index;
	uint8_t reserved[24];

} dentry_t;

/*structure of boot block*/
typedef struct boot_block_table {
	
	int32_t dir_num;
	int32_t inodes_num;
	int32_t data_num;
	uint8_t reserved[BOOT_BLOCK_RESERVED];
	dentry_t dir_entry[63];
	
} boot_block_table;

/*structure of inode*/
typedef struct inode_block_table {
	
	int32_t length_B;		// total length of block used
	int32_t data_block[DATA_BLOCK_IN_INODES]; 

} inode_block_table;


#endif /* ASM */

#endif /* _TYPES_H */
