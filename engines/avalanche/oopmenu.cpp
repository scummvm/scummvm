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

#define __oopmenu_implementation__


#include "oopmenu.h"


/*#include "Crt.h"*/
#include "graph.h"
/*#include "Rodent.h"*/
/*#include "Dos.h"*/


const integer indent = 40;
const integer spacing = 83;

const integer menu_b = blue;
const integer menu_f = yellow;
const integer menu_border = black;
const integer highlight_b = lightblue;
const integer highlight_f = yellow;
const integer disabled = lightgray;

/* menu_b = lightgray;
 menu_f = black;
 menu_border = black;
 highlight_b = black;
 highlight_f = white;
 disabled = darkgray;*/

/* Built-in mouse routine */

char r;
byte fv;

void chalk(integer x,integer y, char t, string z)
{
    byte p;
;
 outtextxy(x,y,z);
 p=pos(t,z); if (p==0)  return; p -= 1;
 outtextxy(x+p*8,y+1,"_");
}

void say(integer x,integer y, char t, string z, byte f,byte b)
{;
 settextjustify(0,2); setfillstyle(1,b); setcolor(f);
 bar(x-3,y-1,x+textwidth(z)+3,y+textheight(z)+1);
 chalk(x,y,t,z);
}

void mblit(integer x1,integer y1,integer x2,integer y2, byte f,byte t) /* NOT The Minstrel Blitter */
{
    pointer p,q; word s;
;
 mark(q);
 s=imagesize(x1,y1,x2,y2); getmem(p,s);
 setactivepage(f); getimage(x1,y1,x2,y2,p);
 setactivepage(t); putimage(x1,y1,p,0);
 setactivepage(0); release(q);
}

void onemenu::start_afresh()
{;
 number=0; width=0; firstlix=false; oldy=0; highlightnum=0;
}

onemenu* onemenu::init()
{;
 menunow=false;
 return this;
}

void onemenu::opt(string n, char tr, string key, boolean val)
{
    integer l;
;
 number += 1;
 l=textwidth(n+key)+30; if (width<l)  width=l;
 {
 optiontype& with = oo[number]; 
 ;
  with.title=n;
  with.trigger=tr;
  with.shortcut=key;
  with.valid=val;
 }
}

void onemenu::displayopt(byte y,byte b,byte f,byte d)
{;
 {
 optiontype& with = oo[y]; 
 ;
  if (with.valid)  setcolor(f); else setcolor(d);
  if (b!=177) 
  {;
   setfillstyle(1,b);
   bar(flx1,3+y*10,flx2,12+y*10);
  }
  settextjustify(2,2);
   if (with.shortcut>"")  outtextxy(flx2,4+y*10,with.shortcut);
  settextjustify(0,2);
   chalk(left+3,4+y*10,with.trigger,with.title);
 }
}

void onemenu::display()
{
    byte y;
;
 setfillstyle(1,menu_b); setcolor(menu_border);
 firstlix=true;
 flx1=left-2; flx2=left+width; fly=14+number*10;
 mblit(flx1-3,11,flx2+1,fly+1,0,1);
 menunow=true;

 bar(flx1,12,flx2,fly);
 rectangle(flx1-1,11,flx2+1,fly+1);

 setcolor(menu_f); settextjustify(0,2);
 displayopt(1,highlight_b,highlight_f,177);
 for( y=2; y <= number; y ++)
  { optiontype& with = oo[y];  displayopt(y,177,menu_f,disabled);}
}

void onemenu::wipe()
{;
 { headtype& with = m.ddms[o.menunum];  say(flx1+2,1,with.trigger,with.title,menu_f,menu_b);}
 mblit(flx1-3,11,flx2+1,fly+1,1,0);
 menunow=false; firstlix=false;
}

