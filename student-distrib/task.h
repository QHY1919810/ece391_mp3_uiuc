#ifndef _TASK_H
#define _TASK_H

#include "types.h"
#include "lib.h"
#include "filesys_struct.h"

#define FD_ARRAY_SIZE           8
#define STACK_BASE_8_MB         0x800000
#define STACK_SIZE_8_KB         0x2000
#define MASK_ADDR_8_KB_BOUND    0xFFFFE000   
#define MAX_TASK_NUM 16
#define MAX_ARGUMENT_SIZE       127         // in accordance with terminal's limit


typedef struct pcb_t {
    int32_t             pid;           // pid start from 0
    int32_t             parent_pid;
    struct pcb_t*       parent_pcb;
    uint32_t            esp;
    uint32_t            ebp;
    uint8_t             present;        // whether this entry is being occupied

    uint32_t            file_desc_num;
    file_desc_t         file_desc_array[FD_ARRAY_SIZE];
    uint8_t             argument[MAX_ARGUMENT_SIZE];

    int32_t             pcb_freq;      // Virtual frequency of pcb
    volatile int32_t    tick_count;    // Counter of ticks, when ticks equal to zero, it should be a interrupt
    volatile int32_t    int_flag;      // Interrupt flag, 0 means no interrupt, 1 means need interrupt. 
} pcb_t;

/* 
 * init_all_pcb
 *  DESCRIPTION:
 *      This function should be called upon system start. 
 *      It set all tasks to empty and initialize cooresponding variables.
 *  INPUTS: none
 *  OUTPUTS: none
 */
pcb_t* create_pcb(uint32_t pid);

/*
 * get_current_pcb:
 * DESCRIPTION: get the current pcb with esp
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: the pointer to the pcb of the current process
 * SIDE EFFECTS: none
 */
pcb_t* get_current_pcb();

/*
 * allocate_pid
 *  DESCRIPTION:
 *      Allocate one available pid. 
 *  INPUT: NONE
 *  OUTPUT: 
 *      -1   - cannot allocate more pid
 *      else - pid
 */
uint32_t allocate_pid();

/* 
 * init_all_pcb
 *  DESCRIPTION:
 *      This function should be called upon system start. 
 *      It set all tasks to empty and initialize cooresponding variables.
 *  INPUTS: none
 *  OUTPUTS: none
 */
void init_all_pcb();

/*
 * reset_pcb
 *  DESCRIPTION:
 *      Given a pid, the function finds the pcb struct and reset the pcb struct 
 *  INPUT:
 *      pid - process id to th struct
 *  OUTPUT:
 *      -1 - reset unsuccessful (invalid pid)
 *       0 - reset succeeded
 *  NOTE:
 *      This function should not be used to reset a running process , can cause unknown bugs.
 */
int32_t reset_pcb(uint32_t pid);

/* 
 * get_pcb_by_pid
 *  DESCRIPTION: 
 *      Given a valid pid, the function returns the pointer to the PCB struct in memory
 *  INPUT:
 *      pid - the pid of the PCB
 *  OUTPUT:
 *      NULL - pid is invalid
 *      else - pointer to the PCB struct
 */
pcb_t* get_pcb_by_pid(uint32_t pid);

/* number of running processes */

#endif
