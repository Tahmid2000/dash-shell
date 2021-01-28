#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main()
{

    char b[32];
    char *buffer = b;
    size_t len = 32;
    ssize_t characters;
    char *token;
    const char space[2] = " ";
    int i;
    printf("dash> ");
    while (1)
    {
        characters = getline(&buffer, &len, stdin);
        char *args[100];
        token = strtok(buffer, space);
        i = 0;
        while (token != NULL)
        {
            args[i] = token;
            /* printf("%s\n", args[i]); */
            token = strtok(NULL, space);
            //i++;
        }

        args[i + 1] = NULL;

        int id = fork();
        if (id == 0)
        {
            /* printf("%d", i);
            for (int j = 0; j < 5; j++)
            {
                printf("%s", args[j]);
            } */
            /* execv(buffer, args); */
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