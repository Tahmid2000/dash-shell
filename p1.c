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

const char space[2] = " ";
const char redirectSymbol[2] = ">";

void removeNewLine(char *buffer)
{
    char *newline = strchr(buffer, '\n');
    if (newline)
        *newline = 0;
}

char *parseForRedirect(char *buffer)
{
    char *tokenForRedirect;
    char *redirectFile;
    tokenForRedirect = strtok(buffer, redirectSymbol);
    while (tokenForRedirect != NULL)
    {
        tokenForRedirect = strtok(NULL, redirectSymbol);
        if (tokenForRedirect)
        {
            tokenForRedirect = strtok(tokenForRedirect, space);
            redirectFile = tokenForRedirect;
        }
    }
    return redirectFile;
}

void parseForSpace(char *buffer, char *args[])
{
    char *token;
    token = strtok(buffer, space);
    int i = 0;
    args[i] = token;
    while (token != NULL)
    {
        i++;
        token = strtok(NULL, space);
        args[i] = token;
    }
}

void shellBatch(char *file)
{
    char *buffer;
    size_t len = 32;
    buffer = (char *)malloc(len * sizeof(char));
    ssize_t characters;
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
            char *redirectFile;
            removeNewLine(buffer);
            redirectFile = parseForRedirect(buffer);
            parseForSpace(buffer, args);
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
                    if (redirectFile[0] != '\0')
                    {
                        int fd = open(redirectFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                        dup2(fd, 1);
                        dup2(fd, 2);
                        close(fd);
                    }
                    if (execv(buffer, args) != 0)
                    {
                        char error_message[35] = "An error has occurred execv error\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        //return;
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
    free(buffer);
    buffer = NULL;
}

void shellInteractive()
{
    char *buffer;
    size_t len = 32;
    buffer = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    printf("dash> ");
    while (1)
    {
        characters = getline(&buffer, &len, stdin);
        char *args[100];
        char *redirectFile;
        removeNewLine(buffer);
        redirectFile = parseForRedirect(buffer);
        parseForSpace(buffer, args);
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
            printf("dash> ");
        }
        else
        {
            int id = fork();
            if (id == 0)
            {
                if (redirectFile[0] != '\0')
                {
                    int fd = open(redirectFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    dup2(fd, 1);
                    dup2(fd, 2);
                    close(fd);
                }
                if (execv(buffer, args) != 0)
                {
                    char error_message[30] = "An error has occurred execv\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return;
                }
            }
            else
            {
                wait(NULL);
                printf("dash> ");
            }
        }
        /* free(buffer);
        buffer = NULL; */
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