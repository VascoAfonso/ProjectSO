#include "./fs/operations.h"
#include "./fs/state.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define N_THREADS 10
#define COUNT 20
#define SIZE 500

struct args_struct{
    char path[10];
    size_t size;
    char c;
};

void * func3(void * arguments){
    struct args_struct *args = (struct args_struct *)arguments;
    size_t size = args->size;
    char path[10];
    strcpy(path, args->path);
    char input[size];
    memset(input, args->c, size);
    int fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1);

    assert(tfs_write(fd, input, size) == size);

    assert(tfs_close(fd) != -1);

    return NULL;
}



int main(){


    tfs_init();
    pthread_t threads[N_THREADS];
    struct args_struct args;

    args.c = 'A';
    strcpy(args.path, "/test");
    args.size = 256;

    for (int i = 0; i < N_THREADS; i++){
        if (pthread_create(&threads[i], NULL, func3, &args) != 0) {
            printf("Error creating thread.\n");
            return -1;
        }
    }
    for (int i = 0; i < N_THREADS; i++){
        if(pthread_join(threads[i], NULL) != 0) {
            printf("Error joining thred.\n");
            return -1;
        }
    }





    printf("Successful Test!!!!\n");

    
    return 0;
}  