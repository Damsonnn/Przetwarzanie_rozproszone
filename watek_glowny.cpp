#include "main.h"
#include "watek_glowny.h"
#include <unistd.h>

void mainLoop()
{
    packet_t packet;
    packet.fraction = fraction;
    packet.src = rank;
    while (true) {
        packet.type = REQUEST;
        for (int i = 0; i < hotels; i++){
            packet.resource = i;
            for (int j = 0; j < size; j++){
                sendPacket(&packet, j, 0);
            }   
        }
        debug("czekam na hotel");

        while (state != GoHotel) {
            sleep(1);
            continue;
        }
        debug("wchodzę do hotelu nr %d", myHotel);
        
        packet.type = RELASE;
        for (int i = 0; i < hotels; i++){
            if (i != myHotel && availability[i] > 0){
                packet.resource = i;
                //debug("Hotel nr %d niepotrzebny, mam swój", i);
                for (int j = 0; j < size; j++){
                    sendPacket(&packet, j, 0);
                }  
            }
        }
        sleep(1);
        //debug("dej mi przewodnika");
        packet.type = REQUEST;
        packet.resource = GUIDE;
        for (int i = 0; i < size; i++){
            sendPacket(&packet, i, 0);
        }
        //debug("czekam na przewodnika");
    
        while (state != GoGuide){
            sleep(1);
            continue;
        }
        debug("wychodzę z przewodnikiem");
        //Rób swoje z przewodnikiem, potem roześlij relase'y
        sleep(1); //Coś tam robię
        debug("zwalniam zasoby, hotel nr %d", myHotel);
        packet.type = RELASE;
        for (int i = 0; i < size; i++){
            sendPacket(&packet, i, 0);
        }
        packet.resource = myHotel;
        for (int i = 0; i < size; i++){
            sendPacket(&packet, i, 0);
        }
        changeState(DoNothing);
        sleep(1);//Czekam przed kolejną chęcią wejścia do hotelu
    }
}
