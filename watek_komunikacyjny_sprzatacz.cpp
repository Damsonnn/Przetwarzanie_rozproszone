#include "main.h"
#include "watek_komunikacyjny_sprzatacz.h"
#include "watek_komunikacyjny.h"
#include <algorithm>
#include <cstdlib>

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatekSprzatacz(void *ptr)
{
    std::vector<pending> hotelsQueue[hotels];
    MPI_Status status;
    bool is_message = false, foundHotel = false;
    packet_t packet;
    int numberReceived = 0;
    int destination, neededAckHotel, randomHotel;
    neededAckHotel = hotels * (size - 1); 
    struct pending newRequest;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (true) {
	    //debug("czekam na recv");
        MPI_Recv( &packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        //debug("odebrałem wiadomość CZAS: %d, PROCES: %d, TYP: %d, FRAKCJA: %d, ZASÓB: %d", packet.ts ,packet.src, packet.type, packet.fraction, packet.resource);
        incrementClock(&packet, lamportClock);
        
        switch (packet.type)
        {
        case REQUEST:
            // Obsługa request'a   
            
            newRequest.clock = packet.ts;
            newRequest.fraction = packet.fraction;
            newRequest.rank = packet.src;
            
            if (packet.resource != GUIDE){
                hotelsQueue[packet.resource].push_back(newRequest);
                std::sort(hotelsQueue[packet.resource].begin(), hotelsQueue[packet.resource].end(), comparePending);
                //debug("Pierwszy w kolejce CZAS: %d, FRAKCJA: %d, ZASÓB: %d", hotelsQueue[packet.resource][0].clock, hotelsQueue[packet.resource][0].fraction, packet.resource);
            }
            if (packet.src != rank){
                destination = packet.src;
                packet.type = ACKNOWLEDGE;
                sendPacket(&packet, destination, 0);
            }
            break;
        
        case ACKNOWLEDGE:        
            numberReceived++;
            if (numberReceived == neededAckHotel){
                foundHotel = false;
                changeState(WaitHotel);
                // if (true){
                //     for (int i = 0; i < hotels; i++){
                //         for (int j = 0; j < hotelsQueue[i].size(); j++){
                //             debug("Hotel %d, Miejsce: %d, Czas: %d, Proces: %d, Frakcja %d", i, j, hotelsQueue[i][j].clock, hotelsQueue[i][j].rank, hotelsQueue[i][j].fraction)
                //         }
                //     }
                // }
                for (int i = 0; i < hotels; i++) availability[i] = 0;

                for (int i = 0; i < hotels; i++){
                    for (int j = 0; j < hotelsQueue[i].size(); j++){
                        if (hotelsQueue[i][j].rank == rank) {
                            availability[i] = j + 1;
                            foundHotel = true;
                            break;  
                        }else if (hotelsQueue[i][j].fraction == CLEANER){
                            availability[i] = -1;
                            break;
                        }
                    }
                }
                
                for (int i = 0; i < hotels; i++){
                    debug("Hotel %d: %d", i, availability[i]);
                }
                if (!foundHotel){
                    randomHotel = rand() % hotels;
                    for (int i = 0; i < hotelsQueue[randomHotel].size(); i++){
                        if (hotelsQueue[randomHotel][i].rank == rank){
                            availability[randomHotel] = i + 1;
                            break;
                        }
                    }
                }
                packet.type = RELASE;

                for (int i = 0; i < hotels; i++){       
                    if (availability[i] == -1){
                        //debug("Hotel nr %d mi nie potrzebny", i);
                        for (int j = 0; j < size; j++){
                            packet.resource = i;
                            sendPacket(&packet, j, 0);
                        }
                    }
                }
                for (int i = 0; i < hotels; i++){
                    if (availability[i] == 1){
                        myHotel = i;
                        changeState(GoHotel);
                        break;
                    }
                }
                numberReceived = 0;
            }        
            break;

        case RELASE:
            if (packet.resource != GUIDE){
                for (int i = 0; i < hotelsQueue[packet.resource].size(); i++){
                    if (hotelsQueue[packet.resource][i].rank == packet.src){
                        hotelsQueue[packet.resource].erase(hotelsQueue[packet.resource].begin() + i);
                        if (state == WaitHotel && i + 1 < availability[packet.resource]){
                            availability[packet.resource]--;
                            if (availability[packet.resource] == 1){
                                myHotel = packet.resource;
                                changeState(GoHotel);
                            }
                        }
                    }
                }
            }
            break;

        default:
            debug("Coś poszło bardzo nie tak!!!");
            break;
        }
    }
    return 0;
}