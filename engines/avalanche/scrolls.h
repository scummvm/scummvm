#ifndef __scrolls_h__
#define __scrolls_h__


#include "gyro.h"
#include "joystick.h"


const boolean aboutscroll = false; /* Is this the about box? */

void state(byte x);      /* Sets "Ready" light to whatever */

void drawscroll(proc gotoit);      /* This is one of the oldest procs in the game. */

void bubble(proc gotoit);

void resetscroll();

void calldrivers();

void display(string z);

boolean ask(string question);

void natural();

string lsd();

void okay();    /* Says "Okay!" */

void musical_scroll();

#endif
