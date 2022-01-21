#include "./fs/operations.h"

#include "./fs/state.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define N_THREADS 3
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

    char input[size]; 
    memset(input, args->c, (size_t)size);
    char path[10];
    strcpy(path, args->path);
    
    char output[size];



    int fd = tfs_open(path, TFS_O_CREAT);

    assert(fd != -1);
    for (int i = 0; i < COUNT; i++) {
        assert(tfs_write(fd, input, size) == size);
    }
    assert(tfs_close(fd) != -1);
    fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1 );

    for (int i = 0; i < COUNT; i++) {
        assert(tfs_read(fd, output, size) == size);    
        assert(memcmp(input, output, size) == 0);
        
    }
    assert(tfs_close(fd) != -1);
    return NULL;
}



int main(){

    pthread_t threads[N_THREADS];

    tfs_init();

    struct args_struct args[N_THREADS];
    strcpy(args[0].path,"/f1");
    strcpy(args[1].path,"/f2");
    strcpy(args[2].path, "/f3");
    args[0].size = 128;
    args[1].size = 256;
    args[2].size = 512;
    args[0].c = 'A';
    args[1].c = 'B';
    args[2].c = 'C';


    for (int i = 0; i < N_THREADS; i++){
        if (pthread_create(&threads[i], NULL, func3, &args[i]) != 0) {
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