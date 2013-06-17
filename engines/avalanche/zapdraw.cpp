#include "ptoc.h"


#include "graph.h"
/*#include "Crt.h"*/
/*#include "Dos.h"*/


untyped_file f;
byte bit;
byte a;  /*absolute $A000:0; */
integer gd,gm;

void graphmode(integer mode)
{
    registers regs;
;
 regs.ax=(mode % 0x100);
 intr(0x10,regs);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 assign(f,"d:avltzap.raw"); reset(f,1);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  for( gd=0; gd <= 199; gd ++)
   blockread(f,mem[0xa000*gd*80],40); /* 28000 */
 }
 close(f);
 setwritemode(xorput);
 rectangle(  0,  0,  5,  8);
 rectangle(  0, 10, 27, 19);
return EXIT_SUCCESS;
}
