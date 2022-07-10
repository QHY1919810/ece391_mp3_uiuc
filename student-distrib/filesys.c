#include "filesys.h"

#include "task.h"

boot_block_t boot_block;
inode_t* inode_start;
data_block_t* data_block_start;
file_op_table_t file_op_table = {.open = file_open, .close = file_close, .read = file_read, .write = file_write};
file_op_table_t dir_op_table = {.open = dir_open, .close = dir_close, .read = dir_read, .write = dir_write};


/* 
 * read_dentry_by_name
 *  DESCRIPTION:
 *      Given a filename, the function searches the file in the file system. 
 *      If found, the input dentry struct is filled with correct dentry information.
 *  INPUTS:
 *      fname - filename
 *      dentry - dentry_t struct ptr to be filled
 *  RETURN VALUES：
 *      -1 - cannot find the dentry with specified filename.
 *       0 - file dentry is found
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    int i;

    if (fname == NULL || dentry == NULL)
        return -1;

    if (strlen((int8_t*) fname) > FILE_NAME_LENGTH)
        return -1;

    for (i = 0; i < boot_block.dentry_num; i++) {
        if (!strncmp((int8_t*) fname, (int8_t*) boot_block.dentry_table[i].file_name, FILE_NAME_LENGTH)) {
            *dentry = boot_block.dentry_table[i];
            return 0;
        }
    }

    return -1;
}

/* 
 * read_dentry_by_name
 *  DESCRIPTION:
 *      Given a index, the function extracts the dentry from file system. 
 *      If found, the input dentry struct is filled with correct dentry information.
 *  INPUTS:
 *      index - index of the desired dentry
 *      dentry - dentry_t struct ptr to be filled
 *  RETURN VALUES：
 *      -1 - cannot find the dentry.
 *       0 - dentry is found
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    if (dentry == NULL)
        return -1;

    if (index >= boot_block.dentry_num)
        return -1;

    *dentry = boot_block.dentry_table[index];

    return 0;
}


/*
 * read_data
 *  DESCRIPTION:
 *      Given inode index, buffer, offset and length (both in bytes), the function 
 *      fetches specified data from file system and the data is replicated in the buffer.    
 *  INPUTS:
 *      - inode  : index of the inode
 *      - offset : starting position (in bytes), must be smaller than the size of file
 *      - buf    : where result will be stored
 *      - length : length of the data to be read (in bytes)
 *  RETURN VALUES:
 *      - -1     : failed reading (invalid inode or invalid block number)
 *      - non-zero value : number of bytes read 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    uint32_t i;                 /* index variable to traverse the data block table inside an inode */
    uint32_t pre_block_idx;     /* index of the first block to be read */
    uint32_t post_block_idx;    /* index of the last block to be read */ 
    uint32_t pre_offset_bytes;  /* offset in bytes relative to the first block */
    uint32_t post_offset_bytes; /* number of bytes to be read from the last block */
    inode_t  file_inode;
    int bytes_copied = 0;

    /* check if buffer is valid */
    if (buf == NULL) { return -1; }

    /* check if inode is valid */
    if (inode >= boot_block.inode_num) { return -1; }

    /* fetch the file inode from file system */
    (void)memcpy(&file_inode, inode_start + inode, sizeof(inode_t));

    /* check the offset and length */
    if (offset >= file_inode.file_length) { return 0; }
    if (length == 0) { return 0; }

    pre_offset_bytes = offset % sizeof(data_block_t);
    pre_block_idx = offset / sizeof(data_block_t); 
    if (offset + length >= file_inode.file_length) {
        /* if the tail of data to be read exceeded the end of the file */ 
        post_offset_bytes = (file_inode.file_length - 1) % sizeof(data_block_t) + 1;
        post_block_idx = (file_inode.file_length - 1) / sizeof(data_block_t);
    }
    else {
        post_offset_bytes = (offset + length - 1) % sizeof(data_block_t) + 1;
        post_block_idx = (offset + length - 1) / sizeof(data_block_t); 
    }    

    for (i = pre_block_idx; i <= post_block_idx; i++) {
        /* check if the block specified exceeded the allowed value */
        if (file_inode.dblock_table[i] > boot_block.dblock_num) {
            return -1;
        }
        
        if (i == pre_block_idx && i == post_block_idx) {
            (void)memcpy(buf + bytes_copied, (uint8_t*)(data_block_start + file_inode.dblock_table[i]) + pre_offset_bytes, post_offset_bytes - pre_offset_bytes);
            bytes_copied += post_offset_bytes - pre_offset_bytes;
        }
        else if (i == pre_block_idx) {
            (void)memcpy(buf + bytes_copied, (uint8_t*)(data_block_start + file_inode.dblock_table[i]) + pre_offset_bytes, sizeof(data_block_t) - pre_offset_bytes);
            bytes_copied += sizeof(data_block_t) - pre_offset_bytes;
        }
        else if (i == post_block_idx){
            (void)memcpy(buf + bytes_copied, (uint8_t*)(data_block_start + file_inode.dblock_table[i]), post_offset_bytes);
            bytes_copied += post_offset_bytes;
        }
        else {
            (void)memcpy(buf + bytes_copied, (uint8_t*)(data_block_start + file_inode.dblock_table[i]), sizeof(data_block_t));
            bytes_copied += sizeof(data_block_t);
        }
    }

    // /* end of file has been reached */
    // if (offset + length >= file_inode.file_length) {return 0;}

    /* end of file has not been reached, return number of bytes */
    return bytes_copied;
}
/*
 * filesys_init
 *  DESCRIPTION:
 *      Initiate the file system. Setup useful variables.
 *  INPUT:  
 *      filesys_start - the address where file system is loaded into memory
 */
