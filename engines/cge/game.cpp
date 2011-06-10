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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include	"cge/game.h"
#include	"cge/mouse.h"
#include	<stdlib.h>
#include	<dos.h>





uint8 * Glass (DAC * pal, uint8 r, uint8 g, uint8 b)
{
  uint8 * x = new uint8[256];
  if (x)
    {
      uint16 i;
      for (i = 0; i < 256; i ++)
	{
	  x[i] = Closest(pal, MkDAC(((uint16)(pal[i].R) * r) / 255,
				    ((uint16)(pal[i].G) * g) / 255,
				    ((uint16)(pal[i].B) * b) / 255));
	}
    }
  return x;
}





uint8 * Mark (DAC * pal)
{
  #define f(c) (c ^ 63)
  uint8 * x = new uint8[256];
  if (x)
    {
      uint16 i;
      for (i = 0; i < 256; i ++)
	{
	  x[i] = Closest(pal, MkDAC(f(pal[i].R),
				    f(pal[i].G),
				    f(pal[i].B))  );
	}
    }
  return x;
  #undef f
}





//--------------------------------------------------------------------------



int	FLY::L = 20,
	FLY::T = 40,
	FLY::R = 110,
	FLY::B = 100;



FLY::FLY (BITMAP ** shpl)
: SPRITE(shpl), Tx(0), Ty(0)
{
  Step(random(2));
  Goto(L+random(R-L-W), T+random(B-T-H));
}




void FLY::Tick (void)
{
  Step();
  if (! Flags.Kept)
    {
      if (random(10) < 1)
	{
	  Tx = random(3) - 1;
	  Ty = random(3) - 1;
	}
      if (X + Tx < L || X + Tx + W > R) Tx = -Tx;
      if (Y + Ty < T || Y + Ty + H > B) Ty = -Ty;
      Goto(X + Tx, Y + Ty);
    }
}


//--------------------------------------------------------------------------

