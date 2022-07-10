#include "keyboard.h"

#include "syscall.h"
#include "terminal.h"
#include "i8259.h"

static int32_t capslock_f = 0;  //the flag for capslock
static int32_t left_shift_f = 0;  //the flag for capslock
static int32_t right_shift_f = 0;  //the flag for capslock
static int32_t ctrl_f = 0;  //the flag for capslock
static int32_t alt_f = 0;  //the flag for capslock

static int32_t dir_up_f = 0;  //the flag for directions
static int32_t dir_down_f = 0;

static int32_t function_f = 0;  //the flag bit vector for F1 to F12

uint8_t scancodes_table[MAX_SANCODES][2] = {
    {0x0, 0x0}, {CODE_ESC, CODE_ESC},     
    {'1', '!'}, {'2', '@'},
    {'3', '#'}, {'4', '$'},
    {'5', '%'}, {'6', '^'},
    {'7', '&'}, {'8', '*'},
    {'9', '('}, {'0', ')'},
    {'-', '_'}, {'=', '+'},
    {CODE_BACKSPACE, CODE_BACKSPACE}, {CODE_TAB_PRESS, CODE_TAB_PRESS},    
    {'q', 'Q'}, {'w', 'W'},
    {'e', 'E'}, {'r', 'R'},
    {'t', 'T'}, {'y', 'Y'},
    {'u', 'U'}, {'i', 'I'},
    {'o', 'O'}, {'p', 'P'},
    {'[', '{'}, {']', '}'},
    {CODE_ENTER, CODE_ENTER}, {0x0, 0x0},  
    {'a', 'A'}, {'s', 'S'},
    {'d', 'D'}, {'f', 'F'},
    {'g', 'G'}, {'h', 'H'},
    {'j', 'J'}, {'k', 'K'},
    {'l', 'L'}, {';', ':'},
    {CODE_QUOTE_S, '"'}, {'`', '~'},
    {0x0, 0x0}, {CODE_SLASH, '|'},
    {'z', 'Z'}, {'x', 'X'},
    {'c', 'C'}, {'v', 'V'},
    {'b', 'B'}, {'n', 'N'},
    {'m', 'M'}, {',', '<'},
    {'.', '>'}, {'/', '?'},
    {0x0, 0x0}, {0x0, 0x0},   //reserved places
    {0x0, 0x0}, {CODE_SPACE, CODE_SPACE},  
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0},
    {0x0, 0x0}, {0x0, 0x0},
    {0x0, 0x0}, {0x0, 0x0},
    {0x0, 0x0}, {0x0, 0x0},
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}, 
    {0x0, 0x0}, {0x0, 0x0}  
};

/*
 * get_ctrl_f:
 * DESCRIPTION: get the flag of ctrl key
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: none
 * SIDE EFFECTS: none
 */
int32_t get_ctrl_f(){
    return ctrl_f;
}

/*
 * get_function_f:
 * DESCRIPTION: get the flag of terminal switch
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: function_f
 * SIDE EFFECTS: none
 */
int32_t get_function_f(){
    return function_f;
}

/*
 * get_alt_f:
 * DESCRIPTION: get the flag of alt key
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: none
 * SIDE EFFECTS: none
 */
int32_t get_alt_f(){
    return alt_f;
}

int32_t get_dir_up_f() {
    return dir_up_f;
}

int32_t get_dir_down_f() {
    return dir_down_f;
}

/*
 * keyboard_init:
 * DESCRIPTION: simply enable the irq port to initialize the keyboard
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: none
 * SIDE EFFECTS: none
 */
void keyboard_init() {
    enable_irq(KEYBORAD_IRQ);
    // key_init_f = 1;
    // asm volatile("int $33");
    // printf("Done Initiating Keyboard\n");
}

/*
 * keyboard_setflag:
 * DESCRIPTION: check the key pressed and set the flags
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: 1 if the key pressed will change a flag, else return 0
 * SIDE EFFECTS: check whether the key pressed is a functional key
 */
