#include "terminal.h"

#include "filesys.h"
#include "keyboard.h"
#include "syscall.h"
#include "scheduler.h"

static int32_t curr_string_len;
static uint8_t* keyboard_buffer; 
static volatile int32_t enter_flag = 0;
static int32_t halt_flag = 0; // bit vector for whether halt in each terminal

file_op_table_t terminal_op_table = {.open = terminal_open, .close = terminal_close, .read = terminal_read, .write = terminal_write};


/*
 * terminal_init:
 * DESCRIPTION: initialize the terminal stuff
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: none
 * SIDE EFFECTS: initialize the cursor
 */
// void terminal_init(){
//     clear();
//     init_cursor();
// }


/*
 * terminal_open:
 * DESCRIPTION: open and initialize the terminal stuff
 * INPUTS: 
 *   - filename (not used)
 * OUTPUTS: none
 * RETURN: 0
 * SIDE EFFECTS: none
 */
int32_t terminal_open(const uint8_t* filename){
    // terminal_init();
    //while(1){printf("114514");}
    return 0;
}

/*
 * terminal_close:
 * DESCRIPTION: clear the terminal and simply return 0
 * INPUTS: 
 *      - fd (unused)
 * OUTPUTS: none
 * RETURN: 0
 * SIDE EFFECTS: none
 */
int32_t terminal_close(int32_t fd){
    clear();
    return -1;
}

/*
 * terminal_handler:
 * DESCRIPTION: handle the different input from the keyboard and put into buffer
 * INPUTS: curr_ascii_code, the current ascii code given by keyboard
 * OUTPUTS: none
 * RETURN: none
 * SIDE EFFECTS: handle the terminal which multiple cases
 */
void terminal_handler(uint8_t curr_ascii_code){
    //the case that ctrl + L is pressed
    if (get_ctrl_f() == 1) {
        switch (curr_ascii_code) {
            case 'L':
                set_active_terminal();
                clear();
                init_cursor();
                restore_running_terminal();
                return;
            case 'l':
                set_active_terminal();
                clear();
                init_cursor();
                restore_running_terminal();
                return;
            case 'C':
                if (curr_active_terminal == curr_running_terminal) {
                    halt(255);
                } else {
                    halt_flag |= 1 << curr_active_terminal;
                }
                return;
            case 'c':
                if (curr_active_terminal == curr_running_terminal) {
                    halt(255);
                } else {
                    halt_flag |= 1 << curr_active_terminal;
                }
                return;
            default:
                return;
        }
    }
    
    if (get_alt_f() == 1){
        switch(get_function_f()){
            case 1:
                switch_active_terminal(0);
                return;
            case 2:
                switch_active_terminal(1);
                return;
            case 4:
                switch_active_terminal(2);
                return;
        }
    }

    if (get_dir_up_f() || get_dir_down_f()) {
        set_active_terminal();
        scroll_and_view_history(get_dir_up_f(), get_dir_down_f());
        restore_running_terminal();
    }
    
    if (curr_ascii_code == 0){
        return;
    }

    //if backspace is hit, decrement the length and clear the last character
    if (curr_ascii_code == CODE_BACKSPACE){
        set_active_terminal();

        //check if there is a expression, if there is not, simply return
        if (curr_string_len == 0){
            restore_running_terminal();
            return;
        }

        //modify the buffer
        curr_string_len--;
        keyboard_buffer[curr_string_len] = NULL;
        //modify the screen
        backspace_handler();
        update_cursor();

        restore_running_terminal();
        return;
    }
    // if enter is pressed, switch to new line and call terminal_read
    if (curr_ascii_code == CODE_ENTER){
        //if pressed enter, set the flag
        terminal_info_array[curr_active_terminal].enter_flag = 1;
        enter_flag = terminal_info_array[curr_running_terminal].enter_flag;
        return;
    }
    //check the limit, while the last place of the buffer is reserved for an LINE FEED
    if (curr_string_len < MAX_TERMINAL_BUF_CHARACTERS - 1){
        //if the string length does not exceed the max, put it to the keyboard buffer
        set_active_terminal();

        keyboard_buffer[curr_string_len] = curr_ascii_code;
        curr_string_len ++;
        putc(curr_ascii_code);
        update_cursor();

        restore_running_terminal();
        return;
    }else{
        //if it exceed the maximum number,do not put into buffer, just put it to screen and and return
        set_active_terminal();

        putc(curr_ascii_code);
        update_cursor();

        restore_running_terminal();
        return;
    }

}

