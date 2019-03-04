//Zadanie 4. Kody zakończenia procesów potomnych
//W pliku shell.c znajduje się prosty shell umożliwiajmy wykonywanie poleceń podanych w wierszu wywołania.
//Uzupełnij go o zwracanie informacji o sposobie zakończenia wykonywanego polecenia.

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int     shellcmd(char *);
void    prstat(int);

int main(void)
{
    int status;
    char command[BUFSIZ];

    /*
     * Forever...
     */
    for (;;) {
        printf("Enter a command: ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            putchar('\n');
            exit(0);
        }
        command[strlen(command)-1] = '\0';
        status = shellcmd(command);
        prstat(status);
        putchar('\n');
    }
}

int shellcmd(char *command)
{
    int status;
    pid_t pid;
    extern int errno;

    if ((pid = fork()) < 0)
        status = -1;

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, NULL);
        _exit(127);
    }
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            status = -1;
            break;
        }
    }
    return(status);
}

void prstat(int status)
{
    if (WIFEXITED(status)) {
        printf("Polecenie zakonczone normalnie z kodem: %d\n", WEXITSTATUS(status));
    } else {
        printf("Polecenie zakonczony sygnalem (kod %d)\n", WSTOPSIG(status));
    }
}
