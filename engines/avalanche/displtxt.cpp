


/*#include "Crt.h"*/


const char fn[] = "text3.scr";

typedef array<1,3840,char> atype;

file<atype> f;
word fv,ff,fq,st;
char r;
byte tl,bl;
atype q;
atype a /*absolute $B800:0*/;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=0; clrscr;
 assign(f,fn); reset(f); f >> q; close(f);
 for( fv=1; fv <= 40; fv ++)
 {;
  if (fv>36)  {; tl=1; bl=24; }
   else {; tl=12-fv / 3; bl=12+fv / 3; }
  for( fq=tl; fq <= bl; fq ++)
   for( ff=80-fv*2; ff <= 80+fv*2; ff ++)
    a[fq*160-ff]=q[fq*160-ff];
  delay(5);
 }
 gotoxy(1,25); textattr=31; clreol; gotoxy(1,24);
return EXIT_SUCCESS;
}
