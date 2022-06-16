#ifndef WATEK_GLOWNY_H
#define WATEK_GLOWNY_H

/* pętla główna aplikacji: zmiany stanów itd */
#include "main.h"
void mainLoopAlien();
packet_t createPacket(int type);
void sendHotelRequests();
void waitForState(state_t needed);
void relaseNotNeededHotels();

#endif
