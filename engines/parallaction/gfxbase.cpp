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

	// animation Z is not set here, but controlled by game scripts and user interaction.
	// it is always >=0 and <screen height
	obj->type = kGfxObjTypeAnim;
	obj->transparentKey = 0;
	_gfxobjList.push_back(obj);
	return obj;
}


GfxObj* Gfx::loadGet(const char *name) {
	GfxObj *obj = _disk->loadStatic(name);
	assert(obj);

	obj->z = kGfxObjGetZ;	// this preset Z value ensures that get zones are drawn after doors but before animations
	obj->type = kGfxObjTypeGet;
	obj->transparentKey = 0;
	_gfxobjList.push_back(obj);
	return obj;
}

GfxObj* Gfx::loadDoor(const char *name) {
	GfxObj *obj = _disk->loadFrames(name);
	assert(obj);

	obj->z = kGfxObjDoorZ;	// this preset Z value ensures that doors are drawn first
	obj->type = kGfxObjTypeDoor;
	obj->transparentKey = 0;
	_gfxobjList.push_back(obj);
	return obj;
}

void Gfx::clearGfxObjects() {
	_gfxobjList.clear();
}

void Gfx::showGfxObj(GfxObj* obj, bool visible) {
//	if (!obj || obj->isVisible() == visible) {
//		return;
//	}

	assert(obj);

	if (visible) {
		obj->setFlags(kGfxObjVisible);
	} else {
		obj->clearFlags(kGfxObjVisible);
//		_gfxobjList.remove(obj);
	}

}



bool compareZ(const GfxObj* a1, const GfxObj* a2) {
	return a1->z < a2->z;
}

void Gfx::sortAnimations() {
	GfxObjList::iterator first = _gfxobjList.begin();
	GfxObjList::iterator last = _gfxobjList.end();

	Common::sort(first, last, compareZ);
}


void Gfx::drawGfxObject(GfxObj *obj, Graphics::Surface &surf, bool scene) {
	if (!obj->isVisible()) {
		return;
	}

	Common::Rect rect;
	byte *data;

	uint scrollX = (scene) ? -_varScrollX : 0;

	obj->getRect(obj->frame, rect);
	rect.translate(obj->x + scrollX, obj->y);
	data = obj->getData(obj->frame);

	if (obj->getSize(obj->frame) == obj->getRawSize(obj->frame)) {
		blt(rect, data, &surf, obj->layer, obj->transparentKey);
	} else {
		unpackBlt(rect, data, obj->getRawSize(obj->frame), &surf, obj->layer, obj->transparentKey);
	}

}


void Gfx::drawGfxObjects(Graphics::Surface &surf) {

	sortAnimations();
	// TODO: some zones don't appear because of wrong masking (3 or 0?)
	// TODO: Dr.Ki is not visible inside the club


	GfxObjList::iterator b = _gfxobjList.begin();
	GfxObjList::iterator e = _gfxobjList.end();

	for (; b != e; b++) {
		drawGfxObject(*b, surf, true);
	}
}



void Gfx::drawText(Font *font, Graphics::Surface* surf, uint16 x, uint16 y, const char *text, byte color) {
	byte *dst = (byte*)surf->getBasePtr(x, y);
	font->setColor(color);
	font->drawString(dst, surf->w, text);
}

void Gfx::drawWrappedText(Font *font, Graphics::Surface* surf, char *text, byte color, int16 wrapwidth) {

	uint16 lines = 0;
	uint16 linewidth = 0;

	uint16 rx = 10;
	uint16 ry = 4;

	uint16 blankWidth = font->getStringWidth(" ");
	uint16 tokenWidth = 0;

	char token[MAX_TOKEN_LEN];

	if (wrapwidth == -1)
		wrapwidth = _vm->_screenWidth;

	while (strlen(text) > 0) {

		text = parseNextToken(text, token, MAX_TOKEN_LEN, "   ", true);

		if (!scumm_stricmp(token, "%p")) {
			lines++;
			rx = 10;
			ry = 4 + lines*10;	// y

			strcpy(token, "> .......");
			strncpy(token+2, _password, strlen(_password));
			tokenWidth = font->getStringWidth(token);
		} else {
			tokenWidth = font->getStringWidth(token);

			linewidth += tokenWidth;

			if (linewidth > wrapwidth) {
				// wrap line
				lines++;
				rx = 10;			// x
				ry = 4 + lines*10;	// y
				linewidth = tokenWidth;
			}

			if (!scumm_stricmp(token, "%s")) {
				sprintf(token, "%d", _score);
			}

		}

		drawText(font, surf, rx, ry, token, color);

		rx += tokenWidth + blankWidth;
		linewidth += blankWidth;

		text = Common::ltrim(text);
	}

}


