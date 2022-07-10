#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "lib.h"
#include "terminal.h"

#define MAX_TERMINAL_NUM    3
#define BUF_VIDEO_MEM_SIZE  (10*NUM_COLS*NUM_ROWS*2)

// typedef struct task_regs
// {
//     uint32_t ebx;
//     uint32_t ecx;
//     uint32_t edx;
//     uint32_t esi;
//     uint32_t edi;
//     uint32_t flags;
// } task_regs_t;


typedef struct terminal_info_t {
    int     screen_x;
    int     screen_y;
    int     current_show_y;
    int     view_history_show_y;
    char*   video_mem;
    char    buf_video_mem[BUF_VIDEO_MEM_SIZE];

    int32_t enter_flag;
    int32_t curr_string_len;
    uint8_t keyboard_buffer[MAX_TERMINAL_BUF_CHARACTERS]; 

    int32_t curr_pid;
    uint32_t esp;
    uint32_t ebp;
    // task_regs_t curr_regs;
    
} terminal_info_t;

int32_t curr_active_terminal;
int32_t curr_running_terminal;
terminal_info_t terminal_info_array[3];

void set_active_terminal();

void restore_running_terminal();

/*
 * switch_active_terminal
 *  DESCRIPTION:
 *      Switch the terminal being displayed on foreground.
 *  INPUTS:
 *      tid - terminal id
 *  OUTPUTS:
 *       0 - succeeded
 *      -1 - failed
 */
int32_t switch_active_terminal(int32_t tid);

/*
 * switch_running_terminal
 *  DESCRIPTION:
 *      Switch the terminal currently being run 
        (This is the scheduler function to be called inside pit interrupt handler) 
 *  INPUTS:
 *      tid - terminal Id
 *  OUTPUTS:
 *       0 - succeeded
 *      -1 - failed
 */
void switch_running_terminal(int32_t tid);

void scheduler_round_robin();

int32_t get_curr_pid();

void set_curr_pid(int32_t pid);

void terminal_init();

#endif /* SCHEDULER_H */
