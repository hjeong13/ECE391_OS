#pragma once

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "terminal.h"
#include "i8259.h"
#include "paging.h"
#include "systemcalls.h"
#include "keyboard.h"

#define PIT_SQUARE_WAVE		0x36
#define	PIT_CMD_REG			0x43
#define	FREQUENCY_MASK		0xFF
#define	PIT_CHANNEL_0		0x40
#define _20HZ				11932
#define PIT_port			0x20
#define NUM_TERMINAL		3
#define STACK_SIZE			4




void pit_init(void);
void pit_handler(void);



