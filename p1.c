#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main()
{
    char *buffer;
    size_t len = 32;
    buffer = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    char *token;
    const char space[2] = " ";
    int i;
    printf("dash> ");
    while (1)
    {
        characters = getline(&buffer, &len, stdin);
        char *args[100];
        char *newline = strchr(buffer, '\n');
        if (newline)
            *newline = 0;
        token = strtok(buffer, space);
        i = 0;
        args[i] = token;
        while (token != NULL)
        {
            i++;
            token = strtok(NULL, space);
            args[i] = token;
        }

        //args[i + 1] = NULL;

        int id = fork();
        if (id == 0)
        {
            execv(buffer, args);
        }
        else
        {
            int id2 = wait(NULL);
            printf("dash> ");
        }
    }
    return 0;
}