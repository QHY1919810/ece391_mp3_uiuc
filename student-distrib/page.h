#ifndef _PAGE_H
#define _PAGE_H

#define VIDEO           0xB8000
#define VIDEO_INDEX     0xB8
// #define USER_PROGRAM    0x8000000
// #define USER_PROGRAM_INDEX  0x20
#define USER_VIDEO          0x8400000
#define USER_VIDEO_INDEX    0x21

/* Set the value on each field of page table and page directory */
void page_init();

/* Refresh CR3(PDBR) */
/* Translation lookaside buffers will be automatically flushed */
void flush_tlb();

/* Set the map to user video memory */
void set_user_video_mem(void* user_video_mem);

#endif /* _PAGE_H */
