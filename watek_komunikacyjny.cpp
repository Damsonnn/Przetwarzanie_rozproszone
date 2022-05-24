#include "main.h"
#include "test/main.h"
#include "watek_komunikacyjny.h"
#include <cstdio>
#include <cstdlib>
#include <math.h>

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
        MPI_Recv( &packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        incrementClock(&packet, lamportClock);
        switch (packet.type)
        {
        case REQUEST:
            // Obsługa request'a
            Pending* newRequest = new Pending;
            newRequest->clock = packet.ts;
            newRequest->fraction = packet.fraction;
            newRequest->type = REQUEST;
            if (packet.resource == GUIDE){
                guideQueue.push_back(newRequest);
            } else{

            }
            hotels[packet.resource]
            destination = packet.src;
            packet.type = ACKNOWLEDGE;
            sendPacket(&packet, destination, 0);
            break;
        
        case ACKNOWLEDGE:
            // Obsługa acknowledge'a
            numberReceived++;
            break;

        case RELASE:
            // Obsługa relase'a
            break;

        default:
            printf("Coś poszło bardzo nie tak!!!");
            break;
        }
	    break;
    }
}
