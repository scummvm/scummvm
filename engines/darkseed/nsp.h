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

namespace Darkseed {

class Sprite {
public:
	uint16 width;
	uint16 height;
	uint16 pitch;
	Common::Array<uint8> pixels;

public:
	Sprite(uint16 width, uint16 height, uint16 pitch);
	bool loadData(Common::SeekableReadStream &readStream);
	void draw(int x, int y, uint16 frameBottom = 0) const;
	void drawScaled(int x, int y, int destWidth, int destHeight, bool flipX) const;
};

class Obt {
public:
	uint8 numFrames;
	Common::Array<int32> deltaX;
	Common::Array<int32> deltaY;
	Common::Array<uint8> frameNo;
	Common::Array<uint8> frameDuration;
	Obt();
	virtual ~Obt();
};

class Nsp {
private:
	Common::Array<Sprite> frames;
	Common::Array<Obt> animations;

public:
	bool load(const Common::Path &filename);
	bool containsSpriteAt(int index) {
		return (int)frames.size() > index;
	}
	const Sprite &getSpriteAt(int index);
	const Obt &getAnimAt(int index);
	int16 getTotalAnim() {
		return (int16)animations.size();
	}

private:
	bool load(Common::SeekableReadStream &readStream);
	bool loadObt(const Common::Path &filename);
};

} // namespace Darkseed

#endif // DARKSEED_NSP_H
