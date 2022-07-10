#ifndef FILESYS_H
#define FILESYS_H

#include "types.h"
#include "lib.h"
#include "filesys_struct.h"

#define FILE_NAME_LENGTH    32
#define BLOCK_SIZE          4096
#define DENTRY_TABLE_SIZE   (64 - 1)
#define DBLOCK_TABLE_SIZE   (1024 - 1)  

#define FD_FLAG_PRESENT     0x00000001

#define RTC_FILE_TYPE       0
#define DIR_FILE_TYPE       1
#define FILE_FILE_TYPE      2


typedef struct dentry_t {
    uint8_t file_name[FILE_NAME_LENGTH];
    uint32_t file_type;
    uint32_t inode_idx;     // index the inode
    uint8_t reserved[24];
} __attribute__ ((packed)) dentry_t;

typedef struct boot_block_t {
    uint32_t dentry_num;
    uint32_t inode_num;
    uint32_t dblock_num;
    uint8_t reserved[52];
    dentry_t dentry_table[DENTRY_TABLE_SIZE];
} __attribute__ ((packed)) boot_block_t;

typedef struct inode_t {
    uint32_t file_length;
    uint32_t dblock_table[DBLOCK_TABLE_SIZE];
} inode_t;

typedef struct data_block_t {
    uint8_t data[BLOCK_SIZE];
} data_block_t;

void filesys_init(void* filesys_start);

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


int32_t file_open(const uint8_t* filename);

int32_t file_close(int32_t fd);

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t dir_open(const uint8_t* filename);

int32_t dir_close(int32_t fd);

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

extern file_op_table_t file_op_table;
extern file_op_table_t dir_op_table;

#endif /* FILESYS_H */
