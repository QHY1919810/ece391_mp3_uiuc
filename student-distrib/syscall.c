#include "syscall.h"

#include "page.h"
#include "x86_desc.h"
#include "terminal.h"
#include "filesys.h"
#include "filesys_struct.h"
#include "rtc.h"
#include "task.h"
#include "scheduler.h"

/*
 * execute:
 * DESCRIPTION: excute system call depending on input command
 * INPUTS: command -- The input command to excute
 * OUTPUTS: none
 * RETURN: 0 -- successful calls
 *         1 -- failed calls
 * SIDE EFFECTS: none
 */
int32_t execute(const uint8_t* command)
{
    /* Steps to be carried out 
        1. Parse the command (arg list not supported in CH3)
        2. Check the file's validity
        3. Set up paging for the user program
        4. create PCB which will be loaded into our kernel stack
        5. prepare for context swtich and push the iret arguments
        6. use iret to jump to the target program
    */

    
    int32_t     i;                           // variable for for loop
    uint32_t    flags;                       // flag for critical part
    uint32_t    pd_idx;                      // index into page directory
    uint8_t     fname[MAX_FILENAME_LEN];     // file name 
    uint8_t     argument[MAX_ARGUMENT_SIZE]; // Buffer for argument
    dentry_t    exe_dentry;                  // dentry to fetch the executable file
    uint32_t    pid;                         // pid
    pcb_t*      pcb;                         // pointer to the pcb entry specified by pid
    

    //check validity of the argument
    if (command == NULL){
        return -1;
    }
    
    pid = allocate_pid();
    if (pid == -1) {
        // cannot allocate more pid
        printf("Number of processes reached the limit (%d)\n", MAX_TASK_NUM);
        return -1;
    }


    // Parse file name 
    memset(fname, NULL, MAX_FILENAME_LEN);
    memset(argument, NULL, MAX_ARGUMENT_SIZE);
    // eliminate preceding spaces
    for (i = 0; i < MAX_ARGUMENT_SIZE; i++){
        if (command[i] != ' ') { 
            if (MAX_FILENAME_LEN > (MAX_ARGUMENT_SIZE - i)) {
                // avoid copying out of the command buffers
                memcpy(fname, command + i, MAX_ARGUMENT_SIZE - i);
            } else {
                memcpy(fname, command + i, MAX_FILENAME_LEN);
            }
            break; 
        }
    }
    int j;
    for (j = 0; j < MAX_FILENAME_LEN; j++){
        if (fname[j] == ' ') {
            fname[j] = '\0';
            break;
        }
    }

    /* Parse commands
        Commands will be copied into a temporary buffer and later copied into the PCB
     */
    // skip the executable name
    for ( ; i < MAX_ARGUMENT_SIZE; i++) {
        if (command[i] == ' ') break;
    }
    // skip the leading spaces
    for ( ; i < MAX_ARGUMENT_SIZE; i++) {
        if (command[i] != ' ') break;
    }
    memcpy(argument, command + i, MAX_ARGUMENT_SIZE - i);



    //Check for existence 
    if(read_dentry_by_name(fname,&exe_dentry) == -1){
        printf("filename does not exist!\n");
        return -1;
    }  
    //Check the file type
    if (exe_dentry.file_type != FILE_FILE_TYPE){
        printf("filetype check fails!\n");
        return -1;
    }
    //Check for executable
    //read the data
    uint8_t buf[4]; 
    if (read_data(exe_dentry.inode_idx, 0, buf, 4) != 4){
        printf("Read data fails!\n");
        return -1;
    }
    //use four magic numbers to check for executable
    if (buf[0] != EXE_MAGIC_NUMBER_0 || buf[1] != EXE_MAGIC_NUMBER_1 || buf[2] != EXE_MAGIC_NUMBER_2 || buf[3] != EXE_MAGIC_NUMBER_3){
        printf("Magic number check fails!\n");
        return -1;    
    }
    // done checking, safe to move on now


    //Set up paging
    pd_idx = (uint32_t) (USER_IMG_ADDR  >> PAGE_4MB_SHIFT);  //the index of the page directory should be 32 (128 MB)

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
    pd[pd_idx].pde_4m.addr_31_22 = pid + 2;  // the per-process 4MB space starts from 8MB in the physical address   
    flush_tlb();

    
    //Load file image into the corresponding address in the memory (0x08048000 user program addr)
    int32_t bytes_read;
    int32_t offset = 0;
    while(1){
        bytes_read = read_data(exe_dentry.inode_idx, offset, (void*)(USER_IMG_ADDR + offset), BLOCK_SIZE);
        if (bytes_read < BLOCK_SIZE) {
            break;
        }
        else if (bytes_read == -1) {
            printf("Load file fails in reading data!\n");
            return -1;
        }
        else {
            offset += bytes_read;
        }
    }
    
    // set PCB struct
    pcb = create_pcb(pid);

    pcb->present = 1;
    // open stdin & stdout for the task
    pcb->file_desc_num = 2;
    pcb->file_desc_array[0].file_op_table = &terminal_op_table;
    pcb->file_desc_array[0].flags = FD_FLAG_PRESENT;
    pcb->file_desc_array[1].file_op_table = &terminal_op_table;
    pcb->file_desc_array[1].flags = FD_FLAG_PRESENT;

    // set parent process info
    pcb->parent_pid = get_curr_pid();
    if (pcb->parent_pid == -1)
        pcb->parent_pcb = NULL;
    else
        pcb->parent_pcb = get_current_pcb();
    set_curr_pid(pid);

    // copy argument
    memcpy(pcb->argument, argument, MAX_ARGUMENT_SIZE);

    // 5. prepare for context swtich and push the iret arguments
    cli_and_save(flags);
    // set kernal mode
    tss.ss0 = KERNEL_DS;
    //get the addr of stack (the firt 4 bytes are reserved for the pointer of the struct of the pcb)
    tss.esp0 = STACK_BASE_8_MB - pcb->pid * STACK_SIZE_8_KB - sizeof(uint32_t);
    restore_flags(flags);

    //save the esp and ebp
    asm volatile("        \n\
        movl %%esp, %0    \n\
        movl %%ebp, %1    \n\
        "
        : "=r"(pcb->esp), "=r"(pcb->ebp)
        :
        : "memory"
    );
 
    /* Context Switch */
    uint8_t eip_buf[4];
    uint32_t return_addr;
    read_data(exe_dentry.inode_idx,24,eip_buf,sizeof(uint32_t));
    return_addr=*((uint32_t *)eip_buf);   
    
    uint32_t user_esp = USER_MEM_END - sizeof(int32_t);

    sti();
    asm volatile("              \n\
        andl   $0x00FF, %%ebx   \n\
        movw   %%bx, %%ds       \n\
        pushl  %%ebx            \n\
        pushl  %%edx            \n\
        pushfl                  \n\
        popl   %%ebx            \n\
        orl    $0x200, %%ebx    \n\
        pushl  %%ebx            \n\
        pushl  %%ecx            \n\
        pushl  %%eax            \n\
        iret                    \n\
        "
        :
        : "a"(return_addr), "b"(USER_DS), "c"(USER_CS), "d"(user_esp)
        : "memory"
    );

    return -1;
}



