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

#define __cadburys_implementation__


#include "cadburys.h"


#include "graph.h"

namespace Avalanche {

void mgrab(integer x1,integer y1,integer x2,integer y2, word size)
{
    integer yy; word aapos; byte length,bit;
;
 if (size>arraysize) 
 {;
  output << "*** SORRY! *** Increase the arraysize constant to be greater" << NL;
  output << " than " << size << '.' << NL;
  exit(0);
 }

 aapos=0;

 length=x2-x1;

 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  for( yy=y1; yy <= y2; yy ++)
  {;
   move(mem[0xa400*yy*80+x1],aa[aapos],length);
   aapos += length;
  }
 }
 bit=getpixel(0,0);

}

void grab(integer x1,integer y1,integer x2,integer y2,integer realx,integer realy, flavourtype flav,
 boolean mem,boolean nat)
 /* yes, I *do* know how to spell "really"! */
{
 word s;
 pointer p;
 chunkblocktype ch;
;
/* rectangle(x1,y1,x2,y2); exit;*/
 this_chunk += 1;
 offsets[this_chunk]=filepos(f);


 s=imagesize(x1,y1,x2,y2);
 getmem(p,s);
 getimage(x1,y1,x2,y2,p);

 {;
  ch.flavour=flav;
  ch.x=realx; ch.y=realy;

  ch.xl=x2-x1;
  ch.yl=y2-y1;
  ch.size=s;
  ch.memorise=mem;
  ch.natural=nat;
 }

 setvisualpage(1);
 setactivepage(1);
 input >> NL;
 putimage(ch.x,ch.y,p,0);

 if (flav==ch_ega) 
 {;
  freemem(p,s);
  s=4*(((x2 / 8)-(x1 / 8))+2)*(y2-y1+1);
  {;
   ch.size=s;
   ch.x=ch.x / 8;
   ch.xl=((realx-ch.x*8)+(x2-x1)+7) / 8;
   mgrab(ch.x,ch.y,ch.x+ch.xl,ch.y+ch.yl,s);
  }
 } else
             /* For BGI pictures. */
  {;
   ch.x=ch.x / 8;
   ch.xl=(ch.xl+7) / 8;
   ch.size=imagesize(ch.x*8,ch.y,(ch.x+ch.xl)*8,ch.y+ch.yl);
  }

 input >> NL;
 setvisualpage(0);
 setactivepage(0);

 blockwrite(f,ch,sizeof(ch));

 switch (flav) {
  case ch_ega : if (! nat)  blockwrite(f,aa,s); break;
  case ch_bgi : {;
            if (! nat)  blockwrite(f,p,s);
            freemem(p,s);
           }
           break;
 }
/* rectangle(x1,y1,x2,y2);*/
}

} // End of namespace Avalanche.