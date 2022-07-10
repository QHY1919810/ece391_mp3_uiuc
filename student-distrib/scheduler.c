#include "scheduler.h"

#include "x86_desc.h"
#include "page.h"
#include "task.h"
#include "syscall.h"

int32_t curr_active_terminal;
int32_t curr_running_terminal;
terminal_info_t terminal_info_array[3];

/*
 * set_active_terminal()
 *  DESCRIPTION:
 *     set up information of active terminals.
 *  INPUTS:
 *      None
 *  OUTPUTS:
 *      None
 */
void set_active_terminal() {
    terminal_info_t* curr_active_terminal_ptr = &terminal_info_array[curr_active_terminal];
    terminal_info_t* curr_running_terminal_ptr = &terminal_info_array[curr_running_terminal];

    if (curr_active_terminal != curr_running_terminal) {
        curr_running_terminal_ptr->screen_x = get_screen_x();
        curr_running_terminal_ptr->screen_y = get_screen_y();
        curr_running_terminal_ptr->current_show_y = get_current_show_y();
        curr_running_terminal_ptr->view_history_show_y = get_view_history_show_y();
        curr_running_terminal_ptr->enter_flag = get_enter_flag();
        curr_running_terminal_ptr->curr_string_len = get_curr_string_len();
        set_all_x_y(curr_active_terminal_ptr->screen_x, curr_active_terminal_ptr->screen_y, curr_active_terminal_ptr->current_show_y, curr_active_terminal_ptr->view_history_show_y);
        set_video_mem(curr_active_terminal_ptr->video_mem);
        set_buf_video_mem(curr_active_terminal_ptr->buf_video_mem);
        set_enter_flag(curr_active_terminal_ptr->enter_flag);
        set_curr_string_len(curr_active_terminal_ptr->curr_string_len);
        set_keyboard_buffer(curr_active_terminal_ptr->keyboard_buffer);
    }
}
/*
 * restore_running_terminal()
 *  DESCRIPTION:
 *     Restore information of running terminals.
 *  INPUTS:
 *      None
 *  OUTPUTS:
 *      None
 */
void restore_running_terminal() {
    terminal_info_t* curr_active_terminal_ptr = &terminal_info_array[curr_active_terminal];
    terminal_info_t* curr_running_terminal_ptr = &terminal_info_array[curr_running_terminal];

    if (curr_active_terminal != curr_running_terminal) {
        curr_active_terminal_ptr->screen_x = get_screen_x();
        curr_active_terminal_ptr->screen_y = get_screen_y();
        curr_active_terminal_ptr->current_show_y = get_current_show_y();
        curr_active_terminal_ptr->view_history_show_y = get_view_history_show_y();
        curr_active_terminal_ptr->enter_flag = get_enter_flag();
        curr_active_terminal_ptr->curr_string_len = get_curr_string_len();
        set_all_x_y(curr_running_terminal_ptr->screen_x, curr_running_terminal_ptr->screen_y, curr_running_terminal_ptr->current_show_y, curr_running_terminal_ptr->view_history_show_y);
        set_video_mem(curr_running_terminal_ptr->video_mem);
        set_buf_video_mem(curr_running_terminal_ptr->buf_video_mem);
        set_enter_flag(curr_running_terminal_ptr->enter_flag);
        set_curr_string_len(curr_running_terminal_ptr->curr_string_len);
        set_keyboard_buffer(curr_running_terminal_ptr->keyboard_buffer);
    }
}

/*
 * switch_active_terminal(int32_t tid)
 *  DESCRIPTION:
 *      Switch the terminal being displayed on foreground.
 *  INPUTS:
 *      tid - terminal id
 *  OUTPUTS:
 *       0 - succeeded
 *      -1 - failed
 */
