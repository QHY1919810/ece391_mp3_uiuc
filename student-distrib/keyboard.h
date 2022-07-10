#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "lib.h"

//define the special key codes
#define CODE_ESC             0x1B
#define CODE_BACKSPACE       0x08
#define CODE_ENTER           0x0A
#define CODE_RS_PRESS        0x36
#define CODE_RS_RELEASE      0xB6
#define CODE_LS_PRESS        0x2A
#define CODE_LS_RELEASE      0xAA
#define CODE_CTRL_PRESS      0x1D
#define CODE_CTRL_RELEASE    0x9D
#define CODE_ALT_PRESS       0x38
#define CODE_ALT_RELEASE     0xB8
#define CODE_CAP_PRESS       0x3A
#define CODE_TAB_PRESS       0x20
#define CODE_TAB_RELEASE     0xA0
#define CODE_QUOTE_S         0x27
#define CODE_SLASH           0x5C     
#define CODE_DIR_UP          0x48
#define CODE_DIR_DOWN        0x50
#define CODE_DIR_RIGHT       0x4D
#define CODE_DIR_LEFT        0x4B
#define CODE_DIR_UP_R        0xC8
#define CODE_DIR_DOWN_R      0xD0
#define CODE_DIR_RIGHT_R     0xCD
#define CODE_DIR_LEFT_R      0xCB
#define CODE_SPACE           0x20

#define CODE_F1_PRESS        0x3B
#define CODE_F2_PRESS        0x3C
#define CODE_F3_PRESS        0x3D
#define CODE_F1_RELEASE      0xBB
#define CODE_F2_RELEASE      0xBC
#define CODE_F3_RELEASE      0xBD

#define KEY_DATA_PORT        0x60
#define KEYBORAD_IRQ         0x01
#define EMPTY                0x00
#define MAX_SANCODES         0x5A

extern int32_t get_ctrl_f(void);

extern int32_t get_alt_f(void);

extern int32_t get_function_f(void);

int32_t get_dir_up_f();

int32_t get_dir_down_f();

// keyboard launcher
extern void keyboard_handler();

//keyboard initialization function
extern void keyboard_init();

#endif /* KEYBOARD_H */
