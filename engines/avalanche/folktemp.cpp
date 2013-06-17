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

/* Get 'em back! */
#include "graph.h"


const integer picsize = 966;
const integer number_of_objects = 19;

const array<1,65,char> thinks_header = 
 string("This is an Avalot file, which is subject to copyright. Have fun.")+'\32';

const array<0,19,byte> order = 
 {{ 4, 19,  1, 18, 15,  9, 12, 13, 17, 10,  2,  6,  3,  5,  7, 14,
  16,

   0, 11,  8}};

/*
 pAvalot=#150; pSpludwick=#151; pCrapulus=#152; pDrDuck=#153;
 pMalagauche=#154; pFriarTuck=#155; pRobinHood=#156; pCwytalot=#157;
 pduLustie=#158; pDuke=#159; pDogfood=#160; pTrader=#161;
 pIbythneth=#162; pAyles=#163; pPort=#164; pSpurge=#165;
 pJacques=#166;

 pArkata=#175; pGeida=#176; pWiseWoman=#178;
*/

integer gd,gm;
untyped_file f;
pointer p;
byte noo;

void load()
{
 byte a0 /*absolute $A000:1200*/;
 byte bit;
 untyped_file f;
;
 assign(f,"d:folk.avd"); reset(f,1);
 seek(f,177);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  blockread(f,a0,12080);
 }
 close(f); bit=getpixel(0,0);
}

void get_one()
{;

 gd=((order[noo] % 9)*70+10);
 gm=((order[noo] / 9)*40+20);

 getimage(gd,gm,gd+59,gm+29,p);
 putimage(gd,gm,p,notput);
 blockwrite(f,p,picsize);

}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");

 load(); noo=0;

 assign(f,"folk.avd");
 getmem(p,picsize);
 rewrite(f,1);
 blockwrite(f,thinks_header,65);

 for( noo=0; noo <= number_of_objects; noo ++)
  get_one();

 close(f); freemem(p,picsize);
return EXIT_SUCCESS;
}
