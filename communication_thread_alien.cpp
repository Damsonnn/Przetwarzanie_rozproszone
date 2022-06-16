#include "main.h"
#include "communication_thread_alien.h"
#include <algorithm>
#include <cstdlib>
#include <mpi.h>

std::vector<pending> guideQueue;
int guideAvailability;

bool comparePending(const pending &a, const pending &b){
    if (a.clock == b.clock) return a.rank < b.rank;
    else return a.clock < b.clock;
}

bool checkHotelForAlien(std::vector<pending> hotel){
    for (int i = 0; i < hotelSpace; i++){
        if (hotel[i].fraction != fraction){
            return false;
        }if (hotel[i].rank != rank){
            return true;
        }
    }
    return true;
}

void limitClock(int limit){
    if (limit != 0 && lamportClock > limit){
        debug("Zakończono na %d", lamportClock);
        exit(1);
    } 
}

void handleGuideAck(){
    changeState(WaitGuide);     
    for (int i = 0; i < guideQueue.size(); i++){
        if (guideQueue[i].rank == rank) {
            guideAvailability = i + 1;
            if (guideAvailability <= guides){
                //debug("zabieram przewodnika ACK");
                changeState(GoGuide);
            }
            break;
        }
    }
}

void handleHotelAck(packet_t packet, std::vector<pending> hotelsQueue[]){
    bool foundHotel = false;
    changeState(WaitHotel);
    // for (int i = 0; i < hotels; i++){
    //     for (int j = 0; j < hotelsQueue[i].size(); j++){
    //         debug("Hotel %d, Miejsce: %d, Czas: %d, Proces: %d, Frakcja %d", i, j, hotelsQueue[i][j].clock, hotelsQueue[i][j].rank, hotelsQueue[i][j].fraction)
    //     }
    // }
    for (int i = 0; i < hotels; i++) availability[i] = 0;

    for (int i = 0; i < hotels; i++){
        for (int j = 0; j < hotelsQueue[i].size(); j++){
            if (hotelsQueue[i][j].fraction == CLEANER){
                availability[i] = 0;
            }else if (hotelsQueue[i][j].fraction != fraction){
                availability[i] = -1;
            }else if (hotelsQueue[i][j].rank == rank) {
                if (availability[i] >= 0) {
                    availability[i] = j + 1;
                    foundHotel = true;
                }
                break;
            }
        }
    }
    if (!foundHotel){
        int randomHotel = rand() % hotels;
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
        if (availability[i] > 0 && availability[i] <= hotelSpace && checkHotelForAlien(hotelsQueue[i])){
            //debug("ACK")
            myHotel = i;
            changeState(GoHotel);
            break;
        }
    }
}

void handleRequest(packet_t packet, std::vector<pending> hotelsQueue[]){
    struct pending newRequest;
    int destination;
    newRequest.clock = packet.ts;
    newRequest.fraction = packet.fraction;
    newRequest.rank = packet.src;
    
    if (packet.resource == GUIDE){
        guideQueue.push_back(newRequest);
        std::sort(guideQueue.begin(), guideQueue.end(), comparePending);
    } else{
        
        hotelsQueue[packet.resource].push_back(newRequest);
        std::sort(hotelsQueue[packet.resource].begin(), hotelsQueue[packet.resource].end(), comparePending);
        //debug("Pierwszy w kolejce CZAS: %d, FRAKCJA: %d, ZASÓB: %d", hotelsQueue[packet.resource][0].clock, hotelsQueue[packet.resource][0].fraction, packet.resource);
    }
    if (packet.src != rank){
        destination = packet.src;
        packet.type = ACKNOWLEDGE;
        sendPacket(&packet, destination, 0);
    }
}

void handleGuideRelase(packet_t packet){
    for (int i = 0; i < guideQueue.size(); i++){
        if (guideQueue[i].rank == packet.src){
            guideQueue.erase(guideQueue.begin() + i);
            if (state == WaitGuide && i + 1 < guideAvailability){
                guideAvailability--;
                //debug("Sprawdzam czy guide dostępny");
                if (guideAvailability <= guides){
                    //debug("zabieram przewodnika RELASE");
                    changeState(GoGuide);
                }
            }
        }
    }
}

void handleHotelRelase(packet_t packet, std::vector<pending> hotelsQueue[]){
    for (int i = 0; i < hotelsQueue[packet.resource].size(); i++){
        if (hotelsQueue[packet.resource][i].rank == packet.src){
            hotelsQueue[packet.resource].erase(hotelsQueue[packet.resource].begin() + i);
            if (state == WaitHotel && i + 1 < availability[packet.resource]){
                availability[packet.resource]--;
                if (availability[packet.resource] <= hotelSpace){
                    if (checkHotelForAlien(hotelsQueue[packet.resource])) {
                        myHotel = packet.resource;
                        changeState(GoHotel);
                    }
                }
            }
        }
    }
}

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startComThreadAlien(void *ptr)
{   
    MPI_Status status;
    packet_t packet;
    std::vector<pending> hotelsQueue[hotels];
    int numberReceived = 0, guideAvailability = 0;
    int neededAckHotel, neededAckGuide;
    neededAckHotel = hotels * (size - 1); 
    neededAckGuide = size - 1;

    while (true) {
        limitClock(1000);
        MPI_Recv( &packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        //debug("odebrałem wiadomość CZAS: %d, PROCES: %d, TYP: %d, FRAKCJA: %d, ZASÓB: %d", packet.ts ,packet.src, packet.type, packet.fraction, packet.resource);
        incrementClock(&packet, lamportClock);
        
        switch (packet.type)
        {
        case REQUEST:
            handleRequest(packet, hotelsQueue);
            break;
        
        case ACKNOWLEDGE:        
            numberReceived++;
            if (packet.resource == GUIDE && numberReceived == neededAckGuide){ 
                handleGuideAck();
                numberReceived = 0;
            }
            else if (packet.resource != GUIDE && numberReceived == neededAckHotel){
                handleHotelAck(packet, hotelsQueue);
                numberReceived = 0;
            }        
            break;

        case RELASE:
            if (packet.resource == GUIDE){
                handleGuideRelase(packet);
            }else {
                handleHotelRelase(packet, hotelsQueue);
            }
            break;

        default:
            debug("Coś poszło bardzo nie tak!!!");
            break;
        }
    }
    return 0;
}
