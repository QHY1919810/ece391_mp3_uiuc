#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "lib.h"
#include "filesys_struct.h"

void rtc_set_reg(int8_t reg, int8_t value);
int8_t rtc_get_reg(int8_t reg);
void rtc_init();
void set_freq(int8_t freq_rate);
void rtc_handler();
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int8_t get_rate(uint32_t val);

extern file_op_table_t rtc_op_table;

#endif
