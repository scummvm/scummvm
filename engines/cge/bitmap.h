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

#ifndef		__BITMAP__
#define		__BITMAP__

#include	"cge/general.h"

#define		EOI		0x0000
#define		SKP		0x4000
#define		REP		0x8000
#define		CPY		0xC000

#define		TRANS		0xFE


typedef	struct	{ word b : 2;
		  word B : 6;
		  word g : 2;
		  word G : 6;
		  word r : 2;
		  word R : 6;
		  word Z : 8;
		} BGR4;


typedef	struct	{ word skip; word hide; } HideDesc;




class BITMAP
{
  Boolean BMPLoad (XFILE * f);
  Boolean VBMLoad (XFILE * f);
public:
  static DAC far * Pal;
  word W, H;
  byte far * M, far * V; HideDesc far * B;
  BITMAP (const char * fname, Boolean rem = TRUE);
  BITMAP (word w, word h, byte far * map);
  BITMAP (word w, word h, byte fill);
  BITMAP (const BITMAP& bmp);
  ~BITMAP (void);
  BITMAP * FlipH (void);
  BITMAP * Code ();
  BITMAP& operator = (const BITMAP& bmp);
  void Hide (int x, int y);
  void Show (int x, int y);
  void XShow (int x, int y);
  Boolean SolidAt (int x, int y);
  Boolean VBMSave (XFILE * f);
  word MoveVmap (byte far * buf);
};



typedef	BITMAP *	BMP_PTR;




#endif