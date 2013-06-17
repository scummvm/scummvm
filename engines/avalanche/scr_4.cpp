/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Crt.h"*/


const varying_string<4> codes = " ÞÝÛ";

typedef matrix<'\0','\377',0,15,byte> fonttype;

byte x,xx,y;
string qq;
file<fonttype> f;
fonttype font;
byte code;

void centre(byte y, string z)
{
    byte fv;
;
 for( fv=1; fv <= length(z); fv ++)
 {;
  gotoxy(39-length(z) / 2+fv,y);
  if (z[fv]!='\40')  output << z[fv];
 }
}

void line(byte cy, string title)
{;
 for( y=0; y <= 6; y ++)
 {;
  qq="";
  for( x=1; x <= length(title); x ++)
  {;
   for( xx=3; xx >= 0; xx --)
   {;
    code=(byte)(((1 << (xx*2)) & font[title[x]][y])>0)+
     (byte)(((1 << (xx*2+1)) & font[title[x]][y])>0)*2;
    qq=qq+codes[code+1-1];
   }
  }
  centre(cy+y,qq);
 }
}

void big_t()
{
 text t;
 string x;
 byte y;
;
 assign(t,"c:\\avalot\\t.txt"); reset(t); y=1;
 while (! eof(t)) 
 {;
  t >> x >> NL;
  gotoxy(1,y); output << x; y += 1;
 }
}

void box(byte x1,byte y1,byte x2,byte y2, string z)
{
    byte fv;
;
 gotoxy(x1,y1); output << z[1]; /* tl */
 gotoxy(x2,y1); output << z[2]; /* tr */
 gotoxy(x1,y2); output << z[3]; /* bl */
 gotoxy(x2,y2); output << z[4]; /* br */
 for( fv=y1+1; fv <= y2-1; fv ++)
 {;
  gotoxy(x1,fv); output << z[5]; /* verticals */
  gotoxy(x2,fv); output << z[6];
 }
 gotoxy(x1+1,y1); for( fv=x1+1; fv <= x2-1; fv ++) output << z[7];
 gotoxy(x1+1,y2); for( fv=x1+1; fv <= x2-1; fv ++) output << z[8];
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=26; clrscr;
 assign(f,"c:\\thomas\\ttsmall.fnt");
 reset(f); f >> font; close(f);
 line(3,"CONGRATULATIONS!");
 textattr=30; box(4,1,74,11,"É»È¼ººÍÍ");
 textattr=33; box(6,2,72,10,"/\\\\/°°Üß");
 textattr=30; centre(12,"Well done!");
 textattr=27; centre(14,"You completed the game!");
return EXIT_SUCCESS;
}