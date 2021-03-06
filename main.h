#ifndef GLOBALH
#define GLOBALH

#define _GNU_SOURCE
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <vector>
/* odkomentować, jeżeli się chce DEBUGI */
#define DEBUG 

#define ROOT 0


/* stany procesu */
typedef enum {DoNothing, WaitHotel, WaitGuide, GoHotel, GoGuide} state_t;
extern state_t state;
extern int rank;
extern int size;

// Usunięcie aktywnego czekania
extern pthread_mutex_t hotelMutex;
extern pthread_mutex_t guideMutex;

/* Nasze zasoby */
const int guides = 3;
const int hotels = 3;
const int hotelSpace = 2;
const int cleaners = 2;
const int users = 10;

//Dostępność hoteli
extern int availability[];

//Frakcja
extern int fraction;

/* Do jakiego hotelu wchodzimy/w jakim jesteśmy */
extern int myHotel;

//Mój zegar lamporta
extern int lamportClock;

/* Typy procesów */
#define CLEANER 0
#define BLUE 1
#define VIOLET 2

/* Typy wiadomości */
#define REQUEST 0
#define ACKNOWLEDGE 1
#define RELASE 2

/* Przewodnicy */
#define GUIDE -1

/* to może przeniesiemy do global... */
#define FIELDNO 5
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;      /* pole nie przesyłane, ale ustawiane w main_loop */
    int type;     /* Typ wiadomości np. req */
    int fraction; /* frakcja np. błekitni */
    int resource; /* nr. hotelu lub 0 dla przewodników */
} packet_t;
extern MPI_Datatype MPI_PAKIET_T;

struct pending{
    int clock; //zegar lamporta
    int fraction; //frakcja
    int rank; // rank procesu
};



/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
//#define debug(FORMAT,...) printf("%c[%d;%dm [%d/%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, lamportClock, rank, ##__VA_ARGS__, 27,0,37);
#define debug(FORMAT,...) printf(" [%d] %c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  lamportClock, 27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

/* printf ale z kolorkami i automatycznym wyświetlaniem RANK. Patrz debug wyżej po szczegóły, jak działa ustawianie kolorków */
#define println(FORMAT, ...) printf("%c[%d;%dm [%d/%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamportClock, ##__VA_ARGS__, 27,0,37);

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void changeState( state_t );
void incrementClock(packet_t *pkt, int myClock);
void finalizuj();
#endif
