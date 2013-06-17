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
/*#include "Dos.h"*/


untyped_file f;
byte bit;
byte a;  /*absolute $A000:0; */
integer gd,gm;

void graphmode(integer mode)
{
    registers regs;
;
 regs.ax=(mode % 0x100);
 intr(0x10,regs);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 assign(f,"d:avltzap.raw"); reset(f,1);
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  for( gd=0; gd <= 199; gd ++)
   blockread(f,mem[0xa000*gd*80],40); /* 28000 */
 }
 close(f);
 setwritemode(xorput);
 rectangle(  0,  0,  5,  8);
 rectangle(  0, 10, 27, 19);
return EXIT_SUCCESS;
}
