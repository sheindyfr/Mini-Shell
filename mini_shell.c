//sheindy frenkel 207191131

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define IN 0
#define OUT 1
#define FALSE 0
#define TRUE 1
#define MAX_STRING 510

char **wordsArray(char *str);

void cutTheString(char *str, char *str1, char *str2, char c);

void forkOneProgram(char **arr);

void forkWithFile(char **arr1, int fd, int flag4);

void forkWithPipe(char **arr1, char **arr2);

int checkSpecialCommand(char *str, char *check);

int execPipe(char *str, char *str1, char* str2);

int execRedirection(char *str, char *str1, char* str2, int flag2, int flag3, int flag4);

int checkSpecialCommand(char *str, char *check);

void freeArray(char** arr);

int main()
{
    int flag1;
    int flag2;
    int flag3;
    int flag4;
    char str[MAX_STRING] = "", str1[MAX_STRING] = "", str2[MAX_STRING] = "";
    char **arr;
    char name1[30];
    gethostname(name1, 30);
/*--------------------------------------------------------------------------*/
    while (TRUE)
    {
        printf("%s@%s$ ", getlogin(), name1); //print the command line
        fgets(str, MAX_STRING, stdin);
        if (str == NULL)
            continue;
        str[strlen(str) - 1] = '\0';  //delete the '\n' from the string

        if ((strcmp(str, "done")) == 0)
            break;

        //-------------------------------------------------------------
        flag1= checkSpecialCommand(str, " | ");
        flag2= checkSpecialCommand(str, " > ");
        flag3= checkSpecialCommand(str, " >> ");
        flag4= checkSpecialCommand(str, " < ");
        //-------------------------------------------------------------

        //if regular command
        if (flag1 == FALSE && flag2 == FALSE && flag3 == FALSE && flag4 == FALSE)
        {
            arr = wordsArray(str);
            forkOneProgram(arr);
            freeArray(arr);
            continue;
        }
        //---------------------------------------------------------------
        //if pipe command
        if(flag1==TRUE)
        {
            if(execPipe(str, str1, str2) < 0)
                continue;
        }
        //--------------------------------------------------------------
        //if redirection command
        else
        {
            if(execRedirection(str, str1, str2, flag2, flag3, flag4) < 0)
                continue;
        }
    }
    //------------------------------------------------
    return 0;
}

//functions:
/*--------------------------------------------------------------------------*/
int execPipe(char *str, char *str1, char* str2)
{
    char **arr1, **arr2;
    cutTheString(str, str1, str2, '|');
    arr1 = wordsArray(str1);
    arr2 = wordsArray(str2);

    if(arr1[0]==NULL || arr2[0]==NULL)
    {
        fprintf(stderr, "pipe location is wrong\n");
        return -1;
    }

    forkWithPipe(arr1, arr2);
    freeArray(arr1);
    freeArray(arr2);
    return 1;
}

/*--------------------------------------------------------------------------*/
int execRedirection(char *str, char *str1, char* str2, int flag2, int flag3, int flag4)
{
    char** arr1, **arr2;
    int fd;

    if(flag2==TRUE || flag3==TRUE)  //if '>' or '>>'
    {
        cutTheString(str, str1, str2, '>');
        arr1 = wordsArray(str1);
        arr2 = wordsArray(str2);

        if(arr1[0]==NULL || arr2[0]==NULL)
        {
            fprintf(stderr, "redirection location is wrong\n");
            return -1;
        }

        if(flag2==TRUE)
            fd=open(arr2[0] ,O_TRUNC | O_WRONLY | O_CREAT, 0600);  //open file
        if(flag3==TRUE)
            fd=open(arr2[0] ,O_WRONLY | O_CREAT | O_APPEND, 0600); //open file with concat

        if(fd<0)
        {
            perror("open file is failed\n");
            return -1;
        }
        forkWithFile(arr1, fd, flag4);
        freeArray(arr1);
        freeArray(arr2);
        return 1;
    }

    //---------------------------------------------------------------

    if(flag4==TRUE)  //if '<'
    {
        cutTheString(str, str1, str2, '<');

        arr1 = wordsArray(str1);
        arr2 = wordsArray(str2);

        if(arr1[0]==NULL || arr2[0]==NULL)
        {
            fprintf(stderr, "redirection location is wrong\n");
            return -1;
        }

        fd= open(arr2[0] ,O_RDONLY, 0600); //open file to read
        if(fd<0)
        {
            perror("open file is failed\n");
            return -1;
        }
        forkWithFile(arr1, fd, flag4);
    }
    return 1;
}

