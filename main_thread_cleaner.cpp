#include "main.h"
#include "main_thread_cleaner.h"
#include "main_thread_alien.h"
#include <cstdlib>
#include <unistd.h>

void freeHotel(){
    packet_t packet = createPacket(RELASE);
    packet.resource = myHotel;
    for (int i = 0; i < size; i++){
        sendPacket(&packet, i, 0);
    }
}

void mainLoopCleaner()
{
    while (true) {
        sleep(5);
        sendHotelRequests();
        debug("Sprzątacz - czekam na hotel");

        waitForState(GoHotel);
        debug("Sprzątacz - wchodzę do hotelu nr %d", myHotel);
        
        relaseNotNeededHotels();
        sleep(3);
        debug("Sprzątacz - zwalniam zasoby, hotel nr %d", myHotel);
        freeHotel();

        changeState(DoNothing);
    }
}