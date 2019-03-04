//Zadanie 10.
//Napisz program, który pokazuje jak najwięcej atrybutów procesu (identyfikatory, bieżący katalog, katalog główny,
//maskę plików, ustalenia dotyczące obsługi sygnałów, otwarte pliki, inne).

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define RESULT_LENGTH 160
#define COMMAND_LENGTH 80
#define BINARY_LENGTH 4
#define SIGNALS_NUMBER 64

#define READ "r"

void catch_ctlc( int sig_num )
{
    printf( "Przechwycono Control-C\n" );
    fflush( stdout );
}
void catch_sigusr1( int sig_num )
{
    printf( "Przechwycono SIGUSR1\n" );
    fflush( stdout );
}

void trim(char *text) {
    int lastChar = strlen(text) - 1;
    if (text[lastChar] == '\n')
        text[lastChar] = '\0';
}

void printIdentification() {
    char *login;
    if ((login = getlogin()) == NULL) {
        perror("Blad");
        exit(EXIT_FAILURE);
    }
    printf("Login wlasciciela procesu: %s\n", login);

    pid_t pid = getpid();
    pid_t ppid = getppid();
    uid_t uid = getuid();
    gid_t gid = getgid();
    uid_t euid = geteuid();
    gid_t egid = getegid();
    pid_t sid = getsid(0);
    if (sid == -1) {
        perror("Blad");
        exit(EXIT_FAILURE);
    }
    pid_t pgrp = getpgrp();
    if (pgrp == -1) {
        perror("Blad");
        exit(EXIT_FAILURE);
    }

    printf("PID: %d, ", pid);
    printf("PPID: %d, ", ppid);
    printf("UID: %d, ", uid);
    printf("GID: %d, ", gid);
    printf("EUID: %d, ", euid);
    printf("EGID: %d, ", egid);
    printf("SID: %d, ", sid);
    printf("PGRP: %d\n", pgrp);
}

char *getResultFromCommand(char *command) {
    char *result = malloc(sizeof(char) * RESULT_LENGTH);

    FILE *cmd = popen(command, READ);
    if (!cmd) {
        perror("Nie udalo sie wykonac polecenia");
        exit(EXIT_FAILURE);
    }

    fgets(result, RESULT_LENGTH, cmd);
    trim(result);

    if (pclose(cmd) == -1) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }

    return result;
}

void printDirectories(pid_t pid) {

    char *cwd, *root;
    char command[COMMAND_LENGTH];

    sprintf(command, "readlink /proc/%d/cwd", pid);
    cwd = getResultFromCommand(command);

    sprintf(command, "readlink /proc/%d/root", pid);
    root = getResultFromCommand(command);

    printf("Biezacy katalog: %s\n", cwd);
    printf("Główny katalog: %s\n", root);

    free(cwd);
    free(root);
}

void printUmask(pid_t pid) {

    char *umask;
    char command[COMMAND_LENGTH];

    sprintf(command, "grep '^Umask:' \"/proc/%d/status\" | cut -f2", pid);
    umask = getResultFromCommand(command);

    printf("Maska plikow: %s\n", umask);

    free(umask);
}

char *convertHexCharToBin(char hex) {
    char * bin = malloc(sizeof(char) * BINARY_LENGTH);

    switch (hex) {
        case '0':
            sprintf(bin, "0000");
            break;
        case '1':
            sprintf(bin, "0001");
            break;
        case '2':
            sprintf(bin, "0010");
            break;
        case '3':
            sprintf(bin, "0011");
            break;
        case '4':
            sprintf(bin, "0100");
            break;
        case '5':
            sprintf(bin, "0101");
            break;
        case '6':
            sprintf(bin, "0110");
            break;
        case '7':
            sprintf(bin, "0111");
            break;
        case '8':
            sprintf(bin, "1000");
            break;
        case '9':
            sprintf(bin, "1001");
            break;
        case 'A':
        case 'a':
            sprintf(bin, "1010");
            break;
        case 'B':
        case 'b':
            sprintf(bin, "1011");
            break;
        case 'C':
        case 'c':
            sprintf(bin, "1100");
            break;
        case 'D':
        case 'd':
            sprintf(bin, "1101");
            break;
        case 'E':
        case 'e':
            sprintf(bin, "1110");
            break;
        case 'F':
        case 'f':
            sprintf(bin, "1111");
            break;
        default:
            printf("Znak nie nalezy do zakresu liczb heksadecymalnych!");
            exit(EXIT_FAILURE);
    }

    return bin;
}

