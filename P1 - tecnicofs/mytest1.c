#include "./fs/operations.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define N_THREADS 15
#define COUNT 20
#define SIZE 55

void * func1(){
    char input[SIZE]; 
    memset(input, 'O', SIZE);
    char *path = "/test";
    int fd = tfs_open(path, TFS_O_APPEND);
    assert(fd != -1);
    int wrote = (int) tfs_write(fd, input, SIZE);
    assert(wrote == SIZE);
    printf("Wrote -> %d\n", wrote);
    tfs_close(fd);
    return NULL;

}

void * func2(){
    char input[SIZE]; 
    memset(input, 'O', SIZE);
    char output [SIZE];
    char *path = "/test";
    int fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1);

    int read = (int)tfs_read(fd, output, SIZE);
    printf("Read  -> %d\n", read);
    tfs_close(fd);   
    return NULL;
}
void * func3(){
    char input[SIZE]; 
    memset(input, 'A', SIZE);
    char *path = "/test2";
    char output [SIZE];
    int fd = tfs_open(path, TFS_O_CREAT);

    assert(fd != -1);
    for (int i = 0; i < COUNT; i++) {
        assert(tfs_write(fd, input, SIZE) == SIZE);
    }
    assert(tfs_close(fd) != -1);
    fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1 );


    for (int i = 0; i < COUNT; i++) {
        assert(tfs_read(fd, output, SIZE) == SIZE);      
        assert (memcmp(input, output, SIZE) == 0);
        
    }
    assert(tfs_close(fd) != -1);
    return NULL;
}


int main(){
    pthread_t threads[N_THREADS];

    assert(tfs_init() != -1);
    char *path = "/test";
    int fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1);
    tfs_close(fd);
    
    for (int i = 0; i < N_THREADS -1; i++ ){

        if (i % 2 == 0){
            if (pthread_create(&threads[i], NULL, func1, NULL) != 0) {
                printf("Error creating thread.\n");
                return -1;
            }
        }
        else{
            if (pthread_create(&threads[i], NULL, func2, NULL) != 0) {
                printf("Error creating thread.\n");
                return -1;
            }
        }
    }
    
    if (pthread_create(&threads[N_THREADS -1], NULL, func3, NULL) != 0) {
        printf("Error creating thread.\n");
        return -1;
    }
    
    

    for (int i = 0; i < N_THREADS; i++){
        if(pthread_join(threads[i], NULL) != 0) {
            printf("Error joining thred.\n");
            return -1;
        }
    }

    char input[SIZE]; 
    memset(input, 'O', SIZE);
    char output [SIZE];
    fd = tfs_open(path, TFS_O_CREAT);
    assert(fd != -1);

    for (int i = 0; i < N_THREADS / 2; i++) {
        int read = (int)tfs_read(fd, output, SIZE); 
        printf("Final read size -> %d\n", read);
        printf("Output -> %s\n", output);
    }

    printf("Successful test\n");

    return 0;
}