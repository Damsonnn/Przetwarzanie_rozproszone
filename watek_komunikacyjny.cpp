#include "main.h"
#include "test/main.h"
#include "watek_komunikacyjny.h"
#include <math.h>

pthread_mutex_t recvMutex = PTHREAD_MUTEX_INITIALIZER;

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    bool is_message = false;
    packet_t packet;
    numberReceived = 0;
    int destination;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (true) {
	debug("czekam na recv");
        pthread_mutex_lock( &recvMutex );
        MPI_Recv( &packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_unlock( &recvMutex );
        lamportClock = max(lamportClock, packet.ts) + 1;
        pthread_mutex_unlock( &recvMutex );
        switch (packet.type)
        {
        case REQUEST:
            // Obsługa request'a
                destination = packet.src;
                packet.type = ACKNOWLEDGE;
                packet.src = rank;
                sendPacket(&packet, destination, 0);
            break;
        
        case ACKNOWLEDGE:
            // Obsługa acknowledge'a
            numberReceived++;
            break;

        case RELASE:
            // Obsługa relase'a
            break;
        /*
        case DONE:
            //Wyszedł z krytycznej, wyślij relase
            break;

        case NEED_HOTEL:
            //Wyślij requesty o hotel
            break;

        case NEED_GUIDE:
            //Wyślij request o przewodnika
        */
        default:
            break;
        }
	    break;
    }
}
