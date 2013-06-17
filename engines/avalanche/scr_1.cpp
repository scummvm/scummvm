


/*#include "Crt.h"*/


const varying_string<4> codes = " ﬂ‹€";

typedef matrix<'\0','\377',0,15,byte> fonttype;

byte x,xx,y;
string title,fn;
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
 assign(f,"TEXT1.SCR");
 rewrite(f); f << a; close(f);
}

void centre(byte y, string z)
{;
 gotoxy(40-length(z) / 2,y); output << z;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
/* write('Title?'); readln(title);
 write('Font?'); readln(fn); */
 textattr=0; clrscr;
 title="Bug Alert!"; fn="";
 for( xx=1; xx <= 77; xx ++)
 {;
  gotoxy(Random(80)+1,Random(24)+1);
  switch (Random(2)) {
   case 0: textattr=red; break;
   case 1: textattr=lightred; break;
  }
  switch (Random(4)) {
   case 0: output << '*'; break;
   case 1: output << '\17'; break;
   case 2: output << '˘'; break;
   case 3: output << '˙'; break;
  }
 }
 textattr=12;
 assign(f,"c:\\thomas\\ttsmall.fnt");
 reset(f); f >> font; close(f);
 for( y=0; y <= 3; y ++)
 {;
  for( x=1; x <= length(title); x ++)
  {;
   for( xx=7; xx >= 0; xx --)
   {;
    code=(byte)(((1 << xx) & font[title[x]][y*2])>0)+
     (byte)(((1 << xx) & font[title[x]][y*2+1])>0)*2;
    gotoxy(1+x*8-xx,y+1);
    if (code>0)  output << codes[code+1-1];
   }
  }
  /*if wherex<>1 then writeln;*/
 }
 textattr=red;
 centre(7,"An internal error has just occurred within the program.");

 textattr=white; gotoxy(26, 9); output << "Error number: ";
  textattr=lightred; output << "   ";
 textattr=white; gotoxy(27,10); output << "at location: ";
  textattr=lightred; output << "           ";
 centre(12,"This screen should never come up...");
 centre(13,"but it just has!");
 textattr=15;
 centre(15,"So, please tell Thorsoft about this as soon as");
 centre(16,"possible, so that we can fix it.");
 textattr=red;
 centre(20,"Thanks...");
 save();
return EXIT_SUCCESS;
}
