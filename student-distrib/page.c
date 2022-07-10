#include "page.h"

#include "x86_desc.h"

static void enable_paging();

/* page_init
 *
 * DESCRIPTION: Set the value on each field of pages
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS: Change the values in paging structure
 */
void page_init() {
    int i; /* loop indicies */

    /* Initialize page directory */
    for (i = 0; i < NUM_PDE; i++) {
        /* Page table of video memory */
        if (i == 0) {
            pd[i].pde_table.present = 1;
            pd[i].pde_table.rw = 1;
            pd[i].pde_table.us = 0;
            pd[i].pde_table.pwt = 0;
            pd[i].pde_table.pcd = 0;
            pd[i].pde_table.accessed = 0;
            pd[i].pde_table.ign = 0;
            pd[i].pde_table.entry_type = 0;
            pd[i].pde_table.ignored = 0;
            pd[i].pde_table.addr_31_12 = (unsigned long)pt_video >> 12;
        }

        /* Kernel page */
        else if (i == 1) {
            pd[i].pde_4m.present = 1;
            pd[i].pde_4m.rw = 1;
            pd[i].pde_4m.us = 0;
            pd[i].pde_4m.pwt = 0;
            pd[i].pde_4m.pcd = 0;
            pd[i].pde_4m.accessed = 0;
            pd[i].pde_4m.dirty = 0;
            pd[i].pde_4m.entry_type = 1;
            pd[i].pde_4m.global = 1;
            pd[i].pde_4m.ignored = 0;
            pd[i].pde_4m.pat = 0;
            pd[i].pde_4m.addr_39_32 = 0;
            pd[i].pde_4m.reserved = 0;
            pd[i].pde_4m.addr_31_22 = 1;
        }

        /* Page table of user program video memory */
        else if (i == USER_VIDEO_INDEX) {
            pd[i].pde_table.present = 1;
            pd[i].pde_table.rw = 1;
            pd[i].pde_table.us = 1;
            pd[i].pde_table.pwt = 0;
            pd[i].pde_table.pcd = 0;
            pd[i].pde_table.accessed = 0;
            pd[i].pde_table.ign = 0;
            pd[i].pde_table.entry_type = 0;
            pd[i].pde_table.ignored = 0;
            pd[i].pde_table.addr_31_12 = (unsigned long)pt_user_video >> 12;
        }
   
        else {
            pd[i].pde_table.present = 0;
            pd[i].pde_table.rw = 0;
            pd[i].pde_table.us = 0;
            pd[i].pde_table.pwt = 0;
            pd[i].pde_table.pcd = 0;
            pd[i].pde_table.accessed = 0;
            pd[i].pde_table.ign = 0;
            pd[i].pde_table.entry_type = 0;
            pd[i].pde_table.ignored = 0;
            pd[i].pde_table.addr_31_12 = 0;
        }
    }

    /* Initialize first page table */
    for (i = 0; i < NUM_PTE; i++) {
        /* Video memory page */
        if (i >= VIDEO_INDEX && i <= VIDEO_INDEX + 3) {
            pt_video[i].present = 1;
            pt_video[i].rw = 1;
            pt_video[i].us = 0;
            pt_video[i].pwt = 0;
            pt_video[i].pcd = 0;
            pt_video[i].accessed = 0;
            pt_video[i].dirty = 0;
            pt_video[i].pat = 0;
            pt_video[i].global = 0;
            pt_video[i].ignored = 0;
            pt_video[i].addr_31_12 = i;
        }

        else {
            pt_video[i].present = 0;
            pt_video[i].rw = 0;
            pt_video[i].us = 0;
            pt_video[i].pwt = 0;
            pt_video[i].pcd = 0;
            pt_video[i].accessed = 0;
            pt_video[i].dirty = 0;
            pt_video[i].pat = 0;
            pt_video[i].global = 0;
            pt_video[i].ignored = 0;
            pt_video[i].addr_31_12 = 0;
        }
    }

    /* Initialize table for user program video memory  */
    for (i = 0; i < NUM_PTE; i++) {
        /* Video memory page */
        if (i == 0) {
            pt_user_video[i].present = 1;
            pt_user_video[i].rw = 1;
            pt_user_video[i].us = 1;
            pt_user_video[i].pwt = 0;
            pt_user_video[i].pcd = 0;
            pt_user_video[i].accessed = 0;
            pt_user_video[i].dirty = 0;
            pt_user_video[i].pat = 0;
            pt_user_video[i].global = 0;
            pt_user_video[i].ignored = 0;
            pt_user_video[i].addr_31_12 = VIDEO_INDEX;
        }

        else {
            pt_user_video[i].present = 0;
            pt_user_video[i].rw = 0;
            pt_user_video[i].us = 0;
            pt_user_video[i].pwt = 0;
            pt_user_video[i].pcd = 0;
            pt_user_video[i].accessed = 0;
            pt_user_video[i].dirty = 0;
            pt_user_video[i].pat = 0;
            pt_user_video[i].global = 0;
            pt_user_video[i].ignored = 0;
            pt_user_video[i].addr_31_12 = 0;
        }
    }

    /* Enable paging */
    enable_paging();
}

/* flush_tlb
 *
 * Refresh CR3(PDBR)
 */
void flush_tlb() {
    asm volatile("                 \n\
        movl %cr3, %eax            \n\
        movl %eax, %cr3            \n\
    ");
}

/* enable_paging
 *
 * Set CR3 to the physical address of page directory
 * Set PGE(bit 7), PSE(bit 4) and clear PAE(bit 5) on CR4
 * Set PG(bit 31) and PE(bit 0) on CR0
 */
static void enable_paging() {
    asm volatile("                 \n\
        movl $pd, %eax             \n\
        movl %eax, %cr3            \n\
        movl %cr4, %eax            \n\
        orl $0x00000010, %eax      \n\
        andl $0xFFFFFFDF, %eax     \n\
        movl %eax, %cr4            \n\
        movl %cr0, %eax            \n\
        orl $0x80000001, %eax      \n\
        movl %eax, %cr0            \n\
        movl %cr4, %eax            \n\
        orl $0x00000080, %eax      \n\
        movl %eax, %cr4            \n\
    ");
}

/* set_user_video_mem
 *
 * Set the map to user video memory
 */
void set_user_video_mem(void* user_video_mem){
    pt_user_video[0].addr_31_12 = (uint32_t)user_video_mem >> 12;
}
