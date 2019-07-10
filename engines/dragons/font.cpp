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
#include <common/memstream.h>
#include <common/textconsole.h>
#include "bigfile.h"
#include "cursor.h"
#include "font.h"
#include "scene.h"
#include "screen.h"
#include "dragons.h"


namespace Dragons {

Font::Font(Common::SeekableReadStream &stream, uint32 mapSize, uint32 pixelOffset, uint32 pixelSize) {
	_size = mapSize / 2;
	_map = (uint16 *)malloc(mapSize);
	if (!_map) {
		error("Allocating memory for font map.");
	}

	for (int i = 0; i < _size; i++) {
		_map[i] = stream.readUint16LE();
	}

	_pixels = (byte *)malloc(pixelSize);
	if (!_pixels) {
		error("Allocating memory for font pixels.");
	}
	stream.seek(pixelOffset);
	stream.read(_pixels, pixelSize);
	_numChars = pixelSize / 64;
}

Font::~Font() {
	free(_map);
	free(_pixels);
}

uint16 mapChar(uint16 in) {
	if (in >= 97) {
		return in - 0x23;
	}
	return in - 0x20;
}
Graphics::Surface *Font::render(uint16 *text, uint16 length, byte *palette) {
	Graphics::Surface *surface = new Graphics::Surface();
	Graphics::PixelFormat pixelFormat16(2, 5, 5, 5, 1, 10, 5, 0, 15); //TODO move this to a better location.
	surface->create(length * 8, 8, pixelFormat16);


	for (int i = 0; i < length; i++) {
		byte *pixels = (byte *)surface->getPixels();
		pixels += i * 8 * 2;
		debug("char: %d size: %d %d", (text[i] - 0x20), _numChars, (30 + i));
		byte *data = _pixels + mapChar(text[i]) * 64;
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				pixels[x * 2] = palette[(data[0]) * 2];
				pixels[x * 2 + 1] = palette[(data[0]) * 2 + 1];
				data++;
			}
			pixels += surface->pitch;
		}
	}

	return surface;
}

FontManager::FontManager(DragonsEngine *vm, Screen *screen, BigfileArchive *bigfileArchive): _vm(vm), _screen(screen) {
	uint32 fileSize;
	byte *data = bigfileArchive->load("fntfiles.dat", fileSize);
	Common::SeekableReadStream *readStream = new Common::MemoryReadStream(data, fileSize, DisposeAfterUse::YES);

	_fonts[0] = loadFont(0, *readStream);
	_fonts[1] = loadFont(1, *readStream);
	_fonts[2] = loadFont(2, *readStream);

	loadPalettes();

	delete readStream;
}

FontManager::~FontManager() {
	delete _fonts[0];
	delete _fonts[1];
	delete _fonts[2];
	free(_palettes);
}

void FontManager::addText(int16 x, int16 y, uint16 *text, uint16 length, uint8 fontType) {
	assert(length < 1024);
	assert(fontType < 4);
	byte *palette = _vm->_cursor->getPalette();
	ScreenTextEntry *screenTextEntry = new ScreenTextEntry();
	screenTextEntry->position = Common::Point(x, y);
	screenTextEntry->surface = _fonts[fontType]->render(text, length, _palettes); //palette);

	_screenTexts.push_back(screenTextEntry);

}

void FontManager::draw() {
	Common::List<ScreenTextEntry*>::iterator it = _screenTexts.begin();
	while (it != _screenTexts.end()) {
		ScreenTextEntry *entry = *it;
//		_screen->copyRectToSurface(*entry->surface, entry->position.x, entry->position.y);
		_screen->copyRectToSurface(*entry->surface, entry->position.x, entry->position.y, Common::Rect(entry->surface->w, entry->surface->h));

		it++;
	}
}

void FontManager::clearText() {
	while (!_screenTexts.empty()) {
		ScreenTextEntry *screenText = _screenTexts.back();
		screenText->surface->free();
		delete screenText->surface;
		delete screenText;
		_screenTexts.pop_back();
	}
}

Font *FontManager::loadFont(uint16 index, Common::SeekableReadStream &stream) {
	Common::File fd;
	if (!fd.open("dragon.exe")) {
		error("Failed to open dragon.exe");
	}
	fd.seek(0x4a144); //TODO handle other game variants
	fd.skip((index * 2)  * 28);

//	fd->read(info.filename, 16);
	fd.skip(16);
	uint32 mapOffset = fd.readUint32LE();
	uint32 mapSize = fd.readUint32LE();
	fd.skip(4); //unk

	fd.skip(16); //filename
	uint32 pixelsOffset = fd.readUint32LE();
	uint32 pixelsSize = fd.readUint32LE();

	fd.close();

	stream.seek(mapOffset);
	return new Font(stream, mapSize, pixelsOffset, pixelsSize);
}

uint16 packColor(uint8 r, uint8 g, uint8 b) {
	return (r / 8) << 10 | (g / 8) << 5 | (b / 8);
}

void FontManager::loadPalettes() {
//	Common::File fd;
//	if (!fd.open("dragon.exe")) {
//		error("Failed to open dragon.exe");
//	}
//	fd.seek(0x5336c); //TODO handle other game variants
//
//	_palettes = (byte *)malloc(256 * 2 * 4);
//	fd.read(_palettes, 256 * 2 * 4);
//
//	_palettes[2 * 256 + 0x21] = 0x80; //HACK make this color transparent
//	fd.close();

	//TODO where does original set its palette???
	_palettes = (byte *)malloc(0x200);
	memset(_palettes, 0, 0x200);
	WRITE_LE_INT16(&_palettes[0], 0x8000);

//	WRITE_LE_INT16(&_palettes[0x11 * 2], packColor(95, 95, 95));
	WRITE_LE_INT16(&_palettes[0x10 * 2], packColor(0, 0, 0) | 0x8000);
	WRITE_LE_INT16(&_palettes[0x11 * 2], packColor(254, 255, 0));
	WRITE_LE_INT16(&_palettes[0x12 * 2], packColor(95, 95, 95));
	WRITE_LE_INT16(&_palettes[0x13 * 2], packColor(175, 175, 175));

	WRITE_LE_INT16(&_palettes[0x20 * 2], packColor(0, 0, 0) | 0x8000);
	WRITE_LE_INT16(&_palettes[0x21 * 2], packColor(254, 255, 0));
	WRITE_LE_INT16(&_palettes[0x22 * 2], packColor(95, 95, 95));
	WRITE_LE_INT16(&_palettes[0x23 * 2], packColor(175, 175, 175));
}

} // End of namespace Dragons
