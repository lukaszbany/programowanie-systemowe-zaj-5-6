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
#include <time.h>

#define WRITE "w"

void saveDataAndSendSignal(int pid, int blockedSignal, int endSignal) {
    FILE *data = fopen("data.dat", WRITE);
    if (!data) {
        perror("Blad otwarcia pliku");
        exit(EXIT_FAILURE);
    }
    fprintf(data, "%d\n%d", blockedSignal, endSignal);

    if (fclose(data)) {
        printf("Blad zamykania pliku.");
        exit(EXIT_FAILURE);
    }

    kill(pid, SIGUSR1);
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Nieprawidlowa liczba argumentow!\n");
        return EXIT_FAILURE;
    }
    int pid = atoi(argv[1]);
    int counter = atoi(argv[2]);
    int blockedSignal = atoi(argv[3]);
    int endSignal = atoi(argv[4]);

    saveDataAndSendSignal(pid, blockedSignal, endSignal);

    // Na moim komputerze przy ustawieniu 1 nanosekundy
    // i 1 000 000 sygnalow trwa to bardzo dlugo.
    // Dlatego ustawilem 0 nanosekund (niewiele to daje,
    // ale odstep miedzy sygnalami moze byc troche krotszy.
    struct timespec t = { 0, 0 };
    for (int i = 0; i < counter; ++i) {
        kill(pid, blockedSignal);

        if (nanosleep(&t, NULL)) {
            perror("Blad funkcji nanosleep");
            exit(EXIT_FAILURE);
        }
    }

    printf("sig_sender: Sygnal %d wyslano %d razy.\n", blockedSignal, counter);
    kill(pid, endSignal);

    return EXIT_SUCCESS;
}