/*
 * terminal_read:
 * DESCRIPTION: wait until an enter is pressed, then read the characters and clear the keyboard buffer
 * INPUTS: fd  - the file descriptor
 *         buf - the user buffer, which read the characters in the keyboard buffer
 *         n -    number of byte to read
 * OUTPUTS: none
 * RETURN: the number of bytes read, else return -1 for failure
 * SIDE EFFECTS: move to the new line on the terminal
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t n){
    int32_t length;
    //check the length
    int32_t i;
    uint8_t* buf_8 = (uint8_t *) buf;
    unsigned long flags;

    //check the null pointer
    if (buf_8 == NULL){
        return -1;
    }
    //wait for the enter
    asm volatile ("terminal_hlt: hlt;");
    if (enter_flag == 0)
        asm volatile ("jmp terminal_hlt;");
    
    //mask the interrupts to protect enter flag
    cli_and_save(flags);
    enter_flag = 0; //set it back
    keyboard_buffer[curr_string_len] = CODE_ENTER; //set the last character of the string to be line feed
    curr_string_len ++;
    putc(CODE_ENTER);  //put the line feed character to the terminal
    length = curr_string_len;
    curr_string_len = 0;  //clear the buffer
    if (curr_active_terminal == curr_running_terminal) {
        update_cursor();
    }

    //copy it to buf
    for (i = 0; i < n; i++){
        if (i < length){
            buf_8[i] = keyboard_buffer[i];
        }else{
            buf_8[i] = NULL;
        }
    }
    restore_flags(flags);

    return length;
}


/*
 * terminal_write:
 * DESCRIPTION: write n bytes from buf to the terminal as well as keyboard buffer
 * INPUTS: fd  - the file descriptor
 *         buf - the user buffer, which write the characters to the keyboard buffer and terminal
 *         n -    number of byte to read
 * OUTPUTS: none
 * RETURN: the number of bytes written, else return -1 for failure
 * SIDE EFFECTS: none
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t n){
    int32_t i;
    uint8_t* buf_8 = (uint8_t*) buf;

    //check the null pointer
    if (buf_8 == NULL){
        return -1;
    }
    
    //copy from buffer and print it to the terminal
    for (i = 0; i < n; i ++){
        if (buf_8[i] != NULL){
            putc(buf_8[i]);
        }
    }
    if (curr_active_terminal == curr_running_terminal) {
        update_cursor();
    }
    return n;
}

int32_t get_enter_flag() {
    return enter_flag;
}

int32_t get_curr_string_len() {
    return curr_string_len;
}

uint8_t* get_keyboard_buffer() {
    return keyboard_buffer;
}

int32_t get_halt_flag(int32_t terminal_id) {
    return (halt_flag & (1 << terminal_id)) ? 1 : 0;
}

void set_enter_flag(int32_t enter_flag_n) {
    enter_flag = enter_flag_n;
}

void set_curr_string_len(int32_t curr_string_len_n) {
    curr_string_len = curr_string_len_n;
}

void set_keyboard_buffer(uint8_t* keyboard_buffer_n) {
    keyboard_buffer = keyboard_buffer_n;
}

void clear_halt_flag(int32_t terminal_id) {
    halt_flag &= ~(1 << terminal_id);
}
