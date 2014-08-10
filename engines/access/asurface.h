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
 */

#ifndef ACCESS_ASURFACE_H
#define ACCESS_ASURFACE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "access/data.h"

namespace Access {

class SpriteResource;
class SpriteFrame;

class ASurface : public Graphics::Surface {
public:
	static int _leftSkip, _rightSkip;
	static int _topSkip, _bottomSkip;
	static int _clipWidth, _clipHeight;
	static int _lastBoundsX, _lastBoundsY;
	static int _lastBoundsW, _lastBoundsH;
	static int _scrollX, _scrollY;

	static void init();
protected:
	virtual void ASurface::plotFrame(SpriteFrame *frame, const Common::Point &pt);
public:
	void clearBuffer();

	void copyBuffer(Graphics::Surface *src) { copyFrom(*src); }

	bool clip(Common::Rect &r);

	void plotImage(SpriteResource *sprite, int frameNum, const Common::Point &pt);

	void copyTo(ASurface *dest, const Common::Point &destPos);
};

class SpriteFrame : public ASurface {
public:
	SpriteFrame(AccessEngine *vm, Common::MemoryReadStream &stream, int frameSize);
	~SpriteFrame();
};

class SpriteResource {
public:
	Common::Array<SpriteFrame *> _frames;
public:
	SpriteResource(AccessEngine *vm, const byte *data, uint32 size,
		DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO);
	~SpriteResource();

	int getCount() { return _frames.size(); }

	SpriteFrame *getFrame(int idx) { return _frames[idx]; }
};


} // End of namespace Access

#endif /* ACCESS_ASURFACE_H */
