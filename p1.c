#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void exitShell()
{
    exit(0);
}

void changeDirectory(char *dir)
{
    if (chdir(dir) != 0)
    {
        char error_message[30] = "An error has occurred w/ cd\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

void shellBatch(char *file)
{
    char *buffer;
    size_t len = 32;
    buffer = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    char *token;
    const char space[2] = " ";
    int i;
    FILE *fp;
    fp = fopen(file, "r");
    if (!fp)
    {
        char error_message[41] = "An error has occurred w opening the file\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    characters = getline(&buffer, &len, fp);
    while (characters >= 0)
    {
        if (buffer[0] != '\n')
        {
            char *args[100];
            char *newline = strchr(buffer, '\n');
            if (newline)
                *newline = 0;
            token = strtok(buffer, space);
            args[i] = token;
            i = 1;
            while (token != NULL)
            {
                token = strtok(NULL, space);
                args[i] = token;
                i++;
            }
            if (strcmp(buffer, "exit") == 0)
            {
                free(buffer);
                buffer = NULL;
                exitShell();
            }
            else if (strcmp(buffer, "cd") == 0)
            {
                if (args[1] && !args[2])
                    changeDirectory(args[1]);
                else
                {
                    char error_message[35] = "An error has occurred w cd args\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
            else
            {
                int id = fork();
                if (id == 0)
                {
                    if (execv(buffer, args) != 0)
                    {
                        char error_message[35] = "An error has occurred execv error\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
                else
                {
                    wait(NULL);
                }
            }
        }
        characters = getline(&buffer, &len, fp);
    }
    fclose(fp);
}

void shellInteractive()
{
    char *buffer;
    size_t len = 32;
    buffer = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    char *token;
    const char space[2] = " ";
    int i;

    while (1)
    {
        printf("dash> ");
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
        if (strcmp(buffer, "exit") == 0)
        {
            free(buffer);
            buffer = NULL;
            exitShell();
        }
        else if (strcmp(buffer, "cd") == 0)
        {
            if (args[1] && !args[2])
                changeDirectory(args[1]);
            else
            {
                char error_message[35] = "An error has occurred w cd args\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
        else
        {
            int id = fork();
            if (id == 0)
            {
                if (execv(buffer, args) != 0)
                {
                    char error_message[30] = "An error has occurred execv\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
            else
            {
                wait(NULL);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        shellBatch(argv[1]);
    }
    else if (argc > 2)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    else
    {
        shellInteractive();
    }
    return 0;
}