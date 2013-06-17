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

                 HELPER           The help system unit. */

#define __helper_implementation__


#include "helpbak.h"


/*#include "Crt.h"*/
/*#include "Lucerna.h"*/


struct buttontype {
             char trigger;
             byte whither;
};

const integer buttonsize = 930;

const integer toolbar = 0; const integer nim = 1; const integer kbd = 2; const integer credits = 3; const integer joystick = 4; const integer troubleshooting = 5; const integer story = 6;
const integer mainscreen = 7; const integer registering = 8; const integer sound = 9; const integer mouse = 10; const integer filer = 11; const integer back2game = 12;
const integer helponhelp = 13; const integer pgdn = 14; const integer pgup = 15;

array<1,10,buttontype> buttons;

void plot_button(integer y, byte which)
{
 untyped_file f;
 pointer p;

 getmem(p,buttonsize);
 assign(f,"buttons.avd");
 reset(f,1);
 seek(f,which*buttonsize);
 blockread(f,p,buttonsize);
 putimage(470,y,p,0);
 close(f);
 freemem(p,buttonsize);
}

void getme(byte which);


static void chalk(byte y, string z)
{
 outtextxy(16,41+y*10,z);
}

void getme(byte which)       /* Help icons are 80x20 */
{
 string x;
 text t;
 byte y,fv;


 str(which,x);
 assign(t,string('h')+x+".raw");
 y=0;
 reset(t);

 t >> x >> NL;
 setfillstyle(1,1); bar(0,0,640,200);
 setfillstyle(1,15); bar(8,40,450,200);
 settextjustify(1,1); setcolor(14);
 outtextxy(320,15,x);
 settextjustify(0,2);
 setcolor(3); settextstyle(0,0,2);
 outtextxy(550,0,"help!");
 /***/ setcolor(0); settextstyle(0,0,1);

 do {
  t >> x >> NL;
  if (x=='!')  flush(); /* End of the help text is signalled with a !. */
  chalk(y,x);
  y += 1;
 } while (!false);

 /* We are now at the end of the text. Next we must read the icons. */

 y=0; settextjustify(1,1); setcolor(3);
 while (! eof(t)) 
 {
  y += 1;
  t >> x >> NL; /* Get the name of this button, and... */
  if (x!='-')    /* (hyphen signals an empty button.) */
  {
   t >> buttons[y].trigger >> NL;
   t >> fv >> NL; plot_button(13+y*27,fv);
   t >> buttons[y].whither >> NL; /* this is the position to jump to */

   switch (buttons[y].trigger) {
     case '˛' : outtextxy(580,25+y*27,"Esc"); break;
     case '÷' : outtextxy(580,25+y*27,"\30"); break;
     case 'ÿ' : outtextxy(580,25+y*27,"\31"); break;
   default:
    outtextxy(580,25+y*27,buttons[y].trigger);
   }

  } else buttons[y].trigger='\0';
 }

 settextjustify(0,2);
 close(t);
}

void continue_help()
{
 char r;
 byte fv;

 do {
  r=upcase(readkey());
  switch (r) {
   case '\33' : return; break;
   case '\0'  : switch (readkey()) {
          case '\110':case '\111': r='÷'; break;
          case '\120':case '\121': r='ÿ'; break;
          case '\73': r='H'; break; /* Help on help */
         }
         break;
  }

  for( fv=1; fv <= 10; fv ++)
   { buttontype& with = buttons[fv]; 
    if (with.trigger==r) 
    {
     dusk;
     getme(with.whither);
     dawn;
     flush();
    }}

 } while (!false);
}

void boot_help()
{
 setactivepage(2);

 getme(0);

 dusk;
 setvisualpage(2);
 dawn;

 continue_help();

 dusk;
 setvisualpage(cp);
 draw_also_lines;
 setactivepage(1-cp);
 dawn;
}

