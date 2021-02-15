#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

//built in exit command
void exitShell()
{
    exit(0);
}

//built in cd command
void changeDirectory(char *dir)
{
    if (chdir(dir) != 0)
    {
        //show error if cd fails
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

//built in path command
int setPath(char *paths[], char *args[], int pathCounter)
{
    //if path is typed in without any arguments
    if (args[1] == NULL)
    {
        //remove all paths from paths array
        while (pathCounter != -1)
        {
            paths[pathCounter] = NULL;
            pathCounter--;
        }
    }
    else
    {
        //set paths in paths array from args and update pathCounter
        int k = 1;
        pathCounter = 0;
        while (args[k] != NULL)
        {
            //use strdup to copy from args to paths
            paths[pathCounter] = strdup(args[k]);
            pathCounter++;
            k++;
        }
    }
    if (pathCounter == -1)
        return 0;
    return pathCounter;
}

//characters that will be parsed with strtok()
const char space[2] = " ";
const char redirectSymbol[2] = ">";
const char parallelSymbol[2] = "&";

//removes all newlines and tabs from buffer
void removeNewLine(char *buffer)
{
    //check to see if newline exists with strchr, if so remove it
    char *newline = strchr(buffer, '\n');
    if (newline)
        *newline = 0;
    int i;
    //check for all tabs and replace with a white space
    for (i = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] == '\t')
            buffer[i] = ' ';
    }
}

//get all parallel commands
int parseForParallel(char *buffer, char *args[])
{
    //parse with strtok with the '&' symbol (parallelSymbol)
    char *tokenForParallel;
    tokenForParallel = strtok(buffer, parallelSymbol);
    int i = 0;
    args[i] = tokenForParallel; //set the first command into the args array
    //loop for all parallel commands and set it in args array
    while (tokenForParallel != NULL)
    {
        i++;
        tokenForParallel = strtok(NULL, parallelSymbol);
        args[i] = tokenForParallel;
    }
    //return how many commands were found
    return i;
}

//returns a redirect file, if one is given in the command
char *parseForRedirect(char *buffer)
{
    //checks if the redirect symbol is in the command
    char *redirect = strchr(buffer, '>');
    int containtsRedirect = 0;
    if (redirect)
        containtsRedirect = 1;
    //if a redirect symbol is found
    if (containtsRedirect == 1)
    {
        //checks that only one redirect symbol is in the command
        int occs = 0;
        int i;
        for (i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] == '>')
                occs++;
        }
        //parse buffer for the redirect file with strtok() and '>' (redirectSymbol)
        char *tokenForRedirect;
        char *redirectFile = (char *)malloc(strlen(buffer) * sizeof(char));
        tokenForRedirect = strtok(buffer, redirectSymbol);
        int k = 0; //checks how many files in the command
        //loop through command to find the redirect file, if there is one or multiple
        while (tokenForRedirect != NULL)
        {
            tokenForRedirect = strtok(NULL, redirectSymbol);
            if (tokenForRedirect)
            {
                //remove whitespace from token and set it to the redirect file
                tokenForRedirect = strtok(tokenForRedirect, space);
                redirectFile = tokenForRedirect;
            }
            k++;
        }
        //if no redirect file was found after '>' symbol, if there is only whitespace after the '>' symbol, multiple '>'
        //symbols were found, or multiple files were inputted, show an error and return NULL
        if (redirectFile == NULL || redirectFile[0] == '\0' || k > 2 || occs > 1)
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return NULL;
        }
        //passes all tests
        return redirectFile;
    }
    //if there is no '>' symbol
    return "";
}

//finds all arguments of a command and remove extra whitespace
void parseForSpace(char *buffer, char *args[])
{
    //parse with strtok with the ' ' symbol (spaceSymbol)
    char *token;
    token = strtok(buffer, space);
    int i = 0;
    //check if only whitespace was inputted in the command
    if (token == NULL)
        args[i] = "";
    else
    {
        //parse the buffer for each argument and put it in args array
        args[i] = token;
        while (token != NULL)
        {
            i++;
            token = strtok(NULL, space);
            args[i] = token;
        }
    }
}

