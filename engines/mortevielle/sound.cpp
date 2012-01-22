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
#include "mortevielle/sound.h"

namespace Mortevielle {

const int tab[16] = { -96, -72, -48, -32, -20, -12, -8, -4, 0, 4, 8, 12, 20, 32, 48, 72 };

/**
 * Decode music data
 */
void demus(const byte *PSrc, byte *PDest, int NbreSeg) {
	int seed = 128;
	int v;

	for (int idx1 = 0; idx1 < (NbreSeg * 2); ++idx1) {
		for (int idx2 = 0; idx2 < 64; ++idx2) {
			byte srcByte = *PSrc++;
			v = tab[srcByte >> 4];
			seed += v;
			*PDest++ = seed & 0xff;

			v = tab[srcByte & 0xf];
			seed += v;
			*PDest++ = seed & 0xff;
		}
	}
}

} // End of namespace Mortevielle