/*
 * halt:
 * DESCRIPTION: halt the current process
 * INPUTS: status of current process
 * OUTPUTS: none
 * RETURN: exit status
 * SIDE EFFECTS: none
 */
int32_t halt (uint8_t status)
{
    /* 
        1. restore parent paging
        2. close all files
        3. return to parent execution
    */

    // decrement the number of processes

    pcb_t* pcb = get_current_pcb();
    uint32_t pd_idx;
    uint32_t flags;

    if (pcb->parent_pid != -1){

        // Write Parent process’ info back to TSS 
        cli_and_save(flags);
        tss.ss0 = KERNEL_DS;
        tss.esp0 = STACK_BASE_8_MB - (pcb->parent_pid) * STACK_SIZE_8_KB - sizeof(uint32_t);
        restore_flags(flags);

        // Restore parent's paging
        pd_idx = (uint32_t) (USER_IMG_ADDR  >> PAGE_4MB_SHIFT );  //the index of the page table
    
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
        pd[pd_idx].pde_4m.addr_31_22 = pcb->parent_pid + 2;  //2 is the where the program on pd begin
        //flush the tlb
        flush_tlb();
    }
    
    // here we "lazy" clean up the pcb. The full clean up is done when calling "execute".

    // set pcb to not present
    pcb->present = 0;
    set_curr_pid(pcb->parent_pid);

    // Close any relevant FDs in use
    int32_t tmp_fd = 0;
    for (tmp_fd = 0; tmp_fd < FD_ARRAY_SIZE; tmp_fd++){
        if (pcb->file_desc_array[tmp_fd].flags & FD_FLAG_PRESENT){
            close(tmp_fd);  
        }
    }
    if (pcb->parent_pid == -1){
        execute((uint8_t*)"shell");
    }else{
        
        asm volatile("     \n\
        movl %%edx, %%eax  \n\
        movl %%ebx, %%esp  \n\
        movl %%ecx, %%ebp  \n\
        leave     \n\
        ret       \n\
        "
        :
        : "b" (pcb->esp), "c" (pcb->ebp), "d" (status)
        : "esp","ebp","eax"
        );
    }

    return 1;
}

