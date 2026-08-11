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

#ifndef DARKSEED_PIC_H
#define DARKSEED_PIC_H

#include "common/array.h"
#include "common/file.h"
#include "common/rect.h"

namespace Darkseed {

class Pic {
	uint16 _width = 0;
	uint16 _height = 0;
	Common::Array<uint8> _pixels;

	bool _hasReadByte = false;
	byte _currentDataByte = 0;

public:
	Pic() {}
	bool load(const Common::Path &filename);
	Common::Array<uint8> &getPixels() {
		return _pixels;
	}

	uint16 getWidth() const {
		return _width;
	}
	uint16 getHeight() const {
		return _height;
	}

	void draw();
	void draw(int xOffset, int yOffset);
	void drawRect(const Common::Rect &rect);

private:
	bool load(Common::SeekableReadStream &readStream);

	byte readNextNibble(Common::SeekableReadStream &readStream);
};

} // namespace Darkseed

#endif // DARKSEED_PIC_H
