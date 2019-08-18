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

#include "common/debug.h"
#include "common/file.h"
#include "graphics/managed_surface.h"

#include "cryomni3d/font_manager.h"

namespace CryOmni3D {

FontManager::FontManager() : _currentFont(nullptr), _transparentBackground(false),
	_spaceWidth(0), _charSpacing(0), _lineHeight(30), _foreColor(0), _blockTextRemaining(nullptr) {
}

FontManager::~FontManager() {
	for (Common::Array<Font *>::iterator it = _fonts.begin(); it != _fonts.end(); it++) {
		delete *it;
	}
}

void FontManager::loadFonts(const Common::Array<Common::String> &fontFiles) {
	_fonts.reserve(_fonts.size() + fontFiles.size());

	for (Common::Array<Common::String>::const_iterator it = fontFiles.begin(); it != fontFiles.end();
	        it++) {
		Common::File font_fl;
		//debug("Open font file %s", it->c_str());
		if (!font_fl.open(*it)) {
			error("Can't open file %s", it->c_str());
		}
		loadFont(font_fl);
	}
}

void FontManager::loadFont(Common::ReadStream &font_fl) {
	byte magic[8];

	font_fl.read(magic, sizeof(magic));
	if (memcmp(magic, "CRYOFONT", 8)) {
		error("Invalid font magic");
	}

	// 3 unknown uint16
	(void) font_fl.readUint16BE();
	(void) font_fl.readUint16BE();
	(void) font_fl.readUint16BE();

	Font *font = new Font();

	font->maxHeight = font_fl.readSint16BE();
	//debug("Max char height %d", font.maxHeight);

	font_fl.read(font->comment, sizeof(font->comment));
	//debug("Comment %s", font.comment);

	for (uint i = 0; i < Font::kCharactersCount; i++) {
		uint16 h = font_fl.readUint16BE();
		uint16 w = font_fl.readUint16BE();
		uint sz = font->chars[i].setup(w, h);
		//debug("Char %d sz %dx%d %d", i, w, h, sz);
		font->chars[i].offX = font_fl.readSint16BE();
		font->chars[i].offY = font_fl.readSint16BE();
		font->chars[i].printedWidth = font_fl.readUint16BE();
		//debug("Char %d offX %d offY %d PW %d", i, font.chars[i].offX, font.chars[i].offY, font.chars[i].printedWidth);

		font_fl.read(font->chars[i].data, sz);
		//debug("Char %d read %d", i, v);
	}

	_fonts.push_back(font);
}

void FontManager::setCurrentFont(int currentFont) {
	if (currentFont == -1) {
		currentFont = 0;
	}
	_currentFontId = currentFont;
	_currentFont = _fonts[currentFont];

	setSpaceWidth(0);
}

void FontManager::setSpaceWidth(uint additionalSpace) {
	if (_currentFont) {
		_spaceWidth = additionalSpace + _currentFont->chars[0].printedWidth;
	} else {
		_spaceWidth = 0;
	}
}

uint FontManager::displayStr_(uint x, uint y,
                              const Common::String &text) const {
	uint offset = 0;
	for (Common::String::const_iterator it = text.begin(); it != text.end(); it++) {
		offset += displayChar(x + offset, y, *it);
	}
	return offset;
}

uint FontManager::displayChar(uint x, uint y, unsigned char c) const {
	if (!_currentFont) {
		error("There is no current font");
	}
	if (!_currentSurface) {
		error("There is no current surface");
	}

	if (c < ' ' || c >= 255) {
		c = '?';
	}
	c -= 32;

	const Character &char_ = _currentFont->chars[c];
	int realX = x + char_.offX;
	int realY = y + char_.offY + _currentFont->maxHeight - 2;

	if (!_transparentBackground) {
		_currentSurface->fillRect(Common::Rect(realX, realY, realX + char_.w, realY + char_.h), 0xff);
	}
	Graphics::Surface src;
	src.init(char_.w, char_.h, char_.w, char_.data, Graphics::PixelFormat::createFormatCLUT8());
	_currentSurface->transBlitFrom(src, Common::Point(realX, realY), 0, false, _foreColor);

	// WORKAROUND: in Versailles game the space width is calculated differently in this function and in the getStrWidth one, let's try to be consistent
#define KEEP_SPACE_BUG
#ifndef KEEP_SPACE_BUG
	if (c == 0) {
		return _spaceWidth;
	} else {
		return _charSpacing + char_.printedWidth;
	}
#else
	return _charSpacing + char_.printedWidth;
#endif
}

uint FontManager::getStrWidth(const Common::String &text) const {
	uint width = 0;
	for (Common::String::const_iterator it = text.begin(); it != text.end(); it++) {
		unsigned char c = *it;
		if (c == ' ') {
			width += _spaceWidth;
		} else {
			if (c < ' ' || c >= 255) {
				c = '?';
			}
			c -= 32;
			width += _charSpacing;
			width += _currentFont->chars[c].printedWidth;
		}
	}
	return width;
}

bool FontManager::displayBlockText(const Common::String &text,
                                   Common::String::const_iterator begin) {
	bool notEnoughSpace = false;
	Common::String::const_iterator ptr = begin;
	Common::Array<Common::String> words;

	if (begin != text.end()) {
		_blockTextRemaining = nullptr;
		while (ptr != text.end() && !notEnoughSpace) {
			uint finalPos;
			bool has_cr;
			calculateWordWrap(text, &ptr, &finalPos, &has_cr, words);
			uint spacesWidth = (words.size() - 1) * _spaceWidth;
			uint remainingSpace = (_blockRect.right - finalPos);
			uint spaceConsumed = 0;
			double spaceWidthPerWord;
			if (words.size() == 1) {
				spaceWidthPerWord = _spaceWidth;
			} else {
				spaceWidthPerWord = (double)spacesWidth / (double)words.size();
			}
			Common::Array<Common::String>::const_iterator word;
			uint word_i;
			for (word = words.begin(), word_i = 0; word != words.end(); word++, word_i++) {
				_blockPos.x += displayStr_(_blockPos.x, _blockPos.y, *word);
				if (!_justifyText || has_cr) {
					_blockPos.x += _spaceWidth;
				} else {
					double sp = (word_i + 1) * spaceWidthPerWord - spaceConsumed;
					_blockPos.x += int16(sp);
					spaceConsumed += uint(sp);
					remainingSpace -= uint(sp);
				}
			}
			if (_blockPos.y + _lineHeight + getFontMaxHeight() >= _blockRect.bottom) {
				notEnoughSpace = true;
				_blockTextRemaining = ptr;
			} else {
				_blockPos.x = _blockRect.left;
				_blockPos.y += _lineHeight;
			}
		}
	}
	return notEnoughSpace;
}

uint FontManager::getLinesCount(const Common::String &text, uint width) {
	if (text.size() == 0) {
		// One line even if it's empty
		return 1;
	}
	if (text.size() > 1024) {
		// Too long text, be lazy
		return getStrWidth(text) / width + 3;
	}

	uint lineCount = 0;
	Common::String::const_iterator textP = text.begin();
	uint len = text.size();

	while (len > 0) {
		Common::String buffer;
		uint lineWidth = 0;
		lineCount++;
		while (lineWidth < width && len > 0 && *textP != '\r') {
			buffer += *(textP++);
			len--;
			lineWidth = getStrWidth(buffer);
		}

		if (lineWidth >= width) {
			// We overrun the line, get backwards
			while (buffer.size()) {
				if (buffer[buffer.size() - 1] == ' ') {
					break;
				}
				buffer.deleteLastChar();
				textP--;
				len++;
			}
			if (!buffer.size()) {
				// Word was too long: fail
				return 0;
			}
			if (*textP == ' ') {
				textP++;
			}
			// Continue with next line
			continue;
		}

		if (len == 0) {
			// Job is finished
			break;
		}
		if (*textP == '\r') {
			// Next line
			len--;
			textP++;
		}
	}
	return lineCount;
}

void FontManager::calculateWordWrap(const Common::String &text,
                                    Common::String::const_iterator *position, uint *finalPos, bool *hasCr,
                                    Common::Array<Common::String> &words) const {
	*hasCr = false;
	uint offset = 0;
	bool wordWrap = false;
	uint lineWidth = _blockRect.right - _blockRect.left;
	Common::String::const_iterator ptr = *position;

	words.clear();

	if (ptr == text.end() || *ptr == '\r') {
		ptr++;
		*hasCr = true;
		*position = ptr;
		*finalPos = offset;
		return;
	}

	while (!wordWrap) {
		Common::String::const_iterator begin = ptr;
		for (; ptr != text.end() && *ptr != '\r' && *ptr != ' '; ptr++) { }
		Common::String word(begin, ptr);
		uint width = getStrWidth(word);
		if (width + offset >= lineWidth) {
			wordWrap = true;
			// word is too long: just put pointer back at begining
			ptr = begin;
		} else {
			words.push_back(word);
			offset += width + _spaceWidth;
			for (; ptr != text.end() && *ptr == ' '; ptr++) { }
			for (; ptr != text.end() && *ptr == '\r'; ptr++) {
				wordWrap = true;
				*hasCr = true;
			}
		}
	}

	if (words.size() > 0) {
		offset -= _spaceWidth;
	}
	*finalPos = offset;
	*position = ptr;
}

FontManager::Character::Character() : h(0), w(0), offX(0), offY(0), printedWidth(0), data(0) {
}

FontManager::Character::~Character() {
	delete[] data;
}

uint FontManager::Character::setup(uint16 width, uint16 height) {
	w = width;
	h = height;
	uint sz = w * h;
	data = new byte[sz];
	return sz;
}

} // End of namespace CryOmni3D
