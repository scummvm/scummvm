/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Dos.h"*/

struct infotype {
            matrix<128,255,1,8,byte> chars;
            string data;
};
infotype table;
pointer where;
byte w,fv,ff,num;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 getintvec(0x1f,where); move(where,table,1024);
 for( w=128; w <= 255; w ++)
 {; output << w << NL;
 for( fv=1; fv <= 8; fv ++)
 {;
  num=table.chars[w][fv];
  for( ff=1; ff <= 8; ff ++)
  {;
   if ((num & 128)==0)  output << "  "; else output << "лл";
   num=num << 1;
  }
  output << NL;
 } }
return EXIT_SUCCESS;
}