int32_t switch_active_terminal(int32_t tid){
    /* Below are steps to be done
     1. Input sanity check
     2. Store current video memory to the buffered area
     3. Save keyboard buffer
     4. Perform Context Switch
     */
    // input sanity check
    if (tid < 0 || tid >= MAX_TERMINAL_NUM || tid == curr_active_terminal) {
        return -1;
    }

    terminal_info_t* curr_terminal_ptr = &terminal_info_array[curr_active_terminal];
    terminal_info_t* next_terminal_ptr = &terminal_info_array[tid];

    // save video memory into buffer
    curr_terminal_ptr->video_mem = (char*) VIDEO + (1 + curr_active_terminal) * (1 << 12);
    memcpy(curr_terminal_ptr->video_mem, (char*) VIDEO, NUM_COLS * NUM_ROWS * 2);   // NUM_COLS * NUM_ROWS * 2BYTE - SIZE OF ONE FRAME

    memcpy((char*) VIDEO, next_terminal_ptr->video_mem, NUM_COLS * NUM_ROWS * 2);   // NUM_COLS * NUM_ROWS * 2BYTE - SIZE OF ONE FRAME
    next_terminal_ptr->video_mem = (char*) VIDEO;

    curr_active_terminal = tid;

    set_active_terminal();
    update_cursor();
    restore_running_terminal();

    return 0;
}

/*
 * switch_running_terminal(int32_t tid)
 *  DESCRIPTION:
 *      Switch the terminal currently being run 
        (This is the scheduler function to be called inside pit interrupt handler) 
 *  INPUTS:
 *      tid - terminal Id
 *  OUTPUTS:
 *       0 - succeeded
 *      -1 - failed
 */
void switch_running_terminal(int32_t tid){
    // 1. input sanity check
    if (tid < 0 || tid >= MAX_TERMINAL_NUM || tid == curr_running_terminal) {
        return;
    }
    
    terminal_info_t* curr_sched_terminal_ptr = &terminal_info_array[curr_running_terminal];
    terminal_info_t* next_sched_terminal_ptr = &terminal_info_array[tid];

    // save current terminal's esp and ebp
    asm volatile("        \n\
        movl %%esp, %0    \n\
        movl %%ebp, %1    \n\
        "
        : "=r"(terminal_info_array[curr_running_terminal].esp), "=r"(terminal_info_array[curr_running_terminal].ebp)
        :
        : "memory"
    );

    // save current terminal info
    curr_sched_terminal_ptr->screen_x = get_screen_x();
    curr_sched_terminal_ptr->screen_y = get_screen_y();
    curr_sched_terminal_ptr->current_show_y = get_current_show_y();
    curr_sched_terminal_ptr->view_history_show_y = get_view_history_show_y();
    curr_sched_terminal_ptr->enter_flag = get_enter_flag();
    curr_sched_terminal_ptr->curr_string_len = get_curr_string_len();

    // load next terminal info
    set_all_x_y(next_sched_terminal_ptr->screen_x, next_sched_terminal_ptr->screen_y, next_sched_terminal_ptr->current_show_y, next_sched_terminal_ptr->view_history_show_y);
    set_video_mem(next_sched_terminal_ptr->video_mem);
    set_buf_video_mem(next_sched_terminal_ptr->buf_video_mem);
    set_user_video_mem(next_sched_terminal_ptr->video_mem);
    set_enter_flag(next_sched_terminal_ptr->enter_flag);
    set_curr_string_len(next_sched_terminal_ptr->curr_string_len);
    set_keyboard_buffer(next_sched_terminal_ptr->keyboard_buffer);

    // 3. Modify current running terminal
    curr_running_terminal = tid;

    // 4. Perform Context Switch
    int32_t next_task_pid = next_sched_terminal_ptr->curr_pid;

    // curr_task = get_pcb_by_pid(curr_task_pid);
    if (next_task_pid == -1) {
        clear();
        execute((uint8_t*)"shell");
    } else {
        // next_task = get_pcb_by_pid(next_task_pid);

        // modify program page
        uint32_t pd_idx = (uint32_t) (USER_IMG_ADDR  >> PAGE_4MB_SHIFT);  //the index of the page directory should be 32 (128 MB)

        pd[pd_idx].pde_4m.present = 1;
        pd[pd_idx].pde_4m.rw = 1;
        pd[pd_idx].pde_4m.us = 1;
        pd[pd_idx].pde_4m.pwt = 0;
        pd[pd_idx].pde_4m.pcd = 0;
        pd[pd_idx].pde_4m.accessed = 0;
        pd[pd_idx].pde_4m.dirty = 0;
        pd[pd_idx].pde_4m.entry_type = 1;
        pd[pd_idx].pde_4m.global = 0;
        pd[pd_idx].pde_4m.ignored = 0;
        pd[pd_idx].pde_4m.pat = 0;
        pd[pd_idx].pde_4m.addr_39_32 = 0;
        pd[pd_idx].pde_4m.reserved = 0;
        pd[pd_idx].pde_4m.addr_31_22 = next_task_pid + 2;  // the per-process 4MB space starts from 8MB in the physical address   
        flush_tlb();

        // modify tss
        tss.ss0 = KERNEL_DS;
        tss.esp0 = STACK_BASE_8_MB - next_task_pid * STACK_SIZE_8_KB - sizeof(uint32_t);

        // restore next terminal's esp and ebp
        asm volatile("     \n\
        movl %%ebx, %%esp  \n\
        movl %%ecx, %%ebp  \n\
        leave     \n\
        ret       \n\
        "
        :
        : "b" (terminal_info_array[curr_running_terminal].esp), "c" (terminal_info_array[curr_running_terminal].ebp)
        : "esp","ebp"
        );
    }
}


