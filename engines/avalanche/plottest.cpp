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

integer gd,gm; byte fv;
matrix<1,35,0,39,byte> a;
array<0,3,0,4,1,35,byte> b;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 for( gd=0; gd <= 39; gd ++)
  for( gm=1; gm <= 35; gm ++)
   a[gm][gd]=(gd+gm) % 16;
 for( gd=0; gd <= 39; gd ++)
  for( gm=1; gm <= 35; gm ++)
   putpixel(gd+100,gm+100,a[gm][gd]);

 fillchar(b,sizeof(b),'\0');
 for( gm=1; gm <= 35; gm ++)
  for( gd=0; gd <= 39; gd ++)
  {;
   for( fv=0; fv <= 3; fv ++)
   {;
    b[fv][gd / 8][gm]=(b[fv][gd / 8][gm] << 1);
    b[fv][gd / 8][gm] += ((cardinal)(a[gm][gd] & (1 << fv)) >> fv);
   }
  }

 for( gd=1; gd <= 35; gd ++)
  for( gm=0; gm <= 4; gm ++)
  {;
   for( fv=0; fv <= 3; fv ++)
   {;
    port[0x3c4]=2; port[0x3ce]=4;
    port[0x3c5]=1 << fv; port[0x3cf]=fv;
    mem[0xa000*gd*80+gm]=b[fv][gm][gd];
   }
  }
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.