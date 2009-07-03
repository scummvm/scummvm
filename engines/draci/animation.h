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
 
#ifndef DRACI_ANIMATION_H
#define DRACI_ANIMATION_H

#include "draci/sprite.h"

namespace Draci {

enum { kOverlayImage = -1 };

class DraciEngine;

struct AnimObj {
	uint _id;	
	uint _currentFrame;
	uint _z;
	Common::Array<Drawable*> _frames;
};

class Animation {

public:
	Animation(DraciEngine *vm) : _vm(vm) {};
	~Animation() { deleteAll(); }

	void addAnimation(uint id, uint z = 0);
	void addFrame(uint id, Drawable *frame);
	void addOverlay(Drawable *overlay, uint z = 0);
	void deleteAnimation(uint id);
	void deleteAll();
	void drawScene(Surface *surf);
	Common::List<AnimObj>::iterator getAnimation(uint id);

private:
	
	void insertAnimation(AnimObj &animObj);

	DraciEngine *_vm;
	Common::List<AnimObj> _animObjects;
};

}

#endif // DRACI_ANIMATION_H
