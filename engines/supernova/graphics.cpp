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
#include "common/config-manager.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "supernova/graphics.h"
#include "supernova/msn_def.h"
#include "supernova/screen.h"
#include "supernova/supernova.h"

namespace Supernova {

MSNImage::MSNImage() {
	_palette = nullptr;
	_encodedImage = nullptr;
	_filenumber = -1;
	_pitch = 0;
	_numSections = 0;
	_numClickFields = 0;

	for (int i = 0; i < kMaxSections; ++i) {
		_section[i].x1 = 0;
		_section[i].x2 = 0;
		_section[i].y1 = 0;
		_section[i].y2 = 0;
		_section[i].next = 0;
		_section[i].addressLow = 0xFFFF;
		_section[i].addressHigh = 0xFF;
	}

	for (int i = 0; i < kMaxClickFields; ++i) {
		_clickField[i].x1 = 0;
		_clickField[i].x2 = 0;
		_clickField[i].y1 = 0;
		_clickField[i].y2 = 0;
		_clickField[i].next = 0;
	}
}

MSNImage::~MSNImage() {
	destroy();
}

bool MSNImage::init(int filenumber) {
	Common::File file;
	if (!file.open(Common::String::format("msn_data.%03d", filenumber))) {
		warning("Image data file msn_data.%03d could not be read!", filenumber);
		return false;
	}

	_filenumber = filenumber;
	loadStream(file);

	return true;
}

bool MSNImage::loadFromEngineDataFile() {
	Common::String name;
	if (_filenumber == 1)
		name = "IMG1";
	else if (_filenumber == 2)
		name = "IMG2";
	else
		return false;

	Common::String cur_lang = ConfMan.get("language");

	// Note: we don't print any warning or errors here if we cannot find the file
	// or the format is not as expected. We will get those warning when reading the
	// strings anyway (actually the engine will even refuse to start).
	Common::File f;
	if (!f.open(SUPERNOVA_DAT))
		return false;

	char id[5], lang[5];
	id[4] = lang[4] = '\0';
	f.read(id, 3);
	if (strncmp(id, "MSN", 3) != 0)
		return false;
	int version = f.readByte();
	if (version != SUPERNOVA_DAT_VERSION)
		return false;

	while (!f.eos()) {
		f.read(id, 4);
		f.read(lang, 4);
		uint32 size = f.readUint32LE();
		if (f.eos())
			break;
		if (name == id && cur_lang == lang) {
			return f.read(_encodedImage, size) == size;
		} else
			f.skip(size);
	}

	return false;
}

bool MSNImage::loadStream(Common::SeekableReadStream &stream) {
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
		_section[i].next = 0;
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
	for (int i = _numClickFields; i < kMaxClickFields; ++i) {
		_clickField[i].x1 = 0;
		_clickField[i].x2 = 0;
		_clickField[i].y1 = 0;
		_clickField[i].y2 = 0;
		_clickField[i].next = 0;
	}

	// Newspaper images may be in the engine data file. So first try to read
	// it from there.
	if (!loadFromEngineDataFile()) {
		// Load the image from the stream
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
	}

	loadSections();

	return true;
}

bool MSNImage::loadSections() {
	bool isNewspaper = _filenumber == 1 || _filenumber == 2;
	int imageWidth = isNewspaper ? 640 : 320;
	int imageHeight = isNewspaper ? 480 : 200;
	_pitch = imageWidth;

	for (int section = 0; section < _numSections; ++section) {
		Graphics::Surface *surface = new Graphics::Surface;
		_sectionSurfaces.push_back(surface);

		if (isNewspaper) {
			surface->create(imageWidth, imageHeight, g_system->getScreenFormat());
			byte *surfacePixels = static_cast<byte *>(surface->getPixels());
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
			uint32 offset = (_section[section].addressHigh << 16) + _section[section].addressLow;
			if (offset == kInvalidAddress || _section[section].x2 == 0) {
				return false;
			}
			int width = _section[section].x2 - _section[section].x1 + 1;
			int height = _section[section].y2 - _section[section].y1 + 1;
			surface->create(width, height, g_system->getScreenFormat());
			byte *surfacePixels = static_cast<byte *>(surface->getPixels());
			Common::copy(_encodedImage + offset, _encodedImage + offset + width * height, surfacePixels);
		}
	}

	return true;
}

void MSNImage::destroy() {
	if (_palette) {
		delete[] _palette;
		_palette = nullptr;
	}
	if (_encodedImage) {
		delete[] _encodedImage;
		_encodedImage = nullptr;
	}
	for (Common::Array<Graphics::Surface *>::iterator it = _sectionSurfaces.begin();
		 it != _sectionSurfaces.end(); ++it) {
		(*it)->free();
	}
}

}
