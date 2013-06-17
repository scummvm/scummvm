/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Crt.h"*/

text t;
string x;
byte fv;
untyped_file f;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(f,"v:paralogo.scr");
 clrscr;
 reset(f,1);
 blockread(f,mem[0xb800*0],4000);
 close(f);
 gotoxy(1,1); insline;
 gotoxy(1,23); clreol;
return EXIT_SUCCESS;
}