// this is the maximum size of an unpacked frame in BRA
byte _unpackedBitmap[640*401];

#if 0
void Gfx::unpackBlt(const Common::Rect& r, byte *data, uint size, Graphics::Surface *surf, uint16 z, byte transparentColor) {

	byte *d = _unpackedBitmap;

	while (size > 0) {

		uint8 p = *data++;
		size--;
		uint8 color = p & 0xF;
		uint8 repeat = (p & 0xF0) >> 4;
		if (repeat == 0) {
			repeat = *data++;
			size--;
		}

		memset(d, color, repeat);
		d += repeat;
	}

	blt(r, _unpackedBitmap, surf, z, transparentColor);
}
#endif
void Gfx::unpackBlt(const Common::Rect& r, byte *data, uint size, Graphics::Surface *surf, uint16 z, byte transparentColor) {

	byte *d = _unpackedBitmap;
	uint pixelsLeftInLine = r.width();

	while (size > 0) {
		uint8 p = *data++;
		size--;
		uint8 color = p & 0xF;
		uint8 repeat = (p & 0xF0) >> 4;
		if (repeat == 0) {
			repeat = *data++;
			size--;
		}
		if (repeat == 0) {
			// end of line
			repeat = pixelsLeftInLine;
			pixelsLeftInLine = r.width();
		} else {
			pixelsLeftInLine -= repeat;
		}

		memset(d, color, repeat);
		d += repeat;
	}

	blt(r, _unpackedBitmap, surf, z, transparentColor);
}


void Gfx::blt(const Common::Rect& r, byte *data, Graphics::Surface *surf, uint16 z, byte transparentColor) {

	Common::Point dp;
	Common::Rect q(r);

	Common::Rect clipper(surf->w, surf->h);

	q.clip(clipper);
	if (!q.isValidRect()) return;

	dp.x = q.left;
	dp.y = q.top;

	q.translate(-r.left, -r.top);

	byte *s = data + q.left + q.top * r.width();
	byte *d = (byte*)surf->getBasePtr(dp.x, dp.y);

	uint sPitch = r.width() - q.width();
	uint dPitch = surf->w - q.width();


	if (_varRenderMode == 2) {

		for (uint16 i = 0; i < q.height(); i++) {

			for (uint16 j = 0; j < q.width(); j++) {
				if (*s != transparentColor) {
					if (_backgroundInfo.mask.data && (z < LAYER_FOREGROUND)) {
						byte v = _backgroundInfo.mask.getValue(dp.x + j, dp.y + i);
						if (z >= v) *d = 5;
					} else {
						*d = 5;
					}
				}

				s++;
				d++;
			}

			s += sPitch;
			d += dPitch;
		}

    } else {
		if (_backgroundInfo.mask.data && (z < LAYER_FOREGROUND)) {

			for (uint16 i = 0; i < q.height(); i++) {

				for (uint16 j = 0; j < q.width(); j++) {
					if (*s != transparentColor) {
						byte v = _backgroundInfo.mask.getValue(dp.x + j, dp.y + i);
						if (z >= v) *d = *s;
					}

					s++;
					d++;
				}

				s += sPitch;
				d += dPitch;
			}

		} else {

			for (uint16 i = q.top; i < q.bottom; i++) {
				for (uint16 j = q.left; j < q.right; j++) {
					if (*s != transparentColor)
						*d = *s;

					s++;
					d++;
				}

				s += sPitch;
				d += dPitch;
			}

		}
	}

}


} // namespace Parallaction
