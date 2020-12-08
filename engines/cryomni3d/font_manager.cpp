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
#include "common/hash-ptr.h"
#include "common/hash-str.h"
#ifdef USE_FREETYPE2
#include "graphics/fonts/ttf.h"
#endif
#include "graphics/managed_surface.h"

#include "cryomni3d/font_manager.h"
#include "cryomni3d/fonts/cryoextfont.h"
#include "cryomni3d/fonts/cryofont.h"

namespace CryOmni3D {

FontManager::FontManager() : _currentFont(nullptr), _transparentBackground(false),
	_spaceWidth(0), _charSpacing(0), _lineHeight(30), _foreColor(0), _blockTextRemaining(nullptr),
	_useSpaceDelimiter(true), _keepASCIIjoined(true), _codepage(Common::kCodePageInvalid),
	_toUnicode(false) {
}

FontManager::~FontManager() {
	Common::HashMap<Graphics::Font *, bool> deletedFonts;
	for (Common::Array<Graphics::Font *>::iterator it = _fonts.begin(); it != _fonts.end(); it++) {
		if (deletedFonts.find(*it) != deletedFonts.end()) {
			// Already deleted
			continue;
		}

		deletedFonts[*it] = true;
		delete *it;
	}
}

void FontManager::loadFonts(const Common::Array<Common::String> &fontFiles,
                            Common::CodePage codepage) {
	assert(codepage != Common::kCodePageInvalid);
	_codepage = codepage;
	setupWrapParameters();

	// Cryo fonts are never in Unicode
	_toUnicode = false;

	_fonts.clear();
	_fonts.reserve(fontFiles.size());

	Common::HashMap<Common::String, Graphics::Font *> fontsCache;

	for (Common::Array<Common::String>::const_iterator it = fontFiles.begin(); it != fontFiles.end();
	        it++) {
		Graphics::Font *fontEntry = fontsCache.getVal(*it, nullptr);
		if (fontEntry) {
			_fonts.push_back(fontEntry);
			continue;
		}

		// New font

		// For now only support CP950 in extended cryo font
		if (_codepage == Common::kWindows950) {
			CryoExtFont *font = new CryoExtFont();
			font->load(*it, _codepage);
			_fonts.push_back(font);
			fontsCache[*it] = font;
		} else {
			CryoFont *font = new CryoFont();
			font->load(*it);
			_fonts.push_back(font);
			fontsCache[*it] = font;
		}
	}
}

void FontManager::loadTTFList(const Common::String &ttfList, Common::CodePage codepage) {
#ifdef USE_FREETYPE2
	assert(codepage != Common::kCodePageInvalid);
	_codepage = codepage;
	setupWrapParameters();

	// Freetype2 is configured to use Unicode
	_toUnicode = true;

	_fonts.clear();

	Common::File list;

	if (!list.open(ttfList)) {
		error("can't open file %s", ttfList.c_str());
	}

	Common::String line = list.readLine();
	uint32 num = atoi(line.c_str());

	_fonts.reserve(num);

	for (uint i = 0; i < num; i++) {
		line = list.readLine();
		if (line.size() == 0) {
			error("Invalid font list: missing line");
		}

		uint32 sharpFile = line.find("#");
		if (sharpFile == Common::String::npos) {
			error("Invalid font list: missing #");
		}
		uint32 sharpFlags = line.find("#", sharpFile + 1);
		if (sharpFlags == Common::String::npos) {
			error("Invalid font list: missing #");
		}

		Common::String fontFace(line.begin(), line.begin() + sharpFile);
		Common::U32String uniFontFace = fontFace.decode(codepage);
		Common::String fontFile(line.begin() + sharpFile + 1, line.begin() + sharpFlags);
		Common::String sizeFlags(line.begin() + sharpFlags + 1, line.end());

		uint32 size = atoi(sizeFlags.c_str());
		bool bold = sizeFlags.contains('B');
		bool italic = sizeFlags.contains('I');

		Common::Array<Common::String> fontFiles;
		fontFiles.push_back(fontFile);

		// Use 96 dpi as it's the default under Windows
		Graphics::Font *font = Graphics::findTTFace(fontFiles, uniFontFace, bold, italic, -(int)size,
		                       96, Graphics::kTTFRenderModeMonochrome);
		if (!font) {
			error("Can't find required face (line %u) in %s", i, fontFile.c_str());
		}
		_fonts.push_back(font);
	}
#else
	error("TrueType support not compiled in");
#endif
}

Common::U32String FontManager::toU32(const Common::String &str) const {
	assert(_codepage != Common::kCodePageInvalid);

	if (_toUnicode) {
		return str.decode(_codepage);
	}

	// Beware: when not using Unicode, U32String will contain codepoints not corresponding to Unicode
	switch (_codepage) {
	case Common::kUtf8:
		error("UTF-8 not supported");
	case Common::kWindows932:
	case Common::kWindows949:
	case Common::kWindows950: {
		/* if high-order bit is 1, then character is 2 bytes else it's 1 byte
		 * We don't check validity of the codepoint */
		Common::U32String ret;
		for (uint32 i = 0; i < str.size(); i++) {
			uint32 c = (byte)str[i];
			if ((c & 0x80) && (i + 1 < str.size())) {
				c <<= 8;
				i++;
				c |= str[i] & 0xff;
			}
			ret += c;
		}
		return ret;
	}
	default:
		// All other codepages are SBCS: one byte is one character
		// We use kISO8859_1 as it's the identity function [0-255] to [0-255]
		return str.decode(Common::kISO8859_1);
	}
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
	// For now space character is still the same in all encodings: 0x20
	if (_currentFont) {
		_spaceWidth = additionalSpace + _currentFont->getCharWidth(' ');
	} else {
		_spaceWidth = 0;
	}
}

uint FontManager::displayStr_(uint x, uint y,
                              const Common::U32String &text) const {
	uint offset = 0;
	for (Common::U32String::const_iterator it = text.begin(); it != text.end(); it++) {
		_currentFont->drawChar(_currentSurface, *it, x + offset, y, _foreColor);
		offset += _currentFont->getCharWidth(*it) + _charSpacing;
	}
	return offset;
}

uint FontManager::getStrWidth(const Common::U32String &text) const {
	uint width = 0;
	for (Common::U32String::const_iterator it = text.begin(); it != text.end(); it++) {
		uint32 c = *it;
		if (c == ' ') {
			width += _spaceWidth;
		} else {
			width += _currentFont->getCharWidth(*it) + _charSpacing;
		}
	}
	return width;
}

bool FontManager::displayBlockText(const Common::U32String &text,
                                   Common::U32String::const_iterator begin) {
	bool notEnoughSpace = false;
	Common::U32String::const_iterator ptr = begin;
	Common::Array<Common::U32String> words;

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
			Common::Array<Common::U32String>::const_iterator word;
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

uint FontManager::getLinesCount(const Common::U32String &text, uint width) {
	if (text.size() == 0) {
		// One line even if it's empty
		return 1;
	}
	if (text.size() >= 1024) {
		// Too long text, be lazy
		return getStrWidth(text) / width + 3;
	}

	uint lineCount = 0;
	Common::U32String::const_iterator textP = text.begin();
	uint len = text.size();

	while (len > 0) {
		Common::U32String buffer;
		uint lineWidth = 0;
		lineCount++;
		while (lineWidth < width && len > 0 && *textP != '\r') {
			buffer += *(textP++);
			len--;
			lineWidth = getStrWidth(buffer);
		}

		if (lineWidth >= width) {
			// We overrun the line, get backwards
			if (_useSpaceDelimiter) {
				uint bufferSize = buffer.size();
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
					// Split in middle of something
					textP += bufferSize - 1;
					len -= bufferSize - 1;
				}
				if (*textP == ' ') {
					textP++;
				}
			} else {
				if (buffer.size()) {
					buffer.deleteLastChar();
					textP--;
					len++;
				} else {
					// fail
					return 0;
				}
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

void FontManager::calculateWordWrap(const Common::U32String &text,
                                    Common::U32String::const_iterator *position, uint *finalPos, bool *hasCr,
                                    Common::Array<Common::U32String> &words) const {
	*hasCr = false;
	uint offset = 0;
	bool wordWrap = false;
	uint lineWidth = _blockRect.right - _blockRect.left;
	Common::U32String::const_iterator ptr = *position;

	words.clear();

	if (ptr == text.end() || *ptr == '\r') {
		ptr++;
		*hasCr = true;
		*position = ptr;
		*finalPos = offset;
		return;
	}

	while (!wordWrap) {
		Common::U32String::const_iterator begin = ptr;
		for (; ptr != text.end() && *ptr != '\r' && (!_useSpaceDelimiter || *ptr != ' '); ptr++) { }
		Common::U32String word(begin, ptr);
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
	} /**/ else {
		// couldn't get a word (too long): we are at start of line
		Common::U32String::const_iterator begin = ptr;
		// Start with one character
		for (ptr++; ptr != text.end(); ptr++) {
			Common::U32String word(begin, ptr);
			uint width = getStrWidth(word);
			if (width >= lineWidth) {
				break;
			}
			offset = width;
		}
		// We overran: go back
		if (ptr != begin) {
			ptr--;
		}
		if (_keepASCIIjoined) {
			Common::U32String::const_iterator end = ptr;
			// Until now ptr was pointing after the last character
			// As we want to look at it, go back
			if (ptr != begin) {
				ptr--;
			}
			for (; ptr != begin; ptr--) {
				// Try to split at space or non-ASCII character
				if (*ptr >= 0x80) {
					break;
				}
				if (Common::isSpace(*ptr)) {
					break;
				}
			}
			if (ptr == begin) {
				// Too bad: we have to split in middle of something
				ptr = end;
			} else {
				// Go back just after last character
				ptr++;
			}
		}
		Common::U32String word(begin, ptr);
		words.push_back(word);
	} /**/
	*finalPos = offset;
	*position = ptr;
}

void FontManager::setupWrapParameters() {
	switch (_codepage) {
	case Common::kWindows932:
		_useSpaceDelimiter = true;
		_keepASCIIjoined = false;
		break;
	case Common::kWindows949:
		_useSpaceDelimiter = true;
		_keepASCIIjoined = false;
		break;
	case Common::kWindows950:
		_useSpaceDelimiter = false;
		_keepASCIIjoined = true;
		break;
	default:
		_useSpaceDelimiter = true;
		_keepASCIIjoined = false;
		break;
	}
}

} // End of namespace CryOmni3D
