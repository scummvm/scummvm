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

#include "sludge/allfiles.h"
#include "sludge/fonttext.h"
#include "sludge/graphics.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sprites.h"
#include "sludge/sludge.h"
#include "sludge/version.h"

namespace Sludge {

TextManager::TextManager() {
	init();
}

TextManager::~TextManager() {
	kill();
}

void TextManager::init() {
	_theFont.total = 0;
	_theFont.sprites = nullptr;

	_fontHeight = 0;
	_numFontColours = 0;
	_loadedFontNum = 0;
	_fontSpace = -1;

	_pastePalette.init();
	_fontTable.clear();
}

void TextManager::kill() {
	GraphicsManager::forgetSpriteBank(_theFont);
	_pastePalette.kill();
}

bool TextManager::isInFont(const Common::String &theText) {
	if (_fontTable.empty())
		return 0;
	if (theText.empty())
		return 0;

	Common::U32String str32 = theText.decode(Common::kUtf8);

	// We don't want to compare strings. Only single characters allowed!
	if (str32.size() > 1)
		return false;

	uint32 c = str32[0];

	// check if font order contains the utf8 char
	return _fontOrder.contains(c);
}

int TextManager::stringLength(const Common::String &theText) {
	Common::U32String str32 = theText.decode(Common::kUtf8);
	return str32.size();
}

int TextManager::stringWidth(const Common::String &theText) {
	int xOff = 0;

	if (_fontTable.empty())
		return 0;

	Common::U32String str32 = theText.decode(Common::kUtf8);

	for (uint i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		xOff += _theFont.sprites[fontInTable(c)].surface.w + _fontSpace;
	}

	return xOff;
}

void TextManager::pasteString(const Common::String &theText, int xOff, int y, SpritePalette &thePal) {
	if (_fontTable.empty())
		return;

	xOff += (int)((float)(_fontSpace >> 1) / g_sludge->_gfxMan->getCamZoom());

	Common::U32String str32 = theText.decode(Common::kUtf8);

	for (uint32 i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		Sprite *mySprite = &_theFont.sprites[fontInTable(c)];
		g_sludge->_gfxMan->fontSprite(xOff, y, *mySprite, thePal);
		xOff += (int)((double)(mySprite->surface.w + _fontSpace) / g_sludge->_gfxMan->getCamZoom());
	}
}

void TextManager::pasteStringToBackdrop(const Common::String &theText, int xOff, int y) {
	if (_fontTable.empty())
		return;

	Common::U32String str32 = theText.decode(Common::kUtf8);

	xOff += _fontSpace >> 1;
	for (uint32 i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		Sprite *mySprite = &_theFont.sprites[fontInTable(c)];
		g_sludge->_gfxMan->pasteSpriteToBackDrop(xOff, y, *mySprite, _pastePalette);
		xOff += mySprite->surface.w + _fontSpace;
	}
}

void TextManager::burnStringToBackdrop(const Common::String &theText, int xOff, int y) {
	if (_fontTable.empty())
		return;

	Common::U32String str32 = theText.decode(Common::kUtf8);

	xOff += _fontSpace >> 1;
	for (uint i = 0; i < str32.size(); ++i) {
		uint32 c = str32[i];
		Sprite *mySprite = &_theFont.sprites[fontInTable(c)];
		g_sludge->_gfxMan->burnSpriteToBackDrop(xOff, y, *mySprite, _pastePalette);
		xOff += mySprite->surface.w + _fontSpace;
	}
}

bool TextManager::loadFont(int filenum, const Common::String &charOrder, int h) {
	_fontOrder = charOrder.decode(Common::kUtf8);

	g_sludge->_gfxMan->forgetSpriteBank(_theFont);

	_loadedFontNum = filenum;

	// get max value among all utf8 chars
	Common::U32String fontOrderString = _fontOrder;

	// create an index table from utf8 char to the index
	if (!_fontTable.empty()) {
		_fontTable.clear();
	}

	for (uint i = 0; i < fontOrderString.size(); ++i) {
		uint32 c = fontOrderString[i];
		_fontTable[c] = i;
	}

	if (!g_sludge->_gfxMan->loadSpriteBank(filenum, _theFont, true)) {
		fatal("Can't load font");
		return false;
	}

	_numFontColours = _theFont.myPalette.total;
	_fontHeight = h;
	return true;
}

// load & save
void TextManager::saveFont(Common::WriteStream *stream) {
	stream->writeByte(!_fontTable.empty());
	if (!_fontTable.empty()) {
		stream->writeUint16BE(_loadedFontNum);
		stream->writeUint16BE(_fontHeight);
		writeString(_fontOrder.encode(Common::kUtf8), stream);
	}
	stream->writeSint16LE(_fontSpace);
}

void TextManager::loadFont(int ssgVersion, Common::SeekableReadStream *stream) {
	bool fontLoaded = stream->readByte();
	int fontNum = 0;
	Common::String charOrder = "";
	if (fontLoaded) {
		fontNum = stream->readUint16BE();
		_fontHeight = stream->readUint16BE();

		if (ssgVersion < VERSION(2, 2)) {
			char *tmp = new char[257];
			for (int a = 0; a < 256; a++) {
				int x = stream->readByte();
				tmp[x] = a;
			}
			tmp[256] = 0;
			charOrder = tmp;
			delete []tmp;
		} else {
			charOrder = readString(stream);
		}
	}
	loadFont(fontNum, charOrder, _fontHeight);

	_fontSpace = stream->readSint16LE();
}

} // End of namespace Sludge
