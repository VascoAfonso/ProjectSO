#include "operations.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tfs_init() {
    state_init();

    /* create root inode */
    int root = inode_create(T_DIRECTORY);
    if (root != ROOT_DIR_INUM) {
        return -1;
    }

    return 0;
}

int tfs_destroy() {
    state_destroy();
    return 0;
}

static bool valid_pathname(char const *name) {
    return name != NULL && strlen(name) > 1 && name[0] == '/';
}


int tfs_lookup(char const *name) {
    if (!valid_pathname(name)) {
        return -1;
    }

    // skip the initial '/' character
    name++;

    return find_in_dir(ROOT_DIR_INUM, name);
}

int tfs_open(char const *name, int flags) {
    int inum;
    size_t offset;

    /* Checks if the path name is valid */
    if (!valid_pathname(name)) {
        return -1;
    }

    inum = tfs_lookup(name);
    if (inum >= 0) {
        /* The file already exists */
        inode_t *inode = inode_get(inum);
        if (inode == NULL) {
            return -1;
        }

        /* Trucate (if requested) */
        if (flags & TFS_O_TRUNC) {
            if (inode->i_size > 0) {
                for (int i = 0; i < FILEBLOCKS + 1; i++){
                    if (data_block_free(inode->i_data_block[i]) == -1) {
                        return -1;
                    }
                }
                inode->i_size = 0;
            }
        }
        /* Determine initial offset */
        if (flags & TFS_O_APPEND) {
            offset = inode->i_size;
        } else {
            offset = 0;
        }
    } else if (flags & TFS_O_CREAT) {
        /* The file doesn't exist; the flags specify that it should be created*/
        /* Create inode */
        inum = inode_create(T_FILE);
        if (inum == -1) {
            return -1;
        }
        /* Add entry in the root directory */
        if (add_dir_entry(ROOT_DIR_INUM, inum, name + 1) == -1) {
            inode_delete(inum);
            return -1;
        }
        offset = 0;
    } else {
        return -1;
    }

    /* Finally, add entry to the open file table and
     * return the corresponding handle */
    return add_to_open_file_table(inum, offset);

    /* Note: for simplification, if file was created with TFS_O_CREAT and there
     * is an error adding an entry to the open file table, the file is not
     * opened but it remains created */
}


int tfs_close(int fhandle) { return remove_from_open_file_table(fhandle); }


