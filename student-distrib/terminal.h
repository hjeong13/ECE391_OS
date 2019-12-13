#pragma once
#include "types.h"

void terminal_init();
//this function is what we use to read the keyboard buffer
int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes);
//this function is what we use to write to the terminal screen
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);
//this function is what we use to open the terminal
int32_t terminal_open(const uint8_t * filename);
//this function is what we use to close the terminal
int32_t terminal_close(int32_t fd);

