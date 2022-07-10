#ifndef _IDT_HANDLER_H
#define _IDT_HANDLER_H

#include "types.h"
#include "lib.h"

/* Exception Handler Declaration (named according to IA-32 Intel doc). They will be loaded into IDT from 0x00 to 0x13 */
void EXCEPTION_0();     // 0x00: Division Error     
void EXCEPTION_1();     // 0x01: Debug
void EXCEPTION_2();     // 0x02: NMI Interrupt
void EXCEPTION_3();     // 0x03: Break Point
void EXCEPTION_4();     // 0x04. Overflow
void EXCEPTION_5();     // 0x05. Bound Range Exceeded
void EXCEPTION_6();     // 0x06. Invalid Opcode (Undefined Opcode)
void EXCEPTION_7();     // 0x07. Device Not Available (No Math Coprocessor)
void EXCEPTION_8();     // 0x08. Double Fault
void EXCEPTION_9();     // 0x09. Coprocessor Segment Overrun
void EXCEPTION_A();     // 0x0A. Invalid TSS
void EXCEPTION_B();     // 0x0B. Segment Not Present
void EXCEPTION_C();     // 0x0C. Stack Segment Fault
void EXCEPTION_D();     // 0x0D. General Protection
void EXCEPTION_E();     // 0x0E. Page Fault
void EXCEPTION_F();     // 0x0F. Reserved
void EXCEPTION_10();    // 0x10. Floating-Point Error (Math Fault)
void EXCEPTION_11();    // 0x11. Alignment Check
void EXCEPTION_12();    // 0x12. Machine Check
void EXCEPTION_13();    // 0x13. SIMD Floating-Point Exception

#endif /* _IDT_HANDLER_H */
