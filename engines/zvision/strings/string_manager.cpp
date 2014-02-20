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

#include "common/scummsys.h"

#include "zvision/strings/string_manager.h"

#include "zvision/fonts/truetype_font.h"

#include "common/file.h"
#include "common/tokenizer.h"
#include "common/debug.h"

#include "graphics/fontman.h"
#include "graphics/colormasks.h"


namespace ZVision {

StringManager::StringManager(ZVision *engine)
	: _engine(engine) {
}

StringManager::~StringManager() {
	for (Common::HashMap<Common::String, TruetypeFont *>::iterator iter = _fonts.begin(); iter != _fonts.end(); ++iter) {
		delete iter->_value;
	}
}

void StringManager::initialize(ZVisionGameId gameId) {
	if (gameId == GID_NEMESIS) {
		// TODO: Check this hardcoded filename against all versions of Nemesis
		parseStrFile("nemesis.str");
	} else if (gameId == GID_GRANDINQUISITOR) {
		// TODO: Check this hardcoded filename against all versions of Grand Inquisitor
		parseStrFile("inquis.str");
	}
}

void StringManager::parseStrFile(const Common::String &fileName) {
	Common::File file;
	if (!file.open(fileName)) {
		warning("%s does not exist. String parsing failed", fileName.c_str());
		return;
	}

	uint lineNumber = 0;
	while (!file.eos()) {
		_lastStyle.align = Graphics::kTextAlignLeft;
		_lastStyle.color = 0;
		_lastStyle.font = nullptr;

		Common::String asciiLine = readWideLine(file);
		if (asciiLine.empty()) {
			continue;
		}

		char tagString[150];
		uint tagStringCursor = 0;
		char textString[150];
		uint textStringCursor = 0;
		bool inTag = false;

		for (uint i = 0; i < asciiLine.size(); ++i) {
			switch (asciiLine[i]) {
			case '<':
				inTag = true;
				if (!_inGameText[lineNumber].fragments.empty()) {
					_inGameText[lineNumber].fragments.back().text = Common::String(textString, textStringCursor);
					textStringCursor = 0;
				}
				break;
			case '>':
				inTag = false;
				parseTag(Common::String(tagString, tagStringCursor), lineNumber);
				tagStringCursor = 0;
				break;
			default:
				if (inTag) {
					tagString[tagStringCursor] = asciiLine[i];
					tagStringCursor++;
				} else {
					textString[textStringCursor] = asciiLine[i];
					textStringCursor++;
				}
				break;
			}
		}

		if (textStringCursor > 0) {
			_inGameText[lineNumber].fragments.back().text = Common::String(textString, textStringCursor);
		}

		lineNumber++;
		assert(lineNumber <= NUM_TEXT_LINES);
	}
}

void StringManager::parseTag(const Common::String &tagString, uint lineNumber) {
	Common::StringTokenizer tokenizer(tagString);

	Common::String token = tokenizer.nextToken();

	Common::String fontName;
	bool bold = false;
	Graphics::TextAlign align = _lastStyle.align;
	int point = _lastStyle.font != nullptr ? _lastStyle.font->_fontHeight : 12;
	int red = 0;
	int green = 0;
	int blue = 0;

	while (!token.empty()) {
		if (token.matchString("font", true)) {
			fontName = tokenizer.nextToken();
		} else if (token.matchString("bold", true)) {
			token = tokenizer.nextToken();
			if (token.matchString("on", false)) {
				bold = true;
			}
		} else if (token.matchString("justify", true)) {
			token = tokenizer.nextToken();
			if (token.matchString("center", false)) {
				align = Graphics::kTextAlignCenter;
			} else if (token.matchString("right", false)) {
				align = Graphics::kTextAlignRight;
			}
		} else if (token.matchString("point", true)) {
			point = atoi(tokenizer.nextToken().c_str());
		} else if (token.matchString("red", true)) {
			red = atoi(tokenizer.nextToken().c_str());
		} else if (token.matchString("green", true)) {
			green = atoi(tokenizer.nextToken().c_str());
		} else if (token.matchString("blue", true)) {
			blue = atoi(tokenizer.nextToken().c_str());
		}

		token = tokenizer.nextToken();
	}

	TextFragment fragment;

	if (fontName.empty()) {
		fragment.style.font = _lastStyle.font;
	} else {
		Common::String newFontName;
		if (fontName.matchString("*times new roman*", true)) {
			if (bold) {
				newFontName = "timesbd.ttf";
			} else {
				newFontName = "times.ttf";
			}
		} else if (fontName.matchString("*courier new*", true)) {
			if (bold) {
				newFontName = "courbd.ttf";
			} else {
				newFontName = "cour.ttf";
			}
		} else if (fontName.matchString("*century schoolbook*", true)) {
			if (bold) {
				newFontName = "censcbkbd.ttf";
			} else {
				newFontName = "censcbk.ttf";
			}
		} else if (fontName.matchString("*garamond*", true)) {
			if (bold) {
				newFontName = "garabd.ttf";
			} else {
				newFontName = "gara.ttf";
			}
		} else {
			debug("Could not identify font: %s. Reverting to Arial", fontName.c_str());
			if (bold) {
				newFontName = "zorknorm.ttf";
			} else {
				newFontName = "arial.ttf";
			}
		}

		Common::String fontKey = Common::String::format("%s-%d", newFontName.c_str(), point);
		if (_fonts.contains(fontKey)) {
			fragment.style.font = _fonts[fontKey];
		} else {
			fragment.style.font = new TruetypeFont(_engine, point);
			fragment.style.font->loadFile(newFontName);
			_fonts[fontKey] = fragment.style.font;
		}
	}

	fragment.style.align = align;
	fragment.style.color = Graphics::ARGBToColor<Graphics::ColorMasks<565> >(0, red, green, blue);
	_inGameText[lineNumber].fragments.push_back(fragment);

	_lastStyle = fragment.style;
}

Common::String StringManager::readWideLine(Common::SeekableReadStream &stream) {
	Common::String asciiString;

	// Don't spam the user with warnings about UTF-16 support.
	// Just do one warning per String
	bool charOverflowWarning = false;

	uint16 value = stream.readUint16LE();
	while (!stream.eos()) {
		// Check for CRLF
		if (value == 0x0A0D) {
			// Read in the extra NULL char
			stream.readByte(); // \0
			// End of the line. Break
			break;
		}

		// Crush each octet pair to a single octet with a simple cast
		if (value > 255) {
			charOverflowWarning = true;
			value = '?';
		}
		char charValue = (char)value;

		asciiString += charValue;

		value = stream.readUint16LE();
	}

	if (charOverflowWarning) {
		warning("UTF-16 is not supported. Characters greater than 255 are replaced with '?'");
	}

	return asciiString;
}

StringManager::TextStyle StringManager::getTextStyle(uint stringNumber) {
	return _inGameText[stringNumber].fragments.front().style;
}

} // End of namespace ZVision
