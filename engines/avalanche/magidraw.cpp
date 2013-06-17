/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



#include "graph.h"
/*#include "Crt.h"*/


const word nextcode = 17717;

integer gd,gm;
untyped_file magic; /* of word;*/
word next;
array<1,16401,word> buffer;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 assign(magic,"v:magic2.avd"); reset(magic,1);
 blockread(magic,buffer,sizeof(buffer));
 close(magic);
/* while not eof(magic) do*/
 for( gd=1; gd <= 16401; gd ++)
 {;
/*  read(magic,next);
  if next<>nextcode then*/
  if (buffer[gd]!=nextcode) 
   mem[0xa000*buffer[gd]]=255;
  else
   delay(1);
 }
/* close(magic);*/
return EXIT_SUCCESS;
}
