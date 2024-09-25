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

#include "darkseed/pic.h"
#include "darkseed/darkseed.h"
#include "common/debug.h"

namespace Darkseed {

bool Pic::load(const Common::Path &filename) {
	Common::File file;
	Common::Path fullPath = g_engine->getPictureFilePath(filename);
	if (!file.open(fullPath)) {
		debug("Failed to load %s", fullPath.toString().c_str());
		return false;
	}
	bool ret = load(file);
	file.close();
	if (ret) {
		debug("Loaded %s (%d,%d)", fullPath.toString().c_str(), _width, _height);
	}
	return ret;
}

bool Pic::load(Common::SeekableReadStream &readStream) {
	_width = readStream.readUint16BE();
	_height = readStream.readUint16BE();
	_pixels.resize(_width * (_height + 1), 0);

	int curX = 0;
	int curY = 0;
	while (curY < _height) {
		int rleCommand = readNextNibble(readStream);

		if (rleCommand < 8) {
			// read nibble count of nibbles pixels
			for (int i = 0; i < rleCommand + 1; i++) {
				byte pixel = readNextNibble(readStream);
				_pixels[curX + curY * _width] = pixel;
				curX++;
				if (curX == _width) {
					curX = 0;
					curY++;
				}
			}
		} else {
			// fetch next nibble and repeat if n times.
			byte pixel = readNextNibble(readStream);
			for (int i = 16; i >= rleCommand; i--) {
				_pixels[curX + curY * _width] = pixel;
				curX++;
				if (curX == _width) {
					curX = 0;
					curY++;
				}
			}
		}
	}
	return true;
}

byte Pic::readNextNibble(Common::SeekableReadStream &readStream) {
	if (!_hasReadByte) {
		_currentDataByte = readStream.readByte();
		if (readStream.eos()) {
			debug("Argh!");
		}
		_hasReadByte = true;
		return _currentDataByte >> 4;
	} else {
		_hasReadByte = false;
		return _currentDataByte & 0xf;
	}
}

void Pic::draw() {
	draw(0, 0);
}

void Pic::draw(int xOffset, int yOffset) {
	g_engine->_screen->copyRectToSurface(getPixels().data(), getWidth(), xOffset, yOffset, getWidth(), getHeight());
}

void Pic::drawRect(const Common::Rect &rect) {
	void *ptr = getPixels().data() + rect.left + (rect.top * getWidth());
	g_engine->_screen->copyRectToSurface(ptr, getWidth(), rect.left, rect.top, rect.width(), rect.height());
}

} // End of namespace Darkseed
