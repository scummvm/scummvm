/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif	

struct ResHdr {
	uint32 tag, size;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

#define RES_DATA(x) (((const byte*)x) + sizeof(ResHdr))
#define RES_SIZE(x) (READ_BE_UINT32(&((const ResHdr* )x)->size))

enum {
	OF_OWNER_MASK = 0x0F,
	OF_STATE_MASK = 0xF0,
	
	OF_STATE_SHL = 4
};

enum {
	RF_LOCK = 0x80,
	RF_USAGE = 0x7F,
	RF_USAGE_MAX = RF_USAGE
};


const byte *findResource(uint32 tag, const byte *searchin, int index);
const byte *findResourceSmall(uint32 tag, const byte *searchin, int index);
const byte *findResource(uint32 tag, const byte *searchin);
const byte *findResourceSmall(uint32 tag, const byte *searchin);

#endif