void onemenu::movehighlight(shortint add)
{
    shortint hn;
;
 if (add!=0) 
 {;
  hn=highlightnum+add;
  if ((hn<0) || (hn>=(unsigned char)number))  return;
  highlightnum=hn;
 }
 hidemousecursor;
 displayopt(oldy+1,menu_b,menu_f,disabled);
 displayopt(highlightnum+1,highlight_b,highlight_f,disabled);
 showmousecursor;
 oldy=highlightnum;
}

void onemenu::lightup()    /* This makes the menu highlight follow the mouse.*/
{;
 if ((mousex<flx1) || (mousex>flx2)
  || (mousey<=12) || (mousey>fly-3))  return;
 highlightnum=(mousey-12) / 10;
 if (highlightnum==oldy)  return;
 movehighlight(0);
}

void onemenu::select(byte n)      /* Choose which one you want. */
{;
 if (! oo[n+1].valid)  return;
 choicenum=n;
 m.ddms[menunum].do_choose();
 wipe();
}

void onemenu::keystroke(char c)
{
    byte fv;
;
 c=upcase(c);
 for( fv=1; fv <= number; fv ++)
  if (upcase(oo[fv].trigger)==c)  select(fv-1);
}

void bleep()
{;
 sound(177); delay(7); nosound;
}

  headtype* headtype::init
   (char trig,char alttrig, string name, byte p, proc dw,proc dc)
  {;
   trigger=trig; alttrigger=alttrig; title=name;
   position=p; xpos=(position-1)*spacing+indent;
   xright=xpos+textwidth(name)+3;
   do_setup=dw; do_choose=dc;
   return this;
  }

  void headtype::display()
  {;
   say(xpos,1,trigger,title,menu_f,menu_b);
  }

  void headtype::highlight()
  {;
   say(xpos,1,trigger,title,highlight_f,highlight_b);
   {;
    o.left=xpos;
    o.menunow=true; o.menunum=position;
   }
  }

  boolean headtype::extdparse(char c)
  {boolean extdparse_result;
  ;
   if (c!=alttrigger)  {; extdparse_result=true; return extdparse_result; }
   extdparse_result=false;
  return extdparse_result;
  }

  menuset* menuset::init()
  {;
   howmany=0;
   return this;
  }

  void menuset::create(char t, string n, char alttrig, proc dw,proc dc)
  {;
   howmany += 1;
   ddms[howmany].init(t,alttrig,n,howmany,dw,dc);
  }

  void menuset::update()
  {
      byte fv;
  ;
   setfillstyle(1,menu_b); bar(0,0,640,10);
   for( fv=1; fv <= howmany; fv ++) ddms[fv].display();
  }

  void menuset::extd(char c)
  {
      byte fv;
  ;
   fv=1;
   while ((fv<=howmany) && (ddms[fv].extdparse(c)))  fv += 1;
   if (fv>howmany)  return; getmenu(fv*spacing-indent);
  }

  void menuset::getmenu(integer x)
  {
      byte fv;
  ;
   fv=0;
   do {
    fv += 1;
    if ((x>ddms[fv].xpos-3) && (x<ddms[fv].xright)) 
     { headtype& with = ddms[fv]; 
      {;
       if (o.menunow) 
       {;
        wipe(); /* get rid of menu */
        if (o.menunum==with.position)  return; /* click on own highlight */
       }
       highlight(); do_setup();
       return;
      }}
   } while (!(fv>howmany));
  }

void parsekey(char r,char re)
{;
  switch (r) {
   case '\0': {;
        switch (re) {
         case 'K': {;
               wipe();
               m.getmenu((o.menunum-2)*spacing+indent);
              }
              break;
         case 'M': {;
               wipe();
               m.getmenu((o.menunum*spacing+indent));
              }
              break;
         case 'H': movehighlight(-1); break;
         case 'P': movehighlight(1); break;
         default: m.extd(re);
         }
        }
        break;
   case '\33': if (o.menunow)  wipe(); break;
   case '\15': select(o.highlightnum); break;
   default:
   {;
    if (o.menunow)  keystroke(r); else
     kbuffer=kbuffer+r+re;
   }
  }
}

