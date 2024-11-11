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

#ifndef DARKSEED_IMG_H
#define DARKSEED_IMG_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/file.h"

namespace Darkseed {

class Img {
	uint16 _x = 0;
	uint16 _y = 0;
	uint16 _width = 0;
	uint16 _height = 0;
	byte _mode = 0;
	Common::Array<uint8> _pixels;

public:
	bool load(const Common::Path &filename);
	bool load(Common::SeekableReadStream &readStream);
	bool loadWithoutPosition(Common::SeekableReadStream &readStream);
	void draw(int drawMode = 0, int drawWidth = 0);
	void drawAt(uint16 xPos, uint16 yPos, int drawMode = 0, int drawWidth = 0);

	Common::Array<uint8> &getPixels();
	uint16 getX() const {
		return _x;
	}
	uint16 getY() const {
		return _y;
	}
	uint16 getWidth() const {
		return _width;
	}
	uint16 getHeight() const {
		return _height;
	}

private:
	bool unpackRLE(Common::SeekableReadStream &readStream, Common::Array<uint8> &buf);
	void unpackPlanarData(Common::Array<uint8> &planarData, uint16 headerOffset);
};

} // namespace Darkseed

#endif // DARKSEED_IMG_H
