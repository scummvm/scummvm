/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Crt.h"*/

struct inirex {
          varying_string<12> a;
          word num;
};

text i;
file<inirex> o;
inirex x;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(i,"v:init0.dat"); reset(i);
 assign(o,"v:init.avd"); rewrite(o);

 while (! eof(i)) 
 {;
  i >> x.a >> NL;
  i >> x.num >> NL;
  o << x;
  output << '.';
 }

 close(i); close(o);
 output << NL;
return EXIT_SUCCESS;
}