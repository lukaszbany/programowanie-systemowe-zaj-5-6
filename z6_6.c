//Zadanie 6. Obsługa sygnałów
//Napisz program, który co n sekund wypisuje na ekranie komunikat: "Działam dalej". Naciśnięcie klawiszy CTRL+c
//(sygnał INT) wypisuje napis "Przechwycilem sygnal" i program kontynuuje działanie. Jakiej funkcji użyjesz do
//odmierzania czasu? Jak zakończyć ten program?

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

void sigint_handler( int sig_num )
{
    printf("Przechwycilem sygnal.\n");
    fflush(stdout);
}

void sigtstp_handler( int sig_num )
{
    printf("Wylaczam.\n");
    fflush(stdout);

    kill(getpid(), SIGKILL);
}

int main(int argc, char * argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Brak parametru (liczba sekund)\n");
        return -1;
    }
    int seconds = atoi(argv[1]);

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    while (1) {
        printf("Dzialam dalej\n");
        fflush(stdout);
        sleep(seconds);
    }

    /* Wylaczenie programu poprzez CTRL + Z */
}

