/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



#include "graph.h"
/*#include "Tommys.h"*/
/*#include "Crt.h"*/


enum flavourtype {ch_ega,ch_bgi, last_flavourtype};

struct chunkblocktype {
                  flavourtype flavour;
                  integer x,y;
                  integer xl,yl;
                  longint size;
                  boolean natural;

                  boolean memorise; /* Hold it in memory? */
};

const array<1,44,char> chunkheader = 
string("Chunk-type AVD file, for an Avvy game.")+'\32'+'\261'+'\x30'+'\x1'+'\x75'+'\261';

const integer arraysize = 12000;

array<1,30,longint> offsets;
byte num_chunks,this_chunk;
integer gd,gm;
untyped_file f;
array<0,arraysize,byte> aa;

void finder()
{
    char r; integer x,y;
; 
 setfillstyle(0,0); setcolor(14);
 x=320; y=100; setwritemode(xorput);
 do {
  bar(0,0,200,10);
  outtextxy(0,0,strf(x)+','+strf(y));
  line(x-20,y,x+20,y); line(x,y-20,x,y+20);
  do {; } while (!keypressed());
  line(x-20,y,x+20,y); line(x,y-20,x,y+20);
  switch (readkey()) {
   case '\15': return; break;
   case '8': y -= 10; break;
   case '4': x -= 10; break;
   case '6': x += 10; break;
   case '2': y += 10; break;
    case '\0': switch (readkey()) {
         case cup: y -= 1; break;
         case cdown: y += 1; break;
         case cleft: x -= 1; break;
         case cright: x += 1; break;
        }
        break;
  }
 } while (!false);
}

void load()
{
 byte a1 /*absolute $A400:800*/;
 byte bit;
 untyped_file f;
;

 assign(f,"place15.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a1,12080);
 }

 close(f);
 bit=getpixel(0,0);
 setvisualpage(1); setactivepage(1);
 finder();
 setvisualpage(0); setactivepage(0);
}

void loadtemp()
{
 byte a0 /*absolute $A000:800*/;
 byte bit;
 untyped_file f;
;

 assign(f,"d:chunkbi3.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a0,12080);
 }
 close(f);

 bit=getpixel(0,0);

 finder();
}

void open_chunk()
{;
 assign(f,"chunk15.avd");
 rewrite(f,1);
 blockwrite(f,chunkheader,sizeof(chunkheader));
 blockwrite(f,num_chunks,1);
 blockwrite(f,offsets,num_chunks*4);

 this_chunk=0;
}

void close_chunk()
{;
 seek(f,45);
 blockwrite(f,offsets,num_chunks*4); /* make sure they're right! */
 close(f);
}

void mgrab(integer x1,integer y1,integer x2,integer y2, word size)
{
    integer yy; word aapos; byte length,bit;
;
 if (size>arraysize) 
 {;
  restorecrtmode();
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
   ch.xl=(ch.xl / 8)+2;
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

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 load();
 loadtemp();
 setwritemode(xorput);

 num_chunks=3;

 open_chunk();

 grab(535, 11,633, 74, 46, 97,ch_ega,false,false); /*1 Door half-open. */
 grab(539, 76,637,139, 46, 97,ch_ega,false,false); /*2 Door open. */
 grab(539, 76,637,139, 46, 97,ch_bgi,true ,true ); /*3 Door shut. */
 close_chunk();
return EXIT_SUCCESS;
}
