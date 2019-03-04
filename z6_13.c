//Zadanie 13.
//Napisz program, który wyświetla identyfikator procesu (PID) i nazwę związanego z nim polecenia dla wszystkich
//procesów uruchomionych przez użytkownika podanego w wierszu wywołania programu. Wskazówka: informacje te
//można uzyskać przeglądając katalog /proc z plików (interesują nas katalogi, których właścicielem jest dany
//użytkownik) i pliki /proc/PID/status.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>

#define RESULT_LENGTH 160
#define COMMAND_LENGTH 80

#define READ "r"

void trim(char *text) {
    int lastChar = strlen(text) - 1;
    if (text[lastChar] == '\n')
        text[lastChar] = '\0';
}

void printPidsAndCommandsForUser(char *username) {
    char command[COMMAND_LENGTH];
    sprintf(command, "ps -u %s -o pid=PID,comm=POLECENIE,cmd=PELNE_POLECENIE", username);

    FILE *cmd = popen(command, READ);
    if (!cmd) {
        perror("Nie udalo sie wykonac polecenia");
        exit(EXIT_FAILURE);
    }

    char result[RESULT_LENGTH];
    while (fgets(result, RESULT_LENGTH, cmd)) {
        trim(result);
        printf("%s\n", result);
    }

    if (pclose(cmd) == -1) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }
}

void checkUserExist(char *username) {
    if (getpwnam(username) == NULL) {
        printf("Podany uzytkownik nie istnieje!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Potraktowalem korzystanie z /proc/(pid)/status jako sugestie.
 * Mozna takie informacje uzyskac korzystajac z polecenia ps.
 * Z /proc korzystalem przy poprzednich zadaniach.
 * */
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Brak parametru (nazwa uzytkownika)\n");
        return -1;
    }

    char *username = argv[1];
    checkUserExist(username);

    printPidsAndCommandsForUser(username);

    return EXIT_SUCCESS;
}