/*
 * halt:
 * DESCRIPTION: halt the current process
 * INPUTS: status of current process
 * OUTPUTS: none
 * RETURN: 256 (exit with exception)
 * SIDE EFFECTS: none
 */
int32_t exception_halt (void)
{
    /* 
        1. restore parent paging
        2. close all files
        3. return to parent execution
    */

    // decrement the number of processes

    pcb_t* pcb = get_current_pcb();
    uint32_t pd_idx;
    uint32_t flags;

    if (pcb->parent_pid != -1){

        // Write Parent process’ info back to TSS 
        cli_and_save(flags);
        tss.ss0 = KERNEL_DS;
        tss.esp0 = STACK_BASE_8_MB - (pcb->parent_pid) * STACK_SIZE_8_KB - sizeof(uint32_t);
        restore_flags(flags);

        // Restore parent's paging
        pd_idx = (uint32_t) (USER_IMG_ADDR  >> PAGE_4MB_SHIFT );  //the index of the page table
    
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
        pd[pd_idx].pde_4m.addr_31_22 = pcb->parent_pid + 2;  //2 is the where the program on pd begin
        //flush the tlb
        flush_tlb();
    }
    
    // here we "lazy" clean up the pcb. The full clean up is done when calling "execute".

    // set pcb to not present
    pcb->present = 0;
    set_curr_pid(pcb->parent_pid);

    // Close any relevant FDs in use
    int32_t tmp_fd = 0;
    for (tmp_fd = 0; tmp_fd < FD_ARRAY_SIZE; tmp_fd++){
        if (pcb->file_desc_array[tmp_fd].flags & FD_FLAG_PRESENT){
            close(tmp_fd);  
        }
    }
    if (pcb->parent_pid == -1){
        execute((uint8_t*)"shell");
    }else{
        
        asm volatile("     \n\
        movl %%edx, %%eax  \n\
        movl %%ebx, %%esp  \n\
        movl %%ecx, %%ebp  \n\
        leave     \n\
        ret       \n\
        "
        :
        : "b" (pcb->esp), "c" (pcb->ebp), "d" (256)
        : "esp","ebp","eax"
        );
    }

    return 1;
}

/*
 * open:
 * DESCRIPTION: 
 *  Get the current task using get_current_pcb. The function tries to look for a file with specified name. 
 *  If yes, the function opens the file by extracting the file type from dentry and calls the corresponding handler 
 * INPUTS: 
 *   - filename : name of the file
 * RETURN: 
 *   -1 : cannot open file
 *   else : fd of the file 
 */
int32_t open (const uint8_t* filename)
{
    int32_t fd;
    pcb_t* curr_pcb;
    dentry_t dentry;
    
    // check for null ptr
    if (filename == NULL) return -1;

    // get current task
    curr_pcb = get_current_pcb();

    // check if we can allocate more files
    if (curr_pcb->file_desc_num >= FD_ARRAY_SIZE) return -1;

    // find the first entry not present
    for (fd = 0; fd < FD_ARRAY_SIZE; fd++) {
        if (curr_pcb->file_desc_array[fd].flags & FD_FLAG_PRESENT) {
            continue;
        }
        break;
    }

    // extract file info
    if (read_dentry_by_name(filename, &dentry) == -1) {
        // file cannot be found
        return -1;
    }    

    switch (dentry.file_type)
    {
    case DIR_FILE_TYPE:
        curr_pcb->file_desc_array[fd].file_op_table = &dir_op_table;
        break;

    case FILE_FILE_TYPE:
        curr_pcb->file_desc_array[fd].file_op_table = &file_op_table;
        break;
        
    case RTC_FILE_TYPE:
        curr_pcb->file_desc_array[fd].file_op_table = &rtc_op_table;
        break;
    
    default:
        // unrecognized file type
        return -1;
    }
    
    // increment number of opened files
    curr_pcb->file_desc_num++;
    
    // open the file
    curr_pcb->file_desc_array[fd].flags = 1;
    curr_pcb->file_desc_array[fd].file_position = 0;
    curr_pcb->file_desc_array[fd].inode_idx = dentry.inode_idx;

    curr_pcb->file_desc_array[fd].file_op_table->open(filename);


    return fd;
}


/*
 * close:
 * DESCRIPTION: 
 *      call the corresponding close function 
 * INPUTS: 
 * RETURN: 
 *       0 : close file successful
 *      -1 : close file failed  
 * SIDE EFFECTS: 
 */
