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

/*$S-*/
/*#include "dos.h"*/
/*#include "crt.h"*/
/*#include "tommys.h"*/


typedef array<0,49999,byte> bigtextarray;
struct chaptertype {
 varying_string<60> headername;
 word headeroffset;
};
typedef array<1,1120,byte> sbtype;

const integer contsize = 29; /*number of headers in AVALOT.DOC*/
const varying_string<80> contentsheader = " -=- The contents of the Lord AVALOT D'Argent (version 1.3) documentation -=-";
const array<0,15,byte> listpal = {{1,0,3,0,7,0,7,7,0,0,0,0,0,0,0,0}};
const array<0,15,byte> blankpal = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};
const integer tabstop = 8; /*length of tab stops*/

/*NOTE: Tabs are not properly implemented. The program just interprets them*/
/*as a number of spaces.*/

matrix<1,256, 0,15,byte> textvar,textvar8;
array<0,1500,word> posof13;
array<1,65535,byte> scvar;      /*absolute $A000:$0000;*/
sbtype stbar,stbar2;
pointer dpt;
word sot,nol,bat,bab,tlab,nosl,bfseg,bfofs,useless;
bigtextarray* textmem;
boolean atsof,fast,regimode;
byte hol;
varying_string<80> stline;
array<1,contsize,chaptertype> contlist;
integer lat;

void wipeit(longint pos, word count)       /*Like fillchar, but wraps*/
{
    longint wpos;

 wpos=(word)(pos);
 fillchar(mem[longint(0xa000)+wpos / longint(16)*wpos % longint(16)],count,0);
}

void wrapcopy(sbtype fromarr, longint pos)       /*Like fillchar, but wraps*/
{
    longint wpos;

 wpos=(word)(pos);
 move(fromarr,mem[longint(0xa000)+wpos / longint(16)*wpos % longint(16)],1120);
}

void blankscreen()     /*blanks the screen (!)*/
{
    registers r;

 r.ax=0x1002;
 r.es=seg(blankpal);
 r.dx=ofs(blankpal);
 intr(0x10,r);
}

void showscreen()     /*shows the screen (!)*/
{
    registers r;

 r.ax=0x1002;
 r.es=seg(listpal);
 r.dx=ofs(listpal);
 intr(0x10,r);
}

void wipesb(word wheretop)
{
    byte plane;

 for( plane=2; plane <= 3; plane ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << plane; port[0x3cf]=plane;
  fillchar(scvar[(wheretop+336)*80+1],1120,0);
 }
 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=2; port[0x3cf]=1;
}

void displstat(byte wipepos)       /*displays the status bar*/
{
    byte plane;

 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=4; port[0x3cf]=2;
 wrapcopy(stbar,(lat+336)*80);
 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=8; port[0x3cf]=3;
 wrapcopy(stbar2,(lat+336)*80);
 for( plane=2; plane <= 3; plane ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << plane; port[0x3cf]=plane;
  switch (wipepos) {
   case 0: wipeit((lat+335)*80-1,80); break;
   case 1: wipeit(lat*80-1,80); break;
  }
 }
 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=2; port[0x3cf]=1;
}

void udstat()     /*updates the status bar*/
{
    varying_string<3> pt;
    byte fv,fv2;

 fillchar(pt,4,'\0');
 str(round((real)((tlab-21))/(nol-21)*100),pt);
 for( fv=1; fv <= 3; fv ++)
  for( fv2=0; fv2 <= 13; fv2 ++)
  {;
   stbar[fv2*80+fv+68]=~ textvar[ord(pt[fv])+1][fv2];
   stbar2[fv2*80+fv+68]=textvar[ord(pt[fv])+1][fv2];
  }
}

