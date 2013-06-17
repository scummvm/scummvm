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

#include "graph.h"
/*#include "Dos.h"*/
/*#include "Crt.h"*/
/*#include "Tommys.h"*/

namespace Avalanche {

const integer msize = 100;
const integer flag = -20047;
const integer perm = -1;

const integer avvy_shoot = 87;
const integer facing_right = 88;
const integer facing_left = 94;

const integer avvy_y = 150;

const integer left_margin = 10;
const integer right_margin = 605;

const array<0,6,byte> shooting = {{87,80,81,82,81,80,87}};

const integer stocks = 28;

const integer framedelaymax = 2;

const integer maxrunners = 4;

const integer times_a_second = /*31*/18;

const integer flash_time = 20; /* If flash_time is <= this, the word "time" will flash. */
                 /* Should be about 20. */

/*  --- Scores for various things ---  */

const integer score_for_hitting_face = 3;
const integer bonus_for_hitting_escaper = 5;

struct mtype {
         shortint ix,iy;
         integer x,y;
         byte p;
         integer timeout;
         boolean cameo;
         byte cameo_frame;
         boolean missile;
         boolean wipe;
};

struct xtype {
         word s;
         pointer p;
};

class rectype {
public:
           integer x1,y1,x2,y2;
};

class plottype : public rectype {
public:
            byte which;
};

class plotmasktype : public plottype {
public:
                byte whichmask;
};

array<1,100,xtype> x;
array<1,100,mtype> m;
matrix<0,1,1,100,rectype> r;
array<0,1,byte> rsize;
byte cp;
word score; byte time1;

byte shiftstate;    /*ABSOLUTE $40:$17; */

word avvypos;
word avvywas;
byte avvyanim;
byte avvyfacing;

byte was_facing;

boolean alt_was_pressed_before;

byte throw_next;

boolean firing;

array<0,6,byte> stockstatus;

array<1,maxrunners,struct A1 {
                        integer x,y;
                        byte frame;
                        byte toohigh,lowest;
                        shortint ix,iy;
                        byte framedelay;
                   }> running;

varying_string<5> score_is;
varying_string<3> time_is;

byte time_this_second;

word escape_count;
byte escape_stock;
boolean escaping,got_out;

array<0,6,boolean> has_escaped;

byte count321;

word storage_seg,storage_ofs;

byte how_many_have_escaped;

void flippage()
{
 setactivepage(cp);
 cp=1-cp;
 setvisualpage(cp);
}

void flesh_colours() {;        /*assembler;
asm
  mov ax,$1012;
  mov bx,21;                 { 21 = light pink (why?) */
  /*mov cx,1;
  mov dx,seg    @flesh;
  mov es,dx;
  mov dx,offset @flesh;
  int $10;

  mov dx,seg    @darkflesh;
  mov es,dx;
  mov dx,offset @darkflesh;
  mov bx,5;                 { 5 = dark pink. */
  /*int $10;

  jmp @TheEnd;

 @flesh:
  db 56,35,35;

 @darkflesh:
  db 43,22,22;

 @TheEnd: */
                     }

boolean overlap(word a1x,word a1y,word a2x,word a2y,word b1x,word b1y,word b2x,word b2y)
{     /* By De Morgan's law: */
 boolean overlap_result;
 overlap_result=(a2x>=b1x) && (b2x>=a1x) && (a2y>=b1y) && (b2y>=a1y);
 return overlap_result;
}

void getsize(byte w, integer& xx,integer& yy)
{
    array<0,1,integer> n;

 move(x[w].p,n,4);
 xx=n[0]; yy=n[1];
}

void display(integer xx,integer yy, byte w)
{
 putimage(xx,yy,x[w].p,0);
}

byte get_stock_number(byte x)
{
 byte get_stock_number_result;
 while (has_escaped[x]) 
 {
  x += 1;
  if (x==7)  x=0;
 }
 get_stock_number_result=x;
 return get_stock_number_result;
}

void cameo_display(integer xx,integer yy, byte w1,byte w2)
{
 putimage(xx,yy,x[w2].p,andput);
 putimage(xx,yy,x[w1].p,xorput);
}

void blankit()
{
    byte fv;

 for( fv=1; fv <= rsize[cp]; fv ++)
 { rectype& with = r[cp][fv]; 
  bar(with.x1,with.y1,with.x2,with.y2);}
 rsize[cp]=0;
}

void blank(integer xx1,integer yy1,integer xx2,integer yy2)
{
 rsize[cp] += 1;
 {
  rectype& with = r[cp][rsize[cp]]; 

  with.x1=xx1;
  with.y1=yy1;
  with.x2=xx2;
  with.y2=yy2;
 }
}

void movethem()
{
    byte fv;

 for( fv=1; fv <= msize; fv ++)
  { mtype& with = m[fv]; 
   if (with.x!=flag) 
   {
    with.x=with.x+with.ix;
    with.y=with.y+with.iy;
   }}
}

void plotthem()
{
    byte fv; integer xx,yy;

 for( fv=1; fv <= msize; fv ++)
  { mtype& with = m[fv]; 
  if (with.x!=flag) 
   {
    if (with.cameo) 
    {
     cameo_display(with.x,with.y,with.p,with.cameo_frame);
     if (cp==0)  { with.cameo_frame += 2; with.p += 2; }
    } else display(with.x,with.y,with.p);
    getsize(with.p,xx,yy);
    if (with.wipe)  blank(with.x,with.y,with.x+xx,with.y+yy);
    if (with.timeout>0) 
    {
     with.timeout -= 1;
     if (with.timeout==0)  with.x=flag;
    }
   }}
}

void define(integer xx,integer yy, byte pp, shortint ixx,shortint iyy, integer timetime,
 boolean is_a_missile,boolean do_we_wipe)
{
    byte which;

 for( which=1; which <= msize; which ++)
  {
   mtype& with = m[which]; 

   if (with.x==flag) 
   {
    with.x=xx;
    with.y=yy;
    with.p=pp;
    with.ix=ixx;
    with.iy=iyy;
    with.timeout=timetime;
    with.cameo=false;
    with.missile=is_a_missile;
    with.wipe=do_we_wipe;

    return;
   }
  }
}

void define_cameo(integer xx,integer yy, byte pp, integer timetime)
{
    byte which;

 for( which=1; which <= msize; which ++)
  {
   mtype& with = m[which]; 

   if (with.x==flag) 
   {
    with.x=xx;
    with.y=yy;
    with.p=pp;
    with.ix=0;
    with.iy=0;
    with.timeout=timetime;
    with.cameo=true;
    with.cameo_frame=pp+1;
    with.missile=false;
    with.wipe=false;

    return;
   }
  }
}

void get_score()
{
    byte fv;

 str(score,5,score_is);
 for( fv=1; fv <= 5; fv ++)
  if (score_is[fv]==' ') 
   score_is[fv]='0';
}

void get_time1()
{
    byte fv;

 str(time1,5,time_is);
 for( fv=1; fv <= 3; fv ++)
  if (time_is[fv]==' ') 
   time_is[fv]='0';
}

void display_const(integer x,integer y, byte what)
{
    byte page_;

 for( page_=0; page_ <= 1; page_ ++)
 {
  setactivepage(page_);
  display(x,y,what);
 }
 setactivepage(1-cp);
}

void show_stock(byte x)
{
 if (escaping && (x==escape_stock)) 
 {
  display_const(x*90+20,30,stocks+2);
  return;
 }
 if (stockstatus[x]>5)  return;
 display_const(x*90+20,30,stocks+stockstatus[x]);
 stockstatus[x]=1-stockstatus[x];
}

void show_score()
{
 byte fv;
 varying_string<5> score_was;

 score_was=score_is; get_score();
 for( fv=1; fv <= 5; fv ++)
  if (score_was[fv]!=score_is[fv]) 
   display_const(30+fv*10,0,ord(score_is[fv])-47);
}

void show_time()
{
 byte fv;
 varying_string<3> time_was;

 time_was=time_is; get_time1();
 for( fv=1; fv <= 3; fv ++)
  if (time_was[fv]!=time_is[fv]) 
   display_const(130+fv*10,0,ord(time_is[fv])-47);
}

void gain(shortint howmuch)
{
 if (-howmuch>score)  score=0; else
  score=score+howmuch;
 show_score();
}

void new_escape()
{
 escape_count=Random(18)*20;
 escaping=false;
}

void instructions();

static void nextpage()
{
    char c;

 outtextxy(400,190,"Press a key for next page >");
 c=readkey();
 cleardevice();
}

void instructions()
{
 display(25,25,facing_right);
 outtextxy(60,35,"< Avvy, our hero, needs your help - you must move him around.");
 outtextxy(80,45,"(He's too terrified to move himself!)");

 outtextxy(0,75,"Your task is to prevent the people in the stocks from escaping");
 outtextxy(0,85,"by pelting them with rotten fruit, eggs and bread. The keys are:");
  outtextxy(80,115,"LEFT SHIFT"); outtextxy(200,115,"Move left.");
 outtextxy(72,135,"RIGHT SHIFT"); outtextxy(200,135,"Move right.");
        outtextxy(136,155,"ALT"); outtextxy(200,155,"Throw something.");
 nextpage();

 display(25,35,stocks);
 outtextxy(80,35,"This man is in the stocks. Your job is to stop him getting out.");
 outtextxy(88,45,"UNFORTUNATELY... the locks on the stocks are loose, and every");
 outtextxy(88,55,"so often, someone will discover this and try to get out.");
 display(25, 85,stocks+2);
 outtextxy(80, 85,"< Someone who has found a way out!");
 outtextxy(88, 95,"You MUST IMMEDIATELY hit people smiling like this, or they");
 outtextxy(88,105,"will disappear and lose you points.");
 display(25,125,stocks+5);
 display(25,155,stocks+4);
 outtextxy(80,125,"< Oh dear!");
 nextpage();

 outtextxy(0,35,"Your task is made harder by:");
 display(25,55,48);
 outtextxy(60,55,"< Yokels. These people will run in front of you. If you hit");
 outtextxy(68,65,"them, you will lose MORE points than you get hitting people");
 outtextxy(68,75,"in the stocks. So BEWARE!");
 outtextxy(80,125,"Good luck with the game!");
 nextpage();
}

void setup()
{
 integer gd,gm;
 untyped_file f;
 char rkv;

 rkv=upcase(readkey());
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 flesh_colours();

 assign(f,"notts.avd");
 reset(f,1); gd=1;
 score=0; time1=120; score_is="(c)94";
 time_this_second=0;

 while (! eof(f)) 
  {
   xtype& with = x[gd]; 

   blockread(f,with.s,2);
   getmem(with.p,with.s);
   blockread(f,with.p,with.s);
   gd += 1;
  }
 close(f);

 if ((rkv=='I') || ((rkv=='\0') && (readkey()==cf1)))  instructions();

 for( gd=0; gd <= 6; gd ++)
 {
  stockstatus[gd]=Random(2);
  show_stock(gd);
 }

 fillchar(m,sizeof(m),'\261');
 setfillstyle(1,0);
 cp=0;
 flippage();
 fillchar(rsize,sizeof(rsize),'\0');
 avvywas=320;
 avvypos=320;
 avvyanim=1;
 avvyfacing=facing_left;

 alt_was_pressed_before=false;
 throw_next=74;
 firing=false;

 for( gd=1; gd <= maxrunners; gd ++)
  {
   A1& with = running[gd]; 

   with.x=flag;
  }

 new_escape();
 fillchar(has_escaped,sizeof(has_escaped),'\0'); /* All false. */
 count321=255;  /* Counting down. */

 /* Set up status line. */

 display_const(0,0,17); /* Score: */
 show_score();            /* value of score (00000 here) */
 display_const(110,0,20); /* Time: */
 show_time();            /* value of time */

 Randomize();

 how_many_have_escaped=0;
}

void init_runner(integer xx,integer yy, byte f1,byte f2, shortint ixx,shortint iyy)
{
    byte fv;

 for( fv=1; fv <= maxrunners; fv ++)
  { A1& with = running[fv]; 
   if (with.x==flag) 
   {
    with.x=xx; with.y=yy;
    with.frame=f1;
    with.toohigh=f2;
    with.lowest=f1;
    with.ix=ixx; with.iy=iyy;
    if ((with.ix==0) && (with.iy==0))  with.ix=2; /* To stop them running on the spot! */
    with.framedelay=framedelaymax;
    return;
   }}
}

void titles()
{
 registers r;
 byte a; /*absolute $A000:0; */
 untyped_file f;
 byte bit;

 r.ax=0xd;
 intr(0x10,r);

 assign(f,"shoot1.avd");
 reset(f,1);
 for( bit=0; bit <= 3; bit ++)
 {
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a,8000);
 }
 close(f);
}
/*
procedure animate3;
begin
 define(100,100,10,1,0,perm,false,true);
 define( 50,20 ,30,3,3,30,false,true);
 repeat;
  blankit;
  plotthem;
  movethem;
  flippage;
  delay(100);
 until keypressed;
end;

procedure animate2;
var
 x,n:byte;
 helpx:integer;
 helpdir:shortint;
 helpani:byte;
begin
 x:=0; n:=0; helpani:=0;
 helpx:=10; helpdir:=1;
 setfillstyle(1,0);

 repeat
  display(x*52,0,n+28);
  inc(x); inc(n);

  if x=13 then
  begin
   bar(helpx-2,50,helpx+5,80);
   if helpdir>0 then
    display(helpx,50,80-helpani)
   else
    display(helpx,50,86-helpani);
   helpx:=helpx+helpdir*5;
   if (helpx>600) or (helpx<3) then helpdir:=-helpdir;
   if helpani=0 then helpani:=5 else dec(helpani);

   x:=0;
(*   delay(100);*)
  end;
  if n=6 then n:=0;
 until keypressed;
end;
*/

