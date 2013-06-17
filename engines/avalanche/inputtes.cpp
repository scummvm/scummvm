/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



#include "graph.h"
/*#include "Crt.h"*/

typedef matrix<'\0','\377',0,15,byte> fonttype;

integer gd,gm;
fonttype font;
varying_string<79> current;
char r;

void plottext()
{
    byte x,y;
;
 for( y=0; y <= 7; y ++)
 {;
  for( x=1; x <= length(current); x ++)
   mem[0xa000*12880+y*80+x]=font[current[x]][y];
  fillchar(mem[0xa000*12881+y*80+x],79-x,'\0');
 }
}

void loadfont()
{
    file<fonttype> f;
;
 assign(f,"c:\\thomas\\ttsmall.fnt"); reset(f);
 f >> font; close(f);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\turbo");
 loadfont();
 setfillstyle(1,6); bar(0,0,640,200);
 current="";
 do {
  r=readkey();
  current=current+r;
  plottext();
 } while (!false);
return EXIT_SUCCESS;
}
