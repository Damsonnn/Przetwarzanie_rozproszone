#include "main.h"
#include "main_thread_alien.h"
#include <unistd.h>

packet_t createPacket(int type){
    packet_t packet;
    packet.fraction = fraction;
    packet.src = rank;
    packet.type = type;
    return packet;
}

void sendHotelRequests(){
    packet_t packet = createPacket(REQUEST);

    for (int i = 0; i < hotels; i++){
        packet.resource = i;
        for (int j = 0; j < size; j++){
            sendPacket(&packet, j, 0);
        }   
    }
    //debug("czekam na hotel");
    return;
}

void sendGuideRequests(){
    packet_t packet = createPacket(REQUEST);
    packet.resource = GUIDE;

    for (int i = 0; i < size; i++){
        //debug("Wysyłam request o przewodnika");
        sendPacket(&packet, i, 0);
    }
}

void waitForState(state_t needed){
    while (state != needed) {
        sleep(1);
        continue;
    }
}

void relaseNotNeededHotels(){
    packet_t packet = createPacket(RELASE);

    for (int i = 0; i < hotels; i++){
        if (i != myHotel && availability[i] > 0){
            packet.resource = i;
            //debug("Hotel nr %d niepotrzebny, mam swój", i);
            for (int j = 0; j < size; j++){
                sendPacket(&packet, j, 0);
            }  
        }
    }
}

void freeResources(){
    packet_t packet = createPacket(RELASE);

    packet.resource = GUIDE;
    for (int i = 0; i < size; i++){
        sendPacket(&packet, i, 0);
    }

    packet.resource = myHotel;
    for (int i = 0; i < size; i++){
        sendPacket(&packet, i, 0);
    }
}

void mainLoopAlien()
{
    while (true) {
        sendHotelRequests();
        waitForState(GoHotel);
        debug("wchodzę do hotelu nr %d", myHotel);

        relaseNotNeededHotels();
        sleep(1);
        
        //debug("czekam na przewodnika");
        sendGuideRequests();

        waitForState(GoGuide);
        debug("wychodzę z przewodnikiem");
        //Rób swoje z przewodnikiem, potem roześlij relase'y
        sleep(1); //Coś tam robię

        debug("zwalniam zasoby, hotel nr %d", myHotel);
        freeResources();
        changeState(DoNothing);

        sleep(1);//Czekam przed kolejną chęcią wejścia do hotelu
    }
}
