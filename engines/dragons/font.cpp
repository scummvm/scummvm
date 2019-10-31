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

uint16 Font::mapChar(uint16 in) {
	return _map[in - _map[0x03] + 0x05];
}

Graphics::Surface *Font::render(uint16 *text, uint16 length) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(length * 8, 8, Graphics::PixelFormat::createFormatCLUT8());


	for (int i = 0; i < length; i++) {
		byte *pixels = (byte *)surface->getPixels();
		pixels += i * 8;
//		debug("char: %d size: %d %d", (text[i] - 0x20), _numChars, (30 + i));
		byte *data = _pixels + mapChar(text[i]) * 64;
		for (int y = 0; y < 8; y++) {
			memcpy(pixels, data, 8);
			data += 8;
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

	DAT_80086f48_fontColor_flag = 0;
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
	screenTextEntry->surface = _fonts[fontType]->render(text, length);

	_screenTexts.push_back(screenTextEntry);

}

void FontManager::draw() {
	Common::List<ScreenTextEntry*>::iterator it = _screenTexts.begin();
	while (it != _screenTexts.end()) {
		ScreenTextEntry *entry = *it;
		_screen->copyRectToSurface8bpp(*entry->surface, _screen->getPalette(2), entry->position.x, entry->position.y, Common::Rect(entry->surface->w, entry->surface->h), false, 128);
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
	WRITE_LE_INT16(&_palettes[0x11 * 2], 0x7fe0); //packColor(254, 255, 0));
	WRITE_LE_INT16(&_palettes[0x12 * 2], 0); //packColor(95, 95, 95));
	WRITE_LE_INT16(&_palettes[0x13 * 2], packColor(175, 175, 175));

	WRITE_LE_INT16(&_palettes[0x20 * 2], packColor(0, 0, 0) | 0x8000);
	WRITE_LE_INT16(&_palettes[0x21 * 2], packColor(175, 175, 175));
	WRITE_LE_INT16(&_palettes[0x22 * 2], 0);
	WRITE_LE_INT16(&_palettes[0x23 * 2], packColor(175, 175, 175));
}

void updatePalEntry(uint16 *pal, uint16 index, uint16 newValue) {
	newValue = (uint16)(((uint16)newValue & 0x1f) << 10) | (uint16)(((uint16)newValue & 0x7c00) >> 10) |
			(newValue & 0x3e0) | (newValue & 0x8000);
	WRITE_LE_INT16(pal + index, newValue);
}

void FontManager::updatePalette() {
//	if (( != 0 && ((engine_flags_maybe & 0x200) != 0))) {
	uint16 *palette_f2_font_maybe = (uint16 *)_screen->getPalette(2);
	uint16 cursor3 = 0x14a5 | 0x8000;
	if(_vm->isFlagSet(ENGINE_FLAG_200)) {
		if (!_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_1)) {
			updatePalEntry(palette_f2_font_maybe, 16, cursor3);
		} else {
			updatePalEntry(palette_f2_font_maybe, 16, 0);
		}
		if (_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_4) && DAT_80086f48_fontColor_flag != 0) {
			updatePalEntry(palette_f2_font_maybe, 17, 0x421);
		} else {
			updatePalEntry(palette_f2_font_maybe, 17, 0xfff);
		}
		updatePalEntry(palette_f2_font_maybe, 18, 0x421);
		updatePalEntry(palette_f2_font_maybe, 19, 0x3def);
		updatePalEntry(palette_f2_font_maybe, 32, cursor3);
		updatePalEntry(palette_f2_font_maybe, 49, 0xfff);
		updatePalEntry(palette_f2_font_maybe, 1, 0x8000);

		updatePalEntry(palette_f2_font_maybe, 34, 0x421);
		updatePalEntry(palette_f2_font_maybe, 35, 0x3def);
		updatePalEntry(palette_f2_font_maybe, 48, cursor3);
		updatePalEntry(palette_f2_font_maybe, 50, 0x421);
		updatePalEntry(palette_f2_font_maybe, 51, 0x3def);
		//TODO WRITE_LE_INT16(&palette_f2_font_maybe[33], READ_LE_INT16(&palette_f0[DAT_80084f58 >> 8]) & 0x7fff);
		if (_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_1)) {
			updatePalEntry(palette_f2_font_maybe, 17, 0x3bee);
			updatePalEntry(palette_f2_font_maybe, 33, 0x3bee);
			updatePalEntry(palette_f2_font_maybe, 49, 0x3bee);
		}
	}
}

} // End of namespace Dragons
