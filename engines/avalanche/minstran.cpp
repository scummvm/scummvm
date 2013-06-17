#include "ptoc.h"


#include "graph.h"

integer gd,gm;
pointer p; word s;

void mblit(integer x1,integer y1,integer x2,integer y2) /* Minstrel Blitter */
{
    integer yy,len,pp; byte bit; const integer offset = 16384;
;
 x1=x1 / 8; len=((x2 / 8)-x1)+1;
 for( yy=y1; yy <= y2; yy ++)
 {;
  pp=yy*80+x1;
  for( bit=0; bit <= 3; bit ++)
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   move(mem[0xa000*offset+pp],mem[0xa000*pp],len);
  }
 }
}

const integer fx1 = 100; const integer fy1 = 100; const integer fx2 = 135; const integer fy2 = 145;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 setactivepage(1); setfillstyle(7,9); bar(0,0,640,200);
 mblit(fx1,fy1,fx2,fy2);
 s=imagesize(fx1,fy1,fx2,fy2); getmem(p,s);
 getimage(fx1,fy1,fx2,fy2,p); setactivepage(0);
 putimage(fx1+100,fy1,p,0); freemem(p,s);
return EXIT_SUCCESS;
}
