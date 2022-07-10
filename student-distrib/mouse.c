// #include "mouse.h"
// #include "i8259.h"
// /*
// Reference:https://wiki.osdev.org/PS/2_Mouse
// Reference:https://forum.osdev.org/viewtopic.php?t=10247
// */
// uint8_t mouse_cycle = 0;

// int8_t mouse_byte_data[3];

// uint8_t mouse_cur_state[NUM_STATE];
// uint8_t mouse_pre_state[NUM_STATE];

// // volatile int32_t mouse_x;
// // volatile int32_t mouse_y;

// void mouse_state_machine()
// {
//     if (mouse_cur_state[0] == 1 &&
//         mouse_pre_state[0] == 0)
//     {
//         left_click_state();
//     }
//     if (mouse_cur_state[2] == 1 &&
//         mouse_pre_state[2] == 0)
//     {
//         middle_click_state(); //current no middle click
//     }
//     if(mouse_cur_state[1] == 1 &&
//     mouse_pre_state[1] == 0)
//     {
//         right_click_state(); // Current no right click
//     }
//     if(mouse_cur_state[0] == 0 &&
//     mouse_pre_state[0] == 1)
//     {
//         left_release_state(); // Current no right click
//     }
//     if(mouse_cur_state[1] == 0 &&
//     mouse_pre_state[1] == 1)
//     {
//         right_release_state(); // Current no right click
//     }
//     if(mouse_cur_state[2] == 0 &&
//     mouse_pre_state[2] == 1)
//     {
//         middle_release_state(); // Current no middle click
//     }
// }

// void mouse_handler()
// {   
//     /*
//     Generic PS/2 Mouse Packet Bits
//     BYTE	7	6	5	4	3	2	1	0
//     0	    yo	xo	ys	xs	ao	bm	br	bl
//     1	                 xm
//     2	                 ym
    
//     Code	Description
//     yo	Y-Axis Overflow
//     xo	X-Axis Overflow
//     ys	Y-Axis Sign Bit (9-Bit Y-Axis Relative Offset)
//     xs	X-Axis Sign Bit (9-Bit X-Axis Relative Offset)
//     ao	Always One
//     bm	Button Middle (Normally Off = 0)
//     br	Button Right (Normally Off = 0)
//     bl	Button Left (Normally Off = 0)
//     xm	X-Axis Movement Value
//     ym	Y-Axis Movement Value
// */
//     switch (mouse_cycle)
//     {
//     case 0:
//         mouse_byte_data[0] = mouse_read();
//         //Check if as is on
//         if( !(mouse_byte_data[0] & 0x8) ){
//             break;
//         }
//         //Check if bl is on
//         if((mouse_byte_data[0] & 0x1)) 
//         {
//             //bl is on, so left click
//             mouse_cur_state[0] = 1;
//         }
//         else mouse_cur_state[0] = 0;
//         //Check if br is on    
//         if((mouse_byte_data[0] & 0x2)) 
//         {
//             //br is on, so right click
//             mouse_cur_state[1] = 1;
//         }
//         else mouse_cur_state[1] = 0;
//         //Check if bm is on
//         if((mouse_byte_data[0] & 0x4)) 
//         {
//             //br is on, so right click
//             mouse_cur_state[2] = 1;
//         }
//         else mouse_cur_state[2] = 0;
//         mouse_cycle++;
//         break;
//     case 1:
//         //mouse_byte_data[1] is X-Axis Movement Value
//         mouse_byte_data[1] = mouse_read();
//         mouse_cycle++;
//         break;
//     case 2:
//         //mouse_byte_data[2] is Y-Axis Movement Value
//         mouse_byte_data[2] = mouse_read();
//         //According to reference
//         mouse_x += mouse_byte_data[1];
//         mouse_y += mouse_byte_data[2];
//         //May need to check bound
//         //Now no check
//         mouse_cycle = 0;
//         break;
//     default:
//         mouse_cycle = 0;
//         break;
//     }
//     mouse_state_machine();
//     // Set current as prev
//     memcpy(mouse_pre_state, mouse_cur_state, NUM_STATE);

//     send_eoi(MOUSE_IRQ);
// }

// void mouse_init()
// {
//     // clear previous state & current state of mouse
//     memset(mouse_cur_state, 0, NUM_STATE);
//     memset(mouse_pre_state, 0, NUM_STATE);
//     // initialize mouse location
//     // The value should be changed later
//     mouse_x = 0;
//     mouse_y = 0;

//     uint8_t _status;

//     // Enable the auxiliary mouse device
//     mouse_wait(1); // 1 is default wait signal
//     outb(0xA8, MOUSE_CTRL_PORT);

//     // Enable the interrupts
//     mouse_wait(1); // 1 is default wait signal
//     outb(0x20, MOUSE_CTRL_PORT);
//     mouse_wait(0); // 0 is defualt wait data
//     _status = (inb(MOUSE_DATA_PORT) | 2);
//     mouse_wait(1); // 1 is default wait signal
//     outb(MOUSE_DATA_PORT, MOUSE_CTRL_PORT);
//     mouse_wait(1); // 1 is default wait signal
//     outb(_status, MOUSE_DATA_PORT);

//     // Tell the mouse to use default settings
//     mouse_write(0xF6);
//     mouse_read(); // Acknowledge

//     // Enable the mouse
//     mouse_write(0xF4);
//     mouse_read(); // Acknowledge

//     // Setup the mouse handler
//     enable_irq(MOUSE_IRQ);
//     kprintf("Init mouse successfully");
// }

// void mouse_wait(uint8_t a_type) // unsigned char
// {
//     uint32_t _time_out = TIME_OUT;
//     if (a_type == 0)
//     {
//         while (_time_out--) // Data
//         {
//             if ((inb(MOUSE_CTRL_PORT) & 1) == 1)
//             {
//                 return;
//             }
//         }
//         return;
//     }
//     else
//     {
//         while (_time_out--) // Signal
//         {
//             if ((inb(MOUSE_CTRL_PORT) & 2) == 0)
//             {
//                 return;
//             }
//         }
//         return;
//     }
// }

// void mouse_write(uint8_t a_write) // unsigned char
// {
//     // Wait to be able to send a command
//     mouse_wait(1);
//     // Tell the mouse we are sending a command
//     outb(0xD4, MOUSE_CTRL_PORT);
//     // Wait for the final part
//     mouse_wait(1);
//     // Finally write
//     outportb(a_write, MOUSE_DATA_PORT);
// }

// uint8_t mouse_read()
// {
//     // Get's response from mouse
//     mouse_wait(0);
//     return inb(MOUSE_DATA_PORT);
// }
