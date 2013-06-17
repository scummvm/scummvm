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

/* Phew! */
#include "graph.h"

namespace Avalanche {

/*$S-*/
integer gd,gm;
untyped_file f;
text t;
string x;

boolean subpix(integer x,integer y)
{boolean subpix_result;
;
 subpix_result=getpixel(x,y)==15;
return subpix_result;
}

byte pixel(integer x,integer y)
{byte pixel_result;
;
 pixel_result=(byte)(
   subpix(x*4,y*2) ||
   subpix(x*4+1,y*2) ||
   subpix(x*4+2,y*2) ||
   subpix(x*4+3,y*2) ||
   subpix(x*4,y*2+1) ||
   subpix(x*4+1,y*2+1) ||
   subpix(x*4+2,y*2+1) ||
   subpix(x*4+3,y*2+1)
  )*15;
return pixel_result;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=1; initgraph(gd,gm,"o:");
 assign(f,"v:logo.avd"); reset(f,1);
 for( gd=0; gd <= 180; gd ++)
  blockread(f,mem[0xa000*(gd*80)],53);
 close(f);
 for( gd=1; gd <= 106; gd ++)
  for( gm=0; gm <= 145; gm ++)
   putpixel(gd,gm+181,pixel(gd,gm));
 assign(t,"d:avalot.txt");
 rewrite(t);
 for( gm=1; gm <= 36; gm ++)
 {;
  x="";
  for( gd=1; gd <= 106; gd ++)
   switch (getpixel(gd,gm*2+181)*2+getpixel(gd,gm*2+182)) {
     case 0: x=x+' '; break;
    case 15: x=x+'Ü'; break;
    case 30: x=x+'ß'; break;
    case 45: x=x+'Û'; break;
   }
  t << x << NL;
 }
 close(t);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.