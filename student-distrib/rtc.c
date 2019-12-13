#include "rtc.h"
#include "i8259.h"
#include "tests.h"

#define RTC_port		0x08
#define UIP_MASK_A		0x8A
#define UIP_MASK_B		0x8B	//UIP mask | register B
#define ADDRESS_PORT	0x70	//rtc port
#define DATA_PORT		0x71	//rtc port
#define ENABLE_OR_UPDATE	0x40
#define HIGH_MASK		0xF0
#define LOW_MASK		0x0F
#define MAX_RATE		15
#define MIN_RATE		3

#define MAX_FREQUENCY	8192
#define MIN_FREQUENCY	2
//  got reference from OSdev.org and rtc note from ece 391

//This is written in tests.c
void test_interrupts(void);
void set_frequency(int32_t frequency);
int32_t check_power(int32_t frequency);

volatile uint32_t rtc_flag;	// rtc_flag = 1 when handling,  = 0 when done with handling

/* void rtc_init(void)
 * Inputs: NONE
 * Return Value: NONE
 * Function: Initializing rtc and sets ports on the RTC and sets control register A and B
 */
void rtc_init(void)
{
	char prev;
	cli();
	rtc_flag = 1;
	//select Register B and disable NMI
	outb(UIP_MASK_B, ADDRESS_PORT);
	//get the value in register B
	prev = inb(DATA_PORT);
	//select register b and disable NMI because read will reset index
	outb(UIP_MASK_B, ADDRESS_PORT);
	//write previous value ORed with 0x40. 6 on register B
	outb(prev | ENABLE_OR_UPDATE, DATA_PORT);
	//set frequency to 2HZ
	set_frequency(2);
	
	enable_irq(RTC_port);
	sti();
}

/* void rtc_handler(void)
 * Inputs: NONE
 * Return Value: NONE
 * Function: Sends eoi to RTC IRQ line and this calls test_interrupts if RTC interrupt happens
 */
void rtc_handler(void)
{
	//clear flag
	cli();
	char c;
	outb(0x0C, ADDRESS_PORT); 		// select register C
	c = inb(DATA_PORT);			// just throw away contents
	
	if(rtc_test_flag == 1){
		putc('1');
	}
	
	rtc_flag = 0;
	send_eoi(RTC_port);
	sti();						//store flag

	return;
}

/* int32_t rtc_open(const uint8_t* filename)
 * Inputs: const uint8_t* filename - not used
 * Return Value: 0
 * Function: Initialized RTC frequency to 2HZ
 */
int32_t rtc_open(const uint8_t* filename){
	set_frequency(2);
	return 0;
}

/* int32_t rtc_close(int32_t fd)
 * Inputs: int32_t fd - not used
 * Return Value: 0
 * Function: do nothing
 */
int32_t rtc_close(int32_t fd){
	return 0;
}

/* int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd - not used
 *         void* buf  - not used
 *         int32_t nbytes - not used
 * Return Value: 0
 * Function: return 0 after an interrupt has occurred
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	// return 0 after an interrupt has occurred
	rtc_flag = 1;

	// wait for the next interrupt and until interrupt handler clears it
	while(rtc_flag == 1){

	}
	
	return 0;
}

/* int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: int32_t fd - not used
 *         void* buf  - stores the frequency values
 *         int32_t nbytes - this function only accepts 4 bytes
 * Return Value: ret_val(0 for success, -1 for failure)
 * Function: Change frequency. Frequency must be power of 2 
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
	int32_t ret_val = 0;
	int32_t frequency = 0;
	
	frequency = *(int32_t*)buf;
	ret_val = check_power(frequency);
	
	if(nbytes != 4){
		ret_val = -1;
	}
	
	if(ret_val == 0){
		set_frequency(frequency);
	}
	
	return ret_val;
}

/* int32_t check_power(int32_t frequency)
 * Inputs: int32_t frequency - the setting frequency
 * Return Value: ret_val(0 for success, -1 for failure)
 * Function: check if the frequency is power of 2, also check if frequency is acceptable
 */
int32_t check_power(int32_t frequency){
	int32_t ret_val = 0;
	int32_t count = 0;
	
	if(frequency > MAX_FREQUENCY || frequency < MIN_FREQUENCY){
		return -1;
	}
	
	while(frequency != 0){
		count += (frequency & 0x1);
		frequency = frequency >> 1;
	}
	
	if(count != 1){
		ret_val = -1;
	}
	
	return ret_val;
}




/* void set_frequency(void)
 * Inputs: int32_t frequency - the frequency to set RTC
 * Return Value: NONE
 * Function: set rtc frequency, max frequency is 2^13 = 8192, min is 2
 */
void set_frequency(int32_t frequency){
	unsigned char prev;
	unsigned char rate = MIN_RATE;		// fastest rate we can select
	
	while(frequency != 32768 >> (rate - 1)){
		rate += 1;
		
		// Rate cannot be over 15
		if (rate == MAX_RATE){
			break;
		}
	}
	
	//rate should be above 2 and not over 15;
	rate &= LOW_MASK;
	
	cli();
	outb(UIP_MASK_A, ADDRESS_PORT);	//select register A and disable NMI
	prev = inb(DATA_PORT);			//read in register A
	outb(UIP_MASK_A, ADDRESS_PORT);	//reset index to register A
	outb((HIGH_MASK & prev) | rate, DATA_PORT);	//write the rate to register A
	sti();
}

								  
