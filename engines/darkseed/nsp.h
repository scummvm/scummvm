/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef DARKSEED_NSP_H
#define DARKSEED_NSP_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/file.h"
#include "graphics/surface.h"

namespace Darkseed {

class Sprite {
public:
	uint16 _width;
	uint16 _height;
	uint16 _pitch;
	Common::Array<uint8> _pixels;

public:
	Sprite(uint16 width, uint16 height, uint16 pitch);
	bool loadData(Common::SeekableReadStream &readStream);
	void draw(int x, int y, uint16 frameBottom = 0) const;
	void draw(Graphics::Surface *dst, int x, int y, uint16 frameBottom = 0) const;
	void drawScaled(int x, int y, int destWidth, int destHeight, bool flipX) const;
private:
	void clipToScreen(int x, int y, uint16 frameBottom, uint16 *clippedWidth, uint16 *clippedHeight) const;
};

class Obt {
public:
	uint8 _numFrames;
	Common::Array<int32> _deltaX;
	Common::Array<int32> _deltaY;
	Common::Array<uint8> _frameNo;
	Common::Array<uint8> _frameDuration;
	Obt();
	virtual ~Obt();
};

class Nsp {
private:
	Common::Array<Sprite> _frames;
	Common::Array<Obt> _animations;

public:
	bool load(const Common::Path &filename);
	bool containsSpriteAt(int index) {
		return (int)_frames.size() > index;
	}
	const Sprite &getSpriteAt(int index) const;
	const Obt &getAnimAt(int index);
	int16 getTotalAnim() {
		return (int16)_animations.size();
	}
	int16 getMaxSpriteWidth();

private:
	bool load(Common::SeekableReadStream &readStream);
	bool loadObt(const Common::Path &filename);
};

} // namespace Darkseed

#endif // DARKSEED_NSP_H
