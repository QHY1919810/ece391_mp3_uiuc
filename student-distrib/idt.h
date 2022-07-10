#ifndef IDT_H
#define IDT_H

#include "types.h"

#define TRAP_GATE_TYPE      0xF /* 80386 trap gate type */
#define INTERRUPT_GATE_TYPE 0xE /* 80386 interrupt gate type */

#define PIT_IDT_INDEX       0x20
#define KEYBOARD_IDT_INDEX  0x21 /* 80386 keyboard interrupt handler entry index */
#define RTC_IDT_INDEX       0x28 /* 80386 rtc interrupt handler entry index */

#define SYS_CALL            0x80 /* 80386 system call type */

/* initiate the interrupt descriptor table */
void idt_init();

#endif
