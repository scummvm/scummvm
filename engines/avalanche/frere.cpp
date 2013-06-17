/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Crt.h"*/


typedef array<1,31,byte> tunetype;

const integer lower = 0;
 const integer same = 1;
const integer higher = 2;

const array<1,12,char> keys = "QWERTYUIOP[]";
const array<1,12,word> notes = 
{{196,220,247,262,294,330,350,392,440,494,523,587}};

const tunetype tune = 
{{higher,higher,lower,same,higher,higher,lower,higher,higher,higher,
 lower,higher,higher,
 same,higher,lower,lower,lower,lower,higher,higher,lower,lower,lower,
 lower,same,lower,higher,same,lower,higher}};

byte this_one,last_one;

char pressed;

byte value;

tunetype played;

void store_(byte what)
{;

 move(played[2],played[1],sizeof(played)-1);

 played[31]=what;

}

boolean they_match()
{
    byte fv;
boolean they_match_result;
;

 for( fv=1; fv <= sizeof(played); fv ++)
  if (played[fv]!=tune[fv]) 
  {;
   they_match_result=false;
   return they_match_result;
  }

 they_match_result=true;

return they_match_result;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;

 textattr=30; clrscr; output << NL;

 do {

  pressed=upcase(readkey());

  value=pos(pressed,keys);

  if (value>0) 
  {;

   last_one=this_one;
   this_one=value;

   sound(notes[this_one]);
   delay(100);
   nosound;

   if (this_one<last_one) 
    store_(lower); else

     if (this_one==last_one) 
      store_(same); else

       store_(higher);

   if (they_match()) 
   {;
    textattr=94; clrscr; output << NL;
    output << string('\7')+"It matches!" << NL;
    input >> NL;
    exit(0);
   }

  }

 } while (!(pressed=='\33'));

 output << "*** PROGRAM STOPPED! ***" << NL;
return EXIT_SUCCESS;
}