void printSignalFromNumber(int number) {
    char getSignal[COMMAND_LENGTH];
    sprintf(getSignal, "kill -L %d", number);

    char *cmd = getResultFromCommand(getSignal);
    printf("SIG%s, ", cmd);
    free(cmd);
}

void printSignalsFromBinary(char *binary) {
    for (int i = 0; i < SIGNALS_NUMBER; ++i) {
        if (binary[i] == '1') {
            printSignalFromNumber(SIGNALS_NUMBER - i);
        }
    }
    printf("\n");
}

void printSignalsFromHex(char *hex) {
    int length = strlen(hex);
    char binary[length * BINARY_LENGTH];
    binary[0] = '\0';

    for (int i = 0; i < length; ++i) {
        char *current = convertHexCharToBin(hex[i]);
        strcat(binary, current);
        free(current);
    }

    printSignalsFromBinary(binary);
}

void printSignals(pid_t pid, char *grepFor, char *name) {
    char *signals;
    char SigCgt[COMMAND_LENGTH];

    sprintf(SigCgt, "grep '^%s:' \"/proc/%d/status\" | cut -f2", grepFor, pid);
    signals = getResultFromCommand(SigCgt);

    printf("%s sygnaly: ", name);
    printSignalsFromHex(signals);

    free(signals);
}

void printAllSignals(pid_t pid) {
    printSignals(pid, "SigCgt", "Obslugiwane");
    printSignals(pid, "SigIgn", "Ignorowane");
    printSignals(pid, "SigBlk", "Blokowane");
}

void printFilesOpened(pid_t pid) {
    char file[RESULT_LENGTH];
    char getOpenedFiles[COMMAND_LENGTH];
    sprintf(getOpenedFiles, "ls -l /proc/%d/fd | awk '{ print $10 }'", pid);

    FILE *cmd = popen(getOpenedFiles, READ);
    if (!cmd) {
        perror("Nie udalo sie wykonac polecenia");
        exit(EXIT_FAILURE);
    }

    printf("Otwarte pliki:\n");
    while (fgets(file, RESULT_LENGTH, cmd)) {
        trim(file);

        if (file[0] == '/') {
            printf("%s\n", file);
        }
    }

    if (pclose(cmd) == -1) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }
}

void printProcessName(pid_t pid) {
    char *name;
    char getName[COMMAND_LENGTH];

    sprintf(getName, "grep '^Name:' \"/proc/%d/status\" | cut -f2", pid);
    name = getResultFromCommand(getName);

    printf("Nazwa procesu: %s\n", name);

    free(name);
}

void printCurrentProcessAttributes() {
    pid_t pid = getpid();

    printProcessName(pid);
    printIdentification();
    printDirectories(pid);
    printUmask(pid);
    printAllSignals(pid);
    printFilesOpened(pid);
}

int main()
{
    // Przykladowa obsluga sygnalow
    signal(SIGINT, catch_ctlc);
    signal(SIGUSR1, catch_sigusr1);

    // Przykladowy otwarty plik
    FILE *file = fopen("test.txt", READ);
    if (!file) {
        perror("Blad otwierania pliku");
        return EXIT_FAILURE;
    }

    printCurrentProcessAttributes();

    if ((fclose(file))) {
        perror("Blad zamykania pliku");
        exit(EXIT_FAILURE);
    }

    return 0;
}