void filesys_init(void* filesys_start) {
    (void)memcpy(&boot_block, filesys_start, sizeof(boot_block_t));
    inode_start = filesys_start;
    inode_start++;
    data_block_start = filesys_start;
    data_block_start += 1 + boot_block.inode_num;

    printf("file system loaded at %x \n", (uint32_t) filesys_start);
}

/* 
 * file_open
 *  DESCRIPTION：
 *      Given a filename, the function sets file_position, flags and inode_idx
 *  INPUT: filename
 *  RETURN VALUE:
 *      -1 - cannot open file
 *       0 - opened file successfully
 *  SIDE EFFECTS:
 *      file_descriptor (see filesys.c) is modified.
 */
int32_t file_open(const uint8_t* filename) {
    return 0;
}

/* 
 * file_close
 *  DESCRIPTION: 
 *      Close the file specified by fd
 *  INPUT： 
 *      fd - file descriptor 
 *  RETURN VALUE: 
 *      0 - closed directory successfully
 *      -1 - unsuccessful close (file already closed)
 *  SIDE EFFECTS:
 *      file_descriptor.flags is changed to FILE_FREE    
 */
int32_t file_close(int32_t fd) {
    return 0;
}

/*
 * file_read
 *  DESCRIPTION:
 *      This function performs file read operation. 
 *  INPUTS:
 *      - fd     ： file descriptor
 *      - buf    :  buffer to copy to
 *      - nbytes :  desired number of bytes to be read 
 *  RETURN VALUE: 
 *      - number of bytes read / copied to buffer
 *  SIDE EFFECT:
 *      The file_position associated with the file struct is incremented
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    int32_t length;
    pcb_t* curr_pcb;
    file_desc_t* fd_entry;

    if (buf == NULL || nbytes < 0)
        return -1;

    curr_pcb = get_current_pcb();
    fd_entry = &curr_pcb->file_desc_array[fd];

    length = read_data(fd_entry->inode_idx, fd_entry->file_position, buf, nbytes);

    if (length > 0)
        fd_entry->file_position += length;

    return length;
}

/*
 * file_write
 *  DESCRIPTION: Performs write operation (not really) on a readon filesys. :p
 *  RETURN VALUES: -1 because write is not supported. 
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* 
 * dir_open
 *  DESCRIPTION:
 *      Given a directory name, the function opens the directory.
 *  INPUTS: directory name
 *  RETURN VALUES：
 *      -1 - cannot open directory (invalid filename, cannot find directory, file type incorrect)
 *       0 - opened directory successfully
 */
int32_t dir_open(const uint8_t* filename) {
    return 0;
}

/* 
 * dir_close
 *  DESCRIPTION: 
 *      Close the directory specified by fd
 *  INPUT： 
 *      fd - file descriptor 
 *  RETURN VALUE: 
 *      0 - closed directory successfully
 *     -1 - closed unsuccessfully
 *  SIDE EFFECTS:
 *      file_descriptor.flags is changed to FILE_FREE    
 */
int32_t dir_close(int32_t fd) {
    return 0;
}

/*
 * dir_read
 *  DESCRIPTION:
 *      This function reads the directory. Each read returns a valid file name.
 *  INPUTS:
 *      - fd     ： file descriptor
 *      - buf    :  buffer to copy to
 *      - nbytes :  desired number of bytes to be read 
 *  RETURN VALUE: 
 *      - number of bytes read / copied to buffer (include the terminal null '\0')
 *  SIDE EFFECT:
 *      The file_position associated with the file struct is incremented
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    int8_t temp_buf[FILE_NAME_LENGTH + 1];  /* intermediate buffer to handle name with 32 characters */
    uint32_t actual_nbytes;     /* actual bytes to be copied */
    dentry_t dentry;
    pcb_t* curr_pcb;
    file_desc_t* fd_entry;

    if (buf == NULL || nbytes < 0)
        return -1;

    curr_pcb = get_current_pcb();
    fd_entry = &curr_pcb->file_desc_array[fd];

    /* check if reached end of all entries */
    if (fd_entry->file_position >= boot_block.dentry_num) {
        return 0;
    }

    /* read directory entry */
    read_dentry_by_index(fd_entry->file_position, &dentry);

    /* handle name with 32 characters */
    temp_buf[FILE_NAME_LENGTH] = '\0';
    (void)strncpy(temp_buf, (int8_t*) dentry.file_name, FILE_NAME_LENGTH);

    actual_nbytes = nbytes > strlen(temp_buf) + 1 ? strlen(temp_buf) + 1 : nbytes;
    (void)strncpy(buf, temp_buf, actual_nbytes);
    

    fd_entry->file_position++;

    return actual_nbytes;
}

/*
 * dir_write
 *  DESCRIPTION: Performs write operation (not really) on a readon filesys. :p
 *  RETURN VALUES: -1 because write is not supported. 
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}
