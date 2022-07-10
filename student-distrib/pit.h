#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "lib.h"

#define PIT_IRQ            0x0
#define PIT_DIV            1193180 //PIT_DIV is used to divide hz to get divisor
#define PIT_CMD_REG        0x43
#define PIT_CHL0_REG       0x40
#define PIT_DEFAULT_FREQ   100
#define PIT_MODE3_CMD      0x36 /*Square Wave */


void pit_handler();
void pit_set_freq(int32_t hz);
void pit_init();

#endif