void move_avvy()
{
  if (avvywas<avvypos) 
  {
/*   bar(avvyWas,avvy_Y,avvyPos,Avvy_Y+85);*/
   avvyfacing=facing_right;
  } else if (avvywas>avvypos) 
  {
/*   bar(avvyWas+32,Avvy_Y,avvyPos+33,Avvy_Y+85);*/
   avvyfacing=facing_left;
  }

  if (! firing) 
  {
   if (avvywas==avvypos) 
    avvyanim=1;
   else
   {
    avvyanim += 1;
    if (avvyanim==6)  avvyanim=0;
   }
  }

 if (avvyfacing==avvy_shoot) 
  define(avvypos,avvy_y,shooting[avvyanim],0,0,1,false,true);
 else
  define(avvypos,avvy_y,avvyanim+avvyfacing,0,0,1,false,true);

 avvywas=avvypos;

 if (avvyfacing==avvy_shoot) 
 {
  if (avvyanim==6) 
  {
   avvyfacing=was_facing;
   avvyanim=0;
   firing=false;
  } else avvyanim += 1;
 }
}

void read_kbd()
{

 if (firing)  return;

 if ((shiftstate & 8)>0) 
 {     /* Alt - shoot */
  if ((alt_was_pressed_before) || (count321!=0))  return;
  alt_was_pressed_before=true;
  firing=true;
  define(avvypos+27,avvy_y+5,throw_next,0,-2,53,true,true);
  throw_next += 1; if (throw_next==80)  throw_next=74;
  avvyanim=0;
  was_facing=avvyfacing;
  avvyfacing=avvy_shoot;
  return;
 }

 alt_was_pressed_before=false;

 if ((shiftstate & 1)>0) 
 {     /* Move right. */
  avvypos += 5;
  if (avvypos>right_margin)  avvypos=right_margin;
  return;
 }

 if ((shiftstate & 2)>0) 
 {     /* Move left. */
  avvypos -= 5;
  if (avvypos<left_margin)  avvypos=left_margin;
 }

}

