/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENGRAPHICS_H
#define QUEENGRAPHICS_H

#include "queen/queen.h"


#define MAX_BANK_SIZE      110
#define MAX_FRAMES_NUMBER  256
#define MAX_BANKS_NUMBER    18



struct ObjectFrame {
  uint16 width, height;
  uint16 xhotspot, yhotspot;
  uint8 *data;
};


class QueenGraphics {
public:

	QueenGraphics(QueenResource *resource);

	void bankLoad(const char *bankname, uint32 bankslot);
	void bankUnpack(uint srcframe, uint dstframe, uint32 bankslot);
	void bankOverpack(uint srcframe, uint dstframe, uint32 bankslot);
	void bankErase(uint32 bankslot);
	
private:
	
	struct PackedBank {
		uint32 indexes[MAX_BANK_SIZE];
		uint8 *data;
	};

	ObjectFrame _frames[MAX_FRAMES_NUMBER];
	PackedBank _banks[MAX_BANKS_NUMBER];

	QueenResource *_resource;
	
};


#endif