/*
 * scheduler_round_robin()
 *  DESCRIPTION:
 *    looping through each terminal.
 *  INPUTS:
 *      None
 *  OUTPUTS:
 *      None
 */
void scheduler_round_robin() {
    switch_running_terminal((curr_running_terminal + 1) % MAX_TERMINAL_NUM);
}
/*
 * get_curr_pid()
 *  DESCRIPTION:
 *    get current running terminal's pid of running PCB.
 *  INPUTS:
 *      None
 *  OUTPUTS:
 *      None
 */
int32_t get_curr_pid(){
    return terminal_info_array[curr_running_terminal].curr_pid;
}
/*
 * set_curr_pid()
 *  DESCRIPTION:
 *    set up current running terminal's pid of running PCB.
 *  INPUTS:
 *      None
 *  OUTPUTS:
 *      None
 */
void set_curr_pid(int32_t pid){
    terminal_info_array[curr_running_terminal].curr_pid = pid;
}
/*
 * terminal_init()
 *  DESCRIPTION:
 *    initialize the terminal.
 *  INPUTS:
 *      None
 *  OUTPUTS:
 *      None
 */
void terminal_init(){
    int i;

    for (i = 0; i < MAX_TERMINAL_NUM; i++) {
        terminal_info_array[i].curr_pid = -1;
        terminal_info_array[i].screen_x = 0;
        terminal_info_array[i].screen_y = 0;
        terminal_info_array[i].current_show_y = 0;
        terminal_info_array[i].view_history_show_y = 0;
        terminal_info_array[i].video_mem = (char*) (VIDEO + (1 + i) * (1 << 12));
        terminal_info_array[i].curr_string_len = 0;
    }

    // intialize the value of terminal 0
    curr_active_terminal = 0;
    curr_running_terminal = 0;
    terminal_info_array[0].video_mem = (char*) VIDEO;
    set_all_x_y(terminal_info_array[0].screen_x, terminal_info_array[0].screen_y,
        terminal_info_array[0].current_show_y, terminal_info_array[0].view_history_show_y); 
    set_video_mem((char*) VIDEO);
    set_buf_video_mem(terminal_info_array[0].buf_video_mem);
    set_user_video_mem((char*) VIDEO);
    set_curr_string_len(terminal_info_array[0].curr_string_len);
    set_keyboard_buffer(terminal_info_array[0].keyboard_buffer);

    // execute shell of terminal 0
    clear();
    execute((uint8_t*) "shell");
}