void finddocinfo()     /*finds the line breaks in AVALOT.DOC & finds the
headers by searching for '"""'*/
{
    word wv,oldwv,varpos,varpos2,contlpos;
    boolean thisaheader;
    varying_string<60> headerstring;

 thisaheader=false;
 posof13[0]=65535;  /*this +2 will wrap around to 1*/
 wv=1;
 oldwv=1;
 varpos=1;
 contlist[1].headername="Start of documentation";
 contlist[1].headeroffset=0;
 contlpos=2;
 while (wv<sot) 
 {
  while (((*textmem)[wv]!=13) && ((*textmem)[wv]!=34) && (wv-oldwv<80))  wv += 1;
  switch ((*textmem)[wv]) {
  case 13: {
       posof13[varpos]=wv;
       varpos += 1;
       oldwv=wv+1;
       thisaheader=false;
      }
      break;
  case 34: if (((*textmem)[wv-1]==34) && ((*textmem)[wv-2]==34) && (varpos>12)
       && (thisaheader==false)) 
      {
       thisaheader=true;
       headerstring[0]='\0';
       varpos2=posof13[varpos-2]+2;
       while (((*textmem)[varpos2]==32) || ((*textmem)[varpos2]==9))  varpos2 += 1;
       while (varpos2!=posof13[varpos-1]) 
       {;
        headerstring=headerstring+chr((*textmem)[varpos2]);
        varpos2 += 1;
       }
       contlist[contlpos].headername=headerstring;
       contlist[contlpos].headeroffset=varpos-2;
       contlpos += 1;
      }
      break;
    }
  wv += 1;
 }
 nol=varpos-2;
 nosl=nol*14;
}

void graphmode(byte gm)       /*puts the display adaptor into a specified mode*/
{
    registers regs;

 regs.ax=gm;
 intr(0x10,regs);
}

void setoffset(word where_on_screen) {;          /*assembler; {for scrolling the screen*/
/*asm
 mov bx, where_on_screen
 mov dx, $03D4
 mov ah, bh
 mov al, $C
 out dx, ax

 mov ah, bl
 inc al
 out dx, ax*/
                                     }

void setupsb(byte sbtype)       /*sets up the status bar in several styles*/
{
    integer fv;

 switch (sbtype) {
  case 1: if (regimode==false)  stline=string("Doc lister: PgUp, PgDn, Home & End to move. Esc exits. C=")
   +'\32'+"contents "+'\263'+"   % through   "; else
   stline=string("Doc lister: PgUp, PgDn, Home & End to move. Esc exits to main menu.")
   +'\263'+"   % through";
   break;
  case 2: stline=string("Esc=to doc lister ")+'\263'+" Press the key listed next to the section you wish to jump to"; break;
 }
 for( fv=0; fv <= 1118; fv ++) {;
  stbar[fv+1]=~ textvar[ord(stline[fv % 80+1])+1][fv / 80];
  stbar2[fv+1]=textvar[ord(stline[fv % 80+1])+1][fv / 80];
 }
}

void setup()     /*sets up graphics, variables, etc.*/
{
    untyped_file f;
    integer fv;
    registers r;

 if ((paramstr(1)!="REGI") && (paramstr(1)!="ELMPOYTEN"))

  {
   clrscr;
   output << "This program cannot be run on its own. Run AVALOT.EXE." << NL;
   exit(123);
  }
 val(paramstr(2),bfseg,useless);
 val(paramstr(3),bfofs,useless);
 bfofs += 1;
 atsof=true;
 fast=false;
 assign(f,"avalot.fnt");
 reset(f,1);
 blockread(f,textvar,4096);
 close(f);
 assign(f,"ttsmall.fnt");
 reset(f,1);
 blockread(f,textvar8,4096);
 close(f);
 assign(f,"avalot.doc");
 reset(f,1);
 sot=filesize(f);
 mark(dpt);
 textmem = new bigtextarray;
 blockread(f,*textmem,sot);
 close(f);
 finddocinfo();
 if (paramstr(1)=="REGI") 
  {;
   regimode=true;
   tlab=contlist[contsize].headeroffset+24;
   lat=contlist[contsize].headeroffset*14;
  }
  else
  {;
   lat=0; tlab=24;
   regimode=false;
  }
 setupsb(1);
 graphmode(16);
 directvideo=false;
 showscreen();
 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=2; port[0x3cf]=1;
}

