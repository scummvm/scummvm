/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Crt.h"*/


const varying_string<4> codes = " ﬂ‹€";

typedef matrix<'\0','\377',0,15,byte> fonttype;

byte x,xx,y;
string qq;
file<fonttype> f;
fonttype font;
byte code;


typedef array<1,3840,byte> atype;

void save()
{
 file<atype> f;
 word fv;
 atype a /*absolute $B800:0*/;
;
 assign(f,"TEXT5.SCR");
 rewrite(f); f << a; close(f);
}

void line(byte cx,byte cy, string title)
{;
 for( y=0; y <= 4; y ++)
 {;
  qq="";
  for( x=1; x <= length(title); x ++)
  {;
   for( xx=7; xx >= 0; xx --)
   {;
    code=(byte)(((1 << xx) & font[title[x]][y*2])>0)+
     (byte)(((1 << xx) & font[title[x]][y*2+1])>0)*2;
    qq=qq+codes[code+1-1];
   }
  }
  gotoxy(cx,cy+y); output << qq;
 }
}

void uline()
{
    byte fv,ff;
;
 for( fv=1; fv <= 10; fv ++)
 {;
  gotoxy(16-fv,21-fv); textattr=fv;
  for( ff=1; ff <= fv; ff ++) output << "ƒÕÕ-";
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=0; clrscr;
 assign(f,"c:\\thomas\\ttsmall.fnt");
 reset(f); f >> font; close(f);
 textattr=11;
 line( 1, 3,"Two at");
 line( 3, 7,"once?!");
 textattr=9; gotoxy(55,4); output << string('\4')+'\377';
 textattr=3; output << "You're trying to run";
 gotoxy(54,5); output << "two copies of Avalot";
 gotoxy(54,6); output << "at once.";
 gotoxy(57,8); output << "Although this "; textattr=9; output << "is";
 textattr=3;
 gotoxy(54,9); output << "usually possible, it's";
 gotoxy(54,10); output << "probably more sensible";
 gotoxy(54,11); output << "to type ";
 textattr=7; output << "EXIT ";
 textattr=3; output << "now, which";
 gotoxy(54,12); output << "should return you to the";
 gotoxy(54,13); output << "first copy in memory.";

 textattr=11; gotoxy(55,15); output << "BUT:"; textattr=9;
 gotoxy(40,16); output << "If you want to run two copies anyway,";
 gotoxy(40,17); output << "or you think that I've got it wrong,";
 gotoxy(40,18); output << "(even computers can make mistakes!) then";
 gotoxy(40,19); output << "try running the game again, but this time";
 gotoxy(40,20); output << "use "; textattr=7; output << "/i ";
 textattr=9; output << "on the command line.";
 uline();

 save();
return EXIT_SUCCESS;
}
