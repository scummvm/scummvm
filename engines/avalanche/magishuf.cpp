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

/*#include "Dos.h"*/
/*#include "Crt.h"*/

namespace Avalanche {

const word nextcode = 17717;

typedef array<1,8000,word> big;
typedef array<1,16401,word> circle;

big b;
file<big> f;
registers r;
word gd;
circle c;
file<circle> fc;

void melt(byte c)
{;
 for( gd=1; gd <= 8000; gd ++)
 {;
  mem[0xa000*b[gd]]=c;
  if ((gd % 17)==0)  delay(1);
 }
}

void magicirc(byte cc)
{;
 for( gd=1; gd <= 16401; gd ++)
 {;
  if (c[gd]!=nextcode) 
  {;
   if (odd(c[gd]))  mem[0xa000*c[gd] / 2]=cc;
  } else
   delay(1);
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 r.ax=13; intr(0x10,r);
 assign(f,"d:shuffle.avd"); reset(f); f >> b; close(f);
 assign(fc,"v:magic2.avd"); reset(fc); fc >> c; close(fc);
/* repeat
  melt(170); magicirc(85);
  magicirc(170); melt(85);
  magicirc(170); magicirc(85);
  melt(170); melt(85);
 until keypressed;*/
 do {
  melt(255); magicirc(0);
  magicirc(255); melt(0);
  magicirc(255); magicirc(0);
  melt(255); melt(0);
 } while (!keypressed());
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.