void animate()
{
    byte fv;

 if (Random(10)==1)  show_stock(get_stock_number(Random(6)));
 for( fv=0; fv <= 6; fv ++)
  if (stockstatus[fv]>5) 
  {
   stockstatus[fv] -= 1;
   if (stockstatus[fv]==8) 
   {
    stockstatus[fv]=0;
    show_stock(fv);
   }
  }
}

void collision_check()
{
 byte fv;
 integer dist_from_side;
 byte this_stock;

 for( fv=1; fv <= 100; fv ++)
  { mtype& with = m[fv]; 
   if (with.x!=flag) 
   {
    if ((with.missile) && (with.y<60) && (with.timeout==1)) 
    {
/*     sound(177); delay(1); nosound;*/
     dist_from_side=(with.x-20) % 90;
     this_stock=((with.x-20) / 90);
     if ((! has_escaped[this_stock]) &&
      (dist_from_side>17) && (dist_from_side<34)) 
     {
      sound(999); delay(3); nosound;
      define(with.x+20,with.y,26+Random(2),3,1,12,false,true); /* Well done! */
      define(this_stock*90+20,30,31,0,0,7,false,false); /* Face of man */
      define_cameo(this_stock*90+20+10,35,40,7); /* Splat! */
      define(this_stock*90+20+20,50,34+Random(5),0,2,9,false,true); /* Oof! */
      stockstatus[this_stock]=17;
      gain(score_for_hitting_face);

      if ((escaping) && (escape_stock==this_stock)) 
      {     /* Hit the escaper. */
       sound(1777); delay(1); nosound;
       gain(bonus_for_hitting_escaper);
       escaping=false; new_escape();
      }
     } else
     {
      define(with.x,with.y,83+Random(3),2,2,17,false,true); /* Missed! */
      if ((! has_escaped[this_stock]) &&
       ((dist_from_side>3) && (dist_from_side<43))) 
      {
       define(this_stock*90+20,30,30,0,0,7,false,false); /* Face of man */
       if (dist_from_side>35) 
        define_cameo(with.x-27,35,40,7); /* Splat! */
       else
        define_cameo(with.x-7,35,40,7);
       stockstatus[this_stock]=17;
      }
     }
    }
   }}
}

