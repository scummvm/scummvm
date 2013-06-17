


#include "graph.h"


const longint pagetop = 81920;
const word nextcode = 17717;

integer gd,gm;
file<word> magic,out;
word next,gg;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 assign(magic,"v:magicirc.avd"); reset(magic);
 assign(out,"v:magic2.avd"); rewrite(out);
 move(mem[0xa000*0],mem[0xa000*pagetop],16000);
 while (! eof(magic)) 
 {;
  magic >> next;
  if (next!=nextcode) 
   mem[0xa000*next]=255;
  else
  {;
   for( gg=0; gg <= 16000; gg ++)
    if (mem[0xa000*gg]!=mem[0xa000*gg+pagetop]) 
     out << gg;
   out << nextcode;
   move(mem[0xa000*0],mem[0xa000*pagetop],16000);
  }
 }
 close(magic); close(out);
return EXIT_SUCCESS;
}
