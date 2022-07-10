#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"
#include "lib.h"

#define USER_IMG_ADDR       0x08048000    //address to run the current user process
#define USER_MEM            0x08000000    //start addr of user memory
#define USER_MEM_END        0x08400000

#define PAGE_4MB_SHIFT      22
#define PAGE_4KB_SHIFT      12
#define MAX_FILENAME_LEN    32

#define NEED_TO_ASSIGN      -1

//magic numbers to check for executable
#define EXE_MAGIC_NUMBER_0  0x7F
#define EXE_MAGIC_NUMBER_1  0x45
#define EXE_MAGIC_NUMBER_2  0x4C
#define EXE_MAGIC_NUMBER_3  0x46

// system call functions
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

int32_t exception_halt (void);

#endif /* SYSCALL_H */
