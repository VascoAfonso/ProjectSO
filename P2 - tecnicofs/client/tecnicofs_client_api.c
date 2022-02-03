#include "tecnicofs_client_api.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 40

int session_id;
char * client_path;
char * server_path;
int tx;
int rx;



void send_request(int tx, char op, void const *args, size_t max_size){

    void * buffer;
    size_t written = 0;
    size_t len = max_size + sizeof(char);
    buffer = (void *) malloc(sizeof(char) + BUFFER_SIZE);
    strncpy(buffer, op, sizeof(char));
    strcpy(buffer + sizeof(char), args);
    while (written < len){
        ssize_t ret = write(tx, buffer, len - written);
        written += ret;
    }


}



int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */
    char buffer[BUFFER_SIZE];

    client_path = client_pipe_path;
    server_path = server_pipe_path;


    if (unlink(server_pipe_path) != 0 && errno != ENOENT) {
        return -1;
    }

    // create pipe
    if (mkfifo(server_pipe_path, 0640) != 0) {
        return -1;
    }

    // open pipe for writing
    // this waits for someone to open it for reading
    tx = open(server_pipe_path, O_WRONLY);
    if (tx == -1) {
        return -1;
    }

    int rx = open(client_pipe_path, O_RDONLY);

    //send request here
    strcpy(buffer, client_pipe_path);
    send_request(tx, '1', buffer, BUFFER_SIZE * sizeof(char));





    ssize_t ret = read(rx, buffer, BUFFER_SIZE - 1);
    memcpy(session_id, (int *) buffer, sizeof(int));
    



    return 0;
}

int tfs_unmount() {
    // open pipe for writing
    // this waits for someone to open it for reading
    char buffer[BUFFER_SIZE];
    void * buffer_id = malloc(sizeof(int));
    //send request
    memcpy(buffer_id, session_id, sizeof(int));

    send_request(tx, '2', buffer_id, sizeof(int));

    close(tx);
    close(rx);

    return 0;
}

int tfs_open(char const *name, int flags) {

    char buffer[BUFFER_SIZE];
    void * buffer_id = malloc(sizeof(int)*2 + BUFFER_SIZE * sizeof(char));

    void * offset = buffer_id;
    //send request
    memcpy(offset, session_id, sizeof(int));
    offset = offset + sizeof(int);
    memcpy(offset , name, BUFFER_SIZE);
    offset = offset + sizeof(char) * BUFFER_SIZE;
    memcpy(offset , flags, sizeof(int));
    offset = offset + sizeof(int);

    send_request(tx, '3', buffer_id, offset - buffer_id);

    int ret = read(rx, buffer, client_path);
    

    return 0;
}

int tfs_close(int fhandle) {
    char buffer[BUFFER_SIZE];
    void * buffer_id = malloc(sizeof(int)*2);

    void * offset = buffer_id;
    //send request
    memcpy(offset, session_id, sizeof(int));
    offset = offset + sizeof(int);
    memcpy(offset , fhandle, sizeof(int));
    offset = offset + sizeof(int);

    send_request(tx, '4', buffer_id, offset - buffer_id);

    int ret = read(rx, buffer, client_path);
    

    return 0;

    
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {


    char buffer_read[BUFFER_SIZE];
    void * buffer_id = malloc(sizeof(int)*3 + len * sizeof(char));

    void * offset = buffer_id;
    //send request
    memcpy(offset, session_id, sizeof(int));
    offset = offset + sizeof(int);
    memcpy(offset , fhandle, sizeof(int));
    offset = offset + sizeof(int);
    memcpy(offset , len, sizeof(int));
    offset = offset + sizeof(int);
    memcpy(offset , buffer, len);
    offset = offset + sizeof(char) * len;
    

    send_request(tx, '5', buffer_id, offset - buffer_id);

    int ret = read(rx, buffer, client_path);
    

    return 0;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {


    void * buffer_id = malloc(sizeof(int)*3);

    void * offset = buffer_id;
    //send request
    memcpy(offset, session_id, sizeof(int));
    offset = offset + sizeof(int);
    memcpy(offset , fhandle, sizeof(int));
    offset = offset + sizeof(int);
    memcpy(offset , len, sizeof(int));
    offset = offset + sizeof(int);
    

    send_request(tx, '6', buffer_id, offset - buffer_id);

    int ret = read(rx, buffer, client_path);
    

    return 0;
}

int tfs_shutdown_after_all_closed() {

    char buffer[BUFFER_SIZE];
    void * buffer_id = malloc(sizeof(int));
    //send request
    memcpy(buffer_id, session_id, sizeof(int));

    send_request(tx, '7', buffer_id, sizeof(int));


    return 0;
}
