#include "exception_handler.h"

#include "x86_desc.h"
#include "idt.h"
#include "syscall.h"

/* Exception Handler Definitions */
void EXCEPTION_0(){
    // blue_screen();
    printf("0x00: DIVIDE ERROR\n");
    exception_halt();
}
void EXCEPTION_1(){
    // blue_screen();
    printf("0x01: DEBUG\n");
    exception_halt();
}
void EXCEPTION_2(){
    // blue_screen();
    printf("0x02: NMI INTERRUPT\n");
    exception_halt();
}
void EXCEPTION_3() {
    // blue_screen();
    printf("0x03: BREAKPOINT\n");
    exception_halt();
}
void EXCEPTION_4() {
    // blue_screen();
    printf("0x04: OVERFLOW\n");
    exception_halt();
}
void EXCEPTION_5() {
    // blue_screen();
    printf("0x05: BOUND RANGE EXCEEDED\n");
    exception_halt();
}
void EXCEPTION_6() {
    // blue_screen();
    printf("0x06: INVALID OPCODE (Undefined Opcode)\n");
    exception_halt();
}
void EXCEPTION_7() {
    // blue_screen();
    printf("0x07: DEVICE NOT AVAILABLE (No Math Coprocessor)\n");
    exception_halt();
}
void EXCEPTION_8() {
    // blue_screen();
    printf("0x08: DOUBLE FAULT\n");
    exception_halt();
}
void EXCEPTION_9() {
    // blue_screen();
    printf("0x09: COPROCESSOR SEGMENT OVERRUN (reserved)\n");
    exception_halt();
}
void EXCEPTION_A() {
    // blue_screen();
    printf("0x0A: INVALID TSS \n");
    exception_halt();
}
void EXCEPTION_B() {
    // blue_screen();
    printf("0x0B: SEGMENT NOT PRESENT\n");
    exception_halt();
}
void EXCEPTION_C() {
    // blue_screen();
    printf("0x0C: STACK SEGMENT FAULT\n");
    exception_halt();
}
void EXCEPTION_D() {
    // blue_screen();
    printf("0x0D: GENERAL PROTECTION\n");
    exception_halt();
}
void EXCEPTION_E() {
    // blue_screen();
    printf("0x0E: PAGEFAULT\n");
    exception_halt();
}
void EXCEPTION_F() {
    // blue_screen();
    printf("0x0F: RESERVED\n");
    exception_halt();
}
void EXCEPTION_10() {
    // blue_screen();
    printf("0x10: FLOATING-POINT ERROR (Math Fault)\n");
    exception_halt();
}
void EXCEPTION_11() {
    // blue_screen();
    printf("0x11: ALIGNMENT CHECK\n");
    exception_halt();
}
void EXCEPTION_12() {
    // blue_screen();
    printf("0x12: MACHINE CHECK\n");
    exception_halt();
}
void EXCEPTION_13() {
    // blue_screen();
    printf("0x13: SIMD FLOATING-POINT EXCEPTION\n");
    exception_halt();
}