//batch version of the shell, takes a file
void shellBatch(char *file)
{
    //allocate memory for buffer (user input) and str (for setting a path when execv is called)
    char *buffer;
    size_t len = 200;
    buffer = (char *)malloc(len * sizeof(char));
    char *str = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    //paths is set with "/bin" initially
    char *paths[200];
    paths[0] = "/bin";
    int pathCounter = 1; //how many paths currently
    FILE *fp;
    fp = fopen(file, "r");
    //check for errors with batch file, exit if so
    if (!fp)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    //get first line of file
    characters = getline(&buffer, &len, fp);
    while (characters >= 0)
    {
        if (buffer[0] != '\n') //if the line isn't an empty line
        {
            char *commands[characters]; //make size of commands array = size of buffer so enough memory is available
            removeNewLine(buffer);
            int end = parseForParallel(buffer, commands); //how many commands to be executed
            //show an error if the buffer is only '&'
            if (end == 0)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            int i;
            //loop to run each parallel command without waiting for any of them to finish
            for (i = 0; i < end; i++)
            {
                char *args[strlen(commands[i])]; //make size of args array = length of command so enough memory is available
                char *redirectFile;
                redirectFile = parseForRedirect(commands[i]);
                parseForSpace(commands[i], args);
                //if the buffer is only an empty line
                if (args[0][0] == '\0')
                    continue;
                //if buffer is exit command
                if (strcmp(args[0], "exit") == 0)
                {
                    //if an argument is give with exit, show error
                    if (args[1])
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                    else
                    {
                        //free buffer and str from memory when exit is called
                        free(buffer);
                        buffer = NULL;
                        free(str);
                        str = NULL;
                        exitShell();
                    }
                }
                //if buffer is cd command
                else if (strcmp(args[0], "cd") == 0)
                {
                    //check to make sure only one argument is provided or show an error
                    if (args[1] && !args[2])
                        changeDirectory(args[1]);
                    else
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
                //if buffer is path command
                else if (strcmp(args[0], "path") == 0)
                {
                    pathCounter = setPath(paths, args, pathCounter);
                }
                //if buffer is a system command
                else
                {
                    //check through all paths if the command in buffer is part of that path
                    int j = 0;
                    while (paths[j] != NULL)
                    {
                        //copy the path and the command with a '/' in between to str
                        strcpy(str, paths[j]);
                        strcat(str, "/");
                        strcat(str, args[0]);
                        //check if string is a valid command
                        if (access(str, X_OK) == 0)
                        {
                            //change the first argument to contain the path and break
                            args[0] = str;
                            break;
                        }
                        j++;
                    }
                    //create child process to execute command
                    int id = fork();
                    //if fork fails
                    if (id < 0)
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                    else if (id == 0)
                    {
                        //check if redirect file exists
                        if (redirectFile[0] != '\0')
                        {
                            //open/create/truncate the redirect file wih read/write
                            int fd = open(redirectFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                            dup2(fd, 1); //change output to the file
                            dup2(fd, 2); //change stderror to the file
                            close(fd);
                        }
                        //execute the command, if it fails, show an error and exit(1)
                        if (execv(args[0], args) != 0)
                        {
                            char error_message[30] = "An error has occurred\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            exit(1);
                        }
                    }
                }
            }
            //wait for all child processes/parallel commands to finish executing
            while (end > 0)
            {
                wait(NULL);
                end--;
            }
        }
        //get next line of file
        characters = getline(&buffer, &len, fp);
    }
    //close file, free buffer from memory, and free str from memory when EOF is reached
    fclose(fp);
    free(buffer);
    buffer = NULL;
    free(str);
    str = NULL;
}

//interactive version of the shell
void shellInteractive()
{
    //allocate memory for buffer (user input) and str (for setting a path when execv is called)
    char *buffer;
    size_t len = 200;
    buffer = (char *)malloc(len * sizeof(char));
    char *str = (char *)malloc(len * sizeof(char));
    ssize_t characters;
    //paths is set with "/bin" initially
    char *paths[200];
    paths[0] = "/bin";
    int pathCounter = 1; //how many paths currently
    printf("dash> ");
    while (1)
    {
        characters = getline(&buffer, &len, stdin); //get user input
        if (buffer[0] != '\n')                      //if input isn't an empty line
        {
            char *commands[characters]; //make size of commands array = size of buffer so enough memory is available
            removeNewLine(buffer);
            int end = parseForParallel(buffer, commands); //how many commands to be executed
            int i;
            //show an error if the buffer is only '&'
            if (end == 0)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
            //loop to run each parallel command without waiting for any of them to finish
            for (i = 0; i < end; i++)
            {
                char *redirectFile;
                redirectFile = parseForRedirect(commands[i]);
                char *args[strlen(commands[i])]; //make size of args array = length of command so enough memory is available
                parseForSpace(commands[i], args);
                //if buffer is empty
                if (args[0][0] == '\0')
                    continue;
                //if buffer is exit command
                if (strcmp(args[0], "exit") == 0)
                {
                    //if an argument is give with exit, show error
                    if (args[1])
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                    else
                    {
                        //free buffer and str from memory when exit is called
                        free(buffer);
                        buffer = NULL;
                        free(str);
                        str = NULL;
                        exitShell();
                    }
                }
                //if buffer is cd command
                else if (strcmp(args[0], "cd") == 0)
                {
                    //check to make sure only one argument is provided or show an error
                    if (args[1] && !args[2])
                        changeDirectory(args[1]);
                    else
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
                //if buffer is path command
                else if (strcmp(args[0], "path") == 0)
                {
                    pathCounter = setPath(paths, args, pathCounter);
                }
                //if buffer is a system command
                else
                {
                    //check through all paths if the command in buffer is part of that path
                    int j = 0;
                    while (paths[j] != NULL)
                    {
                        //copy the path and the command with a '/' in between to str
                        strcpy(str, paths[j]);
                        strcat(str, "/");
                        strcat(str, args[0]);
                        //check if string is a valid command
                        if (access(str, X_OK) == 0)
                        {
                            //change the first argument to contain the path and break
                            args[0] = str;
                            break;
                        }
                        j++;
                    }
                    //create child process to execute command
                    int id = fork();
                    //if fork fails
                    if (id < 0)
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                    else if (id == 0)
                    {
                        //check if redirect file exists
                        if (redirectFile[0] != '\0')
                        {
                            //open/create/truncate the redirect file wih read/write
                            int fd = open(redirectFile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                            dup2(fd, 1); //change output to the file
                            dup2(fd, 2); //change stderror to the file
                            close(fd);
                        }
                        //execute the command, if it fails, show an error and exit(1)
                        if (execv(args[0], args) != 0)
                        {
                            char error_message[30] = "An error has occurred\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            exit(1);
                        }
                    }
                }
            }
            //wait for all child processes/parallel commands to finish executing
            while (end > 0)
            {
                wait(NULL);
                end--;
            }
        }
        printf("dash> ");
    }
}

int main(int argc, char *argv[])
{
    //if called 2 arguments, call batch version of shell with the file
    if (argc == 2)
    {
        shellBatch(argv[1]);
    }
    //show an error if called with more than 2 arguments
    else if (argc > 2)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    //call interactive version of shell if only one argument
    else
    {
        shellInteractive();
    }
    return 0;
}