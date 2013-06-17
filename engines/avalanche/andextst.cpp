#include "ptoc.h"


#include "graph.h"


typedef matrix<1,24,0,1,pointer> adxtype;

integer gd,gm;
word s; untyped_file f; byte n,anim,cp,t;
array<0,0,adxtype> adx;
array<0,1,pointer> back;
integer x;
array<0,1,integer> ox;

void andex(integer x,integer y, byte n,byte num)
{;
 putimage(x,y,adx[num][n][0],andput);
 putimage(x,y,adx[num][n][1],xorput);
}

void loadadx(byte num, string x)
{
    byte n;
;
 assign(f,x);
 reset(f,1); seek(f,59);
 blockread(f,n,1); /* No. of images... */
 for( gd=1; gd <= n; gd ++)
  for( gm=0; gm <= 1; gm ++)
  {;
   blockread(f,s,2); /* size of next image... */
   getmem(adx[num][gd][gm],s);
   blockread(f,adx[num][gd][gm],s); /* next image */
  }
 close(f);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 loadadx(0,"d:sprite0.avd");
 loadadx(1,"d:sprite0.avd");
 gd=3; gm=0; initgraph(gd,gm,"");
 for( gd=0; gd <= 1; gd ++)
 {;
  setactivepage(gd); setfillstyle(6,1); bar(0,0,640,200);
  getmem(back[gd],s);
 }
 x=0; anim=1; cp=0; t=2; setactivepage(0);
 do {
  setactivepage(cp); setvisualpage(1-cp);
  for( gm=0; gm <= 1; gm ++)
  {;
   if (t>0)  t -= 1; else
    putimage(ox[cp],77,back[cp],copyput);
   getimage(x,77,x+31,77+35,back[cp]);
   andex(x,177,anim+6,gm);
   ox[gm][cp]=x; x += 5;
  }
  anim += 1; if (anim==7)  anim=1;cp=1-cp;
 } while (!false);
return EXIT_SUCCESS;
}