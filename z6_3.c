//Zadanie 3. Procesy -użycie funkcji fork(), exec(), wait()
//a) Napisz program, który tworzy jeden proces potomny. Proces potomny ma wyświetlić tekst: „Today is:”
//Do wyświetlenia tekstu użyj polecenia /bin/echo. Proces macierzysty ma wyświetlić datę. Użyj do tego
//polecenia /bin/date. Przykład:
//Today is:
//Saturday, December 5, 2015
//b) Wykonaj kilka razy program. Czy zawsze tekst pojawi się przed datą? Zmodyfikuj program tak, aby zawsze tekst
//pojawiał się przed datą.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {

    pid_t pid = fork();

    char * lang = getenv("LANG");
    setenv("LANG", "en_EN", 1);

    switch (pid) {
        case -1:
            perror("Blad");
            break;
        case 0:
            if (execl("/bin/echo", "echo", "Today is: ", NULL) < 0) {
                perror("Blad");
                return -1;
            }
            break;
        default:
            wait(NULL);
            system("/bin/date +\"%A, %B %e, %Y\"");
            break;
    }

    setenv("LANG", lang, 1);
    return 0;
}