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

	Node() {
		_prev = NULL;
		_next = NULL;
	}

	virtual ~Node() {

	}
};

struct WalkNode : public Node {
	int32	_x;
	int32	_y;

public:
	WalkNode() : _x(0), _y(0) {
	}

	WalkNode(int32 x, int32 y) : _x(x), _y(y) {
	}

	WalkNode(const WalkNode& w) : Node(), _x(w._x), _y(w._y) {
		// TODO: This will not properly set _prev and _next
		// -- not sure what would be "correct" here?
	}

	void getPoint(Common::Point &p) const {
		p.x = _x;
		p.y = _y;
	}
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

	StaticCnv() {
		_width = _height = 0;
		_data0 = _data1 = NULL;
	}
};


struct Cnv {
	uint16	_count; 	// # of frames
	uint16	_width; 	//
	uint16	_height;	//
	byte**	field_8;	// unused
	byte*	_data;

public:
	Cnv() {
		_width = _height = _count = 0;
		_data = NULL;
	}

	Cnv(uint16 numFrames, uint16 width, uint16 height, byte* data) : _count(numFrames), _width(width), _height(height), _data(data) {

	}

	~Cnv() {
		if (_count == 0 || _data == NULL) return;
		free(_data);
	}

	byte* getFramePtr(uint16 index) {
		if (index >= _count)
			return NULL;
		return &_data[index * _width * _height];
	}
};

struct Animation;
struct Zone;
struct Label;

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



