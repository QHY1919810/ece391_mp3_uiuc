#include "rtc.h"

#include "i8259.h"
#include "task.h"

// Reference Source: https://wiki.osdev.org/RTC
#define REG_A       0x8A
#define REG_B       0x8B
#define REG_C       0x0C

#define RTC_REG_PORT   0x70
#define RTC_DATA_PORT  0x71
#define BIT_6          0x40
#define RTC_IRQ_NUM    8
#define DEFAULT_LEVEL   6  // 1024Hz
#define TEST_LEVEL      15 // 2Hz
#define MAX_FRQ_RATE    15
#define MIN_FRQ_RATE    1
#define NULL            0
#define REAL_FREQ       1024

file_op_table_t rtc_op_table = {.open = rtc_open, .close = rtc_close, .read = rtc_read, .write = rtc_write};

// The flag to indicate if there is any interrupt occur
// volatile static int8_t RTC_INT_FLAG;

uint8_t pid;
/* 
 *  rtc_set_reg
 *  DESCRIPTION: set an RTC register
 *  INPUTS: reg - the register to be set
  *         value - the value to be set into the register
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: set the value in selected register
 *  Reference source: https://wiki.osdev.org/RTC
 */
void rtc_set_reg(int8_t reg, int8_t value)
{   
    unsigned long flags;
    cli_and_save(flags);		                // important that no interrupts happen (perform a CLI)
    outb(reg, RTC_REG_PORT);	// select Status Register A, and disable NMI (by setting the 0x80 bit)
    outb(value, RTC_DATA_PORT);	// write to CMOS/RTC RAM
    restore_flags(flags);		                // (perform an STI) and reenable NMI if you wish
    return;
}

/* 
 *  rtc_get_reg
 *  DESCRIPTION: get the value of the selected RTC register
 *  INPUTS: reg - the register ng to be get
 *  OUTPUTS: none
 *  RETURN VALUE: int8_t - the value in the selected register
 *  SIDE EFFECTS: get the value of the selected RTC register
 *  Reference source: https://wiki.osdev.org/RTC
 */
int8_t rtc_get_reg(int8_t reg)
{
    outb(reg, RTC_REG_PORT);
    int8_t data = inb(RTC_DATA_PORT);
    return data;
}


/* 
 *  rtc_init
 *  DESCRIPTION: initialize the rtc
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: initialize the rtc
 *  Reference source: https://wiki.osdev.org/RTC
 */
void rtc_init()
{
    unsigned long flags;
    cli_and_save(flags);

    //Turning on IRQ 8 
    int8_t prev;
    prev = rtc_get_reg(REG_B);
    rtc_set_reg(REG_B, prev | BIT_6);
    // pic unmask
    enable_irq(RTC_IRQ_NUM);
    //select register C
    //just throw away contents
    //allow next irq
    rtc_get_reg(REG_C);
    //SET frequency to 1024 HZ
    set_freq(DEFAULT_LEVEL);
    restore_flags(flags);
    printf("Done Initiating RTC\n");
}

/* 
 *  set_freq
 *  DESCRIPTION: set the frequency of rtc
 *  INPUTS: freq_rate - the frequency rate of rtc
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: set the frequency of rtc
 *  Source: https://wiki.osdev.org/RTC
 */

void set_freq(int8_t freq_rate)
{   
    unsigned long flags;
    //frequency =  32768 >> (rate-1);
    //So rate - 1 = freq_rate should be from MIN_FRQ_RATE:1 to MAX_FRQ_RATE:15
    //And the frequency range from 2^1 = 2 to 2^15 = 32768 
    //When input freq_rate = 6, the frequency = 2^10 = 1024
    if(freq_rate < MIN_FRQ_RATE) return;
    // use &0x0F to limited below 15
    int8_t rate = freq_rate & 0x0F;
    cli_and_save(flags);
    int8_t prev = rtc_get_reg(REG_A);

    //reset index to A and write to A
    rtc_set_reg(REG_A, (prev & 0xF0) | rate);
    restore_flags(flags);
    return;
}

/* 
 *  rtc_handler
 *  DESCRIPTION: handle rtc interrupt & support virtual RTC
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: None
 */
void rtc_handler()
{
    // Loop every pcb to check if it is opened or present
    // If not, just skip. If yes, update tick_count
    // If tick_count = 0, set int_flag into 1
    for(pid = 0; pid < MAX_TASK_NUM; pid++){
        pcb_t* cur_pcb = get_pcb_by_pid(pid);
        //if the pcb is not present or pcb is not open, continue for next
        if(cur_pcb->present == 0 || cur_pcb->pcb_freq < 0){
            continue;
        }
        //update pcb tick_count and check if there is an interrupt flag
        //If there is an interrupt, reset every thing & set int_flag to 1
        if (cur_pcb->tick_count > 0) {
            cur_pcb->tick_count -= cur_pcb->pcb_freq;
        } else {
            cur_pcb->int_flag = 1;
        }
    }

    // select register C
    // just throw away contents
    // allow next irq
    rtc_get_reg(REG_C);
    send_eoi(RTC_IRQ_NUM);
}

