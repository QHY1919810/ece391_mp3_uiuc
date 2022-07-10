#ifndef _FILESYS_STRUCT_H
#define _FILESYS_STRUCT_H

#include "types.h"

typedef struct file_op_table_t {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} file_op_table_t;

typedef struct file_desc_t {
    file_op_table_t* file_op_table;
    uint32_t inode_idx;
    uint32_t file_position;
    uint32_t flags;
} file_desc_t;

#endif
