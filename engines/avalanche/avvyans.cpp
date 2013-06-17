#include "ptoc.h"


#include "graph.h"
/*#include "Crt.h"*/

integer gd,gm;
untyped_file f;
array<1,16000,byte> aa;
matrix<0,27,0,35,byte> cols;
text t;
string x;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 fillchar(cols,sizeof(cols),'\0');
 assign(f,"v:avvypic.ptx");
 reset(f,1);
 blockread(f,aa,filesize(f));
 close(f);
 putimage(0,0,aa,0);
 for( gd=0; gd <= 27; gd ++)
  for( gm=0; gm <= 34; gm ++)
   cols[gd][gm+1]=getpixel(gd,gm);

 restorecrtmode();
/*
   asm
      mov ax,$1003
      mov bl,0
      int $10
   end;
*/
 for( gm=0; gm <= 17; gm ++)
  for( gd=0; gd <= 27; gd ++)
  {;
   gotoxy(gd+1,gm+1);
   if (cols[gd][2*gm]==cols[gd][2*gm+1]) 
   {;
    textattr=cols[gd][2*gm]; output << 'Û';
   } else
    if ((cols[gd][2*gm]>7) && (cols[gd][2*gm+1]<8)) 
    {;
     textattr=cols[gd][2*gm]+cols[gd][2*gm+1]*16;
     output << 'ß';
    } else
    {;
     textattr=cols[gd][2*gm]*16+cols[gd][2*gm+1];
     if (textattr>blink)  textattr -= blink;
     output << 'Ü';
    }
  }
  gotoxy(23,5); textattr=red; output << 'ß';

  assign(t,"v:avalot.txt"); reset(t);
  textattr=9; gm=2;
  do {
   gm += 1;
   t >> x >> NL;
   gotoxy(30,gm);
   output << x << NL;
  } while (!(eof(t)));

  textattr=7; gotoxy(35,2); output << "Back in good old A.D. ";
  textattr=15; output << "1176"; textattr=7; output << "...";
  textattr=9; gotoxy(40,4); output << "Lord";
  gotoxy(67,9); output << "d'Argent";
  textattr=yellow;
  gotoxy(37,12); output << "He's back...";
  gotoxy(47,14); output << "And this time,";
  gotoxy(52,15); output << "he's wearing tights...";
  textattr=4;
  gotoxy(36,17); output << "A Thorsoft of Letchworth game. * Requires EGA";
  gotoxy(38,18); output << "and HD. * By Mike, Mark and Thomas Thurman.";
  gotoxy(40,19); output << "Sfx archive- ";
  textattr=9; output << "Download ";
  textattr=14; output << "AVLT10.EXE";
  textattr=9; output << " now!";
  gotoxy(1,1);
  input >> NL;
return EXIT_SUCCESS;
}