shortint sgn(integer a)
{
 shortint sgn_result;
 if (a==0)  { sgn_result=0; return sgn_result; }
 if (a>0)  { sgn_result=1; return sgn_result; }
 sgn_result=-1;
 return sgn_result;
}

void turn_around(byte who, boolean randomx)
{
 {
  A1& with = running[who]; 

  if (randomx) 
  {
   if (with.ix>0)  with.ix=-(Random(5)+1); else with.ix=(Random(5)+1);
  } else
   with.ix=-with.ix;
  with.iy=-with.iy;
 }
}

void bump_folk()
{
    byte fv,ff;

 for( fv=1; fv <= maxrunners; fv ++)
  if (running[fv].x!=flag) 
   for( ff=fv+1; ff <= maxrunners; ff ++)
    if ((running[ff].x!=flag) &&
     overlap(running[fv].x,running[fv].y,
             running[fv].x+17,running[fv].y+24,
             running[ff].x,running[ff].y,
             running[ff].x+17,running[ff].y+24)) 
     {
      turn_around(fv,false); /* Opp. directions. */
      turn_around(ff,false);
     }
}

void people_running()
{
    byte fv;

 if (count321!=0)  return;
 for( fv=1; fv <= maxrunners; fv ++)
  { A1& with = running[fv]; 
   if (with.x!=flag) 
   {
    if (((with.y+with.iy)<=53) || ((with.y+with.iy)>=120)) 
    {
     with.iy=-with.iy;
    }

    if (with.ix<0) 
     define(with.x,with.y,with.frame,0,0,1,false,true);
    else
     define(with.x,with.y,with.frame+7,0,0,1,false,true);
    if (with.framedelay==0) 
    {
     with.frame += 1; if (with.frame==with.toohigh)  with.frame=with.lowest;
     with.framedelay=framedelaymax;
     with.y=with.y+with.iy;
    } else with.framedelay -= 1;

    if (((with.x+with.ix)<=0) || ((with.x+with.ix)>=620))  turn_around(fv,true);

    with.x=with.x+with.ix;
   }}
}

