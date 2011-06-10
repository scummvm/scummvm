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

#ifndef		__GAME__
#define		__GAME__

#include	"cge/vga13h.h"
#include	"cge/bitmaps.h"

namespace CGE {


#define		PAN_HIG		40
#define		LBound(s)	(s->X <= 0)
#define		RBound(s)	(s->X+s->W >= SCR_WID)
#define		TBound(s)	(s->Y <= 0)
#define		BBound(s)	(s->Y+s->H >= SCR_HIG - PAN_HIG)



extern		SPRITE *	Sys;

int		Sinus		(long x);
uint8 *		Glass		(DAC * pal, uint8 r, uint8 g, uint8 b);
uint8 *		Mark		(DAC * pal);





class FLY : public SPRITE
{
  static int L, T, R, B;
public:
  int Tx, Ty;
  FLY (BITMAP ** shpl);
  void Tick (void);
};



} // End of namespace CGE

#endif
