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
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/scummsys.h"

namespace Mortevielle {

/**
 * Decodes a number of 64 byte blocks
 * @param pStart	Start of data
 * @param count		Number of 64 byte blocks
 */
void zzuul(byte *pStart, int count) {
	while (count-- > 0) {
		for (int idx = 0; idx < 64; ++pStart, ++idx) {
			uint16 v = ((*pStart - 0x80) << 1) + 0x80;

			if (v & 0x8000)
				*pStart = 0;
			else if (v & 0xff00)
				*pStart = 0xff;
			else 
				*pStart = (byte)v;
		}
	}
}

} // End of namespace Mortevielle
