#include "main.h"
#include "watek_glowny_sprzatacz.h"
#include <unistd.h>

void mainLoopCleaner()
{
    packet_t packet;
    packet.fraction = fraction;
    packet.src = rank;
    while (true) {
        sleep(5);
        packet.type = REQUEST;
        for (int i = 0; i < hotels; i++){
            packet.resource = i;
            for (int j = 0; j < size; j++){
                sendPacket(&packet, j, 0);
            }   
        }
        debug("Sprzątacz - czekam na hotel");

        while (state != GoHotel) {
            sleep(1);
            continue;
        }
        debug("Sprzątacz - wchodzę do hotelu nr %d", myHotel);
        
        packet.type = RELASE;
        for (int i = 0; i < hotels; i++){
            if (i != myHotel && availability[i] > 0){
                packet.resource = i;
                //debug("Sprzątacz - Hotel nr %d niepotrzebny, mam swój", i);
                for (int j = 0; j < size; j++){
                    sendPacket(&packet, j, 0);
                }  
            }
        }
        sleep(3);
        debug("Sprzątacz - zwalniam zasoby, hotel nr %d", myHotel);
        packet.resource = myHotel;
        for (int i = 0; i < size; i++){
            sendPacket(&packet, i, 0);
        }
        changeState(DoNothing);
    }
}