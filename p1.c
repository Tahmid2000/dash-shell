#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
int main()
{
    char *args[3];
    char line[10];
    //size_t length = 0;
    //ssize_t read;
    args[1] = NULL;
    args[0] = line;
    printf("dash> ");
    while (1)
    {
        scanf("%s", line);
        int id = fork();
        if (id == 0)
        {
            execv(args[0], args);
        }
        else
        {
            int id2 = wait(NULL);
            printf("dash> ");
        }
    }
    return 0;
}