void drawscreenf(integer tl)       /*draws a screen from a line forwards*/
/*N.B. tl>1*/
{
    word fv,fv2,curbyte,plane;
    byte xpos;

 blankscreen();
 wipesb(lat);
 if (tl>nol-24)  tl=nol-24;
 if (tl<0)  tl=0;
 lat=tl*14;
 for( plane=2; plane <= 3; plane ++) /*wipe sb off*/
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << plane; port[0x3cf]=plane;
  /*fillchar(mem [$A000:((lat-1)*80) mod 65536],26800,0);*/
  wipeit(lat*80,26800);
 }
 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=2; port[0x3cf]=1; /*back to normal*/
 if (tl>0)  curbyte=posof13[tl-1]+2; else curbyte=0;
 bat=curbyte;
 for( fv=lat; fv <= lat+335; fv ++)
 {
  fv2=curbyte;
  xpos=1;
  while (xpos<=80) 
  {
   if (fv2<posof13[tl]) 
   {
    if ((*textmem)[fv2]==9) 
    {
     wipeit(fv*80+xpos,tabstop);
     xpos += tabstop;
    } else
    {
     mem[0xa000*(word)(fv*80+xpos-1)]=
      textvar[(*textmem)[fv2]+1][fv % 14];
     xpos += 1;
    }
   } else
   {
    wipeit(fv*80+xpos-1,82-xpos);
    xpos=81;
   }
   fv2 += 1;
  }
  if (fv % 14==0) 
  {
   tl += 1;
   curbyte=posof13[tl-1]+2;
  }
 }
 bab=curbyte;
 tlab=tl;
 udstat();
 displstat(2);
 setoffset((word)(lat*80));
 if (tl-23>1)  atsof=false;
 showscreen();
}

void displcont()     /*displays the contents*/
{
    byte fv,fv2,fv3,keyon,jumppos,plane;
    word olat;
    varying_string<62> curstr;
    char rkv;

 blankscreen();
 olat=lat; lat=0; keyon=1; jumppos=0;
 setoffset(0);
 for( plane=1; plane <= 3; plane ++) /*wipe sb off*/
 {
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << plane; port[0x3cf]=plane;
  fillchar(scvar,26800,0);
 }
 port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=2; port[0x3cf]=1; /*back to normal*/
 setupsb(2);
 displstat(2);
 for( fv=1; fv <= 80; fv ++)
  for( fv2=0; fv2 <= 27; fv2 ++)
  {
   scvar[fv2*80+fv]=textvar[ord(contentsheader[fv-1])+1][fv2 / 2];
  }
 for( fv=1; fv <= contsize; fv ++)
 {
  if (keyon<10)  curstr=strf(keyon)+". "+contlist[fv].headername; else
   curstr=string(chr(keyon+55))+". "+contlist[fv].headername;
  for( fv2=1; fv2 <= length(curstr); fv2 ++)
   for( fv3=0; fv3 <= 7; fv3 ++)
    scvar[(fv+3)*640+fv3*80+fv2]=textvar8[ord(curstr[fv2])+1][fv3];
  keyon += 1;
 }
 showscreen();
 do {; } while (!keypressed());
 rkv=readkey();
 switch (rkv) {
  case RANGE_9('\61','\71'):  jumppos=ord(rkv)-48;
  break;
  case RANGE_26('\101','\132'):  jumppos=ord(rkv)-55;
  break;
  case RANGE_26('\141','\172'): jumppos=ord(rkv)-87;
  break;
  default: lat=olat;
 }
 if (jumppos>0)  lat=contlist[jumppos].headeroffset;
 setupsb(1);
 if (fast==false)  wipesb(0);
 drawscreenf(lat);
}

