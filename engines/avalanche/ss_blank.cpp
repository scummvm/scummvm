#include "ptoc.h"


/*#include "Dos.h"*/
/*#include "Crt.h"*/

byte fv;
boolean test;

boolean the_cows_come_home()
{
    registers rmove,rclick;
boolean the_cows_come_home_result;
;
 rmove.ax=11; intr(0x33,rmove);
 rclick.ax=3; intr(0x33,rclick);
 the_cows_come_home_result=
   (keypressed()) | /* key pressed */
   (rmove.cx>0) || /* mouse moved */
   (rmove.dx>0) ||
   (rclick.bx>0);  /* button clicked */
return the_cows_come_home_result;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 test=the_cows_come_home();
 textattr=0; clrscr;
 do {; } while (!the_cows_come_home());
 textattr=30; clrscr;
 output << "*** Blank Screen *** (c) 1992, Thomas Thurman. (An Avvy Screen Saver.)" << NL;
 for( fv=1; fv <= 46; fv ++) output << '~'; output << NL;
 output << "This program may be freely copied." << NL;
 output << NL;
 output << "Have fun!" << NL;
return EXIT_SUCCESS;
}