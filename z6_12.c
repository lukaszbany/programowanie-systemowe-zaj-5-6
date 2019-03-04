//Zadanie 12.
//Napisz program, który pokazuje, które atrybuty procesu są zachowane przez proces po wykonaniu funkcji exec().

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define RESULT_LENGTH 160
#define COMMAND_LENGTH 80
#define BINARY_LENGTH 4
#define SIGNALS_NUMBER 64
#define MAX_FILES_OPENED 32
#define ATTRIBUTE_LENGTH 32
#define ATTRIBUTES_NUMBER 16

#define BOLDGREEN   "\033[1m\033[32m"
#define RESET   "\033[0m"

#define READ "r"

typedef struct {
    char name[RESULT_LENGTH];
    char login[RESULT_LENGTH];
    pid_t pid;
    pid_t ppid;
    uid_t uid;
    gid_t gid;
    uid_t euid;
    gid_t egid;
    pid_t sid;
    pid_t pgrp;
    char cwd[RESULT_LENGTH];
    char root[RESULT_LENGTH];
    char umask[RESULT_LENGTH];
    char sigCgt[RESULT_LENGTH];
    char sigIgn[RESULT_LENGTH];
    char sigBlk[RESULT_LENGTH];
    char filesOpened[MAX_FILES_OPENED][RESULT_LENGTH];
    int numberOfOpenedFiles;
}ProcessData;

typedef struct {
    int i;
    char inherited[ATTRIBUTES_NUMBER][ATTRIBUTE_LENGTH];
    int d;
    char different[ATTRIBUTES_NUMBER][ATTRIBUTE_LENGTH];
}Differences;

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

