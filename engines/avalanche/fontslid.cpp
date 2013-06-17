#include "ptoc.h"


/*#include "Crt.h"*/

typedef matrix<'\0','\377',0,15,byte> fonttype;

fonttype font1;
byte fv;
char r;
file<fonttype> f;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(f,"v:avalot.fnt"); reset(f); f >> font1; close(f);
 for( r='\0'; r <= '\377'; r ++)
 {;
  for( fv= 0; fv <=  3; fv ++) font1[r][fv]=(cardinal)font1[r][fv] >> 1;
  for( fv= 7; fv <=  8; fv ++) font1[r][fv]=font1[r][fv] << 1;
  for( fv= 9; fv <= 14; fv ++) font1[r][fv]=font1[r][fv] << 2;
 }
 assign(f,"v:avitalic.fnt"); rewrite(f); f << font1; close(f);
return EXIT_SUCCESS;
}