#include "main.h"
#include "test/main.h"
#include "watek_glowny.h"
#include <mpi.h>
#include <unistd.h>

void mainLoop()
{
    packet_t packet;
    packet.fraction = fraction;
    packet.src = rank;
    while (true) {
        packet.type = REQUEST;
        for (int i = 1; i <= hotels->size(); i++){
            packet.resource = i;
            for (int j = 0; j < size; j++){
                if (j != rank) sendPacket(&packet, j, 0);
            }   
        }
        while (stan != GoHotel) {
            continue;
        }
        packet.type = RELASE;
        for (int i = 1; i <= hotels->size(); i++){
            if (i != myHotel){
                packet.resource = i;
                for (int j = 0; j < size; j++){
                    sendPacket(&packet, j, 0);
                }  
            }
        }

        packet.type = REQUEST;
        packet.resource = GUIDE;
        for (int i = 0; i < size; i++){
            if (i != rank) sendPacket(&packet, i, 0);
        }
        while (stan != GoGuide){
            continue;
        }
        //Rób swoje z przewodnikiem, potem roześlij relase'y
        sleep(1); //Coś tam robię

        packet.type = RELASE;
        for (int i = 0; i < size; i++){
            sendPacket(&packet, i, 0);
        }
        packet.resource = myHotel;
        for (int i = 0; i < size; i++){
            sendPacket(&packet, i, 0);
        }

        changeState(Wait);
        sleep(1);//Czekam przed kolejną chęcią wejścia do hotelu
    }
}