void printIdentification(ProcessData *processData) {
    char *login;
    if ((login = getlogin()) == NULL) {
        perror("Blad");
        exit(EXIT_FAILURE);
    }
    strcpy(processData->login, login);
    printf("Login wlasciciela procesu: %s\n", login);

    pid_t pid = getpid();
    processData->pid = pid;

    pid_t ppid = getppid();
    processData->ppid = ppid;

    uid_t uid = getuid();
    processData->uid = uid;

    gid_t gid = getgid();
    processData->gid = gid;

    uid_t euid = geteuid();
    processData->euid = euid;

    gid_t egid = getegid();
    processData->egid = egid;

    pid_t sid = getsid(0);
    if (sid == -1) {
        perror("Blad");
        exit(EXIT_FAILURE);
    }
    processData->sid = sid;

    pid_t pgrp = getpgrp();
    if (pgrp == -1) {
        perror("Blad");
        exit(EXIT_FAILURE);
    }
    processData->pgrp = pgrp;

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

void printDirectories(pid_t pid, ProcessData *processData) {

    char *cwd, *root;
    char command[COMMAND_LENGTH];

    sprintf(command, "readlink /proc/%d/cwd", pid);
    cwd = getResultFromCommand(command);
    strcpy(processData->cwd, cwd);

    sprintf(command, "readlink /proc/%d/root", pid);
    root = getResultFromCommand(command);
    strcpy(processData->root, root);

    printf("Biezacy katalog: %s\n", cwd);
    printf("Główny katalog: %s\n", root);

    free(cwd);
    free(root);
}

void printUmask(pid_t pid, ProcessData *processData) {

    char *umask;
    char command[COMMAND_LENGTH];

    sprintf(command, "grep '^Umask:' \"/proc/%d/status\" | cut -f2", pid);
    umask = getResultFromCommand(command);
    strcpy(processData->umask, umask);

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

void saveSignals(char *signals, char *type, ProcessData *processData) {
    if (strcmp(type, "SigCgt") == 0) {
        strcpy(processData->sigCgt, signals);
    } else if (strcmp(type, "SigIgn") == 0) {
        strcpy(processData->sigIgn, signals);
    } else if (strcmp(type, "SigBlk") == 0) {
        strcpy(processData->sigBlk, signals);
    }
}

void printSignals(pid_t pid, char *grepFor, char *name, ProcessData *processData) {
    char *signals;
    char SigCgt[COMMAND_LENGTH];

    sprintf(SigCgt, "grep '^%s:' \"/proc/%d/status\" | cut -f2", grepFor, pid);
    signals = getResultFromCommand(SigCgt);
    saveSignals(signals, grepFor, processData);

    printf("%s sygnaly: ", name);
    printSignalsFromHex(signals);

    free(signals);
}

void printAllSignals(pid_t pid, ProcessData *processData) {
    printSignals(pid, "SigCgt", "Obslugiwane", processData);
    printSignals(pid, "SigIgn", "Ignorowane", processData);
    printSignals(pid, "SigBlk", "Blokowane", processData);
}

void printFilesOpened(pid_t pid, ProcessData *processData) {
    char file[RESULT_LENGTH];
    char getOpenedFiles[COMMAND_LENGTH];
    sprintf(getOpenedFiles, "ls -l /proc/%d/fd | awk '{ print $10 }'", pid);

    FILE *cmd = popen(getOpenedFiles, READ);
    if (!cmd) {
        perror("Nie udalo sie wykonac polecenia");
        exit(EXIT_FAILURE);
    }

    printf("Otwarte pliki:\n");
    int i = 0;
    while (fgets(file, RESULT_LENGTH, cmd)) {
        trim(file);

        if (file[0] == '/') {
            printf("%s\n", file);
            strcpy(processData->filesOpened[i], file);
            i++;
        }
    }
    processData->numberOfOpenedFiles = i;

    if (pclose(cmd) == -1) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }
}

void printProcessName(pid_t pid, ProcessData *processData) {
    char *name;
    char getName[COMMAND_LENGTH];

    sprintf(getName, "grep '^Name:' \"/proc/%d/status\" | cut -f2", pid);
    name = getResultFromCommand(getName);

    printf("Nazwa procesu: %s\n", name);
    strcpy(processData->name, name);

    free(name);
}

void setParent(ProcessData *data) {
    strcpy(data->name, "test");
    data->pid = 123;
}

void printCurrentProcessAttributes(ProcessData *processData) {
    pid_t pid = getpid();

    printProcessName(pid, processData);
    printIdentification(processData);
    printDirectories(pid, processData);
    printUmask(pid, processData);
    printAllSignals(pid, processData);
    printFilesOpened(pid, processData);
}

void compareFilesOpenedAndAdd(Differences *diffs, ProcessData *parentData, ProcessData *childData) {
    if (parentData->numberOfOpenedFiles != childData->numberOfOpenedFiles) {
        strcpy(diffs->different[diffs->d++], "Otwarte pliki");
        return;
    }

    int size = parentData->numberOfOpenedFiles;
    for (int i = 0; i < size; ++i) {
        if ((strcmp(parentData->filesOpened[i], childData->filesOpened[i]) != 0)) {
            strcpy(diffs->different[diffs->d++], "Otwarte pliki");
            return;
        }
    }

    strcpy(diffs->inherited[diffs->i++], "Otwarte pliki");
}

void compareCharsArraysAndAdd(Differences *diffs, char *name, char *first, char *second) {
    if ((strcmp(first, second) == 0)) {
        strcpy(diffs->inherited[diffs->i++], name);
    } else {
         strcpy(diffs->different[diffs->d++], name);
    }
}

void compareIntegersAndAdd(Differences *diffs, char *name, int first, int second) {
    if (first == second) {
        strcpy(diffs->inherited[diffs->i++], name);
    } else {
        strcpy(diffs->different[diffs->d++], name);
    }
}

void printArray(char array[ATTRIBUTES_NUMBER][ATTRIBUTE_LENGTH], int length) {
    for (int i = 0; i < length; ++i) {
        printf("%s", array[i]);
        if (i < length - 1) {
            printf(", ");
            if (i > 0 && i % 5 == 0) printf("\n");
        }
        if (i == length - 1) {
            printf(".\n");
        }
    }
}

void printDifferencesBetweenProcesses(ProcessData *parentData, ProcessData *childData) {
    Differences diffs = { 0 };
    printf("\n----- ROZNICE MIEDZY PROCESAMI ( funkcja exec() ) -----\n");

    compareCharsArraysAndAdd(&diffs, "Nazwa procesu", parentData->name, childData->name);
    compareCharsArraysAndAdd(&diffs, "Login wlasciciela", parentData->login, childData->login);
    compareIntegersAndAdd(&diffs, "PID", parentData->pid, childData->pid);
    compareIntegersAndAdd(&diffs, "PPID", parentData->ppid, childData->ppid);
    compareIntegersAndAdd(&diffs, "UID", parentData->uid, childData->uid);
    compareIntegersAndAdd(&diffs, "GID", parentData->gid, childData->gid);
    compareIntegersAndAdd(&diffs, "EUID", parentData->euid, childData->euid);
    compareIntegersAndAdd(&diffs, "EGID", parentData->egid, childData->egid);
    compareIntegersAndAdd(&diffs, "SID", parentData->sid, childData->sid);
    compareIntegersAndAdd(&diffs, "PGRP", parentData->pgrp, childData->pgrp);
    compareCharsArraysAndAdd(&diffs, "Katalog biezacy", parentData->cwd, childData->cwd);
    compareCharsArraysAndAdd(&diffs, "Katalog glowny", parentData->root, childData->root);
    compareCharsArraysAndAdd(&diffs, "Maska plikow", parentData->umask, childData->umask);
    compareCharsArraysAndAdd(&diffs, "Obslugiwane sygnaly", parentData->sigCgt, childData->sigCgt);
    compareCharsArraysAndAdd(&diffs, "Ignorowane sygnaly", parentData->sigIgn, childData->sigIgn);
    compareCharsArraysAndAdd(&diffs, "Blokowane sygnaly", parentData->sigBlk, childData->sigBlk);
    compareFilesOpenedAndAdd(&diffs, parentData, childData);

    printf(BOLDGREEN "Odziedziczone: " RESET);
    printArray(diffs.inherited, diffs.i);
    printf("\n");

    printf(BOLDGREEN "Nowe: " RESET);
    printArray(diffs.different, diffs.d);
    printf("\n");
}

void prepareSignals() {
    signal(SIGINT, catch_ctlc);
    signal(SIGUSR1, catch_sigusr1);
    signal(SIGUSR2, SIG_IGN);

    sigset_t blocked;
    if (sigemptyset(&blocked)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&blocked, SIGTSTP)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_SETMASK, &blocked, NULL)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
}

