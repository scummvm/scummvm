/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 CLOSING          The closing screen and error handler. */

#define __closing_implementation__


#include "closing.h"


#include "gyro.h"
#include "Graph.h"
/*#include "Crt.h"*/
#include "lucerna.h"


typedef array<1,3840,char> scrtype;

scrtype q /*absolute $B8FA:0*/; /* Nobody's using the graphics memory now. */
file<scrtype> f;
pointer exitsave;

void get_screen(byte which)
{;
 closegraph();
 textattr=0; clrscr;
 assign(f,string("text")+strf(which)+".scr"); reset(f); f >> q; close(f);
}

void show_screen()
{
 byte fv,ff,fq, tl,bl;
 scrtype a /*absolute $B800:0*/;
;
 for( fv=1; fv <= 40; fv ++)
 {;
  if (fv>36)  {; tl=1; bl=24; }
   else {; tl=12-fv / 3; bl=12+fv / 3; }
  for( fq=tl; fq <= bl; fq ++)
   for( ff=80-fv*2; ff <= 80+fv*2; ff ++)
    a[fq*160-ff]=q[fq*160-ff];
  delay(5);
 }
 gotoxy(1,25); textattr=31; clreol; gotoxy(1,24);
}

void quit_with(byte which,byte errorlev)
{;
 dusk();
 get_screen(which);
 show_screen(); /* No changes. */
 exit(errorlev);
}

void put_in(string x, word where)
{
    word fv;
;
 for( fv=1; fv <= length(x); fv ++)
  q[1+(where+fv)*2]=x[fv];
}

void end_of_program()

{
 const array<0,11,varying_string<11> > nouns = 
  {{"sackbut","harpsichord","camel","conscience","ice-cream","serf",
   "abacus","castle","carrots","megaphone","manticore","drawbridge"}};

 const array<0,11,varying_string<9> > verbs = 
  {{"haunt","daunt","tickle","gobble","erase","provoke","surprise",
   "ignore","stare at","shriek at","frighten","quieten"}};

 string result;
;
 nosound;
 get_screen(scr_nagscreen);
 result=nouns[Random(12)]+" will "+verbs[Random(12)]+" you";
 put_in(result,1628);
 show_screen(); /* No halt- it's already set up. */
}

/*$F+*/

void bug_handler()
{;
 exitproc=exitsave;

 if (erroraddr!=nil) 
 {;     /* An error occurred! */
  if (exitcode==203) 
   get_screen(scr_ramcram);
  else
  {;
   get_screen(scr_bugalert);
   put_in(strf(exitcode),678); /* 678 = [38,8]. */
   put_in(strf(seg(erroraddr))+':'+strf(ofs(erroraddr)),758); /* 758 = [38,9]. */
  }
  show_screen();
  erroraddr=nil;
 }
}

/*$F-*/

class unit_closing_initialize {
  public: unit_closing_initialize();
};
static unit_closing_initialize closing_constructor;

unit_closing_initialize::unit_closing_initialize() {;
 exitsave=exitproc;
 exitproc=&bug_handler;
}
