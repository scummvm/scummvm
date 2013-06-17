/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Enhanced.h"*/

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 do {
  readkeye;
  switch (inchar) {
   case '\0': output << string('[')+extd+']'; break;
   case '\340': output << string('<')+extd+'>'; break;
   default: output << inchar;
  }
 } while (!(inchar=='\33'));
return EXIT_SUCCESS;
}