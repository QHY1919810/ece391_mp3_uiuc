#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "lib.h"
#include "filesys_struct.h"

#define MAX_TERMINAL_BUF_CHARACTERS   128
// #define NUM_TERMINAL_COLS    80
// #define NUM_TERMINAL_ROWS    25

// void terminal_init();

//open the terminal
extern int32_t terminal_open(const uint8_t* filename);
//close the terminal
extern int32_t terminal_close(int32_t fd);
//read function
extern int32_t terminal_read(int32_t fd, void* buf, int32_t n);
//write function
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t n);
//handle different input
extern void terminal_handler(uint8_t curr_ascii_code);

int32_t get_enter_flag();

int32_t get_curr_string_len();

uint8_t* get_keyboard_buffer();

int32_t get_halt_flag(int32_t terminal_id);

void set_enter_flag(int32_t enter_flag_n);

void set_curr_string_len(int32_t curr_string_len_n);

void set_keyboard_buffer(uint8_t* keyboard_buffer_n);

void clear_halt_flag(int32_t terminal_id);

extern file_op_table_t terminal_op_table;
#endif /* TERMINAL_H */
