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
#include "common/memstream.h"
#include "common/textconsole.h"
#include "dragons/bigfile.h"
#include "dragons/cursor.h"
#include "dragons/font.h"
#include "dragons/scene.h"
#include "dragons/screen.h"
#include "dragons/dragons.h"

namespace Dragons {

Font::Font(Common::SeekableReadStream &stream, uint32 mapSize, uint32 pixelOffset, uint32 pixelSize) {
	_size = mapSize / 2;
	_map = (uint16 *)malloc(mapSize);
	if (!_map) {
		error("Allocating memory for font map.");
	}

	for (uint i = 0; i < _size; i++) {
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

	renderToSurface(surface, 0, 0, text, length);

	return surface;
}

void Font::renderToSurface(Graphics::Surface *surface, int16 x, int16 y, uint16 *text, uint16 length) {
	if (x < 0 || y < 0 || x + length * 8 > surface->w || y + 8 > surface->h) {
		return;
	}
	byte *startPixelOffset = (byte *)surface->getPixels() + y * surface->pitch + x * surface->format.bytesPerPixel;
	for (int i = 0; i < length; i++) {
		byte *pixels = startPixelOffset;
		pixels += i * 8;
		byte *data = _pixels + mapChar(text[i]) * 64;
		for (int j = 0; j < 8; j++) {
			memcpy(pixels, data, 8);
			data += 8;
			pixels += surface->pitch;
		}
	}
}

FontManager::FontManager(DragonsEngine *vm, Screen *screen, BigfileArchive *bigfileArchive): _vm(vm), _screen(screen), _numTextEntries(0) {
	uint32 fileSize;
	byte *data = bigfileArchive->load("fntfiles.dat", fileSize);
	Common::SeekableReadStream *readStream = new Common::MemoryReadStream(data, fileSize, DisposeAfterUse::YES);

	_fonts[0] = loadFont(0, *readStream);
	_fonts[1] = loadFont(1, *readStream);
	_fonts[2] = loadFont(2, *readStream);

	delete readStream;

	_dat_80086f48_fontColor_flag = 0;

	_surface = new Graphics::Surface();
	_surface->create(DRAGONS_SCREEN_WIDTH, DRAGONS_SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	clearText(); //clear backing surface.

	_boxFontChr = bigfileArchive->load("boxfont.chr", fileSize);
}

FontManager::~FontManager() {
	delete _fonts[0];
	delete _fonts[1];
	delete _fonts[2];

	_surface->free();
	delete _surface;

	free(_boxFontChr);
}

void FontManager::addText(int16 x, int16 y, uint16 *text, uint16 length, uint8 fontType) {
	assert(length < 1024);
	assert(fontType < 4);
	_fonts[fontType]->renderToSurface(_surface, x, y, text, length);
	++_numTextEntries;
}

void FontManager::draw() {
	if(_numTextEntries > 0) {
		_screen->copyRectToSurface8bpp(*_surface, _screen->getPalette(2), 0, 0, Common::Rect(_surface->w, _surface->h), false, NORMAL);
	}
}

void FontManager::clearText() {
	_numTextEntries = 0;
	_surface->fillRect(Common::Rect(_surface->w, _surface->h), 0);
}

Font *FontManager::loadFont(uint16 index, Common::SeekableReadStream &stream) {
	Common::File fd;
	if (!fd.open("dragon.exe")) {
		error("Failed to open dragon.exe");
	}
	fd.seek(_vm->getFontOffsetFromDragonEXE());
	fd.skip((index * 2)  * 28);

	fd.skip(16); //filename
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

void updatePalEntry(uint16 *pal, uint16 index, uint16 newValue) {
	newValue = (uint16)(((uint16)newValue & 0x1f) << 10) | (uint16)(((uint16)newValue & 0x7c00) >> 10) |
			(newValue & 0x3e0) | (newValue & 0x8000);
	WRITE_LE_INT16(pal + index, newValue);
}

void FontManager::updatePalette() {
	uint16 *palette_f2_font_maybe = (uint16 *)_screen->getPalette(2);
	const uint16 cursor3 = 0x14a5 | 0x8000;
	if (_vm->isInMenu() || _vm->isFlagSet(ENGINE_FLAG_200)) {
		updatePalEntry(palette_f2_font_maybe, 3, cursor3); //TODO move this to palette initialisation
		if (!_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_1)) {
			updatePalEntry(palette_f2_font_maybe, 16, cursor3);
		} else {
			updatePalEntry(palette_f2_font_maybe, 16, 0);
		}
		if (_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_4) && _dat_80086f48_fontColor_flag != 0) {
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
		//TODO WRITE_LE_INT16(&palette_f2_font_maybe[33], READ_LE_INT16(&palette_f0[_dat_80084f58 >> 8]) & 0x7fff); //_dat_80084f58 is set in ActuallyShowMessage()
		updatePalEntry(palette_f2_font_maybe, 33, 0x3def); //temporarily put in standard gray
		if (_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_1)) {
			updatePalEntry(palette_f2_font_maybe, 17, 0x3bee);
			updatePalEntry(palette_f2_font_maybe, 33, 0x3bee);
			updatePalEntry(palette_f2_font_maybe, 49, 0x3bee);
		}
	}
}

void FontManager::drawTextDialogBox(uint32 x1, uint32 y1, uint32 x2, uint32 y2) {
	const uint16 kTileBaseIndex = 1;
	const uint16 kTileIndexTop = kTileBaseIndex + 10;
	const uint16 kTileIndexBottom = kTileBaseIndex + 16;
	const uint16 kTileIndexLeft = kTileBaseIndex + 12;
	const uint16 kTileIndexRight = kTileBaseIndex + 14;
	const uint16 kTileIndexBackground = kTileBaseIndex + 13;
	const uint16 kTileIndexTopLeft = kTileBaseIndex + 9;
	const uint16 kTileIndexTopRight = kTileBaseIndex + 11;
	const uint16 kTileIndexBottomLeft = kTileBaseIndex + 15;
	const uint16 kTileIndexBottomRight = kTileBaseIndex + 17;
	// Fill background
	for (uint yc = y1 + 1; yc <= y2 - 1; ++yc) {
		for (uint xc = x1 + 1; xc <= x2 - 1; ++xc) {
			drawBoxChar(xc, yc, kTileIndexBackground);
		}
	}
	// Fill top and bottom rows
	for (uint xc = x1 + 1; xc <= x2 - 1; ++xc) {
		drawBoxChar(xc, y1, kTileIndexTop);
		drawBoxChar(xc, y2, kTileIndexBottom);
	}
	// Fill left and right columns
	for (uint yc = y1 + 1; yc <= y2 - 1; ++yc) {
		drawBoxChar(x1, yc, kTileIndexLeft);
		drawBoxChar(x2, yc, kTileIndexRight);
	}
	// Fill corners
	drawBoxChar(x1, y1, kTileIndexTopLeft);
	drawBoxChar(x2, y1, kTileIndexTopRight);
	drawBoxChar(x1, y2, kTileIndexBottomLeft);
	drawBoxChar(x2, y2, kTileIndexBottomRight);
	_numTextEntries++;
}

void FontManager::clearTextDialog(uint32 x1, uint32 y1, uint32 x2, uint32 y2) {
	debug(3, "Clear text (%d,%d) -> (%d,%d)", x1, y1, x2, y2);
//	assert(x1 > 0);
//	assert(y1 > 0);
	_surface->fillRect(Common::Rect((x1-1) * 8, (y1-1) * 8, (x2 + 1) * 8 + 1, (y2 + 1) * 8 + 1), 0);
	if (_numTextEntries > 0) {
		_numTextEntries--; //TODO need a better way to check if we should still draw the font surface.
	}
}

void FontManager::drawBoxChar(uint32 x, uint32 y, uint8 tileIndex) {
	byte *pixels = (byte *)_surface->getBasePtr(x * 8, y * 8);
	byte *data = _boxFontChr + tileIndex * 64;
	for (int j = 0; j < 8; j++) {
		memcpy(pixels, data, 8);
		data += 8;
		pixels += _surface->pitch;
	}
}

void FontManager::addAsciiText(int16 x, int16 y, const char *text, uint16 length, uint8 fontType) {
	uint16 wText[41];
	memset(wText, 0, sizeof(wText));
	if (length > 40) {
		length = 40;
	}

	for (int i = 0; i < length; i++) {
		wText[i] = text[i];
	}

	addText(x, y, wText, length, fontType);
}

} // End of namespace Dragons