void update_time()
{
 if (count321!=0)  return;
 time_this_second += 1;
 if (time_this_second < times_a_second)  return;
 time1 -= 1;
 show_time();
 time_this_second=0;
 if (time1<=flash_time) 
  if (odd(time1))  display_const(110,0,20); /* Normal time */
   else display_const(110,0,86); /* Flash time */
}

void hit_people()
{
    byte fv,ff;

 if (count321!=0)  return;
 for( fv=1; fv <= 100; fv ++)
  { mtype& with = m[fv]; 
   if (with.missile && (with.x!=flag)) 
    for( ff=1; ff <= maxrunners; ff ++)
     if ((running[ff].x!=flag) &&
        overlap(with.x,with.y,with.x+7,with.y+10,
                running[ff].x,running[ff].y,
                running[ff].x+17,
                running[ff].y+24)) 
      {
       sound(7177);
/*       setcolor(4);
       rectangle(x,y,x+7,y+10);
       rectangle(running[ff].x,running[ff].y,
                 running[ff].x+17,
                 running[ff].y+24);*/
       nosound;
       with.x=flag;
       gain(-5);
       define(running[ff].x+20,running[ff].y+3,
              34+Random(6),1,3,9,false,true); /* Oof! */
       define(with.x,with.y,83,1,0,17,false,true); /* Oops! */
      }}
}

