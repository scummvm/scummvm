


#include "graph.h"


const integer ttp = 81920;
const integer borland = xorput;
const integer mb1 = 2; /* 2 */
const integer mb2 = 4; /* 4 */

integer gd,gm;
pointer p;
word s;

void mblit()
{
    byte bit; longint st;
;
 st=ttp;
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=mb1;
  portw[0x3ce]=copyput*256+3;
  portw[0x3ce]=0x205;
  port[0x3ce]=0x8;
  port[0x3c5]=1 << bit;
  port[0x3cf]=bit;
  move(mem[longint(0xa000)*st],mem[0xa000*0],7200);
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 settextstyle(1,0,7);
 for( gd=0; gd <= 1; gd ++)
 {;
  setactivepage(gd); setcolor(6*gd+6);
  outtextxy(0,0,chr(65+gd));
 }
 s=imagesize(0,0,90,90); setactivepage(0); getmem(p,s);
 getimage(0,0,90,90,p); putimage(100,100,p,0);
 setactivepage(1); getimage(0,0,90,90,p); setactivepage(0);
 putimage(100,100,p,borland);
 mblit();
return EXIT_SUCCESS;
}
