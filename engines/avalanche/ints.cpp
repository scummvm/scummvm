

              /* Avalot interrupt handler */
/*#include "Dos.h"*/

/*$F+*/
registers r;
void() old1b;

boolean quicko;

void new1b()     /* interrupt; */
{;
 quicko=true;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 getintvec(0x1b,&old1b);
 setintvec(0x1b,addr(new1b()));
 quicko=false;
 do {; } while (!quicko);
 setintvec(0x1b,&old1b);
/*  r.ah:=$02; intr($16,r);
  writeln(r.al and 12); { Only checks Ctrl and Alt. Both on = 12. }
 until false;*/
return EXIT_SUCCESS;
}
