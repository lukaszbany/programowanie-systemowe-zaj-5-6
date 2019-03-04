//Zadanie 8 (3pkt).
//Napisz program, który wyświetli imiona i nazwiska wszystkich użytkowników, którzy uruchomili aplikację o nazwie
//podanej w wierszu wywołania. Jeżeli aplikacja nie działa w systemie, wyświetli komunikat: „Aplikacja … nie jest
//uruchomiona”.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pwd.h>

#define PIDS_LENGTH 200
#define LOGIN_LENGTH 50
#define LOGINS_NUMBER 25
#define COMMAND_LENGTH 80

void checkIfAppIsWorking(char *pids) {
    if (pids[0] == 0) {
        printf("W systemie nie dziala obecnie taka aplikacja.\n");
        exit(0);
    }
}

void trimLogin(char *login) {
    int lastChar = strlen(login) - 1;
    if (login[lastChar] == '\n')
        login[lastChar] = '\0';
}

char *getUsernameForPid(pid_t pid) {
    char *login;
    char command[COMMAND_LENGTH];

    login = malloc(sizeof(char) * LOGIN_LENGTH);
    sprintf(command, "ps -o user= -p %d", pid);

    FILE *cmd = popen(command, "r");
    if (!cmd) {
        perror("Nie udalo sie wykonac polecenia pidof.");
        exit(EXIT_FAILURE);
    }

    fgets(login, LOGIN_LENGTH, cmd);
    trimLogin(login);

    if (pclose(cmd) == -1) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }

    return login;
}

char * getPidsOfApp(char *applicationName) {
    char *line;
    line = malloc(sizeof(char) * PIDS_LENGTH);

    char command[COMMAND_LENGTH];
    sprintf(command, "pidof %s", applicationName);

    FILE *cmd = popen(command, "r");
    if (!cmd) {
        perror("Nie udalo sie wykonac polecenia pidof.");
        exit(EXIT_FAILURE);
    }

    fgets(line, PIDS_LENGTH, cmd);
    checkIfAppIsWorking(line);

    if (pclose(cmd) == -1) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }

    return line;
}

void printNameOfUserWithLogin(char *login) {
    struct passwd *userData;
    userData = getpwnam(login);
    if (userData == NULL) {
        perror("Brak uzytkownika! ");
        exit(EXIT_FAILURE);
    }

    printf("%s", userData->pw_gecos);
}

int loginNotSavedBefore(char *login, char *logins[], int loginsSize) {
    for (int i = 0; i < loginsSize; ++i) {
        if (strcmp(logins[i], login) == 0) {
            return 0;
        }
    }

    return 1;
}

void printUsersOfApp(char *pids) {
    char * token;
    char * logins[LOGINS_NUMBER];
    int current = 0;

    token = strtok(pids, " ");
    while (token != NULL) {
        pid_t pid = strtoul(token, NULL, 10);

        char *login = getUsernameForPid(pid);
        if (loginNotSavedBefore(login, logins, current)) {
            logins[current] = login;
            current++;
        }

        token = strtok(NULL, " ");
    }

    printf("Uzytkownicy korzystajacy z programu:\n");

    int loginsNumber = current - 1;
    for (int i = 0; i <= loginsNumber; ++i) {
        printNameOfUserWithLogin(logins[i]);
        if (i != loginsNumber) printf(", ");
    }
    putchar('\n');
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Brak parametru (nazwa aplikacji)\n");
        return -1;
    }
    char *application = argv[1];

    char * pids = getPidsOfApp(application);
    printUsersOfApp(pids);

    return 0;
}

