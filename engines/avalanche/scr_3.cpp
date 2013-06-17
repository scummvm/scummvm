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
 assign(f,"TEXT3.SCR");
 rewrite(f); f << a; close(f);
}

void centre(byte y, string z)
{
    byte fv;
;
 for( fv=1; fv <= length(z); fv ++)
 {;
  gotoxy(39-length(z) / 2+fv,y);
  if (z[fv]!='\40')  output << z[fv];
 }
}

void line(byte cx,byte cy, string title)
{;
 for( y=0; y <= 4; y ++)
 {;
  qq="";
  for( x=1; x <= length(title); x ++)
  {;
   for( xx=7; xx >= 0; xx --)
   {;
    code=(byte)(((1 << xx) & font[title[x]][y*2])>0)+
     (byte)(((1 << xx) & font[title[x]][y*2+1])>0)*2;
    qq=qq+codes[code+1-1];
   }
  }
  gotoxy(cx,cy+y); output << qq;
 }
}

void big_t()
{
 text t;
 string x;
 byte y;
;
 assign(t,"c:\\avalot\\t.txt"); reset(t); y=1;
 while (! eof(t)) 
 {;
  t >> x >> NL;
  gotoxy(1,y); output << x; y += 1;
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=4; clrscr;
 assign(f,"c:\\thomas\\ttsmall.fnt");
 reset(f); f >> font; close(f);
 textattr=4; line(19,4,"hanks");
 textattr=6; line(23,8,"for");
 line(7,12,"playing"); textattr += 8;
 line(12,16,"Avalot.");
 textattr=12; big_t();
 textattr=8; gotoxy(40,2); output << "(c) 1994, Mike, Mark and Thomas Thurman.";
 textattr=11; gotoxy(50,10); output << "* Goodbyte! *";
 textattr=10; gotoxy(9,20); output << '\20';
 textattr=12; output << " If you'd like to see yet more of these games, then don't forget to";
 gotoxy(12,21); output << "register, or your"; clreol;
 gotoxy(12,22); output << "for the rest of your life!";
 gotoxy(60,22); output << "(Only joking!)";
 save();
return EXIT_SUCCESS;
}