int32_t keyboard_setflag(uint8_t scancode){
    //determin each flags by the scancode
    //printf("%x",scancode);
    switch(scancode){
        case CODE_LS_PRESS:
            left_shift_f = 1;
            //printf("PRESSED! shift_flag=%d \n", left_shift_f | right_shift_f);
            //printf("shift_pressed!\n");
            return 1;
        case CODE_LS_RELEASE:
            left_shift_f = 0;
            //printf("released!, shift_flag=%d\n", left_shift_f | right_shift_f);
            return 1;
        case CODE_RS_PRESS:
            right_shift_f = 1;
            return 1;
        case CODE_RS_RELEASE:
            right_shift_f = 0;
            return 1;
        case CODE_CAP_PRESS:
            if (capslock_f == 0){
                capslock_f = 1;
            }else{
                capslock_f = 0;
            }
            //printf("CAPSLOCK = %d", capslock_f);
            return 1;
        case CODE_CTRL_PRESS:
            ctrl_f = 1;
            return 1;
        case CODE_CTRL_RELEASE:
            ctrl_f = 0;
            return 1;
        case CODE_ALT_PRESS:
            alt_f = 1;
            return 1;
        case CODE_ALT_RELEASE:
            alt_f = 0;
            return 1;
        case CODE_DIR_UP:
            dir_up_f = 1;
            return 0;
        case CODE_DIR_DOWN:
            dir_down_f = 1;
            return 0;
        case CODE_DIR_UP_R:
            dir_up_f = 0;
            return 0;
        case CODE_DIR_DOWN_R:
            dir_down_f = 0;
            return 0;
        case CODE_F1_PRESS:
            function_f |= 1;
            return 0;
        case CODE_F2_PRESS:
            function_f |= 2;
            return 0;
        case CODE_F3_PRESS:
            function_f |= 4;
            return 0;
        case CODE_F1_RELEASE:
            function_f &= -2;
            return 0;
        case CODE_F2_RELEASE:
            function_f &= -3;
            return 0;
        case CODE_F3_RELEASE:
            function_f &= -5;
            return 0;
        //if it is not a functional key pressed
        default:
            return 0;
    }
}

/*
 * keyboard_handler:
 * DESCRIPTION: get the scancode and handle it
 * INPUTS: none
 * OUTPUTS: none
 * RETURN: none
 * SIDE EFFECTS: handle the history viewing of the terminal by direction key.
 */
void keyboard_handler(){
    //printf("handle\n");
    // if (key_init_f == 0){
    //     send_eoi(KEYBORAD_IRQ);
    //     sti();
    //     return;
    // }

    uint8_t scancode = inb(KEY_DATA_PORT);
    //printf("Scan code = %x\n", scancode);
    uint8_t    curr_ascii_code;
    //check the flag scancodes
    int32_t temp;

    //if a functional key is pressed
    temp = keyboard_setflag(scancode);
    if (temp == 1){
        send_eoi(KEYBORAD_IRQ);
        return;
    }

    //if the scancode is NOT used for printing something, sti and return
    if (scancode > MAX_SANCODES){
        send_eoi(KEYBORAD_IRQ);
        return;
    }
    //these are unavailable currently
    if (scancode == 0  || scancode == 1  || scancode == 15 ||
        scancode == 29 || scancode == 42  ){
        send_eoi(KEYBORAD_IRQ);
        return;
    }
    // if the key pressed is controlled by Shift
    if ((scancode <= 13 && scancode >= 2) || scancode == 26 || scancode == 27 ||
        scancode == 41 || scancode == 42 || scancode == 44 || scancode == 51 ||
        scancode == 52 || scancode == 53 ){
        //if one of those shifts is pressed, use the ascii code of shift pressed
        curr_ascii_code = scancodes_table[scancode][left_shift_f | right_shift_f];            
    }else{
        //in the case of characters, which are controled by capslock
        curr_ascii_code = scancodes_table[scancode][capslock_f ^ (left_shift_f | right_shift_f)];
    }

    // if (curr_ascii_code == 0){
    //     send_eoi(KEYBORAD_IRQ);
    //     return;
    // }

    send_eoi(KEYBORAD_IRQ);
    terminal_handler(curr_ascii_code);

    return;
}
