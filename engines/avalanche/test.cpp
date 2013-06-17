/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*$M 2048,0,0*/
/*#include "Dos.h"*/

const array<1,22,char> signature = string("*AVALOT* v1.00 ±tt± ")+'\3'+'\0';
pointer saveint1f;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 getintvec(0x1f,saveint1f);
 setintvec(0x1f,&signature);
 swapvectors;
 exec("c:\\command.com","");
 swapvectors;
 setintvec(0x1f,saveint1f);
return EXIT_SUCCESS;
}
