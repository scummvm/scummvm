


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