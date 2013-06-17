/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Dos.h"*/

const array<1,22,char> signature = string("*AVALOT* v1.00 ±tt± ")+'\3'+'\0';
pointer getint1f;
array<1,22,char> x;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 getintvec(0x1f,getint1f);
 move(getint1f,x,22);
 if (x==signature) 
  output << "Signature found." << NL; else output << "Signature NOT found!" << NL;
return EXIT_SUCCESS;
}