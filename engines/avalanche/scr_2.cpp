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

/*#include "Crt.h"*/


const varying_string<4> codes = " ﬂ‹€";

typedef matrix<'\0','\377',0,15,byte> fonttype;

byte x,xx,y;
string qq;
file<fonttype> f;
fonttype font;
byte code;


typedef array<1,3840,byte> atype;

void save()
{
 file<atype> f;
 word fv;
 atype a /*absolute $B800:0*/;
;
 assign(f,"TEXT2.SCR");
 rewrite(f); f << a; close(f);
}

void centre(byte y, string z)
{
    byte fv;
;
 for( fv=1; fv <= length(z); fv ++)
 {;
  gotoxy(39-length(z) / 2+fv,y);
  if (odd(fv+y))  textattr=2; else textattr=cyan;
  if (z[fv]!='\40')  output << z[fv];
 }
}

void line(byte yy, string title)
{
      const integer offset = 5;
;
 for( y=1; y <= 6; y ++)
 {;
  qq="";
  for( x=1; x <= length(title); x ++)
  {;
   for( xx=7; xx >= 0; xx --)
   {;
    code=(byte)(((1 << xx) & font[title[x]][y*2-offset])>0)+
     (byte)(((1 << xx) & font[title[x]][y*2+1-offset])>0)*2;
    qq=qq+codes[code+1-1];
   }
  }
  centre(y+yy,qq);
 }
}

void chips()
{
    byte fv,x,y;
;
 for( fv=0; fv <= 1; fv ++)
 {;
  textattr=120;
  for( y=2; y <= 6; y ++)
   {;
    gotoxy(fv*67+3,y); output << "     "; if (fv==1)  output << ' ';
   }
  gotoxy(fv*67+4,4);
  if (fv==0)  output << "RAM"; else output << "CRAM";
  textattr=7;
  for( x=0; x <= 1; x ++)
   for( y=2; y <= 6; y ++)
    {; gotoxy(fv*67+2+x*(6+fv),y); output << ''; }
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=0; clrscr;
 chips();
 assign(f,"c:\\thomas\\ttsmall.fnt");
 reset(f); f >> font; close(f);
 line(0,"Out of");
 line(4,"memory!");
 centre(11,"Yes, RAM cram strikes again- Avvy has just run out of RAM (not the");
 centre(12,"hydraulic, woolly or village kinds.) Fortunately, there are a few things");
 centre(13,"you can do about this:");
 textattr=3;
 for( xx=15; xx <= 17; xx ++)
 {;
  gotoxy(23,xx); output << '\20';
 }
 textattr=2;
 gotoxy(25,15); output << "Don't run Avvy in a DOS shell.";
 gotoxy(25,16); output << "If that fails, try un-installing your TSRs.";
 gotoxy(25,17); output << "If you've got DOS 5˘0 or above, try using";
 gotoxy(28,18); textattr=3; output << "dos=high";
 textattr=2; output << ',';
 textattr=3; output << " loadhigh";
 textattr=2; output << " and";
 textattr=3; output << " devicehigh";
 textattr=2; output << '.';
 gotoxy(28,19); output << "See your DOS manual for details...";
 centre(23,"Sorry for any inconvenience...");
 save();
return EXIT_SUCCESS;
}
