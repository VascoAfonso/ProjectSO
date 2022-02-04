#include "tecnicofs_client_api.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int session_id = -1;
char * client_path;
char * server_path;
int tx;
int rx;






void send_request(int tx, Payload * p){


    ssize_t ret = write(tx, p, sizeof(*p));

}



int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */
    char buffer[BUFFER_SIZE];

    client_path = client_pipe_path;
    server_path = server_pipe_path;

    

    if (unlink(client_pipe_path) != 0 && errno != ENOENT) {
        return -1;
    }

    // create pipe
    if (mkfifo(client_pipe_path, 0640) != 0) {
        return -1;
    }
    

    // open pipe for writing
    // this waits for someone to open it for reading
    tx = open(server_pipe_path, O_WRONLY);

    if (tx == -1) {
        return -1;
    }
    

    //send request here

    Payload * p = malloc(sizeof(Payload));
    p->op_code = '1';

    strcpy(p->name, client_pipe_path);
    send_request(tx, p);
    
    Payload p_r;
    
    rx = open(client_pipe_path, O_RDONLY);
    int ret = read(rx, &p_r, sizeof(Payload));

    if (p_r.error == -1) return -1;

    session_id = p_r.id;

    free(p);

    return 0;
}


int tfs_unmount() {

    Payload * p = malloc(sizeof(Payload));

    p->op_code = '2';
    p->id = session_id;
    send_request(tx, p);
    Payload p_r;
    int ret = 0;
    while (ret == 0){
        ret = read(rx, &p_r, sizeof(Payload));   
    }

    if (p_r.error == -1) return -1;

    close(tx);
    close(rx);

    free(p);


    return 0;
}

int tfs_open(char const *name, int flags) {

    Payload * p = malloc(sizeof(Payload));

    p->op_code = '3';
    p->id = session_id;
    p->flags = flags;
    strcpy(p->name, name);

    send_request(tx, p);
    Payload p_r;
    int ret = 0;
    while (ret == 0){
        ret = read(rx, &p_r, sizeof(Payload));   
    }
    int r = p_r.error;
    free(p);

    return r;
}

int tfs_close(int fhandle) {

    Payload * p = malloc(sizeof(Payload));

    p->op_code = '4';
    p->id = session_id;
    p->fhandle = fhandle;

    send_request(tx, p); 
    Payload p_r;
    int ret = 0;
    while (ret == 0){
        ret = read(rx, &p_r, sizeof(Payload));   
    }   
    int r = p_r.error;
    free(p);

    return r;

    
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    
    Payload * p = malloc(sizeof(Payload));

    p->op_code = '5';
    p->id = session_id;
    p->fhandle = fhandle;
    p->len = len;
    strcpy(p->buffer, buffer);

    send_request(tx, p); 
    Payload p_r;
    int ret = 0;
    while (ret == 0){
        ret = read(rx, &p_r, sizeof(Payload));   
    }
    int r = p_r.error;
    free(p);

    return r;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {

    Payload * p = malloc(sizeof(Payload));

    p->op_code = '6';
    p->id = session_id;
    p->fhandle = fhandle;
    p->len = len;

    send_request(tx, p);

    Payload p_r;
    int ret = 0;
    while (ret == 0){
        ret = read(rx, &p_r, sizeof(Payload));   
    }
    int r = p_r.error;
    strcpy(buffer, p_r.buffer);

    free(p);

    return r;
}

int tfs_shutdown_after_all_closed() {

    Payload * p = malloc(sizeof(Payload));

    p->op_code = '7';
    p->id = session_id;

    send_request(tx, p);

    Payload p_r;
    int ret = read(rx, &p_r, sizeof(Payload));
    int r = p_r.error;

    free(p);

    return r;
}

