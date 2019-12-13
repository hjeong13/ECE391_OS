/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
#include "tests.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*
Purpose: i8259 initializes the 8259 PIC by first masking all interrupts and then sending 4 command words to each PIC.
Input: None
Output: None
Once the PICs are initialized, the only interrupt that won't be masked is IRQ2 on the slave PIC.
*/
void i8259_init(void) {
	outb(MASKED_INT, MASTER_8259_PORT_2);			//First we send 0xFF to both 0x21 and 0xA1 as to mask all interrupts.
	outb(MASKED_INT, SLAVE_8259_PORT_2);

	outb(ICW1, MASTER_8259_PORT);				//Then we send the control words to the master PIC. The first command must be sent to 0x20 for the master PIC.
	outb(ICW2_MASTER, MASTER_8259_PORT_2);	//The remaining 3 control words must be send to 0x21 for the master PIC.
	outb(ICW3_MASTER, MASTER_8259_PORT_2);
	outb(ICW4, MASTER_8259_PORT_2);

	outb(ICW1, SLAVE_8259_PORT);		//Here we send the first control word to 0xA0 for the slave PIC.
	outb(ICW2_SLAVE, SLAVE_8259_PORT_2);		//then we send the remaining control words to 0xA1 for the slave PIC.
	outb(ICW3_SLAVE, SLAVE_8259_PORT_2);
	outb(ICW4, SLAVE_8259_PORT_2);
	master_mask = 0xFB;										//The master_mask that we want is 0xFB so that we enable irqs being sent from the slave PIC.
	slave_mask = MASKED_INT;										//The slave_mask will be fully masked
	// 1111 1011
	// 1111 1111
	outb(master_mask, MASTER_8259_PORT_2);	//We then send out the masks to the PIC so as to initialize their individual masks.
	outb(slave_mask, SLAVE_8259_PORT_2);
}

/*
Purpose: enable_irq enables the irq line that is given by irq_num.
Input: irq_num is the number of the IRQ that we want to enable.
Output: None
The mask will have a 0 wherever we wanted the irq to be enabled.
*/
void enable_irq(uint32_t irq_num) {
	if(irq_num < SLAVE_FIRST)		//We either have the irq in the master or slave PIC
	{
		master_mask = inb(MASTER_8259_PORT_2);			//if the irq_num is within the master PIC, we grab the current mask from the master PIC.
		//  0000 0010
		//  1111 1101
		//& 1111 1011
		// ----------
		//  1111 1001
		master_mask = master_mask & ~(1 << irq_num);			//We then unmask the bit that corresponds to the irq_num
		outb(master_mask, MASTER_8259_PORT_2);					//We send the mask back so as to update the mask in the master PIC
	}
	else
	{
		slave_mask = inb(SLAVE_8259_PORT_2);						//If the irq_num is within the slave PIC, we grab the current mask from the slave PIC.
		slave_mask = slave_mask & ~(1 << (irq_num - SLAVE_FIRST));	//Then we unmask the bit that corresponds to the irq_num
		outb(slave_mask, SLAVE_8259_PORT_2);						//We send the mask back so as to update the mask in the slave PIC.
	}													

}

/*
Purpose: disable_irq disables the irq line that is given by irq_num
Input: irq_num is the number of the IRQ that we want to disable.
Output: None
The mask will have a 1 wherever we wanted the irq to be disabled.
*/
void disable_irq(uint32_t irq_num) {

	if(irq_num < SLAVE_FIRST)			//We either have the irq in the master or slave PIC.
	{
		master_mask = inb(MASTER_8259_PORT_2);				//If the irq_num is on the master PIC, we grab the current mask from the master PIC.
		//  1111 1001
		//| 0000 0010
		//  ---------
		//  1111 1011
		master_mask = master_mask | (1 << irq_num);			//We then mask the bit that corresponds to the irq_num
		outb(master_mask, MASTER_8259_PORT_2);				//We send the mask back so as to update the mask in the master PIC.
	}
	else
	{
		slave_mask = inb(SLAVE_8259_PORT_2);				//If the irq_num is on the slave PIC, we grab the current mask from the slave PIC.
		slave_mask = slave_mask | (1 << (irq_num - SLAVE_FIRST));	//We then mask the bit that corresponds to the irq_num
		outb(slave_mask, SLAVE_8259_PORT_2);					//Then we send the mask back to the slave PIC
	}

}

/*
Purpose: send_eoi sends the eoi to the PIC so we can tell the PIC that we've finished our interrupt.
Input: irq_num is the interrupt that we just serviced and want to send the eoi for
Output: None
The EOI will be or'd with the irq_num that is on the corresponding PIC and will then be sent to the PIC.
*/
void send_eoi(uint32_t irq_num) {
	if(irq_num >= SLAVE_FIRST && irq_num <= SLAVE_LAST)		//First we determine if the irq_num is on the master or slave PIC
	{
		outb(EOI | ((uint8_t)irq_num - SLAVE_FIRST), SLAVE_8259_PORT);		//If the irq_num is on the slave PIC, then we subtract 8 from the irq_num and then or it with EOI and then send it to the Slave
		outb(EOI | SLAVE_IRQ, MASTER_8259_PORT);												//We also send the EOI or'd with 2 and send it to the master PIC
	}
	else if(irq_num >= MASTER_FIRST && irq_num <= MASTER_LAST)	//If the irq_num is in the master PIC
	{
		outb(EOI | (uint8_t)irq_num, MASTER_8259_PORT);		//We send the EOI or'd with the irq_num to the master PIC
	}
}
