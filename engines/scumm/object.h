/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#ifndef OBJECT_H
#define OBJECT_H

namespace Scumm {

enum ObjectClass {
	kObjectClassNeverClip = 20,
	kObjectClassAlwaysClip = 21,
	kObjectClassIgnoreBoxes = 22,
	kObjectClassYFlip = 29,
	kObjectClassXFlip = 30,
	kObjectClassPlayer = 31,	// Actor is controlled by the player
	kObjectClassUntouchable = 32
};

struct ObjectData {
	uint32 OBIMoffset;
	uint32 OBCDoffset;
	int16 walk_x, walk_y;
	uint16 obj_nr;
	int16 x_pos;
	int16 y_pos;
	uint16 width;
	uint16 height;
	byte actordir;
	byte parent;
	byte parentstate;
	byte state;
	byte fl_object_index;
	byte flags;
};

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct RoomHeader {
	union {
		struct {
			uint16 width, height;
			uint16 numObjects;
		} GCC_PACK old;

		struct {
			uint32 version;
			uint16 width, height;
			uint16 numObjects;
		} GCC_PACK v7;

		struct {
			uint32 version;
			uint32 width, height;
			uint32 numObjects;
			uint32 numZBuffer;
			uint32 transparency;
		} GCC_PACK v8;
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
			uint16 image_count;
			uint16 unk[1];
			byte flags;
			byte unk1;
			uint16 unk2[2];
			uint16 width;
			uint16 height;
			uint16 hotspot_num;
			struct {
				int16 x, y;
			} GCC_PACK hotspot[15];
		} GCC_PACK old;

		struct {
			uint32 version;
			uint16 obj_id;
			uint16 image_count;
			int16 x_pos, y_pos;
			uint16 width, height;
			byte unk2[3];
			byte actordir;
			uint16 hotspot_num;
			struct {
				int16 x, y;
			} GCC_PACK hotspot[15];
		} GCC_PACK v7;

		struct {
			char name[32];
			uint32 unk_1[2];
			uint32 version;		// 801 in COMI, 800 in the COMI demo
			uint32 image_count;
			uint32 x_pos;
			uint32 y_pos;
			uint32 width;
			uint32 height;
			uint32 actordir;
			uint32 flags;	// This field is missing in the COMI demo (version == 800) !
			struct {
				int32 x, y;
			} GCC_PACK hotspot[15];
		} GCC_PACK v8;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct FindObjectInRoom {
	const CodeHeader *cdhd;
	const byte *obcd;
	const byte *obim;
	const byte *roomptr;
};

enum FindObjectWhat {
	foCodeHeader = 1,
	foImageHeader = 2,
	foCheckAlreadyLoaded = 4
};

} // End of namespace Scumm


#endif
