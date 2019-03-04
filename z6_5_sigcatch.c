//Zadanie 5. Obsługa sygnałów
//a) Zapoznaj się z programem sigcatch.c. Jakie jest zadanie funkcji signal() oraz pause()? Jak program
//zachowa się w przypadku otrzymana sygnału INT a jak w przypadku innego sygnału, np. QUIT, TERM?
//b) Zapoznaj się z programem raise.c. Jakie jest zadanie funkcji signal(), pause(), kill()?

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void catch_ctlc( int sig_num )
{
    printf( "Przechwycono Control-C\n" );
    fflush( stdout );

    return;
}

int main()
{
    /* Funkcja signal() sluzy do wskazania funkcji (drugi argument),
     * ktora zostanie wykonana w momencie przechwycenia sygnalu
     * podanego jako pierwszy argument (tutaj SIGNINT, czyli ctrl+c).
     *
     * Jesli program dostanie inny sygnal, np. QUIT lub TERM,
     * funkcja nie zostanie wywolana, a program zostanie przerwany.
     *
     * Funkcja pause() powoduje, ze program czeka na sygnal i nie wykonuje sie dalej.
     *
     * */
    signal( SIGINT, catch_ctlc );

    printf("Dzialam ...\n");

    pause();

    return 0;
}