void readCurrent(char *current) {
    if ((scanf("%s", current)) == EOF) {
        perror("Blad wczytywania danych z procesu potomnego");
        exit(EXIT_FAILURE);
    }
}

void printChildData(ProcessData *data) {
    printf("\n------------- PROCES POTOMNY -------------\n");
    printf("Nazwa procesu: %s\n", data->name);
    printf("Login wlasciciela procesu: %s\n", data->login);
    printf("PID: %d, ", data->pid);
    printf("PPID: %d, ", data->ppid);
    printf("UID: %d, ", data->uid);
    printf("GID: %d, ", data->gid);
    printf("EUID: %d, ", data->euid);
    printf("EGID: %d, ", data->egid);
    printf("SID: %d, ", data->sid);
    printf("PGRP: %d\n", data->pgrp);
    printf("Biezacy katalog: %s\n", data->cwd);
    printf("Główny katalog: %s\n", data->root);
    printf("Maska plikow: %s\n", data->umask);
    printf("Obslugiwane sygnaly: ");
    printSignalsFromHex(data->sigCgt);
    printf("Ignorowane sygnaly: ");
    printSignalsFromHex(data->sigIgn);
    printf("Blokowane sygnaly: ");
    printSignalsFromHex(data->sigBlk);
    printf("Otwarte pliki:\n");
    for (int i = 0; i < data->numberOfOpenedFiles; ++i) {
        printf("%s\n", data->filesOpened[i]);
    }
}

void readChildData(ProcessData *data) {
    char *current = malloc(sizeof(char) * (RESULT_LENGTH + ATTRIBUTE_LENGTH));

    readCurrent(current);
    strcpy(data->name, current);

    readCurrent(current);
    strcpy(data->login, current);

    readCurrent(current);
    data->pid = atoi(current);

    readCurrent(current);
    data->ppid = atoi(current);

    readCurrent(current);
    data->uid = atoi(current);

    readCurrent(current);
    data->gid = atoi(current);

    readCurrent(current);
    data->euid = atoi(current);

    readCurrent(current);
    data->egid = atoi(current);

    readCurrent(current);
    data->sid = atoi(current);

    readCurrent(current);
    data->pgrp = atoi(current);

    readCurrent(current);
    strcpy(data->cwd, current);

    readCurrent(current);
    strcpy(data->root, current);

    readCurrent(current);
    strcpy(data->umask, current);

    readCurrent(current);
    strcpy(data->sigCgt, current);

    readCurrent(current);
    strcpy(data->sigIgn, current);

    readCurrent(current);
    strcpy(data->sigBlk, current);

    readCurrent(current);
    if ((strcmp(current, "FILES")) != 0) {
        printf("Blad wczytywania danych z procesu potomnego");
        exit(EXIT_FAILURE);
    }

    int i = 0;

    readCurrent(current);
    while ((strcmp(current, "FILESEND")) != 0) {
        strcpy(data->filesOpened[i++], current);
        readCurrent(current);
    }
    data->numberOfOpenedFiles = i;

    printChildData(data);

    free(current);
}

int main()
{
    // Przykladowa obsluga sygnalow
    prepareSignals();

    // Przykladowy otwarty plik
    FILE *file = fopen("test.txt", READ);

    ProcessData processData = { 0 };

    if (!file) {
        perror("Blad otwierania pliku");
        return EXIT_FAILURE;
    }

    int dataPipe[2];
    if (pipe(dataPipe)) {
        perror("Blad inicjalizacji potoku");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    switch (pid) {
        case -1:
            perror("Blad");
            break;
        case 0:
            close(dataPipe[0]);
            dup2(dataPipe[1], 1);
            close(dataPipe[1]);
            if (execl("./z6_12_potomny", "./z6_12_potomny", "", NULL) < 0) {
                perror("Blad");
                return EXIT_FAILURE;
            }
        default:
            close(dataPipe[1]);
            dup2(dataPipe[0], 0);
            close(dataPipe[0]);
            wait(NULL);
            printf("\n------------- PROCES MACIERZYSTY -------------\n");
            printCurrentProcessAttributes(&processData);
            break;
    }

    ProcessData childData = { 0 };
    readChildData(&childData);

    printDifferencesBetweenProcesses(&processData, &childData);

    if ((fclose(file))) {
        perror("Blad zamykania pliku");
        exit(EXIT_FAILURE);
    }

    return 0;
}

