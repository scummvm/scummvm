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

                 FILING           The saving and loading handler. */


/*$R+*/

/*interface*/

/*#include "Gyro.h"*/

namespace Avalanche {

const array<1,12,varying_string<6> > months = 
 {{"Jan*","Feb*","March","April","May","June","July","August",
  "Sept&","Oct&","Nov&","Dec&"}};
const string ednaid = string("TT")+'\261'+'\36'+'\1'+'\113'+'\261'+'\231'+'\261';
const array<1,6,varying_string<7> > systems = 
  {{"DOS","Windows","OS/2","Mac","Amiga","ST"}};

struct edhead { /* Edna header */
          /* This header starts at byte offset 177 in the .ASG file. */
          array<1,9,char> id;     /* signature */
          word revision; /* EDNA revision, here 2 (1=dna256) */
          varying_string<50> game; /* Long name, eg Lord Avalot D'Argent */
          varying_string<15> shortname; /* Short name, eg Avalot */
          word number; /* Game's code number, here 2 */
          word ver; /* Version number as integer (eg 1.00 = 100) */
          varying_string<5> verstr; /* Vernum as string (eg 1.00 = "1.00" */
          varying_string<12> filename; /* Filename, eg AVALOT.EXE */
          byte os; /* Saving OS (here 1=DOS). See below for others.*/

          /* Info on this particular game */

          varying_string<8> fn; /* Filename (not extension ('cos that's .ASG)) */
          byte d,m; /* D, M, Y are the Day, Month & Year this game was... */
          word y;  /* ...saved on. */
          varying_string<40> desc; /* Description of game (same as in Avaricius!) */
          word len; /* Length of DNA (it's not going to be above 65535!) */

          /* Quick reference & miscellaneous */

          word saves; /* no. of times this game has been saved */
          integer cash; /* contents of your wallet in numerical form */
          varying_string<20> money; /* ditto in string form (eg 5/-, or 1 denarius)*/
          word points; /* your score */

          /* DNA values follow, then footer (which is ignored) */
};
  /* Possible values of edhead.os:
     1 = DOS        4 = Mac
     2 = Windows    5 = Amiga
     3 = OS/2       6 = ST */

/*implementation*/

untyped_file f;
byte fv;
array<1,255,word> dna256;
boolean ok;
edhead e;

void info256(string x)       /* info on dna256 *.ASG files */
{
 varying_string<40> describe;
;
 assign(f,x);
 /*$I-*/ reset(f,1);
 seek(f,47);
 blockread(f,describe,40);
 blockread(f,dna256,sizeof(dna256));
 close(f); /*$I+*/
 {;
  e.revision=1;
  e.game="Denarius Avaricius Sextus";
  e.shortname="Avaricius";
  e.number=1;
  e.verstr="[?]";
  e.filename="AVVY.EXE";
  e.os=1; /* Dos */
  e.fn=x;
  e.d=dna256[7]; e.m=dna256[8]; e.y=dna256[9];
  e.desc=describe;
  e.len=512;
  e.saves=dna256[6];
  e.cash=dna256[30];
  e.money=strf(e.cash)+" denari";
  if (e.cash==1)  e.money=e.money+"us"; else e.money=e.money+'i';
  e.points=dna256[36];
 }
}

string enlarge(string x)
{string enlarge_result;
;
 switch (x[length(x)]) {
  case '*': {; x[0] -= 1; x=x+"uary"; } break;
  case '&': {; x[0] -= 1; x=x+"ember"; } break;
 }
 enlarge_result=x;
return enlarge_result;
}

string th(byte x)
{
    varying_string<4> n;
string th_result;
;
 n=strf(x);
 switch (x) {
  case 1:case 21:case 31: n=n+"st"; break;
  case 2:case 22: n=n+"nd"; break;
  case 3:case 23: n=n+"rd"; break;
  default: n=n+"th";
 }
 th_result=n;
return th_result;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 info256("t:justb4.asg");

 {;
  output << "DNA coding: ";
  switch (e.revision) {
   case 1: output << "dna256" << NL; break;
   case 2: output << "E.D.N.A." << NL; break;
   default: output << "Unknown!" << NL;
  }
  output << "Filename: " << e.game << " (version " << e.verstr << ')' << NL;
  output << "Description: " << e.desc << NL;
  output << "Cash: " << e.money << NL;
  output << "Score: " << e.points << NL;
  output << "Date: " << th(e.d) << ' ' << enlarge(months[e.m]) << ' ' << e.y << NL;
  output << "Number of saves: " << e.saves << NL;
 }
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.