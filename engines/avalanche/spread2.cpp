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
/*#include "Crt.h"*/
/*#include "Squeak.h"*/

/*$V-,R+*/

namespace Avalanche {

const integer pattern = 12; /* Pattern for transparencies. */
const fillpatterntype grey50 = {{0xaa, 0x55, 0xaa,
 0x55, 0xaa, 0x55, 0xaa, 0x55}};

struct adxtype {
           varying_string<12> name; /* name of character */
           varying_string<16> comment; /* comment */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of one picture */
           byte fgc,bgc; /* foreground & background bubble colours */
           byte accinum; /* the number according to Acci (1=Avvy, etc.) */
};

untyped_file sf;
adxtype a;
char r;
boolean adxmodi,picmodi;
array<5,2053,byte> mani;
matrix<0,99,0,10,byte> sil;
array<1,16000,byte> aa;
word soa;
byte xw;
byte leftc,rightc;
boolean lmo;
array<1,2,pointer> clip;
integer clipx,clipy;
boolean boardfull;
integer xofs,yofs; /* Distance of TL corner of cut from TL corner of pic. */

byte bigpix_size,bigpix_gap;  /* Size & gap betwixt big pixels. */

void setup()
{
      const integer shouldid = -1317732048;
 boolean ok;
 longint id;
 varying_string<2> sn;
;
 output << "Sprite Editor 2 (c) 1993, Thomas Thurman." << NL;
 bigpix_size=3; bigpix_gap=5;
 do {
  ok=true;
  output << "Number of sprite?"; input >> sn >> NL;
  assign(sf,string("v:\\sprite")+sn+".avd");
  reset(sf,1);
  seek(sf,177);
  blockread(sf,id,4);
  if (id!=shouldid) 
  {;
   output << "That isn't a valid Trip5 spritefile." << NL;
   output << "Please choose another." << NL;
   output << NL;
   ok=false; close(sf);
  } else
  {;
   blockread(sf,soa,2);
   if (soa!=(cardinal)sizeof(a))  /* to change this, just change the "type adx=" bit.*/
   {;
    output << "That spritefile contains an unknown ADX field type." << NL;
    output << "Please choose another." << NL;
    output << NL;
    ok=false; close(sf);
   }
  }
 } while (!ok);
 blockread(sf,a,soa);
 output << filepos(sf) << NL;
 adxmodi=false; picmodi=false;
 getmem(clip[1],a.size); getmem(clip[2],a.size); boardfull=false;
}

string strf(longint x)
{
    string q;
string strf_result;
;
 str(x,q); strf_result=q;
return strf_result;
}

void centre(byte y, string z)
{;
 gotoxy(40-length(z) / 2,y); output << z;
}

void info(byte x,byte y, string p,string q)
{;
 gotoxy(x,y);
 textattr=6;  output << p+":ú";
 textattr=11; output << q;
 textattr=8; output << 'ú';
}

void colours(byte f,byte b)
{;
 gotoxy(35,11);
 textattr=6;
 output << "Bubbles";
 textattr=b*16+f;
 output << " like this! ";
}

void adxinfo()
{;
 {;
  info( 5, 8,"Name",a.name);
  info(35, 8,"Comment",a.comment);
  info( 5, 9,"Width",strf(a.xl));
  info(15, 9,"Height",strf(a.yl));
  info(35, 9,"Size of one pic",strf(a.size));
  info( 5,10,"Number in a stride",strf(a.seq));
  info(35,10,"Number of strides",strf(a.num / a.seq));
  info( 5,11,"Total number",strf(a.num));
  info( 5,12,"Acci number",strf(a.accinum));
  colours(a.fgc,a.bgc);
 }
}

void status()
{;
 textattr=7;
 clrscr;
 textattr=10; centre(3,"Sprite Editor 2 (c) 1993, Thomas Thurman.");
 textattr=6; gotoxy(3,7); output << "ADX information:";
 adxinfo();
 textattr=6;
 gotoxy(3,14); output << "Options:";
 gotoxy(5,15); output << "A) edit ADX information";
 gotoxy(5,16); output << "P) edit pictures";
 gotoxy(5,17); output << "S) save the ADX info (pics are saved automatically)";
}

void enterstring(string w, byte l, string& q)
{
    string t;
;
 textattr=13; clrscr;
 output << NL;
 output << "Press Return for no change, Space+Return for a blank." << NL;
 do {
  output << "New value for " << w << " (max length " << l << ")?";
  input >> t >> NL;
 } while (!(length(t)<=l));
 if (t==' ')  q=""; else if (t!="")  q=t;
 adxmodi=true;
}

void entercolour(string w, byte& c);
static byte fv;

static void loseold() {; output << string('\10')+'\377'; }


static void drawnew() {; gotoxy(3+fv*5,11); output << '\30'; }

void entercolour(string w, byte& c)
{;
 textattr=13; clrscr;
 output << NL; output << "New value for " << w << '?' << NL;
 output << "  Use \32\33 to move about, Enter=OK, Esc=Cancel." << NL;
 for( fv=1; fv <= 15; fv ++)
 {;
  gotoxy(3+fv*5,10); textattr=fv; output << 'þ';
 }
 fv=c;
 do {
  drawnew();
  r=readkey();
  switch (r) {
   case '\33': return; break; /* no change to c */
   case '\15': {;
         c=fv; adxmodi=true;
         return;
        }
        break;
   case '\0': switch (readkey()) {
        case 'G': {; loseold(); fv= 0; drawnew(); } break; /* home */
        case 'O': {; loseold(); fv=15; drawnew(); } break; /* end */
        case 'K': if (fv> 0)  {; loseold(); fv -= 1; drawnew(); } break; /* left */
        case 'M': if (fv<15)  {; loseold(); fv += 1; drawnew(); } break; /* right */
       }
       break;
  }
 } while (!false);
}

void enternum(string w, byte& q)
{
    string t; integer e; byte r;
;
 textattr=13; clrscr;
 output << NL;
 output << "Press Return for no change." << NL;
 do {
  output << "New value for " << w << '?';
  input >> t >> NL;
  if (t=="")  return; /* No change... */
  val(t,r,e);
 } while (!(e==0));
 q=r;  /* Update variable. */
 adxmodi=true;
}

void editadx()
{
    char r;
;
 do {
  clrscr;
  textattr=10; centre(3,"ADX Editor:");
  textattr= 9; centre(5,"N: Name, C: Comment, F: Foreground, B: Background, A: Accinum, X: eXit.");
  adxinfo();
  r=upcase(readkey());
  switch (r) {
   case 'N': enterstring("Name",12,a.name); break;
   case 'C': enterstring("Comment",16,a.comment); break;
   case 'F': entercolour("Foreground",a.fgc); break;
   case 'B': entercolour("Background",a.bgc); break;
   case 'A': enternum("Accinum",a.accinum); break;
   case 'X':case '\33': return; break;
   default: output << '\7';
  }
 } while (!false);
}

void saveit()
{
 char pak;
 integer oldsoa;
;
 textattr=10; clrscr;
 centre(7,"Saving!");
 if (adxmodi) 
 {;
  centre(10,"ADX information being saved...");
  seek(sf,181); /* start of ADX info */
  soa=sizeof(a);
  blockread(sf,oldsoa,2);
/*  if soa=oldsoa then
  begin;*/
   seek(sf,181);
   blockwrite(sf,soa,2);
   blockwrite(sf,a,soa);
   adxmodi=false;
/*  end else write(#7);*/
 } else centre(10,"No changes were made to ADX...");
 centre(25,"Press any key..."); pak=readkey();
}

void quit()
{;
 close(sf);
 exit(0);
}

void getsilmani()     /* Reclaims original sil & mani arrays */
{
    byte x,y,z; word offs;
;

 /* Sil... */

 getimage(500,150,500+a.xl,150+a.yl,aa);

 for( x=0; x <= 3; x ++)
  for( y=0; y <= a.yl; y ++)
   for( z=0; z <= (a.xl / 8); z ++)
   {;
    offs=5+y*xw*4+xw*x+z;
    sil[y][z]=aa[offs];
   }

 /* ...Mani. */

 getimage(500,50,500+a.xl,50+a.yl,aa);

 move(aa[5],mani,sizeof(mani));

}

void explode(byte which)       /* 0 is the first one! */
 /* Each character takes five-quarters of (a.size-6) on disk. */
{
 byte fv,ff; word so1; /* size of one */
;
 {;
  so1=a.size-6; so1 += so1 / 4;
  seek(sf,183+soa+so1*which); /* First is at 221 */
/*  where:=filepos(sf);*/
  xw=a.xl / 8; if ((a.xl % 8)>0)  xw += 1;

  for( fv=0; fv <= a.yl; fv ++)
   blockread(sf,sil[fv],xw);
  blockread(sf,mani,a.size-6);
  aa[a.size-1]=0; aa[a.size]=0; /* footer */
 }
}

void implode(byte which)       /* Writes a pic back onto the disk */
{
 byte fv,ff; word so1; /* size of one */
;
 {;

  getsilmani(); /* Restore original arrays */

  so1=a.size-6; so1 += so1 / 4;
  seek(sf,183+soa+so1*which); /* First is at 221 */

  xw=a.xl / 8; if ((a.xl % 8)>0)  xw += 1;

  for( fv=0; fv <= a.yl; fv ++)
   blockwrite(sf,sil[fv],xw);
  blockwrite(sf,mani,a.size-6);
  aa[a.size-1]=0; aa[a.size]=0; /* footer */
 }
}

void plotat(integer xx,integer yy) /* Does NOT cameo the picture!*/
{;
 move(mani,aa[5],sizeof(mani));
 {;
  aa[1]=a.xl; aa[2]=0; aa[3]=a.yl; aa[4]=0; /* set up x&y codes */
 }
 putimage(xx,yy,aa,0);
}

void plotsil(integer xx,integer yy) /* Plots silhouette- rarely used */
{
    byte x,y,z; word offs;
;
 for( x=0; x <= 3; x ++)
  for( y=0; y <= a.yl; y ++)
   for( z=0; z <= (a.xl / 8); z ++)
   {;
    offs=5+y*xw*4+xw*x+z;
    aa[offs]=sil[y][z];
   }

 {;
  aa[1]=a.xl; aa[2]=0; aa[3]=a.yl; aa[4]=0; /* set up x&y codes */
 }

 putimage(xx,yy,aa,0);

}

void style(byte x)
{;
 if (x==16) 
  /*setfillstyle(pattern,8)*/setfillpattern(grey50,8);
 else
  setfillstyle(1,x);
}

void bigpixel(integer x,integer y)
{;
 if (getpixel(500+x,150+y)==15) 
  /*setfillstyle(pattern,8)*/setfillpattern(grey50,8);
 else
  setfillstyle(1,getpixel(500+x,50+y));

 bar(x*bigpix_gap,y*bigpix_gap,
  x*bigpix_gap+bigpix_size,y*bigpix_gap+bigpix_size);
}

void subplot(byte y, integer x, char c)
{;
 setfillstyle(1,0); bar(x,0,x+9,170); outtextxy(x+5,y*10+5,c);
}

void plotleft()  {; subplot( leftc,239,'\32'); }          /* palette arrows */
void plotright() {; subplot(rightc,351,'\33'); }

void plotbig(byte x,byte y,byte c)
{;
 style(c);
 bar(x*bigpix_gap,y*bigpix_gap,
   x*bigpix_gap+bigpix_size,y*bigpix_gap+bigpix_size);
 if (c==16) 
 {;
  putpixel(500+x,150+y,15);
  putpixel(500+x, 50+y,0);
 } else
 {;
  putpixel(500+x,150+y,0);
  putpixel(500+x, 50+y,c);
 }
}

void changepic()
{;
 mx=mx / bigpix_gap; my=my / bigpix_gap;
           if ((mx>a.xl) || (my>a.yl))  return;
 if (mkey==left) 
  plotbig(mx,my,leftc); else
  plotbig(mx,my,rightc);
}

void changecol()
{;
 my=my / 10; if (my>16)  return;
 if (mkey==left) 
 {;
  leftc=my; plotleft();
 } else
 {;
  rightc=my; plotright();
 }
}

void showcutpic()
{;
 setfillstyle(5,1); bar(20,160,40+clipx,180+clipy);
 putimage(30,170,clip[2],andput);
 putimage(30,170,clip[1],xorput);
}

void movesquare(integer& xc,integer& yc, integer xl,integer yl)
{
    integer x2,y2;
;
 do {
  x2=xl+xc; y2=yl+yc;
  setcolor(15);
  do {
   rectangle(xc*bigpix_gap-1,yc*bigpix_gap-1,
    x2*(bigpix_gap+1)-1,y2*(bigpix_gap+1)-1);
  } while (!keypressed());
  setcolor(0); rectangle(xc*bigpix_gap-1,yc*bigpix_gap-1,
   x2*(bigpix_gap+1)-1,y2*(bigpix_gap+1)-1);
  switch (readkey()) {
   case '\0': switch (readkey()) {
        case '\110': yc -= 1; break;
        case '\113': xc -= 1; break;
        case '\115': xc += 1; break;
        case '\120': yc += 1; break;
       }
       break;
   case '\15': return; break;
  }
  while ((xl+xc)>a.xl)  xc -= 1;
  while ((yl+yc)>a.yl)  yc -= 1;
  if (xc<0)  xc=0;
  if (yc<0)  yc=0;
 } while (!false);
}

void switch_(integer& v1,integer& v2)
 /* Swaps over the values of v1 and v2. */
{
    integer temp;
;
 temp=v1; v1=v2; v2=temp;
}

void choosesquare(integer& x1,integer& y1,integer& x2,integer& y2)
{
    boolean tl;
;
 do {
  setcolor(15);
  do {
   rectangle(x1*bigpix_gap-1,y1*bigpix_gap-1,
    (x2+1)*bigpix_gap-1,(y2+1)*bigpix_gap-1);
  } while (!keypressed());
  setcolor(0);
  rectangle(x1*bigpix_gap-1,y1*bigpix_gap-1,
   (x2+1)*bigpix_gap-1,(y2+1)*bigpix_gap-1);
  switch (readkey()) {
   case '\0': switch (readkey()) {
        case '\110': if (tl)  y1 -= 1; else y2 -= 1; break;
        case '\113': if (tl)  x1 -= 1; else x2 -= 1; break;
        case '\115': if (tl)  x1 += 1; else x2 += 1; break;
        case '\120': if (tl)  y1 += 1; else y2 += 1; break;
       }
       break;
   case '\11': tl=! tl; break;
   case '\15': {;
         if (x1>x2)  switch_(x1,x2);  /* Get the square the right way up. */
         if (y1>y2)  switch_(y1,y2);
         y2 -= y1; x2 -= x1;     /* y1 & y2 have to be the OFFSETS! */
         return;
        }
        break;
  }
  if (x1<0)  x1=0; if (y1<0)  y1=0;
  {;
   if (y2>a.yl)  y2=a.yl; if (x2>a.xl)  x2=a.xl;
  }
 } while (!false);
}

void paste()
{
    byte x,y;
;
 if (! boardfull) 
 {;
  output << '\7';
  return;
 }
  if (! ((clipx==a.xl) && (clipy==a.yl))) 
   movesquare(xofs,yofs,clipx,clipy);
 putimage(500+xofs, 50+yofs,clip[1],0);
 putimage(500+xofs,150+yofs,clip[2],0);
 for( x=0; x <= a.xl; x ++)
  for( y=0; y <= a.yl; y ++)
  {;
   bigpixel(x,y);
  }
}

void cut()
{;
 xofs=0; yofs=0;   /* From the TL. */
 {;
  getimage(500, 50,500+a.xl, 50+a.yl,clip[1]);
  getimage(500,150,500+a.xl,150+a.yl,clip[2]);
  clipx=a.xl; clipy=a.yl;
 }
 showcutpic();
 boardfull=true;
}

void cutsome()
{;
 {;
  choosesquare(xofs,yofs,clipx,clipy);
  getimage(500+xofs, 50+yofs,500+xofs+clipx, 50+yofs+clipy,clip[1]);
  getimage(500+xofs,150+yofs,500+xofs+clipx,150+yofs+clipy,clip[2]);
 }
 showcutpic();
 boardfull=true;
}

boolean confirm(char c, string x)
{
    byte col; char groi;
boolean confirm_result;
;
 while (keypressed())  groi=readkey();
 x=x+"? "+c+" to confirm.";
 col=1;
 do {
  setcolor(col); outtextxy(555,5,x);
  col += 1; if (col==16)  col=1;
 } while (!keypressed());
 confirm_result=upcase(readkey())==c;
 setfillstyle(1,0); bar(470,0,640,10);
return confirm_result;
}

void checkbutton(byte which)
{;
 my=(my-12) / 25;
 switch (my) {
  case 0: if (confirm('S',"Save"))  {; implode(which); lmo=true; } break;
  case 1: if (confirm('C',"Cancel"))  lmo=true; break;
  case 4: cut(); break;
  case 5: if (confirm('P',"Paste"))  paste(); break;
  case 6: cutsome(); break;
 }
}

void animate()
{;
}

void undo()
{;
}

void fliplr();

static void flipline(integer x1,integer x2,integer y)
{
    integer fv,ff;
;
 for( fv=x1; fv <= x2; fv ++) putpixel(fv,0,getpixel(fv,y));
 ff=x2;
 for( fv=x1; fv <= x2; fv ++)
 {;
  putpixel(fv,y,getpixel(ff,0));
  ff -= 1;
 }
}

void fliplr()     /* Flips left-to-right. */
{
    integer fv,ff;
;
  for( fv=0; fv <= a.yl; fv ++)
  {;
   flipline(500,500+a.xl, 50+fv);
   flipline(500,500+a.xl,150+fv);
  }
 for( fv=0; fv <= a.xl; fv ++)
  for( ff=0; ff <= a.yl; ff ++)
   bigpixel(fv,ff);
}

void change_colours()     /* Swaps one colour with another. */
{
    byte fv,ff;
;

 if ((leftc==16) || (rightc==16))  /* See-through can't be one of the colours. */
 {;
  output << '\7'; /* Bleep! */
  return;
 }

  for( fv=0; fv <= a.yl; fv ++)
   for( ff=0; ff <= a.xl; ff ++)
    if (getpixel(500+ff,50+fv)==leftc)
          putpixel(500+ff,50+fv,rightc);

 for( fv=0; fv <= a.xl; fv ++)
  for( ff=0; ff <= a.yl; ff ++)
   bigpixel(fv,ff);
}

void redraw()
{
    byte x,y;
;
 setfillstyle(1,0);
 bar(0,0,250,200);

 for( x=0; x <= a.xl; x ++)
  for( y=0; y <= a.yl; y ++)
  {;
   bigpixel(x,y);
  }
}

void parse(char c)       /* Parses keystrokes */
{;
 switch (upcase(c)) {
  case '\26':case 'P': paste(); break;
  case '\3':case 'C': cut(); break;
  case '\30':case 'X': cutsome(); break;
  case 'A': animate(); break;
  case 'U': undo(); break;
  case '@': fliplr(); break;
  case '!': change_colours(); break;
  case '<': if (bigpix_size>1) 
       {;
        bigpix_size -= 1; bigpix_gap -= 1;
        redraw();
       }
       break;
  case '>': if (bigpix_size<8) 
       {;
        bigpix_size += 1; bigpix_gap += 1;
        redraw();
       }
       break;
  case '\33': if (confirm('X',"Exit"))  lmo=true; break;
 }
}

void editone(byte which)
{
    byte x,y;
;
 cleardevice();
 explode(which);
 plotat(500,50);
 plotsil(500,150);
 for( x=0; x <= a.xl; x ++)
  for( y=0; y <= a.yl; y ++)
  {;
   bigpixel(x,y);
  }
 for( y=0; y <= 16; y ++)
 {;
  style(y);
  bar(251,y*10+1,349,y*10+9);
  rectangle(250,y*10,350,y*10+10);
 }

 settextstyle(0,0,1); leftc=15; rightc=16; plotleft(); plotright(); lmo=false;

 outtextxy(410, 25,"Save");
 outtextxy(410, 50,"Cancel");
 outtextxy(410, 75,"Animate");
 outtextxy(410,100,"Undo");
 outtextxy(410,125,"Cut");
 outtextxy(410,150,"Paste");
 outtextxy(410,175,"X: Cut Some");
 if (boardfull)  showcutpic();
 setfillstyle(6,15);
 for( y=0; y <= 7; y ++)
  bar(370,y*25+12,450,y*25+12);

 do {
  on;
  do {
   if (keypressed())  parse(readkey());
  } while (!(anyo | lmo));
  off;

  if (! lmo) 
  {;
   getbuttonstatus;

   switch (mx) {
      case 1 ... 249: changepic(); break;
    case 250 ... 350: changecol(); break;
    case 370 ... 450: checkbutton(which); break;
   }
  }
 } while (!lmo);
 settextstyle(2,0,7); setcolor(15);
}

void editstride(byte which);

static char whichc;

static shortint first;

static void drawup(byte& which)
{
    byte fv;
;
 whichc=chr(which+48);
 cleardevice();
 outtextxy(320,10,string("Edit stride ")+whichc);
 first=(which-1)*a.seq-1;
 for( fv=1; fv <= a.seq; fv ++)
 {;
  explode(fv+first);
  plotat(fv*73,77);
  outtextxy(17+fv*73,64,chr(fv+48));
 }
 outtextxy(320,177,"Which?");
}

void editstride(byte which)
{
 char r;
;
 drawup(which);
 do {
  r=readkey();
  if ((r>'0') && (r<=chr(a.seq+48))) 
  {;
   editone(ord(r)-48+first);
   drawup(which);
  }
 } while (!(r=='\33'));
}

void editpics();

static byte nds; /* num div seq */

static void drawup1()
{
    byte fv;
;
 setgraphmode(0); directvideo=false; settextjustify(1,1);
           nds=a.num / a.seq;
 settextstyle(2,0,7);
 outtextxy(320,10,"Edit pictures...");
 outtextxy(320,40,"(Usually, 1=away, 2=right, 3=towards, 4=left.)");
 for( fv=1; fv <= nds; fv ++)
 {;
  explode((fv-1)*a.seq);
  plotat(fv*73,100);
  outtextxy(17+fv*73,87,chr(fv+48));
 }
 outtextxy(320,60,string("There are ")+strf(nds)+" strides available.");
 outtextxy(320,177,"Which do you want?");
}

void editpics()
{
 char r; byte which; integer e;
;
 drawup1();
 do {
  r=readkey();
  if ((r>'0') && (r<=chr(nds+48))) 
  {;
   editstride(ord(r)-48);
   drawup1();
  }
  } while (!(r=='\33'));
 restorecrtmode();
}

void titles()
{
 integer gd,gm;
 char pak; word wait;
;
 gd=3; gm=1; initgraph(gd,gm,"c:\\bp\\bgi");
 settextstyle(5,0,10); settextjustify(1,1);
 gm=getmaxy() / 2; wait=0;
 do {
  for( gm=0; gm <= 15; gm ++)
  {;
   setcolor(15-gm);
   for( gd=0; gd <= (150-gm*10); gd ++)
   {;
    outtextxy(320,124-gd,"Spread 2");
    outtextxy(320,125+gd,"Spread 2");
    if ((gd==5) && (gm==0))  wait=2345;
    if ((gd==6) && (gm==0))  wait=0;
    do {
     if (keypressed()) 
     {;
      while (keypressed())  pak=readkey();
      restorecrtmode();
      return;
     }
     if (wait>0)  {; wait -= 1; delay(1); }
    } while (!(wait==0));
   }
  }
 } while (!false);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 titles();
 setup();
 do {
  status();
  r=upcase(readkey());
  switch (r) {
   case 'A': editadx(); break;
   case 'P': editpics(); break;
   case 'S': saveit(); break;
   case 'X':case '\33': quit(); break;
  }
 } while (!false);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.