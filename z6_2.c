//Zadanie 2. Procesy - użycie funkcji fork()
//Napisz program, który tworzy jeden proces potomny. Proces potomny wyświetla n razy zestaw małych liter alfabetu
//(czyli litery od a do z), proces macierzysty wyświetla n razy zestaw wielkich liter alfabetu (czyli litery od A do Z.
//Wartość n jest podawana w wierszu wywołania programu. Sprawdź, czy każde wykonanie programu będzie
//przebiegało tak samo? Spróbuj uruchomić program na różnych komputerach, na przykład na oceanic. Wybierz
//duże n.
//Uwaga: Nie wolno wykorzystywać liczbowych kodów ASCII!

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Brak liczby w parametrze\n");
        exit(1);
    }

    int times = atoi(argv[1]);
    int i = 0;

    pid_t pid = fork();
    switch (pid) {
        case -1:
            perror("Blad: ");
            break;
        case 0:
            for (i = 0; i < times; ++i) {
                for(char c = 'a'; c <= 'z'; c++) {
                    printf("%c", c);
                }
                printf("\n");
            }
            break;
        default:
            for (i = 0; i < times; ++i) {
                for(char c = 'A'; c <= 'Z'; c++) {
                    printf("%c", c);
                }
                printf("\n");
            }
            break;
    }

    // komentarz:
    // Na moim komputerze wynik jest zawsze taki sam (najpierw wykonuje sie proces macierzysty.
    // Przez ssh na oceanicu wyniki sa rozne.

    return 0;
}