/* system call jump table */
.text
.align 4
sys_call_jump_table:
    .long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

.global keyboard_wrap_handler, rtc_wrap_handler, sys_call_handler, pit_wrap_handler

/*
 * keyboard_wrap_handler
 *  DESCRIPTION:
 *      assembly linkage for hardware interrupt handler. 
 *      saves & restores all registers before & after the handler being executed
 */
keyboard_wrap_handler:
    pushal
    call    keyboard_handler
    popal
    iret

/*
 * rtc_wrap_handler
 *  DESCRIPTION:
 *      assembly linkage for hardware interrupt handler. 
 *      saves & restores all registers before & after the handler being executed
 */
rtc_wrap_handler:
    pushal
    call    rtc_handler
    popal
    iret

/*
 * pit_wrap_handler
 *  DESCRIPTION:
 *      assembly linkage for hardware interrupt handler. 
 *      saves & restores all registers before & after the handler being executed
*/
 
pit_wrap_handler:
    pushal
    call    pit_handler
    popal
    iret

/*
 * sys_call_handler
 *  DESCRIPTION:
 *      assembly linkage for system call handler.
 *      saves & restores all registers before & after the handler being executed
 */
sys_call_handler:
    /* save all registers */
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esp
    pushl   %ebp
    pushl   %esi
    pushl   %edi
    pushfl

    /* validate system call number */
    cmpl    $0, %eax
    jz      sys_call_error
    cmpl    $10, %eax
    ja      sys_call_error

    /* push all arguments */
    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    /* system call linkage */
    call    *sys_call_jump_table(, %eax, 4)
    addl    $12, %esp

    jmp     sys_call_return

sys_call_error:
    movl    $-1, %eax
sys_call_return:
    /* restore all registers */
    popfl
    popl   %edi
    popl   %esi
    popl   %ebp
    popl   %esp
    popl   %edx
    popl   %ecx
    popl   %ebx
    iret