/* 
 *  rtc_open()
 *  DESCRIPTION: Set the freq of RTC into 2HZ
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: initializes RTC frequency to 2HZ
 */
int32_t rtc_open(const uint8_t* filename){
    //Set virtual frequency into 2HZ
    //Set tick_count as real_freq for virtual interrupt calculation
    //e.g. The current RTC is 1024, and virtual freq is 2 HZ
    //Every real interrupt, we will use tick_count to minus virtual frequency
    //The time for virtual interrupt is when the tick_count equal to 0
    pcb_t* cur_pcb = get_current_pcb();
    cur_pcb->pcb_freq = 2;
    cur_pcb->tick_count = REAL_FREQ;
    cur_pcb->int_flag = 0;
    return 0;
}
/* 
 *  rtc_close()
 *  DESCRIPTION: Reset PCB infomation into closed states
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: None
 */
int32_t rtc_close(int32_t fd){
    //Get the current PCB & Reset virtual frequency(pcb_freq) into -1, 
    //which indicates the current pcb is not opened.
    //Reset tick_count into -1, which indicates the current pcb is not opened(closed)
    //Also reset interrupt flag back to 0
    pcb_t* cur_pcb = get_current_pcb();
    cur_pcb->pcb_freq = -1;
    cur_pcb->tick_count = -1;
    cur_pcb->int_flag = 0;
    return 0;
}
/* 
 *  rtc_read()
 *  DESCRIPTION: block until next interrupt occur
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: 0
 *  SIDE EFFECTS: block until next interrupt occur
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    unsigned long flags;

    pcb_t* cur_pcb = get_current_pcb();
    //Block current PCB until next virtual interrupt occur.
    while(!cur_pcb->int_flag);
    //Next interrupt come, reset interrupt flag back to 0 and set tick_count to real freq.
    cli_and_save(flags); 
    cur_pcb->int_flag = 0;
    cur_pcb->tick_count = REAL_FREQ;
    restore_flags(flags);
    return 0;
}
/* 
 *  rtc_write()
 *  DESCRIPTION: set new frequency
 *  INPUTS: buf -- pointer to the input frequency value
 *  OUTPUTS: none
 *  RETURN VALUE: 0 -- Get valid frequency value and successfully set the frequency
 *               -1 -- value in buf is not valid
 *  SIDE EFFECTS: RTC_freq will be set and be printed in the screen. 
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    unsigned long flags;
    //Check if set_freq_value is valid.
    pcb_t* cur_pcb = get_current_pcb();
    if(buf == NULL){
        return -1;
    }
    //frequency is from 2^1 = 2 to 2^15 = 32768
    //And int16_t is from -32,768 to 32,767
    //uint16_t is from 0 to 65,535 is enough, but document requires 4 bytes(uint32_t)
    //Below is to get value of buf
    uint32_t val = *(uint32_t *)buf;
    int8_t rate = get_rate(val);
    if(rate == 0x00){
        return -1;
    }
    // If val is valid, update current PCB's virtual RTC.
    cli_and_save(flags);
    cur_pcb->pcb_freq = val;
    restore_flags(flags);
    //for success return 0
    return 0;
}
/* 
 *  get_rate(uint32_t val)
 *  DESCRIPTION: get rate of frequency number
 *  INPUTS: val -- The frequency value
 *  OUTPUTS: none
 *  RETURN VALUE: rate -- from 1 to 15;
 *                0x00 -- if rate is invalid;
 *  SIDE EFFECTS: get rate of frequency number
 */
int8_t get_rate(uint32_t val){
    //frequency =  32768 >> (rate-1);
    switch(val){
        // 2 Hz = 32768 >> (0x0F - 1) = 2^15 >> 14 = 2 Hz
        case 2:
            return 0x0F;
        // 4 Hz = 32768 >> (0x0E - 1) = 2^15 >> 13 = 4 Hz
        case 4:
            return 0x0E;
        // 8 Hz = 32768 >> (0x0D - 1) = 2^15 >> 12 = 8 Hz
        case 8:
            return 0x0D;
        // 16 Hz = 32768 >> (0x0C - 1) = 2^15 >> 11 = 16 Hz
        case 16:
            return 0x0C;
        // 32 Hz = 32768 >> (0x0B - 1) = 2^15 >> 10 = 32 Hz
        case 32:
            return 0x0B;
        // 64 Hz = 32768 >> (0x0A - 1) = 2^15 >> 9 = 64 Hz
        case 64:
            return 0x0A;
        // 128 Hz = 32768 >> (0x09 - 1) = 2^15 >> 8 = 128 Hz
        case 128:
            return 0x09;
        // 256 Hz = 32768 >> (0x08 - 1) = 2^15 >> 7 = 256 Hz
        case 256:
            return 0x08;
        // 512 Hz = 32768 >> (0x07 - 1) = 2^15 >> 6 = 512 Hz
        case 512:
            return 0x07;
        // 1024 Hz = 32768 >> (0x06 - 1) = 2^15 >> 5 = 1024 Hz
        case 1024:
            return 0x06;
        //Our kernel limit frequency further to 1024 Hz according to document
        //So the number exceed 1024  or the number is not multiples of 2 will return 0x00
        default:
            return 0x00;      
    }
}

