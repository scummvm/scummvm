

#define __cadburys_implementation__


#include "cadburys.h"


#include "graph.h"



void mgrab(integer x1,integer y1,integer x2,integer y2, word size)
{
    integer yy; word aapos; byte length,bit;
;
 if (size>arraysize) 
 {;
  output << "*** SORRY! *** Increase the arraysize constant to be greater" << NL;
  output << " than " << size << '.' << NL;
  exit(0);
 }

 aapos=0;

 length=x2-x1;

 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  for( yy=y1; yy <= y2; yy ++)
  {;
   move(mem[0xa400*yy*80+x1],aa[aapos],length);
   aapos += length;
  }
 }
 bit=getpixel(0,0);

}

void grab(integer x1,integer y1,integer x2,integer y2,integer realx,integer realy, flavourtype flav,
 boolean mem,boolean nat)
 /* yes, I *do* know how to spell "really"! */
{
 word s;
 pointer p;
 chunkblocktype ch;
;
/* rectangle(x1,y1,x2,y2); exit;*/
 this_chunk += 1;
 offsets[this_chunk]=filepos(f);


 s=imagesize(x1,y1,x2,y2);
 getmem(p,s);
 getimage(x1,y1,x2,y2,p);

 {;
  ch.flavour=flav;
  ch.x=realx; ch.y=realy;

  ch.xl=x2-x1;
  ch.yl=y2-y1;
  ch.size=s;
  ch.memorise=mem;
  ch.natural=nat;
 }

 setvisualpage(1);
 setactivepage(1);
 input >> NL;
 putimage(ch.x,ch.y,p,0);

 if (flav==ch_ega) 
 {;
  freemem(p,s);
  s=4*(((x2 / 8)-(x1 / 8))+2)*(y2-y1+1);
  {;
   ch.size=s;
   ch.x=ch.x / 8;
   ch.xl=((realx-ch.x*8)+(x2-x1)+7) / 8;
   mgrab(ch.x,ch.y,ch.x+ch.xl,ch.y+ch.yl,s);
  }
 } else
             /* For BGI pictures. */
  {;
   ch.x=ch.x / 8;
   ch.xl=(ch.xl+7) / 8;
   ch.size=imagesize(ch.x*8,ch.y,(ch.x+ch.xl)*8,ch.y+ch.yl);
  }

 input >> NL;
 setvisualpage(0);
 setactivepage(0);

 blockwrite(f,ch,sizeof(ch));

 switch (flav) {
  case ch_ega : if (! nat)  blockwrite(f,aa,s); break;
  case ch_bgi : {;
            if (! nat)  blockwrite(f,p,s);
            freemem(p,s);
           }
           break;
 }
/* rectangle(x1,y1,x2,y2);*/
}


