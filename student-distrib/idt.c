#include "idt.h"

#include "lib.h"
#include "x86_desc.h"
#include "exception_handler.h"
#include "intr_wrap.h"

/* 
 * idt_clear
 *  DESCRIPTION:
 *      Clear the entries in interrupt discriptor table by setting all entries' present field to false
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUES: none
 *  SIDE EFFECTS: none
 */
void idt_clear() {
    int32_t i; /* index variable */

    /* clear all entries' present field */
    for (i = 0; i < NUM_VEC; i++) {
        idt[i].present = 0;
    }
}

/*
 * idt_add_trap_handler
 *  DESCRIPTION:
 *      Given a pointer and index of the handler function, the function adds the function into idt. 
 *      Gate type is set to TRAP. DPL is set to 0 to disable user call.
 *  INPUTS: 
 *      - index     : index of the handler function in idt
 *      - handler   : pointer to the handler function
 *  OUTPUTS: none
 *  RETURN VALUES: none
 *  SIDE EFFECTS: none
 */
void idt_add_trap_handler(uint8_t index, void (*handler)()) {
    /* set idt entry offset*/
    SET_IDT_ENTRY(idt[index], handler);
    /* set idt entry segment selector */
    idt[index].seg_selector = KERNEL_CS;
    idt[index].reserved4 = 0;
    /* set gate type */
    idt[index].gate_type = TRAP_GATE_TYPE;  
    /* Storage Segment*/
    idt[index].reserved0 = 0;
    /* set DPL to kernel to disable user call */
    idt[index].dpl = 0;
    /* A filled entry is present */
    idt[index].present = 1;
}

/* 
 * idt_add_interrupt_handler
 *  DESCRIPTION:
 *      Given a pointer and index of the handler function, the function adds the function into idt. 
 *      Gate type is set to INTERRUPT. DPL is set to 0 to disable user call.
 *  INPUTS: 
 *      - index     : index of the handler function in idt
 *      - handler   : pointer to the handler function
 *  OUTPUTS: none
 *  RETURN VALUES: none
 *  SIDE EFFECTS: none
 */
void idt_add_interrupt_handler(uint32_t index, void (*handler)()){
    /* set idt entry offset*/
    SET_IDT_ENTRY(idt[index], handler);
    /* set idt entry segment selector */
    idt[index].seg_selector = KERNEL_CS;
    idt[index].reserved4 = 0;
    /* set gate type */
    idt[index].gate_type = INTERRUPT_GATE_TYPE;  
    idt[index].reserved0 = 0;
    /* set DPL to kernel to disable user call */
    idt[index].dpl = 0;
    /* A filled entry is present */
    idt[index].present = 1;
}

/* 
 * idt_add_interrupt_handler
 *  DESCRIPTION:
 *      Given a pointer and index of the handler function, the function adds the function into idt. 
 *      Gate type is set to TRAP because system calls are slow. DPL is set to 3 to enable user call.
 *  INPUTS: 
 *      - index     : index of the handler function in idt
 *      - handler   : pointer to the handler function
 *  OUTPUTS: none
 *  RETURN VALUES: none
 *  SIDE EFFECTS: none
 */
void idt_add_sys_call_gate(uint32_t index, void (*handler)()) {
    /* set idt entry offset */
    SET_IDT_ENTRY(idt[index], handler);
    /* set idt entry segment selector */
    idt[index].seg_selector = KERNEL_CS;
    idt[index].reserved4 = 0;
    /* set gate type to trap because system calls (software interrupts) are slow */
    idt[index].gate_type = TRAP_GATE_TYPE;
    idt[index].reserved0 = 0;
    /* set DPL to user to allow for user call */
    idt[index].dpl = 3;
    /* A filled entry is present */
    idt[index].present = 1;
}

/*
 * idt_fill_exception_handler
 *  DESCRIPTION:
 *      fill all exception handlers into idt
 *  INPUTS: none (but all exception handlers must be present in idt_handler.c)
 *  OUTPUS: none
 *  RETURN VALUES: none
 *  SIDE_EFFECTS: idt entries from 0x00 to 0x20 are set.
 */
void idt_fill_exception_handler() {
    idt_add_trap_handler(0x00, EXCEPTION_0);
    idt_add_trap_handler(0x01, EXCEPTION_1);
    idt_add_trap_handler(0x02, EXCEPTION_2);
    idt_add_trap_handler(0x03, EXCEPTION_3);
    idt_add_trap_handler(0x04, EXCEPTION_4);
    idt_add_trap_handler(0x05, EXCEPTION_5);
    idt_add_trap_handler(0x06, EXCEPTION_6);
    idt_add_trap_handler(0x07, EXCEPTION_7);
    idt_add_trap_handler(0x08, EXCEPTION_8);
    idt_add_trap_handler(0x09, EXCEPTION_9);
    idt_add_trap_handler(0x0A, EXCEPTION_A);
    idt_add_trap_handler(0x0B, EXCEPTION_B);
    idt_add_trap_handler(0x0C, EXCEPTION_C);
    idt_add_trap_handler(0x0D, EXCEPTION_D);
    idt_add_trap_handler(0x0E, EXCEPTION_E);
    idt_add_trap_handler(0x0F, EXCEPTION_F);
    idt_add_trap_handler(0x10, EXCEPTION_10);
    idt_add_trap_handler(0x11, EXCEPTION_11);
    idt_add_trap_handler(0x12, EXCEPTION_12);
    idt_add_trap_handler(0x13, EXCEPTION_13);
}

/*
 * idt_fill_hardware_handler
 *  DESCRIPTION:
 *      fill all hardware handlers into idt
 *  INPUTS: none (but all required device interrupt handlers must be present)
 *  OUTPUS: none
 *  RETURN VALUES: none
 *  SIDE_EFFECTS: the corresponding entries in idt are filled with device interrupt handlers.
 */
void idt_fill_hardware_handler(){
    idt_add_interrupt_handler(KEYBOARD_IDT_INDEX, keyboard_wrap_handler);
    idt_add_interrupt_handler(RTC_IDT_INDEX, rtc_wrap_handler);
    idt_add_interrupt_handler(PIT_IDT_INDEX, pit_wrap_handler);
}
/*
 * idt_fill_system_call
 *  DESCRIPTION:
 *      fill system call entry into idt
 *  INPUTS: none (but all required device interrupt handlers must be present)
 *  OUTPUTS: none
 *  RETURN VALUES: none
 *  SIDE_EFFECTS: 
 *      The corresponding entries in idt are filled with system call entries.
 */
void idt_fill_system_call(){
    idt_add_sys_call_gate(SYS_CALL, sys_call_handler);
}

/* 
 * idt_init
 *  DESCRIPTION:
 *      Initaite the interrupt descriptor table. IDTR is also set.
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUES: none
 *  SIDE_EFFECTS: none
 */
void idt_init() {
    idt_clear();

    idt_fill_exception_handler();
    idt_fill_hardware_handler();
    idt_fill_system_call();

    lidt(idt_desc_ptr);
    printf("Done Initiating IDT\n");
}
