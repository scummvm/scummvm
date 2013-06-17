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

namespace Avalanche {

const longint pagetop = 81920;
const word nextcode = 17717;

integer gd,gm;
file<word> magic,out;
word next,gg;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 assign(magic,"v:magicirc.avd"); reset(magic);
 assign(out,"v:magic2.avd"); rewrite(out);
 move(mem[0xa000*0],mem[0xa000*pagetop],16000);
 while (! eof(magic)) 
 {;
  magic >> next;
  if (next!=nextcode) 
   mem[0xa000*next]=255;
  else
  {;
   for( gg=0; gg <= 16000; gg ++)
    if (mem[0xa000*gg]!=mem[0xa000*gg+pagetop]) 
     out << gg;
   out << nextcode;
   move(mem[0xa000*0],mem[0xa000*pagetop],16000);
  }
 }
 close(magic); close(out);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.