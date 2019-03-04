//Zadanie 14 (dodatkowe 4 punkty)
//Napisz aplikację, która wykaże, że sygnały nie są kolejkowane. Aplikacja składa się z dwóch programów:
//a) Programu wysyłającego sygnały: sig_sender. Program ten uruchamiany jest z następującymi argumentami: PID
//procesu, do którego wysyłany będzie sygnał, numer wysyłanego sygnału, liczba wysłanych sygnałów, sygnał
//kończący proces, do którego wysyłane są sygnały.
//b) Programu odbierającego i zliczającego sygnały: sig_receiver. Program ten uruchamiany jest z argumentem
//określającym liczbę sekund, przez którą należy blokować sygnały.
//Przykład wywołania:
//$ sig_receiver 15 & // blokuj sygnały przez 15 sekund
//$ sig_sender 4567 1000000 10 2
//sig_receiver: sygnał 10 wyslano 1000000 razy
//sig_receiver: sygnal 10 odebrano 65 razy

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>

#define LINE_LENGTH 10
#define READ "r"

static volatile int counter = 0;
static volatile int blckdSignal = -1;
static volatile int blockedSig = 0;

void endSignalHandler(int sig_num) {
    printf("sig_receiver: Sygnal %d odebrano %d razy.\n", blockedSig, counter);
    exit(EXIT_SUCCESS);
}

void blockedSignalHandler(int sig_num) {
    counter++;
}

void sigalrm_handler( int sig_num )
{
    blckdSignal = 0;
}

void trim(char *text) {
    int lastChar = strlen(text) - 1;
    if (text[lastChar] == '\n')
        text[lastChar] = '\0';
}

void getSignals(char *block, char *end) {
    FILE *data = fopen("data.dat", READ);
    if (!data) {
        perror("Blad otwarcia pliku");
        exit(EXIT_FAILURE);
    }
    fgets(block, LINE_LENGTH, data);
    fgets(end, LINE_LENGTH, data);
    trim(block);
    trim(end);

    if (fclose(data)) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }
}

void handlerUsr1(int sig_num) {
    signal(SIGUSR1, SIG_DFL);

    char block[LINE_LENGTH], end[LINE_LENGTH];
    getSignals(block, end);

    int signalToEnd = atoi(end);
    int signalToBlock = atoi(block);

    signal(signalToEnd, endSignalHandler);
    signal(signalToBlock, blockedSignalHandler);
    blckdSignal = signalToBlock;
    blockedSig = signalToBlock;
}

void blockSignal() {
    sigset_t toBlock;
    if (sigemptyset(&toBlock)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&toBlock, blckdSignal)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_SETMASK, &toBlock, NULL)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
}

void unblockSignals() {
    sigset_t empty;
    if (sigemptyset(&empty)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_SETMASK, &empty, NULL)) {
        perror("Blad obslugi sygnalow");
        exit(EXIT_FAILURE);
    }
}

void checkChanges() {
    if (blckdSignal > 0) {
        blockSignal();
        blckdSignal = -1;
    } else if (blckdSignal == 0) {
        unblockSignals();
        blckdSignal = -1;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Nieprawidlowa liczba argumentow!\n");
        return EXIT_FAILURE;
    }
    int time = atoi(argv[1]);

    signal(SIGALRM, sigalrm_handler);
    signal(SIGUSR1, handlerUsr1);

    pid_t pid = fork();
    switch (pid) {
        case -1:
            perror("Blad");
            break;
        case 0:
            alarm(time);
            while (1) {
                pause();
                checkChanges();
            }
        default:
            exit(EXIT_SUCCESS);
    }
}