void down()     /*scrolls the screen down one line*/
{
    word fv,xpos,wpos,lab;

 lat += 1;
 lab=lat+335;
 setoffset((word)(lat*80));
 if (lab % 14==0)  {
 bat=posof13[tlab-24]+2; bab=posof13[tlab]+2; tlab += 1; udstat(); }
 fv=bab;
 xpos=1;
 while (xpos<=80) 
 {
  if (fv<posof13[tlab]) 
  {
   if ((*textmem)[fv]==9) 
   {
    wipeit(lab*80+xpos-1,tabstop);
    xpos += tabstop;
   } else
   {
    wpos=(lab*80+xpos) % 65536;
    /*fillchar(mem[$A000+wpos div 16*wpos mod 16],count,0)*/
    mem[0xa000*wpos-1]=textvar[(*textmem)[fv]+1][lab % 14];
    xpos += 1;
   }
  } else
   {
    wipeit(lab*80+xpos-1,81-xpos);
    xpos=81;
   }
  fv += 1;
 }
 atsof=false;
 if (fast==true)  displstat(0);
}

void up()     /*scrolls the screen up one line*/
{
    word fv,xpos,wpos;

 if (lat==0)  { atsof=true; return; }
 if (lat % 14==0) 
  if (tlab>24) 
  {
   tlab -= 1; bat=posof13[tlab-24]+2; bab=posof13[tlab-1]+2; udstat(); } else
  { atsof=true; udstat(); return; }
 lat -= 1;
 setoffset((word)(lat*80));
 fv=bat;
 xpos=1;
 while (xpos<=80) 
 {
  if (fv<posof13[tlab-23]) 
  {
   if ((*textmem)[fv]==9) 
   {
    wipeit(lat*80+xpos-1,tabstop);
    xpos += tabstop;
   } else
   {
    wpos=(word)((lat*80+xpos) % 65536);
    mem[0xa000*wpos-1]=textvar[(*textmem)[fv]+1][lat % 14];
    xpos += 1;
   }
  } else
   {
    wipeit(lat*80+xpos-1,81-xpos);
    xpos=81;
   }
  fv += 1;
 }
 if (fast==true)  displstat(1);
 /*ateof:=false;*/
}

void endit()     /*Ends the program*/
{
 release(dpt);
 graphmode(2);
}

void control()     /*User control*/
{
    char rkv,rkv2/*the sequel*/,rkv3;
    integer fv;
    boolean first;

 if (regimode==false)  displcont(); else drawscreenf(tlab-24);
 first=true;
 do {;
  rkv=readkey();
  switch (rkv) {
   case '\0': {
        rkv2=readkey();
        switch (rkv2) {
         case chome:drawscreenf(0); break;
         case cend: drawscreenf(nol-24); break;
         case cpgdn:
          {;
           memw[bfseg*bfofs]=0;
           if (fast==false)  wipesb(lat);
           fv=1;
           while (((cardinal)lat+336<nosl) && (fv<337)) 
           {
            fv += 1;
            down();
           }
           if ((first==true) && (memw[bfseg*bfofs]<=2))  fast=true;
           if ((fast==false) || (first==true))  displstat(0);
          }
          break;
         case cpgup:
          {;
           memw[bfseg*bfofs]=0;
           if (fast==false)  wipesb(lat);
           fv=1;
           while ((atsof==false) && (fv<337)) 
           {
            fv += 1;
            up();
           }
           if ((first==true) && (memw[bfseg*bfofs]<=2))  fast=true;
           if ((fast==false) || (first==true))  displstat(0);
           }
           break;
/*         cUp:repeat;
              up;
              readkey;
             until (readkey<>cUp) or (atsof=true);
         cDown: repeat;
                 down;
                 readkey;
                until (readkey<>cDown) or (ateof=true);*/
        }
        first=false;
       }
       break;
   case '\33': return; break;
   case '\103':case '\143': if (regimode==false)  { wipesb(lat); displcont(); } break;
  }
 } while (!false);
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 setup();
 control();
 endit();
 return EXIT_SUCCESS;
}
