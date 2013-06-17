#include "ptoc.h"


file<char> inf,outf;
char x;
string q;
byte fv;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(inf,"v:thank.you");
 assign(outf,"d:hibits.out");
 reset(inf); rewrite(outf);

 q=string('\40')+"(Seven is a bit of a lucky number.)"+'\40'+'\215'+'\212'+'\212';

 for( fv=1; fv <= length(q); fv ++) outf << q[fv];

 while (! eof(inf)) 
 {;
  inf >> x;
  if (x<'\200')  x += 128;
  outf << x;
 }
 close(inf); close(outf);
return EXIT_SUCCESS;
}