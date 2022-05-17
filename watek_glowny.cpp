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
                    if (j != rank) sendPacket(&packet, j, 0);
                }  
            }
        }
        /*
        //Rób swoje w hotelu proś o przewodnika
        packet.resource = GUIDE;
        for (int i = 0; i < size; i++){
            if (i != rank) sendPacket(&packet, i, 0);
        }
        while (stan != GoGuide){
            continue;
        }
        //Rób swoje z przewodnikiem, roześlij relase'y
        packet.type = RELASE;
        for (int i = 0; i < size; i++){
            if (i != rank) sendPacket(&packet, i, 0);
        }
        */
    }
}
