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

                 GYRO             It all revolves around this bit! */

#define __gyro_implementation__


#include "gyro.h"


#include "pingo.h"
#include "scrolls.h"
#include "lucerna.h"
#include "visa.h"
#include "acci.h"
#include "trip5.h"
#include "dropdown.h"
#include "basher.h"

namespace Avalanche {

const array<'\1',numobjs,varying_string<15> > things = 
 {{"Wine","Money-bag","Bodkin","Potion","Chastity belt",
 "Crossbow bolt","Crossbow","Lute","Pilgrim's badge","Mushroom","Key",
 "Bell","Scroll","Pen","Ink","Clothes","Habit","Onion"}};

const array<'\1',numobjs,char> thingchar = "WMBParCLguKeSnIohn"; /* V=Vinegar */

const array<'\1',numobjs,varying_string<17> > better = 
 {{"some wine","your money-bag","your bodkin","a potion","a chastity belt",
  "a crossbow bolt","a crossbow","a lute","a pilgrim's badge","a mushroom",
  "a key","a bell","a scroll","a pen","some ink","your clothes","a habit",
  "an onion"}};

const array<'\1',numobjs,char> betterchar = "WMBParCLguKeSnIohn";

void newpointer(byte m)
{
 if (m==cmp)  return; cmp=m;
 {
  ax=9; bx=(word)(mps[m].horzhotspot); cx=(word)(mps[m].verthotspot);
  es=seg(mps[m].mask); dx=ofs(mps[m].mask);
 }
 intr(0x33,r);
 load_a_mouse(m);
}

void wait()     /* makes hourglass */
{
 newpointer(5);
}

void on()
{
 if (set_of_enum(<null>)::of(m_yes,m_virtual, eos).has(visible))  return;

 r.ax=1; intr(0x33,r); visible=m_yes;
 if (oncandopageswap) 
 {
  r.ax=29; r.bx=cp; intr(0x33,r);  /* show mouse on current page */
 }
}

void on_virtual()
{
 switch (visible) {
  case m_virtual: return; break;
  case m_yes: off(); break;
 }

 visible=m_virtual;
}

void off()
{
 switch (visible) {
  case m_no:case m_virtual : return; break;
  case m_yes : { r.ax=2; intr(0x33,r); } break;
 }
 visible=m_no;
}

void off_virtual()
{
    byte fv;

 if (visible!=m_virtual)  return;

 for( fv=0; fv <= 1; fv ++)
 {
  setactivepage(fv);
  wipe_vmc(1-fv);
 }
 setactivepage(1-cp); visible=m_no;
}

void xycheck()     /* only updates mx & my, not all other mouse vars */
{
 r.ax=3; intr(0x33,r); /* getbuttonstatus */
 {
  keystatus=bx;
  mx=cx; my=dx;
 }
}

void hopto(integer x,integer y) /* Moves mouse pointer to x,y */
{
 {
  ax=4;
  cx=x;
  dx=y;
 }
 intr(0x33,r);
}

void check()
{
           { ax=6; bx=0; } intr(0x33,r);         /* getbuttonreleaseinfo */
 {
  mrelease=bx;
  mrx=cx; mry=dx;
 }
           { ax=5; bx=0; } intr(0x33,r);         /* getbuttonpressinfo */
 {
  mpress=bx;
  mpx=cx; mpy=dx;
 }
 xycheck(); /* just to complete the job. */
}

void note(word hertz)
{
 if (soundfx)  sound(hertz);
}

void blip()
{
    byte fv;

 for( fv=1; fv <= 7; fv ++) { sound(177+(fv*200) % 177); delay(1); }
 nosound;
}

string strf(longint x)
{
    string q;

 string strf_result;
 str(x,q); strf_result=q;
 return strf_result;
}

void shadow(integer x1,integer y1,integer x2,integer y2, byte hc,byte sc)
{
    byte fv;

 for( fv=0; fv <= border; fv ++)
 {
  setfillstyle(1,hc);
  bar(x1+fv,y1+fv,x1+fv,y2-fv);
  bar(x1+fv,y1+fv,x2-fv,y1+fv);

  setfillstyle(1,sc);
  bar(x2-fv,y1+fv,x2-fv,y2-fv);
  bar(x1+fv,y2-fv,x2-fv,y2-fv);
 }
}

void shbox(integer x1,integer y1,integer x2,integer y2, string t)
{
      const integer fc = 7;

 off();
 shadow(x1,y1,x2,y2,15,8); setfillstyle(1,fc);
 bar(x1+border+1,y1+border+1,x2-border-1,y2-border-1);
 setcolor(1); x1=(x2-x1) / 2+x1; y1=(y2-y1) / 2+y1;
 outtextxy(x1,y1,t);
 if (length(t)>1) 
 {
  fillchar(t[2],length(t)-1,'\40'); t[1]='_';
  outtextxy(x1-1,y1+1,t);
 }
 on();
}

void newgame()     /* This sets up the DNA for a completely new game. */
{
    byte gd,gm;

 for( gm=1; gm <= numtr; gm ++)
  { triptype& with = tr[gm]; 
  if (with.quick)  done();} /* Deallocate sprite. Sorry, beta testers! */

 tr[1].init(0,true);
 alive=true;

 score=0;  /*for gd:=0 to 5 do which[gd]:=1;*/
 fillchar(dna,sizeof(dna),'\0'); natural();
 normal_edit(); mousepage(0);
 dna.spare_evening="answer a questionnaire";
 dna.like2drink="beer";

 {
  dna.pence=30; /* 2/6 */ dna.rw=stopped; dna.wearing=clothes;
  dna.obj[money]=true; dna.obj[bodkin]=true; dna.obj[bell]=true; dna.obj[clothes]=true;
 }

 thinks='\2'; objectlist();
 ontoolbar=false; seescroll=false;

 ppos[0][1]=-177; /*tr[1].appear(300,117,right);*/ gd=0;
 for( gd=0; gd <= 30; gd ++) for( gm=0; gm <= 1; gm ++) also[gd][gm]=nil;
/* fillchar(previous^,sizeof(previous^),#0); { blank out array } */
 him='\376'; her='\376'; it='\376'; last_person='\376'; /* = Pardon? */
 dna.pass_num=Random(30)+1; after_the_scroll=false;
 dna.user_moves_avvy=false; doing_sprite_run=false;
 dna.avvy_in_bed=true; enid_filename="";

 for( gd=0; gd <= 1; gd ++) { cp=1-cp; getback(); }

 enterroom(1,1); new_game_for_trippancy();
 showscore();

 standard_bar(); clock();
 sprite_run();
}

void click()     /* "Audio keyboard feedback" */
{
 sound(7177); delay(1); nosound;
}

void slowdown()
{
/* repeat until TSkellern>=howlong; TSkellern:=0;*/
 do {; } while (!(memw[storage_seg*skellern]>=howlong));
 memw[storage_seg*skellern]=0;
}

boolean flagset(char x)
{
 boolean flagset_result;
 flagset_result=pos(x,flags)>0;
 return flagset_result;
}

void force_numlock()
{
 if ((locks & num)>0)  locks -= num;
}

boolean pennycheck(word howmuchby)
{
 boolean pennycheck_result;
 dna.pence -= howmuchby;
 if (dna.pence<0) 
 {
  dixi('Q',2); /* "you are now denariusless!" */
  pennycheck_result=false;
  gameover();
 } else
  pennycheck_result=true;
 return pennycheck_result;
}

string getname(char whose)
{
 string getname_result;
 if (whose<'\257')  getname_result=lads[whose]; else getname_result=lasses[whose];
 return getname_result;
}

char getnamechar(char whose)
{
 char getnamechar_result;
 if (whose<'\257')  getnamechar_result=ladchar[whose-1];
  else getnamechar_result=lasschar[whose-1];
 return getnamechar_result;
}

string get_thing(char which)
{
  string get_thing_result;
  switch (which) {
   case wine: switch (dna.winestate) {
          case 1:case 4: get_thing_result=things[which]; break;
          case 3: get_thing_result="Vinegar"; break;
         }
         break;
   case onion: if (dna.rotten_onion) 
           get_thing_result="rotten onion";
          else get_thing_result=things[which];
          break;
   default: get_thing_result=things[which];
  }
  return get_thing_result;
}

char get_thingchar(char which)
{
  char get_thingchar_result;
  switch (which) {
   case wine: if (dna.winestate==3)  get_thingchar_result='V'; /* Vinegar */
          else get_thingchar_result=thingchar[which-1];
          break;
   default: get_thingchar_result=thingchar[which-1];
  }
  return get_thingchar_result;
}

string get_better(char which)
{
 string get_better_result;
 if (which>'\226')  which -= 149;
  switch (which) {
   case wine: switch (dna.winestate) {
          case 0:case 1:case 4: get_better_result=better[which]; break;
          case 3: get_better_result="some vinegar"; break;
         }
         break;
   case onion: if (dna.rotten_onion) 
           get_better_result="a rotten onion";
           else if (dna.onion_in_vinegar) 
            get_better_result="a pickled onion (in the vinegar)";
             else get_better_result=better[which];
             break;
   default:
     if ((which<numobjs) && (which>'\0')) 
        get_better_result=better[which]; else
        get_better_result="";
  }
 return get_better_result;
}

string f5_does()
 /* This procedure determines what f5 does. */
{
  string f5_does_result;
  switch (dna.room) {
   case r__yours:
    {
     if (! dna.avvy_is_awake) 
     {     /* He's asleep, =>= wake up. */
      f5_does_result=string(vb_wake)+"WWake up";
      return f5_does_result;
     }

     if (dna.avvy_in_bed) 
     {     /* In bed. => = get up. */
      f5_does_result=string(vb_stand)+"GGet up";
      return f5_does_result;
     }

    }
    break;

   case r__insidecardiffcastle:
    {
     if (dna.standing_on_dais) 
      f5_does_result=string(vb_climb)+"CClimb down";
     else
      f5_does_result=string(vb_climb)+"CClimb up";
     return f5_does_result;
    }
    break;

   case r__nottspub:
    {
     if (dna.sitting_in_pub) 
      f5_does_result=string(vb_stand)+"SStand up";
     else
      f5_does_result=string(vb_sit)+"SSit down";
     return f5_does_result;
    }
    break;

   case r__musicroom:
    if (infield(7)) 
    {
     f5_does_result=string(vb_play)+"PPlay the harp";
     return f5_does_result;
    }
    break;
  }

 f5_does_result=pardon; /* If all else fails... */
  return f5_does_result;
}

void plot_vmc(integer xx,integer yy, byte page_)
{
 if (visible!=m_virtual)  return;
 {
  xx=xx+vmc.ofsx; if (xx<0)  xx=0;
  yy=yy+vmc.ofsy; if (yy<0)  yy=0;

  setactivepage(1-cp);
  getimage(xx,yy,xx+15,yy+15,vmc.backpic[page_]);
  putimage(xx,yy,vmc.andpic,andput);
  putimage(xx,yy,vmc.xorpic,xorput);

/*  setcolor( 0); outtextxy(xx+8,yy+16,'€'); outtextxy(xx,yy+16,'€');
  setcolor(11+page);
  outtextxy(xx+8,yy+16,chr(48+roomtime mod 10));
  outtextxy(xx  ,yy+16,chr(48+(roomtime div 10) mod 10));*/

  {
   pointtype& with1 = vmc.wherewas[page_]; 

   with1.x=xx;
   with1.y=yy;
  }
 }
}

void wipe_vmc(byte page_)
{
 if (visible!=m_virtual)  return;
  { pointtype& with1 = vmc.wherewas[page_]; 
   if (with1.x!=maxint) 
    putimage(with1.x,with1.y,vmc.backpic[page_],0);}
}

void setup_vmc()
{
    byte fv;

 {
  getmem(vmc.andpic,mouse_size);
  getmem(vmc.xorpic,mouse_size);

  for( fv=0; fv <= 1; fv ++)
  {
   getmem(vmc.backpic[fv],mouse_size);
   vmc.wherewas[fv].x=maxint;
  }
 }
}

void clear_vmc()
{
    byte fv;

 for( fv=0; fv <= 1; fv ++) vmc.wherewas[fv].x=maxint;
}

void setminmaxhorzcurspos(word min,word max)  /* phew */
{
 {
  ax=7;
  cx=min;
  dx=max;
 }
 intr(0x33,r);
}

void setminmaxvertcurspos(word min,word max)
{
 {
  ax=8;  /* A guess. In the book, 7 */
  cx=min;
  dx=max;
 }
 intr(0x33,r);
}

void load_a_mouse(byte which)
{
    untyped_file f;

 assign(f,"mice.avd");
 reset(f,1);
 seek(f,mouse_size*2*(which-1)+134);

 {
  blockread(f,vmc.andpic,mouse_size);
  blockread(f,vmc.xorpic,mouse_size);
  close(f);
  {
   mp& with1 = mps[which]; 

   vmc.ofsx=-with1.horzhotspot;
   vmc.ofsy=-with1.verthotspot;

   setminmaxhorzcurspos(with1.horzhotspot+3,624+with1.horzhotspot);
   setminmaxvertcurspos(with1.verthotspot,199);
  }
 }
}

void background(byte x) { setbkcolor(x); }

void hang_around_for_a_while()
{
    byte fv;

 for( fv=1; fv <= 28; fv ++) slowdown();
}

boolean mouse_near_text()
{
 boolean mouse_near_text_result;
 mouse_near_text_result=(my>144) && (my<188);
 return mouse_near_text_result;
}

/* Super_Off and Super_On are two very useful procedures. Super_Off switches
  the mouse cursor off, WHATEVER it's like. Super_On restores it again
  afterwards. */

void super_off()
{
 super_was_off=visible==m_no;
 if (super_was_off)  return;

 super_was_virtual=visible==m_virtual;

 if (visible==m_virtual)  off_virtual(); else off();
}

void super_on()
{
 if ((visible!=m_no) || (super_was_off))  return;

 if (super_was_virtual)  on_virtual(); else on();
}

} // End of namespace Avalanche.