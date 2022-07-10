#include "pit.h"

#include "i8259.h"
#include "syscall.h"
#include "scheduler.h"

//Reference source: https://wiki.osdev.org/Programmable_Interval_Timer
/* 
 *  pit_handler()
 *  DESCRIPTION: set pit handler
 *  INPUTS:  none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: none
 *  Reference source: none
 */
void pit_handler(){
    send_eoi(PIT_IRQ);
    scheduler_round_robin();
    if (get_halt_flag(curr_running_terminal)) {
        clear_halt_flag(curr_running_terminal);
        halt(255);
    }
}

/* 
 *  pit_set_freq(int hz)
 *  DESCRIPTION: set pit frequency
 *  INPUTS:  hz -- the frequency we want to set, unit is HZ
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  Reference source: http://www.osdever.net/bkerndev/Docs/pit.htm
 */
void pit_set_freq(int32_t hz){
    int32_t divisor = PIT_DIV / hz; /* Calculate our divisor */
    outb(PIT_MODE3_CMD, PIT_CMD_REG); /* Set our command byte 0x36 */
    outb(divisor & 0xFF, PIT_CHL0_REG); /* Set low byte of divisor */
    outb(divisor >> 8, PIT_CHL0_REG); /* Set high byte of divisor */
}
/* 
 *  pit_init
 *  DESCRIPTION: initialize pit and set frequency into 100 HZ
 *  INPUTS:  none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  Reference source: http://www.osdever.net/bkerndev/Docs/pit.htm
 */
void pit_init(){
    pit_set_freq(PIT_DEFAULT_FREQ); /*In the reference, it recommends setting to 100Hz(PIT_DEFAULT_FREQ) in a real kernel. Every 10ms, an interrupt will be raised for scheduler*/
    enable_irq(PIT_IRQ);
}
