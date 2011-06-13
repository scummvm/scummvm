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

#ifndef __BITMAP__
#define __BITMAP__

#include "cge/general.h"

namespace CGE {

#define EOI     0x0000
#define SKP     0x4000
#define REP     0x8000
#define CPY     0xC000

#define TRANS   0xFE


typedef struct  {
	uint16 b : 2;
	uint16 B : 6;
	uint16 g : 2;
	uint16 G : 6;
	uint16 r : 2;
	uint16 R : 6;
	uint16 Z : 8;
} BGR4;


typedef struct  {
	uint16 skip;
	uint16 hide;
} HideDesc;


class BITMAP {
	bool BMPLoad(XFILE *f);
	bool VBMLoad(XFILE *f);
public:
	static DAC *Pal;
	uint16 W, H;
	uint8 *M, * V;
	HideDesc *B;
	BITMAP(const char *fname, bool rem = true);
	BITMAP(uint16 w, uint16 h, uint8 *map);
	BITMAP(uint16 w, uint16 h, uint8 fill);
	BITMAP(const BITMAP &bmp);
	~BITMAP(void);
	BITMAP *FlipH(void);
	BITMAP *Code();
	BITMAP &operator = (const BITMAP &bmp);
	void Hide(int x, int y);
	void Show(int x, int y);
	void XShow(int x, int y);
	bool SolidAt(int x, int y);
	bool VBMSave(XFILE *f);
	uint16 MoveVmap(uint8 *buf);
};


typedef BITMAP     *BMP_PTR;

} // End of namespace CGE

#endif
