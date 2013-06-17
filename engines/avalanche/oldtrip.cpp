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
  ÛßÜ ÛßÜ ÜßßÜ  ßÛß Ûßß  Üß ßÛß      ÜßÛßÜ  ßÛß ÛÜ  Û ÜÛßß  ßÛß ÛßÜ Ûßß Û
  Ûß  ÛÛ  Û  Û Ü Û  Ûßß ßÜ   Û      Û  Û  Û  Û  Û ÛÜÛ  ßßßÜ  Û  ÛÛ  Ûßß Û
  ß   ß ß  ßß   ßß  ßßß   ß  ß      ß  ß  ß ßßß ß  ßß  ßßß   ß  ß ß ßßß ßßß

                 TRIP4            Trippancy IV- "Trip Oop". */

#define __trip4_implementation__
            /* Trippancy IV (Trip Oop) */
#include "oldtrip.h"


/*#include "Scrolls.h"*/
/*#include "Lucerna.h"*/
/*#include "Gyro.h"*/
/*#include "Dropdown.h"*/


void copier(integer x1,integer y1,integer x2,integer y2,integer x3,integer y3,integer x4,integer y4);


static boolean dropin(integer xc,integer yc,integer x1,integer y1,integer x2,integer y2)
/* Dropin returns True if the point xc,yc falls within the 1-2 rectangle. */
{boolean dropin_result;
;
 dropin_result=((xc>=x1) && (xc<=x2) && (yc>=y1) && (yc<=y2));
return dropin_result;
}



static integer lesser(integer a,integer b)
{integer lesser_result;
;
 if (a<b)  lesser_result=a; else lesser_result=b;
return lesser_result;
}



static integer greater(integer a,integer b)
{integer greater_result;
;
 if (a>b)  greater_result=a; else greater_result=b;
return greater_result;
}

void copier(integer x1,integer y1,integer x2,integer y2,integer x3,integer y3,integer x4,integer y4)

{;
 if (dropin(x3,y3,x1,y1,x2,y2)
 || dropin(x3,y4,x1,y1,x2,y2)
 || dropin(x4,y3,x1,y1,x2,y2)
 || dropin(x4,y4,x1,y1,x2,y2)) 
 {;     /* Overlaps */
  mblit(lesser(x1,x3),lesser(y1,y3),greater(x2,x4),greater(y2,y4),1,0);
 } else
 {;     /* Doesn't overlap- copy both of them seperately */
  mblit(x3,y3,x4,y4,1,0); /* backwards- why not...? */
  mblit(x1,y1,x2,y2,1,0);
 }
}

void loadtrip()
{
    byte gm;
;
 for( gm=1; gm <= numtr; gm ++) tr[gm].original();
 tr[1].init(0);
}

byte checkfeet(integer x1,integer x2,integer oy,integer y, byte yl)
{
    byte a,c; integer fv,ff;
byte checkfeet_result;
;
 a=0; setactivepage(2); if (x1<0)  x1=0; if (x2>639)  x2=639;
 if (oy<y) 
  for( fv=x1; fv <= x2; fv ++)
   for( ff=oy+yl; ff <= y+yl; ff ++)
   {;
    c=getpixel(fv,ff);
    if (c>a)  a=c;
   } else
  for( fv=x1; fv <= x2; fv ++)
   for( ff=y+yl; ff <= oy+yl; ff ++)
   {;
    c=getpixel(fv,ff);
    if (c>a)  a=c;
   }
 checkfeet_result=a; setactivepage(1);
return checkfeet_result;
}

void touchcol(byte tc);
static boolean bug; 

static void fr(byte a,byte b) {; fliproom(a,b); bug=false; }

void touchcol(byte tc)
{;
 bug=true;  /* j.i.c. */
 switch (dna.room) {
  case 1: fr(2,3); break;
  case 2: {; /* main corridor */
      switch (tc) {
       case 1: fr(3,1); break; /* to the other corridor */
       case 2: fr(2,1); break; /* to this corridor! Fix this later... */
      }
     }
     break;
  case 3: {; /* turn corridor */
      switch (tc) {
       case 1: fr(2,1); break; /* to the other corridor */
       case 2: fr(12,1); break; /* through Spludwick's door */
      }
     }
     break;
  case 12: fr(3,2); break;
 }
 if (bug) 
 {;
  setactivepage(0);
  display(string('\7')+"Unknown touchcolour ("+strf(tc)+')'+" in "+strf(dna.room)
   +'.'); setactivepage(1); tr[1].bounce();
 }
}

