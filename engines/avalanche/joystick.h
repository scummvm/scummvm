#ifndef __joystick_h__
#define __joystick_h__


void readjoya(word& xaxis, word& yaxis);
/* Reads the X and Y coordinates of Joystick A. */

void readjoyb(word& xaxis, word& yaxis);
/* Reads the X and Y coordinates of Joystick B. */

boolean buttona1();
boolean buttona2();
boolean buttonb1();
boolean buttonb2();
/* These four functions return the status (true = in; false = out) of each
  of the buttons on joystick A and B.  On most models, Button #1 is the
  top button. */

boolean joystickpresent();
/* This function indicates whether a joystick is installed. */

#endif