void escape_check()
{
 if (count321!=0)  return;
 if (escape_count>0)  { escape_count -= 1; return; }

 /* Escape_count = 0; now what? */

 if (escaping) 
 {
  if (got_out) 
  {
   new_escape(); escaping=false;
   display_const(escape_stock*90+20,30,stocks+4);
  } else
  {
   display_const(escape_stock*90+20,30,stocks+5);
   escape_count=20; got_out=true;
   define(escape_stock*90+20,50,25,0,2,17,false,true); /* Escaped! */
   gain(-10);
   has_escaped[escape_stock]=true;

   how_many_have_escaped += 1;

   if (how_many_have_escaped == 7) 
   {
    for( time1=0; time1 <= 1; time1 ++)
    {
     setactivepage(time1);
     cleardevice();
    }
    setactivepage(1-cp);

    memw[storage_seg*storage_ofs+1]=0;
    do {; } while (!(memw[storage_seg*storage_ofs+1]>9));

    setvisualpage(1-cp);
    display(266,90,23);

    memw[storage_seg*storage_ofs+1]=0;
    do {; } while (!(memw[storage_seg*storage_ofs+1]>72));

    setvisualpage(cp);

    memw[storage_seg*storage_ofs+1]=0;
    do {; } while (!(memw[storage_seg*storage_ofs+1]>9));

    time1=0;
   }
  }
 } else
 {
  escape_stock=get_stock_number(Random(7)); escaping=true; got_out=false;
  display_const(escape_stock*90+20,30,stocks+2); /* Smiling! */
  escape_count=200;
 }
}

void check321()
{
 if (count321==0)  return;
 count321 -= 1;
 switch (count321) {
   case 84: define(320, 60,16, 2, 1,94,false,true); break;
  case 169: define(320, 60,15, 0, 1,94,false,true); break;
  case 254: {
        define(320, 60,14,-2, 1,94,false,true);
        define(  0,100,18, 2, 0,254,false,true);
       }
       break;
 }
}

void check_params();

static void not_bootstrap()
{
 output << "This is not a standalone program!" << NL;
 exit(255);
}

void check_params()
{
    integer e;

 if (paramstr(1)!="jsb")  not_bootstrap();

 val(paramstr(2),storage_seg,e); if (e!=0)  not_bootstrap();
 val(paramstr(3),storage_ofs,e); if (e!=0)  not_bootstrap();

}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 check_params();

 titles();
 setup();
 init_runner( 20, 70,48,54, Random(5)+1,Random(4)-2);
 init_runner(600, 70,48,54, Random(5)+1,Random(4)-2);
 init_runner(600,100,61,67,-Random(5)+1,Random(4)-2);
 init_runner( 20,100,61,67,-Random(5)+1,Random(4)-2);
 do {
  memw[storage_seg*storage_ofs+1]=0;

  blankit();
  hit_people();
  plotthem();
  movethem();
  move_avvy();
  bump_folk();
  people_running();
  animate();
  escape_check();

  collision_check();

  update_time();

  check321();

  read_kbd();
  flippage();
  do {; } while (!(memw[storage_seg*storage_ofs+1]>0));

 } while (!(time1==0));

/* textmode(259);
 textattr:=1;
 writeln('Your final score was: ',score,'.');
 readln;*/
 mem[storage_seg*storage_ofs]=score;
 return EXIT_SUCCESS;
}

} // End of namespace Avalanche.