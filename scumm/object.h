/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#ifndef OBJECT_H
#define OBJECT_H

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif	

struct RoomHeader {
	union {
		struct {
			uint32 version;
			uint16 width, height;
			uint16 numObjects;
		} GCC_PACK v7;
		struct {
			uint16 width, height;
			uint16 numObjects;
		} GCC_PACK old;
	} GCC_PACK;
} GCC_PACK;

struct CodeHeader {
	union {
		struct {
			uint16 obj_id;
			byte x, y, w, h;
			byte flags;
			byte parent;
			int16 walk_x;
			int16 walk_y;
			byte actordir;
		} GCC_PACK v5;

		struct {
			uint16 obj_id;
			int16 x, y;
			uint16 w, h;
			byte flags, parent;
			uint16 unk1;
			uint16 unk2;
			byte actordir;
		} GCC_PACK v6;

		struct {
			uint32 version;
			uint16 obj_id;
			byte parent;
			byte parentstate;
		} GCC_PACK v7;
							
	} GCC_PACK;
} GCC_PACK;

struct ImageHeader { /* file format */
	union {
		struct {
			uint16 obj_id;
			uint16 unk[5];
			uint16 width;
			uint16 height;
			uint16 unk_2;
			struct {
				int16 x, y;
			} GCC_PACK hotspot[15];
		} GCC_PACK old;

		struct {
			uint32 version;
			uint16 obj_id;
			uint16 unk[1];
			int16 x_pos, y_pos;
			uint16 width, height;
			byte unk2[3];
			byte actordir;
			uint16 unk_2;
			struct {
				int16 x, y;
			} GCC_PACK hotspot[15];
		} GCC_PACK v7;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct FindObjectInRoom {
	CodeHeader *cdhd;
	byte *obcd;
	ImageHeader *imhd;
	byte *obim;
	byte *roomptr;
};

enum FindObjectWhat {
	foCodeHeader = 1,
	foImageHeader = 2,
	foCheckAlreadyLoaded = 4
};


#endif
