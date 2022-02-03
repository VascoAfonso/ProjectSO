#include "operations.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


int get_operation(char const * buffer)
{
    char op;
    strcpy(op, buffer, sizeof(char));

    return op - '0';
}


int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);


    //Open pipe to read
    
    int rx = open(pipename, O_RDONLY);




    


    return 0;
}