triptype* triptype::init(byte spritenum)
{
    integer gd,gm; word s; untyped_file f; varying_string<2> xx; byte sort,n;
 word bigsize; pointer p,q;
;
 str(spritenum,xx); assign(f,string("c:\\avalot\\sprite")+xx+".avd");
 reset(f,1); seek(f,59);
 blockread(f,a,sizeof(a)); blockread(f,bigsize,2);
 setactivepage(3);
 for( sort=0; sort <= 1; sort ++)
 {;
  mark(q); getmem(p,bigsize);
  blockread(f,p,bigsize);
  off; putimage(0,0,p,0); release(q); n=1;
  { adxtype& with = a; 
   for( gm=0; gm <= (with.num / with.seq)-1; gm ++) /* directions */
    for( gd=0; gd <= with.seq-1; gd ++) /* steps */
    {;
     getmem(pic[n][sort],a.size); /* grab the memory */
     getimage((gm / 2)*(with.xl*6)+gd*with.xl,(gm % 2)*with.yl,
       (gm / 2)*(with.xl*6)+gd*with.xl+with.xl-1,(gm % 2)*with.yl+with.yl-1,
       pic[n][sort]); /* grab the pic */
     n += 1;
   }} on;
 }
 close(f); setactivepage(0);

 x=0; y=0; quick=true; visible=false; getmem(behind,a.size);
 homing=false; ix=0; iy=0; step=0; check_me=a.name=="Avalot";
return this;
}

void triptype::original()
{;
 quick=false;
}

void triptype::getback()
{;
 tax=x; tay=y;
 off; /*getimage(x,y,x+a.xl,y+a.yl,behind^);*/
 mblit(x,y,x+a.xl,y+a.yl,1,3); on;
}

void triptype::andexor()
{
    byte picnum; /* Picnum, Picnic, what ye heck */
;
 picnum=face*a.seq+step+1; off;
 putimage(x,y,pic[picnum][0],andput);
 putimage(x,y,pic[picnum][1],xorput); on;
}

void triptype::turn(byte whichway)
{;
 face=whichway;
}

void triptype::appear(integer wx,integer wy, byte wf)
{;
 x=(wx / 8)*8; y=wy; ox=wx; oy=wy; turn(wf); visible=true; ix=0; iy=0;
}

void triptype::walk()
{
    byte tc;
;
 ox=x; oy=y;
 if ((ix==0) && (iy==0))  return;
 if (homing)  homestep();
 x=x+ix; y=y+iy;
 if (check_me)  {;
  tc=checkfeet(x,x+a.xl,oy,y,a.yl);
  { void& with = magics[tc]; 
   switch (op) {
    case exclaim: blip; break;
    case bounces: bounce(); break;
    case transport: fliproom(hi(data),lo(data)); break;
   }}
 }
/* if x<0 then x:=0; else if x+a.xl>640 then x:=640-a.xl;*/
 if (y<0)  y=0;    /* else if y+a.yl>161 then y:=161-a.yl; */
 step += 1; if (step==a.seq)  step=0; getback();
}

void triptype::bounce()
{; setactivepage(1); putback(); x=ox; y=oy; stopwalk(); return; }

void triptype::do_it()
{;
 if (((ix!=0) || (iy!=0)) & (~ ddm_o.menunow)) 
 {;
  off; copier(ox,oy,ox+a.xl,oy+a.yl,x,y,x+a.xl,y+a.yl);
  putback(); on;
 }
}

void triptype::putback()
{;
/* putimage(tax,tay,behind^,0);*/ mblit(tax,tay,tax+a.xl,tay+a.yl,3,1);
}

void triptype::walkto(integer xx,integer yy)
{;
 speed(xx-x,yy-y); hx=xx; hy=yy; homing=true;
}

void triptype::stophoming()
{;
 homing=false;
}

void triptype::homestep()
{
    integer temp;
;
 if ((hx==x) && (hy==y)) 
 {;     /* touching the target */
  homing=false; return;
 }
 ix=0; iy=0;
 if (hy!=y) 
 {;
  temp=hy-y; if (temp>4)  iy=4; else if (temp<-4)  iy=-4; else iy=temp;
 }
 if (hx!=x) 
 {;
  temp=hx-x; if (temp>4)  ix=4; else if (temp<-4)  ix=-4; else ix=temp;
 }
}

void triptype::speed(shortint xx,shortint yy)
{;
 ix=xx; iy=yy;
 if ((ix==0) && (iy==0))  return; /* no movement */
 if (ix==0) 
 {;     /* No horz movement */
  if (iy<0)  turn(up); else turn(down);
 } else
 {;
  if (ix<0)  turn(left); else turn(right);
 }
}

void triptype::stopwalk()
{;
 ix=0; iy=0; homing=false;
}

