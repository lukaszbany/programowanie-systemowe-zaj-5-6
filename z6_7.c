//Zadanie 7. Obsługa sygnałów
//Napisz program, który prosi o podanie hasła i kończy działanie, jeśli użytkownik nie wprowadzi hasła w określonym
//czasie. Wskazówka: użyj funkcji alarm().

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define TIME 10
#define PASSWORD "proszeopunkt"
#define PASSWORD_LENGTH 15

void sigalrm_handler( int sig_num )
{
    printf("Nie podales poprawnego hasla w ciagu %d sekund.\n", TIME);
    printf("Program konczy dzialanie.\n");
    fflush(stdout);

    kill(getpid(), SIGTERM);
}

int checkPassword(char *password) {
    if (strcmp(password, PASSWORD) == 0) {
        printf("Dobre haslo. Gratulacje!\n");
        return 0;
    }

    printf("Haslo bledne. Sprobuj jeszcze raz:\n");
    return 1;
}

int main()
{
    signal(SIGALRM, sigalrm_handler);
    alarm(TIME);
    printf("Podaj haslo w ciagu %d sekund:\n", TIME);
    char password[PASSWORD_LENGTH];

    int result = 1;
    while (result) {
        scanf("%s", password);
        result = checkPassword(password);
    }

    return 0;
}

