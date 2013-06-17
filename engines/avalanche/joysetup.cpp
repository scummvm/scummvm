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

/* Avalot joystick setup routines. To be incorporated
                         into Setup2 whenever I get a chance. */
/*#include "Joystick.h"*/
/*#include "Crt.h"*/


struct joysetup {
            word xmid,ymid,xmin,ymin,xmax,ymax;
            byte centre; /* Size of centre in tenths */
};

joysetup js;
file<joysetup> jf;

boolean detect()
{
 word x,y,xo,yo;
 byte count;
boolean detect_result;
;
 count=0;
 if (joystickpresent) 
 {;
  detect_result=true;
  return detect_result;
 }
 readjoya(xo,yo);
 do {
  if (count<7)  count += 1;   /* Take advantage of "flutter" */
  if (count==6) 
  {;
   output << "The Bios says you don't have a joystick. However, it's often wrong" << NL;
   output << "about such matters. So, do you? If you do, move joystick A to" << NL;
   output << "continue. If you don't, press any key to cancel." << NL;
  }
  readjoya(x,y);
 } while (!((keypressed()) | (x!=xo) || (y!=yo)));
 detect_result=~ keypressed();
return detect_result;
}

void display()
{;
 {;
  gotoxy(20,10); output << "X min: " << js.xmin << "  ";
  gotoxy(20,11); output << "X max: " << js.xmax << "  ";
  gotoxy(20,12); output << "Y min: " << js.ymin << "  ";
  gotoxy(20,13); output << "Y max: " << js.ymax << "  ";
 }
}

void getmaxmin()
{
    word x,y;
;
 output << "Rotate the joystick around in a circle, as far from the centre as it" << NL;
 output << "can get. Then click a button." << NL;
 {;
  js.xmax=0; js.xmin=maxint;
  js.ymax=0; js.ymin=maxint;
 }
 do {
  readjoya(x,y);
  {;
   if (x<js.xmin)  js.xmin=x;
   if (y<js.ymin)  js.ymin=y;
   if (x>js.xmax)  js.xmax=x;
   if (y>js.ymax)  js.ymax=y;
   display();
  }
 } while (!(buttona1 | buttona2));
 do {; } while (!(~ (buttona1 | buttona2)));
 output << NL;
 output << "Thank you. Now please centre your joystick and hit a button." << NL;
 do {; } while (!(buttona1 | buttona2));
            readjoya(js.xmid,js.ymid);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=2;
 clrscr;
 output << "Avalot joystick setup routine- by TT. Thanks go to David B. Howorth." << NL;
 output << NL;
 if (detect())  output << "You've got a joystick!" << NL; else return 0;
 getmaxmin();
 do {
  output << "Centring factor\? (3-9)";
  input >> js.centre >> NL;
 } while (!(set::of(range(1,9), eos).has(js.centre)));
 assign(jf,"v:joytmp.dat");
 rewrite(jf); jf << js; close(jf);     /* write it all out to disk. */
return EXIT_SUCCESS;
}