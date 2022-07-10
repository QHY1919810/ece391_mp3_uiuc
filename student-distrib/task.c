#include "task.h"

/* 
 * init_all_pcb
 *  DESCRIPTION:
 *      This function should be called upon system start. 
 *      It set all tasks to empty and initialize cooresponding variables.
 *  INPUTS: none
 *  OUTPUTS: none
 */
void init_all_pcb() {
    int pid;

    for (pid = 0; pid < MAX_TASK_NUM; pid++) {
        reset_pcb(pid);
    }
}

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
int32_t reset_pcb(uint32_t pid) {
    pcb_t* pcb;
    int32_t fd;

    pcb = get_pcb_by_pid(pid); 
    if (pcb == NULL) {return -1;}

    pcb->pid = pid;
    pcb->parent_pid = -1;
    pcb->present = 0;
    pcb->tick_count = -1; //-1 is an invalid value to indicate need open
    pcb->pcb_freq = -1;   //-1 is an invalid value to indicate need open
    pcb->int_flag = 0;  
    // clear fd entries
    pcb->file_desc_num = 0;
    for (fd = 0; fd < FD_ARRAY_SIZE; fd++) {
        pcb->file_desc_array[fd].file_op_table = NULL;
        pcb->file_desc_array[fd].file_position = 0;
        pcb->file_desc_array[fd].flags = 0;
        pcb->file_desc_array[fd].inode_idx = -1;
    } 

    // clear the argument list
    memset(pcb->argument, NULL, MAX_ARGUMENT_SIZE);

    return 0;
}

/*
 * create_pcb:
 * DESCRIPTION: create a new pcb
 * INPUTS: pid        -- the pcb id to use, pid start from 1
 * OUTPUTS: none
 * RETURN: the pointer of the pcb created
 * SIDE EFFECTS: create a pcb on the stack
 */
pcb_t* create_pcb(uint32_t pid)
{
    pcb_t* new_pcb = get_pcb_by_pid(pid);
    
    if (new_pcb == NULL) { return NULL; }

    // reset the cooresponding pcb struct
    reset_pcb(pid);

    return new_pcb;
}



/*
 * get_current_pcb:
 * DESCRIPTION: get the current pcb with esp
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: the pointer to the pcb of the current process
 * SIDE EFFECTS: none
 */
pcb_t* get_current_pcb()
{
    uint32_t* curr_pcb = NULL;
    /* since system call executes in the per-process 8KB area, and the area is aligned to the 8KB 
    boundary, we can get the pointer by getting esp and masking out lower bits.
    Note that we should use esp instead of ebp because ebp can be equal to the lower boundary */
    asm volatile ("             \n\
        movl %%esp, %0          \n\
        "
        : "=r"(curr_pcb)
        :           // no input
        : "esp"
        );        // no clobbered registers

    // masking out lower bits 
    return  (pcb_t *) (((uint32_t) curr_pcb) & MASK_ADDR_8_KB_BOUND);

}

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
pcb_t* get_pcb_by_pid(uint32_t pid) {
    // validate the pid
    if (pid < 0 || pid >= MAX_TASK_NUM) { return NULL; }
    return (pcb_t *)(STACK_BASE_8_MB - (pid + 1) * STACK_SIZE_8_KB);
}

/*
 * allocate_pid
 *  DESCRIPTION:
 *      Allocate one available pid. 
 *  INPUT: NONE
 *  OUTPUT: 
 *      -1   - cannot allocate more pid
 *      else - pid
 */
uint32_t allocate_pid() {
    int pid; 

    for (pid = 0; pid < MAX_TASK_NUM; pid++) {
        // no need to check for NULL because pid is valid
        if(get_pcb_by_pid(pid)->present == 0){
            // found a vacant pid
            break;
        }
    }

    return (pid == MAX_TASK_NUM) ? -1 : pid;
}
