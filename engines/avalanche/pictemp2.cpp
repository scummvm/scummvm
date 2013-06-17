/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



                  /* Get 'em back! */
#include "graph.h"


const integer picsize = 966;
const integer number_of_objects = 17;

const array<1,65,char> thinks_header = 
 string("This is an Avalot file, which is subject to copyright. Have fun.")+'\32';

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
 assign(f,"d:thingtmp.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a0,12080);
 }
 close(f); bit=getpixel(0,0);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 load(); noo=0;

 assign(f,"thinks.avd");
 getmem(p,picsize);
 rewrite(f,1);
 blockwrite(f,thinks_header,65);
 gd=10; gm=20;

 while (noo<=number_of_objects) 
 {;
  getimage(gd,gm,gd+59,gm+29,p);
  putimage(gd,gm,p,notput);
  blockwrite(f,p,picsize);
  gd += 70;

  if (gd==640) 
  {;
   gd=10; gm += 40;
  }

  noo += 1;
 }

 close(f); freemem(p,picsize);
return EXIT_SUCCESS;
}
