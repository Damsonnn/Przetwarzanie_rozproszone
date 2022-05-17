#include "main.h"
#include "watek_komunikacyjny.h"
#include <math.h>

pthread_mutex_t recvMutex = PTHREAD_MUTEX_INITIALIZER;

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	debug("czekam na recv");
        pthread_mutex_lock( &recvMutex );
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_unlock( &recvMutex );
        lamportClock = max(lamportClock, pakiet.ts) + 1;
        pthread_mutex_unlock( &recvMutex );
        switch (pakiet.type)
        {
        case REQUEST:
            // Obsługa request'a
            break;
        
        case ACKOWLEDGE:
            // Obsługa acknowledg'a
            break;

        case RELASE:
            // Obsługa relase'a
            break;

        case DONE:
            //Wyszedł z krytycznej, wyślij relase
            break;

        case NEED_HOTEL:
            //Wyślij requesty o hotel
            break;

        case NEED_GUIDE:
            //Wyślij request o przewodnika
        default:
            break;
        }
	    break;
        }
    }
}
