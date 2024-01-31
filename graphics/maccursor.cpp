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

#include "common/textconsole.h"

#include "graphics/maccursor.h"

namespace Graphics {

MacCursor::MacCursor() : _surface(nullptr), _palette(256), _hotspotX(0), _hotspotY(0) {
}

MacCursor::~MacCursor() {
	clear();
}

void MacCursor::clear() {
	delete[] _surface;
	_surface = nullptr;
	_palette.clear();
}

bool MacCursor::readFromStream(Common::SeekableReadStream &stream, bool forceMonochrome, byte monochromeInvertedPixelColor, bool forceCURSFormat) {
	clear();

	const int minCursSize = 32 * 2 + 4;

	// Older Mac CURS monochrome cursors had a set size, but sometimes contain extra unused bytes
	// All crsr cursors are larger than this
	if (stream.size() == minCursSize || (forceCURSFormat && stream.size() >= minCursSize))
		return readFromCURS(stream, monochromeInvertedPixelColor);

	return readFromCRSR(stream, forceMonochrome, monochromeInvertedPixelColor);
}

bool MacCursor::readFromCURS(Common::SeekableReadStream &stream, byte monochromeInvertedPixelColor) {
	// Grab B/W icon data
	_surface = new byte[16 * 16];
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream.readByte();
		for (int b = 0; b < 8; b++)
			_surface[i * 8 + b] = (byte)((imageByte & (0x80 >> b)) > 0 ? 0 : 1);
	}

	// Apply mask data
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream.readByte();
		for (int b = 0; b < 8; b++) {
			if ((imageByte & (0x80 >> b)) == 0) {
				// if an image bit is set outside the mask then the destination pixel
				//  would have been inverted on macintosh, otherwise it's transparent.
				//  we don't currently implement this inversion effect so instead we
				//  use the optional color provided by the caller for these pixels.
				_surface[i * 8 + b] = _surface[i * 8 + b] ? 0xff : monochromeInvertedPixelColor;
			}
		}
	}

	_hotspotY = stream.readUint16BE();
	_hotspotX = stream.readUint16BE();

	// Setup a basic palette
	_palette.data[1 * 3 + 0] = 0xff;
	_palette.data[1 * 3 + 1] = 0xff;
	_palette.data[1 * 3 + 2] = 0xff;

	return !stream.eos();
}

bool MacCursor::readFromCRSR(Common::SeekableReadStream &stream, bool forceMonochrome, byte monochromeInvertedPixelColor) {
	stream.readUint16BE(); // type
	stream.readUint32BE(); // offset to pixel map
	stream.readUint32BE(); // offset to pixel data
	stream.readUint32BE(); // expanded cursor data
	stream.readUint16BE(); // expanded data depth
	stream.readUint32BE(); // reserved

	// Read the B/W data first
	if (!readFromCURS(stream, monochromeInvertedPixelColor))
		return false;

	// Use b/w cursor on backends which don't support cursor palettes
	if (forceMonochrome)
		return true;

	stream.readUint32BE(); // reserved
	stream.readUint32BE(); // cursorID

	// Color version of cursor
	stream.readUint32BE(); // baseAddr

	// Keep only lowbyte for now
	stream.readByte();
	int iconRowBytes = stream.readByte();

	if (!iconRowBytes)
		return false;

	int iconBounds[4];
	iconBounds[0] = stream.readUint16BE();
	iconBounds[1] = stream.readUint16BE();
	iconBounds[2] = stream.readUint16BE();
	iconBounds[3] = stream.readUint16BE();

	stream.readUint16BE(); // pmVersion
	stream.readUint16BE(); // packType
	stream.readUint32BE(); // packSize

	stream.readUint32BE(); // hRes
	stream.readUint32BE(); // vRes

	stream.readUint16BE(); // pixelType
	stream.readUint16BE(); // pixelSize
	stream.readUint16BE(); // cmpCount
	stream.readUint16BE(); // cmpSize

	stream.readUint32BE(); // planeByte
	stream.readUint32BE(); // pmTable
	stream.readUint32BE(); // reserved

	// Pixel data for cursor
	int iconDataSize =  iconRowBytes * (iconBounds[3] - iconBounds[1]);
	byte *iconData = new byte[iconDataSize];

	if (!iconData)
		error("Cannot allocate Mac color cursor iconData");

	stream.read(iconData, iconDataSize);

	// Color table
	stream.readUint32BE(); // ctSeed
	stream.readUint16BE(); // ctFlag
	uint16 ctSize = stream.readUint16BE() + 1;

	// Read just high byte of 16-bit color
	for (int c = 0; c < ctSize; c++) {
		stream.readUint16BE();
		_palette.data[c * 3 + 0] = stream.readUint16BE() >> 8;
		_palette.data[c * 3 + 1] = stream.readUint16BE() >> 8;
		_palette.data[c * 3 + 2] = stream.readUint16BE() >> 8;
	}

	// Find black so that Macintosh black (255) can be remapped.
	// This is necessary because we use 255 for the color key.
	byte black = 0;
	for (byte c = 0; c < 255; c++) {
		if (_palette.data[c * 3 + 0] == 0 &&
			_palette.data[c * 3 + 1] == 0 &&
			_palette.data[c * 3 + 2] == 0) {
			black = c;
			break;
		}
	}

	int pixelsPerByte = (iconBounds[2] - iconBounds[0]) / iconRowBytes;
	int bpp           = 8 / pixelsPerByte;

	// build a mask to make sure the pixels are properly shifted out
	int bitmask = 0;
	for (int m = 0; m < bpp; m++) {
		bitmask <<= 1;
		bitmask  |= 1;
	}

	// Extract pixels from bytes
	for (int j = 0; j < iconDataSize; j++) {
		for (int b = 0; b < pixelsPerByte; b++) {
			int idx = j * pixelsPerByte + (pixelsPerByte - 1 - b);

			if (_surface[idx] != 0xff) { // if mask is not there
				_surface[idx] = (byte)((iconData[j] >> (b * bpp)) & bitmask);
				// Remap Macintosh black
				if (_surface[idx] == 255) {
					_surface[idx] = black;
				}
			}
		}
	}

	delete[] iconData;
	return stream.pos() == stream.size();
}

} // End of namespace Common
