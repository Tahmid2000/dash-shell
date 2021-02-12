#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

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
const char parallelSymbol[2] = "&";

void removeNewLine(char *buffer)
{
    char *newline = strchr(buffer, '\n');
    if (newline)
        *newline = 0;
}

int parseForParallel(char *buffer, char *args[])
{
    char *tokenForParallel;
    tokenForParallel = strtok(buffer, parallelSymbol);
    int i = 0;
    args[i] = tokenForParallel;
    while (tokenForParallel != NULL)
    {
        i++;
        tokenForParallel = strtok(NULL, parallelSymbol);
        args[i] = tokenForParallel;
    }
    return i;
}

char *parseForRedirect(char *buffer)
{
    char *tokenForRedirect;
    char *redirectFile = (char *)malloc(strlen(buffer) * sizeof(char));
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
    /* if (redirectFile[0] == '\0')
        return NULL; */
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
    size_t len = 200;
    buffer = (char *)malloc(len * sizeof(char));
    char *str = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    char *paths[200];
    paths[0] = "/bin";
    int pathCounter = 1;
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
            char *commands[100];
            int end = parseForParallel(buffer, commands);
            int i;
            for (i = 0; i < end; i++)
            {
                char *args[100];
                char *redirectFile;
                removeNewLine(commands[i]);
                redirectFile = parseForRedirect(commands[i]);
                parseForSpace(commands[i], args);
                if (strcmp(args[0], "exit") == 0)
                {
                    free(buffer);
                    buffer = NULL;
                    /*free(str);
                    str = NULL; */
                    exitShell();
                }
                else if (strcmp(args[0], "cd") == 0)
                {
                    if (args[1] && !args[2])
                        changeDirectory(args[1]);
                    else
                    {
                        char error_message[35] = "An error has occurred w cd args\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
                else if (strcmp(args[0], "path") == 0)
                {
                    int k = 1;
                    pathCounter = 0;
                    while (args[k] != NULL)
                    {
                        paths[pathCounter] = strdup(args[k]);
                        pathCounter++;
                        k++;
                    }
                }
                else
                {
                    int j = 0;
                    while (paths[j] != NULL)
                    {
                        strcpy(str, paths[j]);
                        strcat(str, "/");
                        strcat(str, args[0]);
                        if (access(str, X_OK) == 0)
                        {
                            args[0] = str;
                            break;
                        }
                        j++;
                    }
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
                        if (execv(args[0], args) != 0)
                        {
                            char error_message[35] = "An error has occurred execv error\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            exit(1);
                        }
                        exit(0);
                    }
                }
            }
            while (end > 0)
            {
                wait(NULL);
                end--;
            }
        }
        characters = getline(&buffer, &len, fp);
    }
    fclose(fp);
    free(buffer);
    buffer = NULL;
    free(str);
    str = NULL;
}

void shellInteractive()
{
    char *buffer;
    size_t len = 200;
    buffer = (char *)malloc(len * sizeof(char));
    char *str = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    char *paths[200];
    paths[0] = "/bin";
    int pathCounter = 1;
    printf("dash> ");
    while (1)
    {
        characters = getline(&buffer, &len, stdin);
        char *commands[characters];
        int end = parseForParallel(buffer, commands);
        int i;
        for (i = 0; i < end; i++)
        {
            char *redirectFile;
            removeNewLine(commands[i]);
            redirectFile = parseForRedirect(commands[i]);
            char *args[strlen(commands[i])];
            parseForSpace(commands[i], args);
            if (strcmp(args[0], "exit") == 0)
            {
                free(buffer);
                buffer = NULL;
                free(str);
                str = NULL;
                exitShell();
            }
            else if (strcmp(args[0], "cd") == 0)
            {
                if (args[1] && !args[2])
                    changeDirectory(args[1]);
                else
                {
                    char error_message[35] = "An error has occurred w cd args\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
            }
            else if (strcmp(args[0], "path") == 0)
            {
                int k = 1;
                pathCounter = 0;
                while (args[k] != NULL)
                {
                    paths[pathCounter] = strdup(args[k]);
                    pathCounter++;
                    k++;
                }
            }
            else
            {
                int j = 0;
                while (paths[j] != NULL)
                {
                    strcpy(str, paths[j]);
                    strcat(str, "/");
                    strcat(str, args[0]);
                    if (access(str, X_OK) == 0)
                    {
                        args[0] = str;
                        break;
                    }
                    j++;
                }
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

                    if (execv(args[0], args) != 0)
                    {
                        char error_message[30] = "An error has occurred execv\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(1);
                    }
                    exit(0);
                }
            }
            /* free(buffer);
            buffer = NULL; */
        }
        while (end > 0)
        {
            wait(NULL);
            end--;
        }
        printf("dash> ");
    }
    free(buffer);
    buffer = NULL;
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