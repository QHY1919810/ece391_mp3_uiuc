/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

// #include <linux/spinlock.h>

// static spinlock_t i8259_lock = SPIN_LOCK_UNLOCKED;
/*cached_irq_mask for master and slave*/ 
// uint16_t cached_irq_mask = 0xFFFF;
/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0XFF; /* IRQs 0-7  */
uint8_t slave_mask = 0XFF;  /* IRQs 8-15 */

/*
 *   i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initialize master and slave PICs
 *   Reference Sources: 
 *   1. \ece391_share\work\source\linux-2.6.22.5\arch\i386\kernel
 *   2. https://wiki.osdev.org/8259_PIC
 *   3. Lecture 10 Slides
 */
void i8259_init(void) {
    /* 
    The 8259_PORT below means command port of PIC
    The 8259_DATA below means data port of PIC
    When no command is issued, the data port allows us to access the interrupt mask of the 8259 PIC. 
    */

    unsigned long flags;
    cli_and_save(flags);

    //mask both master and slave
    outb(0xFF, MASTER_8259_DATA);
    outb(0xFF, SLAVE_8259_DATA);
    //select master to initial
    outb(ICW1, MASTER_8259_PORT);
    //map master IR0-7 to 0x20 - 0x27
    outb(ICW2_MASTER, MASTER_8259_DATA);
    //Master has a secondary on IRQ2
    outb(ICW3_MASTER, MASTER_8259_DATA);
    // A normal eoi situation due to no auto_eoi input
    outb(ICW4, MASTER_8259_DATA);

    //select slave to initial
    outb(ICW1, SLAVE_8259_PORT);
    //map slave IR0-7 to 0x28 - 0x2f
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    //Master has a secondary on IRQ2
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    // A normal eoi situation due to no auto_eoi input
    outb(ICW4, SLAVE_8259_DATA);
    
    //Restore MASTER & SLAVE IRQ mask
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);

    //enable slave IRQ
    enable_irq(NUM_IRQ);
    restore_flags(flags);

    return;
}

/*
 *   enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: irq_num - number of IRQ, used to decide if MASTER PIC or SLAVE PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enable (unmask) the specified IRQ
 *   Reference Sources: 
 *   1. \ece391_share\work\source\linux-2.6.22.5\arch\i386\kernel
 *   2. https://wiki.osdev.org/8259_PIC
 */
void enable_irq(uint32_t irq_num) {
    unsigned int mask;
    unsigned long flags;

    //Since we are start from bit 0, so we need to shift left 1 to find proper bit.
    //And high means mask, so we use ~
    mask = ~(1 << irq_num);

    cli_and_save(flags);
    // MASTER_SLAVE_DIV = 8 in decimal = 1000 in binary
    // So irq_num & MASTER_SLAVE_DIV is to check if irq_num > 8, if bigger, it is slave
    if(irq_num & MASTER_SLAVE_DIV){
        slave_mask &= (mask >> MASTER_SLAVE_DIV);
        outb(slave_mask, SLAVE_8259_DATA);
    }
    else{
        master_mask &= mask;
        outb(master_mask,MASTER_8259_DATA);
    }
    restore_flags(flags);
}

/*
 *   disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: irq_num - number of IRQ, used to decide if MASTER PIC or SLAVE PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Disable (mask) the specified IRQ
 *   Reference Sources: 
 *   1. \ece391_share\work\source\linux-2.6.22.5\arch\i386\kernel
 *   2. https://wiki.osdev.org/8259_PIC
 */
void disable_irq(uint32_t irq_num) {
    unsigned int mask;
    unsigned long flags;

    //Since we are start from bit 0, so we need to shift left 1 to find proper bit.
    mask = (1 << irq_num);

    cli_and_save(flags);
    // MASTER_SLAVE_DIV = 8 in decimal = 1000 in binary
    // So irq_num & MASTER_SLAVE_DIV is to check if irq_num > 8, if bigger, it is slave
    if(irq_num & MASTER_SLAVE_DIV){
        slave_mask |= (mask >> MASTER_SLAVE_DIV);
        outb(slave_mask, SLAVE_8259_DATA);
    }
    else{
        master_mask |= mask;
        outb(master_mask, MASTER_8259_DATA);
    }
    restore_flags(flags);
    return;
}

/*
 *   send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: irq_num - number of IRQ, used to decide if MASTER PIC or SLAVE PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Send end-of-interrupt signal for the specified IRQ
 *   Reference Sources: 
 *   1. \ece391_share\work\source\linux-2.6.22.5\arch\i386\kernel
 *   2. https://wiki.osdev.org/8259_PIC
 */
void send_eoi(uint32_t irq_num) {
    // MASTER_SLAVE_DIV = 8 in decimal = 1000 in binary
    // So irq_num & MASTER_SLAVE_DIV is to check if irq_num > 8, if bigger, it is slave
    if(irq_num & MASTER_SLAVE_DIV){
        //eoi to slave
        //(irq_num & (MASTER_SLAVE_DIV - 1) ) means (irq_num - MASTER_SLAVE_DIV) to select IRQ_num on slave
        //eoi to master IRQ port connect to Slave PIC.
        outb(EOI | NUM_IRQ, MASTER_8259_PORT);
        //Using & to speed up calculation. 
        outb(EOI | (irq_num & (MASTER_SLAVE_DIV - 1) ), SLAVE_8259_PORT);
    }
    else{
        //eoi to master
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
}
