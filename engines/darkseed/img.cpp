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

#include "common/debug.h"
#include "common/file.h"
#include "darkseed/darkseed.h"
#include "darkseed/img.h"

namespace Darkseed {

bool Img::load(const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename)) {
		return false;
	}
	bool ret = load(file);
	file.close();
	if (ret) {
		debug("Loaded %s (%d,%d) (%d,%d) %x", filename.toString().c_str(), _x, _y, _width, _height, _mode);
	}
	return ret;
}

bool Img::load(Common::SeekableReadStream &readStream) {
	Common::Array<uint8> unpackedData;
	unpackRLE(readStream, unpackedData);
	_x = (uint16)READ_LE_INT16(&unpackedData.data()[0]);
	_y = (uint16)READ_LE_INT16(&unpackedData.data()[2]);
	unpackPlanarData(unpackedData, 4);
	return true;
}

bool Img::loadWithoutPosition(Common::SeekableReadStream &readStream) {
	Common::Array<uint8> unpackedData;
	unpackRLE(readStream, unpackedData);
	_x = 0;
	_y = 0;
	unpackPlanarData(unpackedData, 0);
	return false;
}

bool Img::unpackRLE(Common::SeekableReadStream &readStream, Common::Array<byte> &buf) {
	uint16 size = readStream.readUint16LE();
	uint16 idx = 0;
	buf.resize(size + 1);

	while (idx <= size) {
		uint8 byte = readStream.readByte();
		assert(!readStream.err());
		if (byte & 0x80) {
			uint8 count = byte & 0x7f;
			count++;
			byte = readStream.readByte();
			for (int i = 0; i < count && idx + i < size; i++) {
				buf[idx + i] = byte;
			}
			idx += count;
		} else {
			uint8 count = byte + 1;
			for (int i = 0; i < count && idx + i < size; i++) {
				buf[idx + i] = readStream.readByte();
			}
			idx += count;
		}
	}

	return true;
}

void Img::unpackPlanarData(Common::Array<uint8> &planarData, uint16 headerOffset) {
	_height = (uint16)READ_LE_INT16(&planarData.data()[headerOffset]);
	_width = (uint16)READ_LE_INT16(&planarData.data()[headerOffset + 2]) * 8;
	_mode = planarData.data()[headerOffset + 4];
//	assert(mode == 0xff);
	_pixels.resize(_width * _height, 0);
	for (int py = 0; py < _height; py++) {
		for (int plane = 0; plane < 4; plane++) {
			for (int px = 0; px < _width; px++) {
				int bitPos = (7 - (px % 8));
				int planeBit = (planarData[(headerOffset + 5) + (px / 8) + (_width / 8) * plane + py * (_width / 8) * 4] & (1 << bitPos)) >> bitPos;
				_pixels[px + py * _width] |= planeBit << (3 - plane);
			}
		}
	}
}

Common::Array<uint8> &Img::getPixels() {
	return _pixels;
}

void Img::draw(int drawMode, int drawWidth) {
	drawAt(_x, _y, drawMode, drawWidth);
}

void Img::drawAt(uint16 xPos, uint16 yPos, int drawMode, int drawWidth) {
	int w = drawWidth != 0 ? drawWidth : _width;
	if (drawMode != 0) {
		uint8 *screen = (uint8 *)g_engine->_screen->getBasePtr(xPos, yPos);
		uint8 *imgPixels = _pixels.data();
		for (int sy = 0; sy < _height; sy++) {
			for (int sx = 0; sx < w; sx++) {
				if (drawMode == 1 && imgPixels[sx] != 0) {
					screen[sx] ^= imgPixels[sx];
				} else if (drawMode == 2 && imgPixels[sx] != 15) {
					screen[sx] &= imgPixels[sx];
				} else if (drawMode == 3 && imgPixels[sx] != 0) {
					screen[sx] |= imgPixels[sx];
				}
			}
			imgPixels += _width;
			screen += g_engine->_screen->pitch;
		}
	} else {
		g_engine->_screen->copyRectToSurface(_pixels.data(), _width, xPos, yPos, w, _height);
	}
	g_engine->_screen->addDirtyRect({{(int16)xPos, (int16)yPos}, (int16)w, (int16)_height});
}

} // namespace Darkseed
