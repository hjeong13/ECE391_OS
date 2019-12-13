#include "terminal.h"
#include "lib.h"
#include "keyboard.h"

/*
terminal_read: this function reads the current keyboard buffer and puts it into buf
INPUTS: we take the file descriptor, a pointer to the buffer to read from, and the number of bytes
OUTPUT: The number of bytes read
*/

uint32_t mapping_array[3] = {0xB9000, 0xBA000, 0xBB000};

void terminal_init(){ 
	int i, j;
	for(i = 0; i < 3 ; i++){
		terminal_array[i].running = 0;
		terminal_array[i].x_position = 0;
		terminal_array[i].y_position = 0;
		terminal_array[i].keyboard_buffer_index = 0;
		for(j = 0; j < 128; j++){
			terminal_array[i].keyboard_buffer[j] = 0;
		}
		terminal_array[i].current_process_number = 0;
		
		
		terminal_array[i].video_memory = (uint8_t*)mapping_array[i];
	}
}

int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes)
{
  int i = 0; //i will index through buffers
  char * array = (char *)buf; // array will grab the pointer of buf
  char * key_buf = get_keyboard_buffer(); //key_buf will be the keyboard buffer
  while(!is_enter()) //we need to block until there's a newline inside the keyboard buffer
  {
  }
  while(i < nbytes) //we copy from 0 to the number of bytes
  {
    if(key_buf[i] == 0) //if we encounter a null character, we put a space into the buffer
    {
      array[i] = ' ';
      continue;
    }
  	if(key_buf[i] == '\n') //if we encounter the newline, we break as we're done
  	{
		array[i] = key_buf[i];
		i++;
  		break;
  	}
    array[i] = key_buf[i];
    i++;
  }
  reset_keyboard_buffer(); //we have to reset the keyboard buffer and then return the number of bytes copied
  return i;
}
/*
terminal_write: this function writes data to the screen from buf.
INPUTS: we take the file descriptor, a pointer to the buffer to read from, and the number of bytes
OUTPUT: returns number of bytes written or -1
*/
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes)
{
  char * array = (char *)buf; //array will get the buffer pointer
  int i = 0; //i will index through the arrays

  if(array == NULL) //we have to check if the buffer is valid
  {
	  return -1; //if the pointer isn't valid then we return -1
  }
  while(i < nbytes) //otherwise we put each character from the buffer to the screen
  {
    if(array[i] != 0) //we check to make sure the character isn't NULL
    {
        putc(array[i]);
    }
    else //otherwise we just put a space
    {
      putc(' ');
    }
    i++;
  }
  return i; //we return the number of bytes written
}
/*
terminal_open: this function initializes any variables for terminal
INPUTS: this function takes a pointer to a file name
OUTPUT: returns 0
*/
int32_t terminal_open(const uint8_t * filename)
{
  return 0;
}
/*
terminal_close: this function gets rid of any variables for terminal
INPUTS: this function takes a file descriptor
OUTPUT: returns 0
*/
int32_t terminal_close(int32_t fd)
{
  return 0;
}