void file_block_alloc(int index, inode_t * inode){

    if (index <= FILEBLOCKS){
        inode->i_data_block[index] = data_block_alloc();
        return;
    }

    if (index == FILEBLOCKS + 1){
        inode->i_data_block[FILEBLOCKS + 1] = data_block_alloc();
    }
    void * block = data_block_get(inode->i_data_block[FILEBLOCKS + 1]);
    int diff = index - FILEBLOCKS - 1;
    int i_block = data_block_alloc();
    memcpy(block + diff * (int)sizeof(int), &i_block, sizeof(int));
    


}


ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {
    /*MAYBE DONE*/
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    /* From the open file table entry, we get the inode */
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }

    /* Determine how many bytes to write */
    /*if (to_write + file->of_offset > BLOCK_SIZE) {
        to_write = BLOCK_SIZE - file->of_offset;
    }*/

    int current_block = (int)(file->of_offset - 1) / BLOCK_SIZE;

    int block_to_alocate = (int)(file->of_offset + to_write - 1) / BLOCK_SIZE;
    int i;

    for (i = current_block + 1; i <= block_to_alocate; i++){

       file_block_alloc(i, inode);
    }


    if (to_write > 0) {
        if (inode->i_size == 0) {
            /* If empty file, allocate new block */
           inode->i_data_block[0] = data_block_alloc();
        }

        void *block = data_block_get(inode->i_data_block[0]);
        if (block == NULL) {
            return -1;
        }

        

        /* Perform the actual write */
        /*memcpy(block + file->of_offset, buffer, to_write);*/
        size_t left_to_write = to_write;
        size_t wrote = 0;
        size_t block_offset;
        i = current_block;
        while (left_to_write > 0) {
            if (i >= FILEBLOCKS + (BLOCK_SIZE / sizeof(int))){
                to_write = wrote;
                break;
            }

            
            if (i < FILEBLOCKS){
                block = data_block_get(inode->i_data_block[i]);
                block_offset = file->of_offset - (size_t)(BLOCK_SIZE * i);
                
            }else{
                void * main_block = data_block_get(inode->i_data_block[FILEBLOCKS]);
                block_offset = file->of_offset - (size_t)(BLOCK_SIZE * i);
                memcpy(block, main_block + ((i - FILEBLOCKS - 1) * (int)sizeof(int)), sizeof(int));
               
            }

            

            if (BLOCK_SIZE - block_offset < left_to_write){
                memcpy(block + block_offset, buffer + wrote, BLOCK_SIZE - block_offset);
                wrote += (size_t)BLOCK_SIZE - block_offset;
                left_to_write -= BLOCK_SIZE - block_offset;
            }else{
                memcpy(block + block_offset, buffer + wrote, left_to_write);
                wrote += left_to_write;
                left_to_write = 0;             
            }

            i++;
        }


        /* The offset associated with the file handle is
         * incremented accordingly */
        file->of_offset += to_write;
        if (file->of_offset > inode->i_size) {
            inode->i_size = file->of_offset;
        }
    }

    return (ssize_t)to_write;
}


ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }

    /* From the open file table entry, we get the inode */
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }



    /* Determine how many bytes to read */
    size_t to_read = inode->i_size - file->of_offset;
    if (to_read > len) {
        to_read = len;
    }

    int current_block = (int)(file->of_offset - 1) / BLOCK_SIZE;
    
    int i;

    if (to_read > 0) {
        void *block = data_block_get(inode->i_data_block[0]);
        if (block == NULL) {
            return -1;
        }

        size_t left_to_read = to_read;
        size_t read = 0;
        size_t block_offset;
        i = current_block;

        while (left_to_read > 0){
            if (i >= FILEBLOCKS + (BLOCK_SIZE / sizeof(int))){
                to_read = read;
                break;
            }

            

            if (i < FILEBLOCKS){
                block = data_block_get(inode->i_data_block[i]);
                block_offset = file->of_offset - (size_t)(BLOCK_SIZE * i);
                
            }else{
                void * main_block = data_block_get(inode->i_data_block[FILEBLOCKS]);
                block_offset = file->of_offset - (size_t)(BLOCK_SIZE * i);
                memcpy(block, main_block + ((i - FILEBLOCKS - 1) * (int)sizeof(int)), sizeof(int));
               
            }
            if (BLOCK_SIZE - block_offset < left_to_read){
                memcpy(buffer, block + block_offset, BLOCK_SIZE - block_offset);
                read += (size_t)BLOCK_SIZE - block_offset;
                left_to_read -= BLOCK_SIZE - block_offset;
            }else{
                memcpy(buffer, block + block_offset, left_to_read);
                read += left_to_read;
                left_to_read = 0;             
            }

            i++;

        }


        /* Perform the actual read */
        /*memcpy(buffer, block + file->of_offset, to_read);*/
        /* The offset associated with the file handle is
         * incremented accordingly */
        file->of_offset += to_read;
    }

    return (ssize_t)to_read;
}
    

int tfs_copy_to_external_fs(char const *source_path, char const *dest_path){
    FILE *f;
    int fd = tfs_open(source_path, 0);
    f = fopen(dest_path, "w");

    ssize_t bytes_read;
    char buffer[128];
    

   /* read the contents of the file */
   while((bytes_read = tfs_read(fd, buffer, sizeof(char)*128)) != 0){

   if (bytes_read == -1) return -1;

    fwrite(buffer, 1, (size_t) bytes_read, f);
    }
    
    fclose(f);
    return 0;
}






