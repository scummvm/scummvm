#include "ptoc.h"

                  /* Get 'em back! */
#include "graph.h"


const integer picsize = 966;
const integer number_of_objects = 19;

const array<1,65,char> thinks_header = 
 string("This is an Avalot file, which is subject to copyright. Have fun.")+'\32';

const array<0,19,byte> order = 
 {{ 4, 19,  1, 18, 15,  9, 12, 13, 17, 10,  2,  6,  3,  5,  7, 14,
  16,

   0, 11,  8}};

/*
 pAvalot=#150; pSpludwick=#151; pCrapulus=#152; pDrDuck=#153;
 pMalagauche=#154; pFriarTuck=#155; pRobinHood=#156; pCwytalot=#157;
 pduLustie=#158; pDuke=#159; pDogfood=#160; pTrader=#161;
 pIbythneth=#162; pAyles=#163; pPort=#164; pSpurge=#165;
 pJacques=#166;

 pArkata=#175; pGeida=#176; pWiseWoman=#178;
*/

integer gd,gm;
untyped_file f;
pointer p;
byte noo;

void load()
{
 byte a0 /*absolute $A000:1200*/;
 byte bit;
 untyped_file f;
;
 assign(f,"d:folk.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a0,12080);
 }
 close(f); bit=getpixel(0,0);
}

void get_one()
{;

 gd=((order[noo] % 9)*70+10);
 gm=((order[noo] / 9)*40+20);

 getimage(gd,gm,gd+59,gm+29,p);
 putimage(gd,gm,p,notput);
 blockwrite(f,p,picsize);

}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 load(); noo=0;

 assign(f,"folk.avd");
 getmem(p,picsize);
 rewrite(f,1);
 blockwrite(f,thinks_header,65);

 for( noo=0; noo <= number_of_objects; noo ++)
  get_one();

 close(f); freemem(p,picsize);
return EXIT_SUCCESS;
}
