/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "p4.h"

#include "engines/icb/common/px_common.h"

#include "debug.h"
#include "protocol.h"

#include "res_man.h"

#include "global_objects.h"

namespace ICB {

_animHeader *FetchAnimHeader(uint8 *animFile) {
#ifdef _PSX
	_animHeader *ah;
	// can't do this as not DWORD aligned
	// int ls = sizeof(_standardHeader);
	int ls = 0x2C;
	ah = (_animHeader *)(animFile + ls);
	return ah;
#else
	return (_animHeader *)(animFile + sizeof(_standardHeader));
#endif
}

_cdtEntry *FetchCdtEntry(uint8 *animFile, uint16 frameNo) {
	_animHeader *animHead;

	animHead = FetchAnimHeader(animFile);

#ifdef _PSX
	_cdtEntry *cdte;
	// can't do this as not DWORD aligned
	// int la = sizeof(_animHeader);
	// int lc = sizeof(_cdtEntry );
	int la = 15;
	int lc = 9;
	int l3 = frameNo * lc;
	cdte = (_cdtEntry *)((uint8 *)animHead + la + l3);
	return cdte;
#else
	return (_cdtEntry *)((uint8 *)animHead + sizeof(_animHeader) + frameNo * sizeof(_cdtEntry));
#endif
}

_frameHeader *FetchFrameHeader(uint8 *animFile, uint16 frameNo) {
	// required address = (address of the start of the anim header) + frameOffset
#ifdef _PSX
	_frameHeader *fh;
	// can't do this as not DWORD aligned
	// int ls = sizeof(_standardHeader);
	int ls = 0x2C;
	_cdtEntry *cdte = FetchCdtEntry(animFile, frameNo);
	int fo;
	// Only do memcpy when necessary
	int *ptr = (int *)cdte + 1;

	// memcpy( (unsigned char*)&fo, from, 4 );
	uint8 *from = (uint8 *)ptr;
	uint8 *to = (uint8 *)&fo;
	// 4 byte memcpy
	*to++ = *from++;
	*to++ = *from++;
	*to++ = *from++;
	*to = *from;

	fh = (_frameHeader *)(animFile + ls + fo);
	return fh;
#else
	return (_frameHeader *)(animFile + sizeof(_standardHeader) + (FetchCdtEntry(animFile, frameNo)->frameOffset));
#endif
}

} // End of namespace ICB
