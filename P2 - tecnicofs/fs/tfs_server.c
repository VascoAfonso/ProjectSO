#include "operations.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sessions[S];

void send_request(int tx, Payload * p){

    ssize_t ret = write(tx, p, sizeof(*p));
}

int get_operation(Payload p)
{
    return p.op_code - '0';
}

int get_free_pos(){

    for (int i = 0; i < S; i++){
        if (sessions[i] == -1) return i;   
    }
    return -1;
}



void server_mount(Payload p){

    int tx = open(p.name, O_WRONLY);
    int id;
    if (id = get_free_pos() == -1){
        //erro
        p.error = -1;
    }else{
        sessions[id] = tx;
        p.id = id;
        p.error = 0;
    }
    send_request(tx, &p);

}

void server_unmount(Payload p){
    
    if (sessions[p.id] == -1){
        p.error = -1;
    }else{
        
        p.error = 0;
    }
    send_request(sessions[p.id], &p);
    close(sessions[p.id]);
    sessions[p.id] = -1;
}

void server_open(Payload p){


    if (sessions[p.id] == -1){
        p.error = -1;
    }

    int ret = tfs_open(p.name, p.flags);

    p.error = ret;

    send_request(sessions[p.id], &p);

}

void server_close(Payload p){

    if (sessions[p.id] == -1){
        p.error = -1;
    }
    else{
        int ret = tfs_close(p.fhandle);
        p.error = ret;
    }
    send_request(sessions[p.id], &p);
}

void server_write(Payload p){
    if (sessions[p.id] == -1){
        p.error = -1;
    }else{

        size_t written = tfs_write(p.fhandle, p.buffer, p.len);
        
        p.error = written;
    }
    send_request(sessions[p.id], &p);

}

void server_read(Payload p){
    if (sessions[p.id] == -1){
        p.error = -1;
    }else{
        size_t read = tfs_read(p.fhandle, p.buffer, p.len);
        p.error = read;
    }
    send_request(sessions[p.id], &p);
}



void server_shutdown(Payload p){
    if (sessions[p.id] == -1) p.error = -1;
    else{
        int ret = tfs_destroy_after_all_closed();
        p.error = ret;
    }
    send_request(sessions[p.id], &p);
}


int main(int argc, char **argv) {

    tfs_init();
    
    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }
    for (int i = 0; i < S; i++){
        sessions[i] = -1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);

    if (unlink(pipename) != 0 && errno != ENOENT) {
        return -1;
    }

    // create pipe
    if (mkfifo(pipename, 0640) != 0) {
        return -1;
    }
    
    //Open pipe to read

    int rx = open(pipename, O_RDONLY);
    int ret = 0;
    Payload p;
    Payload p_r;


    while (1){
        ret = read(rx, &p, sizeof(Payload));
        //printf("%s\n", p.name);

        int op = get_operation(p);
    
        if (TFS_OP_CODE_MOUNT == op) server_mount(p);
        if (TFS_OP_CODE_UNMOUNT == op) server_unmount(p);
        if (TFS_OP_CODE_OPEN == op) server_open(p);
        if (TFS_OP_CODE_CLOSE == op) server_close(p);
        if (TFS_OP_CODE_WRITE == op) server_write(p);
        if (TFS_OP_CODE_READ == op) server_read(p);
        if (TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED == op) server_shutdown(p);
        
        //p = p_r;
    
    
    }


    


    return 0;
}