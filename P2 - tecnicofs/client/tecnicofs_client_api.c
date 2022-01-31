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

void send_code(int tx, char const *str){
    
    ssize_t ret = write(tx, str, BUFFER_SIZE);

}



int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {
    /* TODO: Implement this */
    char buffer[BUFFER_SIZE];

    if (unlink(server_pipe_path) != 0 && errno != ENOENT) {
        return -1;
    }

    // create pipe
    if (mkfifo(server_pipe_path, 0640) != 0) {
        return -1;
    }

    // open pipe for writing
    // this waits for someone to open it for reading
    int tx = open(server_pipe_path, O_WRONLY);
    if (tx == -1) {
        return -1;
    }

    int rx = open(client_pipe_path, O_RDONLY);

    ssize_t ret = read(rx, buffer, BUFFER_SIZE - 1);

    




    return 0;
}

int tfs_unmount() {
    /* TODO: Implement this */
    return -1;
}

int tfs_open(char const *name, int flags) {
    /* TODO: Implement this */
    return -1;
}

int tfs_close(int fhandle) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    /* TODO: Implement this */
    return -1;
}

int tfs_shutdown_after_all_closed() {
    /* TODO: Implement this */
    return -1;
}