/*--------------------------------------------------------------------------*/
char **wordsArray(char *str)
{
    const char s[2] = " ";
    char *token;
    int count = 0, len=strlen(str);
    int i;
    char **arr;

    for (i = 0; i < len; i++)  //count the words in the string
    {
        if (str[i] != s[0])
        {
            count++;
            for (; str[i] != s[0]; i++);
            i--;
        }

    }
    arr = malloc(sizeof(char *) * (count + 1));
    if (arr == NULL)
    {
        perror("malloc is failed\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(str, s);
    i = 0;
    while (token != NULL)
    {
        arr[i] = malloc(sizeof(char) * strlen(token));
        if (arr[i] == NULL)
        {
            perror("malloc is failed\n");
            exit(EXIT_FAILURE);
        }
        arr[i] = token;  //words from the string
        i++;

        token = strtok(NULL, s);
    }
    arr[i] = NULL;

    return arr;
}

/*-------------------------------------------------------------*/
void cutTheString(char *str, char *str1, char *str2, char c) {

    int i, j = MAX_STRING, len = strlen(str);
    for (i = 0; i <= len; i++)
    {
        if (str[i] == c)
        {
            str1[i] = '\0';
            j = i;
            continue;
        }
        if (i < j)
            str1[i] = str[i];
        else str2[i - j - 1] = str[i];
    }
}

/*-------------------------------------------------------------*/
void forkOneProgram(char **arr)
{
    pid_t p;
    p = fork();
    if (p < 0)   //the fork is failed
    {
        printf("the fork is failed");
        exit(EXIT_FAILURE);
    }
    if (p == 0)  //this is the son
    {
        execvp(arr[0], arr);
        fprintf(stderr, "%s: command not found\n", arr[0]);
        exit(EXIT_FAILURE);
    }
    else    //this is the father
    {
        wait(NULL);
    }
}
/*----------------------------------------------------------------*/
void forkWithPipe(char **arr1, char **arr2)
{
    int pipe_d[2];

    pid_t p1, p2;
    if (pipe(pipe_d) == -1)
    {
        perror("can't open pipe\n");
        exit(EXIT_FAILURE);
    }
    p1 = fork();
    if (p1 < 0)
    {
        perror("fork is failed\n");
        exit(EXIT_FAILURE);
    }

    if (p1 == 0)
    {
        close(pipe_d[IN]); //child 1 writer
        dup2(pipe_d[OUT], STDOUT_FILENO);
        close(pipe_d[OUT]);
        execvp(arr1[0], arr1);
        fprintf(stderr, "%s: command not found\n", arr1[0]);
        exit(EXIT_FAILURE);

    }
    if (p1 > 0)
    {
        p2 = fork();
        if (p2 < 0)
        {
            perror("fork is failed\n");
            exit(EXIT_FAILURE);
        }

        if (p2 == 0)
        {
            close(pipe_d[OUT]);  //child 2 reader
            dup2(pipe_d[IN], STDIN_FILENO);
            close(pipe_d[IN]);
            execvp(arr2[0], arr2);
            fprintf(stderr, "%s: command not found\n", arr2[0]);
            exit(EXIT_FAILURE);
        }

        if (p2 > 0)
        {
            close(pipe_d[IN]);
            close(pipe_d[OUT]);
            wait(NULL);
        }
        wait(NULL);
    }
}
/*----------------------------------------------------------------*/
void forkWithFile(char **arr1, int fd, int flag4)
{
    pid_t p1;
    p1=fork();
    if (p1 < 0)   //the fork is failed
    {
        printf("the fork is failed");
        exit(EXIT_FAILURE);
    }
    if (p1 == 0)  //this is the son
    {
        if(flag4==TRUE)
        {
            dup2(fd, STDIN_FILENO);
            close(fd);
        } else{
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(arr1[0], arr1);
        fprintf(stderr, "%s: command not found\n", arr1[0]);
        exit(EXIT_FAILURE);
    }
    else    //this is the father
    {
        wait(NULL);
    }
}
/*----------------------------------------------------------------*/
int checkSpecialCommand(char *str, char *check)
{
    char *c;
    c = strstr(str, check);  //check if special command
    if (c != NULL)
        return TRUE;
    return FALSE;
}

/*----------------------------------------------------------------*/
void freeArray(char** arr)
{
    int i;
    for (i = 0; arr[i] != NULL; i++)
    {
        arr[i] = NULL;
        free(arr[i]);
    }
    arr = NULL;
    free(arr);
}