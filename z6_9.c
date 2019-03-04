//Zadanie 9 (2pkt).
//Tak, jak zadanie 8 z tym, że w pliku o podanej ścieżce przekazujemy listę nazw aplikacji.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pwd.h>

#define PIDS_LENGTH 200
#define LOGIN_LENGTH 50
#define LOGINS_NUMBER 25
#define COMMAND_LENGTH 80
#define APPNAME_LENGTH 80

int appIsWorking(char *pids) {
    if (pids[0] == 0) {
        return 0;
    }

    return 1;
}

void trimEndNewLine(char *text) {
    int lastChar = strlen(text) - 1;
    if (text[lastChar] == '\n')
        text[lastChar] = '\0';
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
    trimEndNewLine(login);

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

    if (pclose(cmd) == -1) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }

    if (!appIsWorking(line)) {
        printf("W systemie nie dziala obecnie aplikacja %s.\n", applicationName);
        return NULL;
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

void printUsersOfApp(char *pids, char *appName) {
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

    printf("Uzytkownicy korzystajacy z programu %s:\n", appName);

    int loginsNumber = current - 1;
    for (int i = 0; i <= loginsNumber; ++i) {
        printNameOfUserWithLogin(logins[i]);
        if (i != loginsNumber) printf(", ");
    }
    putchar('\n');
}

void printNamesForApplicationsFromFile(char * filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Blad otwierania pliku");
        exit(EXIT_FAILURE);
    }

    char application[APPNAME_LENGTH];
    while (fgets(application, APPNAME_LENGTH, file)) {
        trimEndNewLine(application);
        char * pids;
        if ((pids = getPidsOfApp(application)) != NULL) {
            printUsersOfApp(pids, application);
        }
    }

    if (fclose(file) != 0) {
        perror("Blad zamykania pliku");
        exit(EXIT_FAILURE);
    }
}

/*Program przyjmuje plik z nazwami aplikacji (kazda nazwa powinna znajdowac sie w nowej linii).*/
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Brak parametru (plik z nazwami aplikacji)\n");
        return -1;
    }
    char *filename = argv[1];
    printNamesForApplicationsFromFile(filename);


    return 0;
}

