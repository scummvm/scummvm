/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdafx.h>
#include "codec47.h"
#include "chunk.h"
#include "blitter.h"

DumpDecoder::~DumpDecoder() {
}

bool DumpDecoder::decode(Blitter & dst, Chunk & src) {
	int n = 0, i = 0;
	int seq = src.getWord();
	int codec = src.getByte();
	int flags = src.getByte();
	int unknown[22];
	for(i = 0; i < 0; i++) {
		unknown[i] = src.getByte();
	}
	if(codec == 5 || codec == 1) {
		do {
			int code = src.getByte();
			int length = (code >> 1) + 1;
			if (code & 1)
				dst.put(src.getChar(), length);
			else
				dst.blit(src, length);
		} while (!src.eof());
	}

	return true;
}
