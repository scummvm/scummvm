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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "graphics.h"
#include "msn_def.h"

namespace Supernova {

MSNImageDecoder::MSNImageDecoder()
    : _surface(NULL)
    , _palette(NULL)
    , _encodedImage(NULL) {
}

MSNImageDecoder::~MSNImageDecoder() {
	destroy();
}

bool MSNImageDecoder::init(int filenumber) {
	Common::File file;
	if (!file.open(Common::String::format("msn_data.%03d", filenumber))) {
		error("File %s could not be read!", file.getName());
	}

	_filenumber = filenumber;
	loadStream(file);

	return true;
}

bool MSNImageDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	uint size = 0;
	size  = (stream.readUint16LE() + 0xF) >> 4;
	size |= (stream.readUint16LE() & 0xF) << 12;
	size += 0x70;    // zus_paragraph
	size *= 16;      // a paragraph is 16 bytes
	_encodedImage = new byte[size];

	_palette = new byte[717];
	g_system->getPaletteManager()->grabPalette(_palette, 16, 239);

	byte pal_diff;
	byte flag = stream.readByte();
	if (flag == 0) {
		pal_diff = 0;
		_palette[141] = 0xE0;
		_palette[142] = 0xE0;
		_palette[143] = 0xE0;
	} else {
		pal_diff = 1;
		for (int i = flag * 3; i != 0; --i) {
			_palette[717 - i] = stream.readByte() << 2;
		}
	}

	_numSections = stream.readByte();
	for (uint i = 0; i < kMaxSections; ++i) {
		_section[i].addressHigh = 0xff;
		_section[i].addressLow = 0xffff;
		_section[i].x2 = 0;
	}
	for (int i = 0; i < _numSections; ++i) {
		_section[i].x1 = stream.readUint16LE();
		_section[i].x2 = stream.readUint16LE();
		_section[i].y1 = stream.readByte();
		_section[i].y2 = stream.readByte();
		_section[i].next = stream.readByte();
		_section[i].addressLow = stream.readUint16LE();
		_section[i].addressHigh = stream.readByte();
	}

	_numClickFields = stream.readByte();
	for (int i = 0; i < _numClickFields; ++i) {
		_clickField[i].x1 = stream.readUint16LE();
		_clickField[i].x2 = stream.readUint16LE();
		_clickField[i].y1 = stream.readByte();
		_clickField[i].y2 = stream.readByte();
		_clickField[i].next = stream.readByte();
	}

	byte zwCodes[256] = {0};
	byte numRepeat = stream.readByte();
	byte numZw = stream.readByte();
	stream.read(zwCodes, numZw);
	numZw += numRepeat;

	byte input = 0;
	uint i = 0;

	while (stream.read(&input, 1)) {
		if (input < numRepeat) {
			++input;
			byte value = stream.readByte();
			for (--value; input > 0; --input) {
				_encodedImage[i++] = value;
			}
		} else if (input < numZw) {
			input = zwCodes[input - numRepeat];
			--input;
			_encodedImage[i++] = input;
			_encodedImage[i++] = input;
		} else {
			input -= pal_diff;
			_encodedImage[i++] = input;
		}
	}

	return true;
}

bool MSNImageDecoder::loadSection(int section) {
	int imageWidth = 320;
	int imageHeight = 200;

	if (_surface)
		_surface->free();

	_surface = new Graphics::Surface;

	if (_filenumber == 1 || _filenumber == 2) {
		imageWidth = 640;
		imageHeight = 480;
		_pitch = 640;

		_surface->create(imageWidth, imageHeight, g_system->getScreenFormat());
		byte *surfacePixels = static_cast<byte *>(_surface->getPixels());
		for (int i = 0; i < imageWidth * imageHeight / 8; ++i) {
			*surfacePixels++ = (_encodedImage[i] & 0x80) ? kColorWhite63 : kColorBlack;
			*surfacePixels++ = (_encodedImage[i] & 0x40) ? kColorWhite63 : kColorBlack;
			*surfacePixels++ = (_encodedImage[i] & 0x20) ? kColorWhite63 : kColorBlack;
			*surfacePixels++ = (_encodedImage[i] & 0x10) ? kColorWhite63 : kColorBlack;
			*surfacePixels++ = (_encodedImage[i] & 0x08) ? kColorWhite63 : kColorBlack;
			*surfacePixels++ = (_encodedImage[i] & 0x04) ? kColorWhite63 : kColorBlack;
			*surfacePixels++ = (_encodedImage[i] & 0x02) ? kColorWhite63 : kColorBlack;
			*surfacePixels++ = (_encodedImage[i] & 0x01) ? kColorWhite63 : kColorBlack;
		}
	} else {
		_pitch = 320;
		_surface->create(imageWidth, imageHeight, g_system->getScreenFormat());
		byte *surfacePixels = static_cast<byte *>(_surface->getPixels());

		const uint32 kInvalidAddress = 0x00FFFFFF;

		uint image = section;
		if (image < 128) {
			do {
				uint32 offset = (_section[image].addressHigh << 16) + _section[image].addressLow;
				if (offset == kInvalidAddress || _section[image].x2 == 0) {
					return false;
				}
				int width = _section[image].x2 - _section[image].x1 + 1;
				int height = _section[image].y2 - _section[image].y1 + 1;
				uint32 destAddress = imageWidth * _section[image].y1 + _section[image].x1;
				while (height) {
					Common::copy(_encodedImage + offset, _encodedImage + offset + width, surfacePixels + destAddress);
					offset += width;
					destAddress += imageWidth;
					--height;
				}

				image = _section[image].next;
			} while (image != 0);
		} else {
			image -= 128;
			do {
				int width = _section[image].x2 - _section[image].x1 + 1;
				int height = _section[image].y2 - _section[image].y1 + 1;
				uint32 destAddress = imageWidth * _section[image].y1 + _section[image].x1;
				uint32 offset = (_section[image].addressHigh << 16) + _section[image].addressLow + destAddress;
				while (height) {
					Common::copy(_encodedImage + offset, _encodedImage + offset + width, surfacePixels + destAddress);
					offset += imageWidth;
					destAddress += imageWidth;
					--height;
				}

				image = _section[image].next;
			} while (image != 0);
		}
	}

	return true;
}

void MSNImageDecoder::destroy() {
	if (_palette) {
		delete[] _palette;
		_palette = NULL;
	}
	if (_surface) {
		_surface->free();
		_surface = NULL;
	}
	if (_encodedImage) {
		delete[] _encodedImage;
		_encodedImage = NULL;
	}
}

}