int32_t close (int32_t fd)
{
    pcb_t* curr_pcb;
    curr_pcb = get_current_pcb();


    // stdin & stdout cannot be closed and fd should < FD_ARRAY_SIZE
    if (fd < 2 || fd >= FD_ARRAY_SIZE) {
        return -1;
    }

    /* close file only when the file is opened */
    if ((curr_pcb->file_desc_array[fd].flags & FD_FLAG_PRESENT) && curr_pcb->file_desc_array[fd].file_op_table->close(fd) == 0) {
        // close file succeeded

        curr_pcb->file_desc_num--;
        curr_pcb->file_desc_array[fd].file_op_table = NULL;
        curr_pcb->file_desc_array[fd].file_position = 0;
        curr_pcb->file_desc_array[fd].flags = 0;
        curr_pcb->file_desc_array[fd].inode_idx = -1;

        return 0;
    } else {
        // close file failed
        return -1;
    }
}


/*
 * read:
 * DESCRIPTION: 
 *      call the corresponding read function 
 * INPUTS: 
 *      - fd     : index into file_desc_array
 *      - buf    : the read buffer
 *      - nbytes : number of bytes to be read 
 * OUTPUTS: 
 *      -1   : read failed
 *      else : number of bytes read
 * SIDE EFFECTS: 
 *      file_position will be increased (+= bytes read)
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes)
{
    if (buf == NULL || nbytes < 0) return -1;

    // fd should < FD_ARRAY_SIZE and stdout should not be read
    if (fd < 0 || fd == 1 || fd >= FD_ARRAY_SIZE) {
        return -1;
    }
    
    pcb_t* curr_pcb;
    curr_pcb = get_current_pcb();

    if ((curr_pcb->file_desc_array[fd].flags & FD_FLAG_PRESENT) == 0) {
        // if file is not opened
        return -1;
    } 
    
    return curr_pcb->file_desc_array[fd].file_op_table->read(fd, buf, nbytes);
}

/*
 * write:
 * DESCRIPTION: 
 *      call the corresponding write function 
 * INPUTS: 
 *      - fd     : index into file_desc_array
 *      - buf    : the write buffer
 *      - nbytes : number of bytes to be write 
 * OUTPUTS: 
 *      -1   : write failed
 *      else : number of bytes written
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes)
{
    if (buf == NULL || nbytes < 0) return -1;

    // fd should be within the valid range and stdin should not be read
    if (fd <= 0 || fd >= FD_ARRAY_SIZE) return -1;

    pcb_t* curr_pcb;
    curr_pcb = get_current_pcb();

    if ((curr_pcb->file_desc_array[fd].flags & FD_FLAG_PRESENT) == 0) {
        // if file is not opened
        return -1;
    }
    
    return curr_pcb->file_desc_array[fd].file_op_table->write(fd, buf, nbytes);
}

/* 
 * getargs
 *  DESCRIPTION:
 *      The getargs system call
 *  INPUTS:
 *      buf    - buffer to be copied to
 *      nbytes - number of bytes desired to copy.
 *  RETURN VALUES:
 *      -1  - cannot copy argument due to invalid inputs
 *       0  - success 
 */

int32_t getargs (uint8_t* buf, int32_t nbytes){
    pcb_t*  curr_pcb; 

    if (buf == NULL || nbytes < 0) return -1;
    // if (nbytes == 0) return 0;

    curr_pcb = get_current_pcb();
    
    // no argument
    if (curr_pcb->argument[0] == NULL || strlen((int8_t*) curr_pcb->argument) + 1 > nbytes) return -1;

    memcpy(buf, curr_pcb->argument, strlen((int8_t*) curr_pcb->argument) + 1);

    return 0;
}


/* 
 * vidmap
 *  DESCRIPTION:
 *      The getargs system call
 *  OUTPUTS:
 *      screen_start - pointer to video memory address
 *  RETURN VALUES:
 *      -1  - video map error
 *       0  - success
 */
int32_t vidmap (uint8_t** screen_start){
    /* Check whether the address falls in user-level page */
    if (screen_start < (uint8_t**)USER_MEM || screen_start >= (uint8_t**)USER_MEM_END)
        return -1;

    /* Write the user video memory address */
    *screen_start = (uint8_t*)USER_VIDEO;

    return 0;
}
//Extra points part, not for now.
int32_t set_handler (int32_t signum, void* handler_address){
    if (handler_address == NULL) return -1;
    return 0;
}
//Extra points part, not for now
int32_t sigreturn (void){
    return 0;
}
