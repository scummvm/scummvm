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

#ifndef OBJECT_H
#define OBJECT_H

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
			uint32 unk2;
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
			uint16 unk[5];
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
			uint16 unk[1];
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
			// Most of these seem to have length 0x58.
			// But system-cursor-icon has length 0x60 ?!? --------+
			char name[32];      //                                |
			uint32 unk_1[2];	// always 0 ?                     v
			uint32 version;		// 801; 801; 801; 801; 801; 801; 801
			uint32 unk_2;		//   0;   0;   0;   0;   0:   1;   2
			uint32 x_pos;		//   0; 184; 264; 336; 450; 272;   0
			uint32 y_pos;		//   0; 272; 248; 216; 168; 320;   0
			uint32 width;		//  64; 128; 120; 128;  80;  48;  80
			uint32 height;		// 270;  80;  80;  72;  56;  56;  56
			uint32 actordir;	// 225;  45;  45;  45;  45;   0;   0
			uint32 hotspot_num;	//   0;   0;   0;   0;   0;   0;   1
								// -50; -84; -49; -19;  11; -16;  22
								// 456; 118;  86;  76;  53: -64;  19
								//                                22
								//                                19
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
	const ImageHeader *imhd;
	const byte *obim;
	const byte *roomptr;
};

enum FindObjectWhat {
	foCodeHeader = 1,
	foImageHeader = 2,
	foCheckAlreadyLoaded = 4
};


#endif
