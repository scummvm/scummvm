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


#include "graphics.h"
#include "disk.h"

#include "common/algorithm.h"
#include "parallaction/parallaction.h"

namespace Parallaction {

GfxObj::GfxObj(uint objType, Frames *frames, const char* name) : type(objType), _frames(frames), x(0), y(0), z(0), frame(0), layer(3), _flags(0), _keep(true) {
	if (name) {
		_name = strdup(name);
	} else {
		_name = 0;
	}
}

GfxObj::~GfxObj() {
	delete _frames;
	free(_name);
}

void GfxObj::release() {
//	_keep = false;
	delete this;
}

const char *GfxObj::getName() const {
	return _name;
}


uint GfxObj::getNum() {
	return _frames->getNum();
}


void GfxObj::getRect(uint f, Common::Rect &r) {
	_frames->getRect(f, r);
}


byte *GfxObj::getData(uint f) {
	return _frames->getData(f);
}

uint GfxObj::getRawSize(uint f) {
	return _frames->getRawSize(f);
}
uint GfxObj::getSize(uint f) {
	return _frames->getSize(f);
}


void GfxObj::setFlags(uint32 flags) {
	_flags |= flags;
}

void GfxObj::clearFlags(uint32 flags) {
	_flags &= ~flags;
}

GfxObj* Gfx::loadAnim(const char *name) {
	GfxObj *obj = _disk->loadFrames(name);
	assert(obj);

	obj->type = kGfxObjTypeAnim;
	return obj;
}


GfxObj* Gfx::loadGet(const char *name) {
	GfxObj *obj = _disk->loadStatic(name);
	assert(obj);

	obj->type = kGfxObjTypeGet;
	return obj;
}

GfxObj* Gfx::loadDoor(const char *name) {
	GfxObj *obj = _disk->loadFrames(name);
	assert(obj);

	obj->type = kGfxObjTypeDoor;
	return obj;
}

void Gfx::clearGfxObjects() {
	_gfxobjList[0].clear();
	_gfxobjList[1].clear();
	_gfxobjList[2].clear();
}

void Gfx::showGfxObj(GfxObj* obj, bool visible) {
	if (!obj || obj->isVisible() == visible) {
		return;
	}

	if (visible) {
		obj->setFlags(kGfxObjVisible);
		_gfxobjList[obj->type].push_back(obj);
	} else {
		obj->clearFlags(kGfxObjVisible);
		_gfxobjList[obj->type].remove(obj);
	}

}



bool compareAnimationZ(const GfxObj* a1, const GfxObj* a2) {
	return a1->z < a2->z;
}

void Gfx::sortAnimations() {
	GfxObjList::iterator first = _gfxobjList[kGfxObjTypeAnim].begin();
	GfxObjList::iterator last = _gfxobjList[kGfxObjTypeAnim].end();

	Common::sort(first, last, compareAnimationZ);
}

void Gfx::drawGfxObjects(Graphics::Surface &surf) {

	Common::Rect rect;
	byte *data;

	sortAnimations();
	// TODO: some zones don't appear because of wrong masking (3 or 0?)
	// TODO: Dr.Ki is not visible inside the club


	for (uint i = 0; i < 3; i++) {

		GfxObjList::iterator b = _gfxobjList[i].begin();
		GfxObjList::iterator e = _gfxobjList[i].end();

		for (; b != e; b++) {
			GfxObj *obj = *b;
			if (obj->isVisible()) {
				obj->getRect(obj->frame, rect);
				rect.translate(obj->x - _varScrollX, obj->y);
				data = obj->getData(obj->frame);
				if (obj->getSize(obj->frame) == obj->getRawSize(obj->frame)) {
					blt(rect, data, &surf, obj->layer, 0);
				} else {
					unpackBlt(rect, data, obj->getRawSize(obj->frame), &surf, obj->layer, 0);
				}
			}
		}
	}
}

} // namespace Parallaction