void triptype::chatter()
{;
 talkx=x+a.xl / 2; talky=y; talkf=a.fgc; talkb=a.bgc;
}


getsettype* getsettype::init()
{;
 numleft=0;  /* initialise array pointer */
 return this;
}

void remember(fieldtype r)
{;
 numleft += 1;
 gs[numleft]=r;
}

void recall()
{void recall_result;
;
 recall_result=gs[numleft];
 numleft -= 1;
return recall_result;
}

void rwsp(byte t,byte r)
{
      const integer xs = 4; const integer ys = 2;
;
 { triptype& with = tr[t];  switch (r) {
      case up: speed(  0,-ys); break; case down: speed(  0, ys); break; case left: speed(-xs,  0); break;
   case right: speed( xs,  0); break;   case ul: speed(-xs,-ys); break;   case ur: speed( xs,-ys); break;
      case dl: speed(-xs, ys); break;   case dr: speed( xs, ys); break;
  }}
}

void apped(byte trn,byte np)
{;
 {
 triptype& with = tr[trn]; 
 ; { void& with1 = peds[np];  appear(with.x-with.a.xl / 2,with.y-with.a.yl,dir);}
  rwsp(trn,tr[trn].face); }
}

void trippancy();

static boolean allstill()
{
    boolean xxx; byte fv;
boolean allstill_result;
;
 xxx=true;
 for( fv=1; fv <= numtr; fv ++)
  { triptype& with = tr[fv]; 
   if (with.quick && ((with.ix!=0) || (with.iy!=0)))  xxx=false;}
 allstill_result=xxx;
return allstill_result;
}

void trippancy()
{
    byte fv;
;
 if ((ddm_o.menunow) | ontoolbar | seescroll | allstill())  return;
 setactivepage(1);
 for( fv=1; fv <= numtr; fv ++)
  { triptype& with = tr[fv]; 
  if (with.quick) 
  {;
   walk();
   if (with.visible && ((with.ix!=0) || (with.iy!=0)))  andexor();
   do_it();
  }}
 setactivepage(0);
}

void tripkey(char dir);

static void stopwalking()
{;
 tr[1].stopwalk(); dna.rw=stopped;
}

void tripkey(char dir)
{;
 { triptype& with = tr[1]; 
  {;
   switch (dir) {
    case 'H': if (rw!=up)    
            {; rw=up;    rwsp(1,rw); } else stopwalking();
            break;
    case 'P': if (rw!=down)  
            {; rw=down;  rwsp(1,rw); } else stopwalking();
            break;
    case 'K': if (rw!=left)  
            {; rw=left;  rwsp(1,rw); } else stopwalking();
            break;
    case 'M': if (rw!=right) 
            {; rw=right; rwsp(1,rw); } else stopwalking();
            break;
    case 'I': if (rw!=ur)    
            {; rw=ur;    rwsp(1,rw); } else stopwalking();
            break;
    case 'Q': if (rw!=dr)    
            {; rw=dr;    rwsp(1,rw); } else stopwalking();
            break;
    case 'O': if (rw!=dl)    
            {; rw=dl;    rwsp(1,rw); } else stopwalking();
            break;
    case 'G': if (rw!=ul)    
            {; rw=ul;    rwsp(1,rw); } else stopwalking();
            break;
    case 'L': stopwalking(); break;
   }
 }}
}

void fliproom(byte room,byte ped)
{;
 dusk; tr[1].putback(); dna.room=room; load(room); apped(1,ped);
 oldrw=dna.rw; dna.rw=tr[1].face; showrw; dawn;
}

boolean infield(byte x)           /* returns True if you're within field "x" */
{
    integer ux,uy;
boolean infield_result;
;
 {
 triptype& with = tr[1]; 
 ;
  ux=with.x;
  uy=with.y+with.a.yl;
 }
 {
 void& with = fields[x]; 
 ;
  infield_result=(ux>=x1) && (ux<=x2) && (uy>=y1) && (uy<=y2);
 }
return infield_result;
}

boolean neardoor()         /* returns True if you're near a door! */
{
    integer ux,uy; byte fv; boolean nd;
boolean neardoor_result;
;
 if (numfields<9) 
 {;     /* there ARE no doors here! */
  neardoor_result=false;
  return neardoor_result;
 }
 {
 triptype& with = tr[1]; 
 ;
  ux=with.x;
  uy=with.y+with.a.yl;
 } nd=false;
 for( fv=9; fv <= numfields; fv ++)
  {
  void& with = fields[fv]; 
  ;
   if ((ux>=x1) && (ux<=x2) && (uy>=y1) && (uy<=y2))  nd=true;
  }
 neardoor_result=nd;
return neardoor_result;
}

