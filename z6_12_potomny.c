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
    printf("%s\n", login);

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

    printf("%d\n", pid);
    printf("%d\n", ppid);
    printf("%d\n", uid);
    printf("%d\n", gid);
    printf("%d\n", euid);
    printf("%d\n", egid);
    printf("%d\n", sid);
    printf("%d\n", pgrp);
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

    printf("%s\n", cwd);
    printf("%s\n", root);

    free(cwd);
    free(root);
}

void printUmask(pid_t pid, ProcessData *processData) {

    char *umask;
    char command[COMMAND_LENGTH];

    sprintf(command, "grep '^Umask:' \"/proc/%d/status\" | cut -f2", pid);
    umask = getResultFromCommand(command);
    strcpy(processData->umask, umask);

    printf("%s\n", umask);

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
    printf("%s\n", signals);

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

    printf("FILES\n");
    int i = 0;
    while (fgets(file, RESULT_LENGTH, cmd)) {
        trim(file);

        if (file[0] == '/') {
            printf("%s\n", file);
            strcpy(processData->filesOpened[i], file);
            i++;
        }
    }
    processData->numberOfOpenedFiles = i + 1;
    printf("FILESEND\n");

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

    printf("%s\n", name);

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

int filesOpenedDifferent(ProcessData *parentData, ProcessData *childData) {
    if (parentData->numberOfOpenedFiles != childData->numberOfOpenedFiles) {
        return -1;
    }

    int size = parentData->numberOfOpenedFiles;
    for (int i = 0; i < size; ++i) {
        if ((strcmp(parentData->filesOpened[i], childData->filesOpened[i]) != 0))
            return -1;
    }

    return 0;
}

int main()
{
    ProcessData processData = { 0 };

    printCurrentProcessAttributes(&processData);

    return 0;
}

