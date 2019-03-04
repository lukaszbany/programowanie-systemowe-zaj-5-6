//Zadanie 5. Obsługa sygnałów
//a) Zapoznaj się z programem sigcatch.c. Jakie jest zadanie funkcji signal() oraz pause()? Jak program
//zachowa się w przypadku otrzymana sygnału INT a jak w przypadku innego sygnału, np. QUIT, TERM?
//b) Zapoznaj się z programem raise.c. Jakie jest zadanie funkcji signal(), pause(), kill()?

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

void usr1_handler( int sig_num )
{

    printf( "Rodzic (%d) otrzymal sygnal SIGUSR1\n", getpid() );

}

int main()
{
    /* Funkcja signal() sluzy do wskazania, ze w momencie otrzymania
     * sygnalu SIGUSR1 przez proces macierzysty, zostanie wykonana funkcja
     * usr1_handler.
     *
     * Funkcja pause() wstrzymuje proces macierzysty w oczekiwaniu na sygnal.
     *
     * Funkcja kill() w procesie potomnym przerywa ten proces i wysyla do
     * proces o wskazanym jako parametr pid (w tym wypadku do procesu
     * macierzystego) sygnal SIGUSR1.
     * */
    pid_t ret;
    int   status;
    int   role = -1;

    ret = fork();
    if (ret > 0) {
        printf( "Rodzic: to jest proces macierzysty (pid %d)\n",
                getpid() );
        signal( SIGUSR1, usr1_handler );
        role = 0;
        pause();
        printf( "Rodzic: czekam na zakonczenie potomka\n" );
        ret = wait( &status );
    }
    else if (ret == 0) {
        printf( "Potomek: to jest proces potomny (pid %d)\n",
                getpid() );
        role = 1;
        sleep( 1 );
        printf( "Potomek: Wysylam SIGUSR1 do pid %d\n", getppid() );
        kill( getppid(), SIGUSR1 );
        sleep( 2 );

    }
    else {
        printf( "Rodzic: Blad podczas proby wykonania fork() (%d)\n", errno );

    }

    printf( "%s: Koncze ...\n",
            ((role == 0) ? "Rodzic" : "Potomek") );

    return 0;
}

