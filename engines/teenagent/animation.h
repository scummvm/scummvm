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


#ifndef TEENAGENT_ANIMATION_H__
#define TEENAGENT_ANIMATION_H__

#include "common/stream.h"
#include "surface.h"

namespace TeenAgent {
class Animation  {
public: 
	uint16 id, x, y;
	bool loop;
	
	enum Type {TypeLan, TypeVaria, TypeInventory};
	
	Animation();
	void load(Common::SeekableReadStream * s, Type type = TypeLan);
	void free();
	
	Surface * currentFrame(int dt = 1);
	~Animation();
	
	bool empty() const { return frames == NULL; }
	
	//uint16 width() const { return frames? frames[0].w: 0; }
	//uint16 height() const { return frames? frames[0].h: 0; }

protected:
	byte * data;
	uint16 data_size;
	
	uint16 frames_count;
	Surface * frames;
	uint16 index;
};
}


#endif

