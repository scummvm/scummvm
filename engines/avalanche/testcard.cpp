#include "ptoc.h"


#include "graph.h"
/*#include "Dos.h"*/

integer gd,gm;
string filename;

untyped_file f; byte bit;
byte a /*absolute $A000:800*/;
searchrec r;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 for( gd=0; gd <= 14; gd ++)
  for( gm=0; gm <= 11; gm ++)
  {;
   setfillstyle(gm,gd+1);
   bar(gd*43,gm*12+10,42+gd*43,gm*12+21);
  }

 output << NL; output << NL; output << NL; output << NL;

 output << "Thorsoft testcard." << NL;
 output << NL;
 output << "Room number? "; input >> filename >> NL;

 findfirst(string("place")+filename+".avd",anyfile,r);
 if (doserror==0) 
 {;
  output << "*** ALREADY EXISTS! CANCELLED! ***" << NL;
  input >> NL;
  exit(0);
 }

 output << NL;
 output << "*** Until this room is drawn, this screen is standing in for it. ***" << NL;
 output << NL;
 output << "Any other comments? ";
 input >> NL;

 assign(f,string("place")+filename+".avd");
 rewrite(f,1);
 blockwrite(f,gd,177); /* just anything */
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockwrite(f,a,12080);
 }
 close(f);
return EXIT_SUCCESS;
}
