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

#ifndef PARALLACTION_WALK_H
#define PARALLACTION_WALK_H

#include "parallaction/defs.h"

namespace Parallaction {

struct Animation;
struct Job;

struct WalkNode {
	int16	_x;
	int16	_y;

public:
	WalkNode();
	WalkNode(int16 x, int16 y);
	WalkNode(const WalkNode& w);

	void getPoint(Common::Point &p) const;
};

typedef ManagedList<WalkNode*> WalkNodeList;


void 		 jobWalk(void*, Job *j);

struct PathBuffer {
	// handles a 1-bit depth buffer used for masking non-walkable areas

	uint16	w;
	uint16  internalWidth;
	uint16	h;
	uint	size;
	byte	*data;

public:
	PathBuffer() : w(0), internalWidth(0), h(0), size(0), data(0) {
	}

	void create(uint16 width, uint16 height) {
		w = width;
		internalWidth = w >> 3;
		h = height;
		size = (internalWidth * h);
		data = (byte*)calloc(size, 1);
	}

	void free() {
		if (data)
			::free(data);
		data = 0;
		w = 0;
		h = 0;
		internalWidth = 0;
		size = 0;
	}

	inline byte getValue(uint16 x, uint16 y);
};


class PathBuilder {

	Animation 		*_anim;

	WalkNodeList 	*_list;
	Common::List<WalkNode*> 	_subPath;

	void correctPathPoint(Common::Point &to);
	uint32 buildSubPath(const Common::Point& pos, const Common::Point& stop);
	uint16 walkFunc1(int16 x, int16 y, WalkNode *Node);

public:
	PathBuilder(Animation *anim);
	WalkNodeList* buildPath(uint16 x, uint16 y);

};


}

#endif
