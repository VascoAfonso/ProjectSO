#include "./fs/operations.h"
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
        printf("%s - %d\n",path, fd);
        assert(tfs_write(fd, input, size) == size);
    }
    assert(tfs_close(fd) != -1);
    fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1 );

    for (int i = 0; i < COUNT; i++) {
        assert(tfs_read(fd, output, size) == size);
        printf("%s - %d\n",path, fd);
        printf("Input  - %s\nOutput - %s\n", input, output);     
        assert(memcmp(input, output, size) == 0);
        /*if (memcmp(input, output, size) != 0) printf("AIUQUA|\n");*/
        
    }
    assert(tfs_close(fd) != -1);
    return NULL;
}



int main(){


    tfs_init();

    struct args_struct args[N_THREADS];
    strcpy(args[0].path,"/f1");
    strcpy(args[1].path,"/f2");
    strcpy(args[2].path, "/f3");
    args[0].size = 256;
    args[1].size = 513;
    args[2].size = 1024;
    args[0].c = 'A';
    args[1].c = 'B';
    args[2].c = 'C';

    func3(&args[0]);
    func3(&args[1]);
    func3(&args[2]);
    
    printf("Successful Test!!!!\n");

    
    return 0;
}  