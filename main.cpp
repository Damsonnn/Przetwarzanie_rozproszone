#include "main.h"
#include "communication_thread_alien.h"
#include "main_thread_alien.h"
#include "communication_thread_cleaner.h"
#include "main_thread_cleaner.h"
/* wątki */
#include <cstdlib>
#include <pthread.h>

/* sem_init sem_destroy sem_post sem_wait */
//#include <semaphore.h>
/* flagi dla open */
//#include <fcntl.h>
int fraction, myHotel;
int availability[hotels];

state_t state = DoNothing;
volatile char end = false;
int size,rank,lamportClock; /* nie trzeba zerować, bo zmienna globalna statyczna */
MPI_Datatype MPI_PAKIET_T;

pthread_t threadKom, threadMon;

pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clockMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t hotelMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t guideMutex = PTHREAD_MUTEX_INITIALIZER;

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}

/* srprawdza, czy są wątki, tworzy typ MPI_PAKIET_T
*/
void inicjuj(int *argc, char ***argv)
{
    int provided;
    time_t t;
    srand(rank);
    MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    const int nitems=5; /* bo packet_t ma trzy pola */
    int       blocklengths[5] = {1,1,1,1,1};
    MPI_Datatype typy[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[5]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, type); //REQUEST, ACKNOWLEDGE, RELASE
    offsets[3] = offsetof(packet_t, fraction); //BLUE, VIOLET, CLEANER
    offsets[4] = offsetof(packet_t, resource); //GUIDE, nr hotelu od 1 do n


    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    lamportClock = rank;

    if (rank < cleaners) fraction = CLEANER;
    else if (rank < (users - cleaners) / 2 + cleaners) fraction = BLUE;
    else fraction = VIOLET;
    if (fraction == CLEANER) pthread_create( &threadKom, NULL, startComThreadCleaner , 0);
    else pthread_create( &threadKom, NULL, startComThreadAlien , 0);
    
    debug("jestem");
}

/* usunięcie zamkków, czeka, aż zakończy się drugi wątek, zwalnia przydzielony typ MPI_PAKIET_T
   wywoływane w funkcji main przed końcem
*/
void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    if (rank==0) pthread_join(threadMon,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}


/* opis patrz main.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = new packet_t; freepkt=1;}
    pkt->src = rank;
    pthread_mutex_lock( &clockMutex );
    lamportClock += 1;
    pkt->ts = lamportClock;
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    pthread_mutex_unlock( &clockMutex );
    
    if (freepkt) free(pkt);
}

void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    state = newState;
    pthread_mutex_unlock( &stateMut );
}

void incrementClock(packet_t *pkt, int myClock){
    pthread_mutex_lock(&clockMutex);
    lamportClock = std::max(pkt->ts, myClock) + 1;
    pthread_mutex_unlock(&clockMutex);
}

int main(int argc, char **argv)
{
    /* Tworzenie wątków, inicjalizacja itp */
    inicjuj(&argc,&argv); // tworzy wątek komunikacyjny w "watek_komunikacyjny.c"
    if (fraction == CLEANER) mainLoopCleaner();          // w pliku "watek_glowny.c"
    else mainLoopAlien();
    finalizuj();
    return 0;
}

