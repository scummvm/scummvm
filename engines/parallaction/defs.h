/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_DEFS_H
#define PARALLACTION_DEFS_H

#include "common/stdafx.h"
#include "common/system.h"

namespace Parallaction {

#define PATH_LEN	200




struct Node {
	Node*	_prev;
	Node*	_next;
};

struct WalkNode {
	Node	_node;
	int32	_x;
	int32	_y;
};

struct Point {
	int16	_x;
	int16	_y;
};

struct Rect {
	int16	_left;
	int16	_top;
	int16	_right;
	int16	_bottom;
};

struct SpeakData;
struct Question;
typedef Question Dialogue;
struct Instruction;
struct LocalVariable;

struct StaticCnv {
	uint16	_width; 	//
	uint16	_height;	//
	byte*	_data0; 	// bitmap
	byte*	_data1; 	// unused
};


struct Cnv {
	uint16	_width; 	//
	uint16	_height;	//
	byte**	field_8;	// unused
	uint16	_count; 	// # of frames
	byte**	_array; 	// frames data

public:
	byte* getFramePtr(uint16 index) {
		if (index >= _count)
			error("frame %i does not exist", index);
		return _array[index];
	}
};

struct Animation;
struct Zone;
struct ZoneLabel;

struct Command;

typedef void (*callable)(void*);

struct Credit {
	const char *_role;
	const char *_name;
};

void errorFileNotFound(const char*);

void beep();

enum {
	kDebugDisk = 1 << 0,
	kDebugWalk = 1 << 1,
	kDebugLocation = 1 << 2,
	kDebugDialogue = 1 << 3,
	kDebugGraphics = 1 << 4,
	kDebugJobs = 1 << 5,
	kDebugInput = 1 << 6
};

} // namespace Parallaction


#endif


