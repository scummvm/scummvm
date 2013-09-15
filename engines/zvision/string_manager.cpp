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

#include "common/scummsys.h"

#include "common/file.h"
#include "common/tokenizer.h"
#include "common/debug.h"

#include "graphics/fontman.h"

#include "zvision/string_manager.h"
#include "zvision/truetype_font.h"


namespace ZVision {

const Graphics::PixelFormat StringManager::_pixelFormat565 = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

StringManager::StringManager(ZVision *engine) 
	: _engine(engine) {
}

StringManager::~StringManager() {
	for (Common::HashMap<Common::String, TruetypeFont *>::iterator iter = _fonts.begin(); iter != _fonts.end(); iter++) {
		delete (*iter)._value;
	}
}

void StringManager::initialize(ZVisionGameId gameId) {
	if (gameId == ZorkNemesis) {
		// TODO: Check this hardcoded filename against all versions of Nemesis
		parseStrFile("nemesis.str");
	} else if (gameId == ZorkGrandInquisitor) {
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

	Common::String line = file.readLine();

	uint lineNumber = 0;
	while (!file.eos()) {
		Common::String asciiLine = wideToASCII(line.c_str(), line.size());

		char tagString[150];
		uint tagStringCursor = 0;
		char textString[150];
		uint textStringCursor = 0;
		bool inTag = false;

		for (uint i = 0; i < asciiLine.size(); i++) {
			switch (asciiLine[i]) {
			case '<':
				inTag = true;
				break;
			case '>':
				inTag = false;
				parseTag(Common::String(tagString, tagStringCursor), Common::String(textString, textStringCursor), i);
				tagStringCursor = 0;
				textStringCursor = 0;
				break;
			default:
				if (inTag) {
					textString[tagStringCursor] = asciiLine[i];
					tagStringCursor++;
				} else {
					textString[textStringCursor] = asciiLine[i];
					textStringCursor++;
				}
				break;
			}
		}

		// STR files add a null character after the CR/LF. We need to skip over that before we can read another line
		file.readByte();

		line = file.readLine();
		lineNumber++;
	}
}

void StringManager::parseTag(const Common::String &tagString, const Common::String &textString, uint lineNumber) {
	Common::StringTokenizer tokenizer(tagString);

	Common::String token = tokenizer.nextToken();

	Common::String fontName;
	bool bold = false;
	Graphics::TextAlign align = Graphics::kTextAlignLeft;
	int point = 12;
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

	Common::String newFontName;
	if (fontName.matchString("times new roman", true)) {
		if (bold) {
			newFontName = "timesbd.ttf";
		} else {
			newFontName = "times.ttf";
		}
	} else if (fontName.matchString("courier new", true)) {
		if (bold) {
			newFontName = "courbd.ttf";
		} else {
			newFontName = "cour.ttf";
		}
	} else if (fontName.matchString("century schoolbook", true)) {
		if (bold) {
			newFontName = "censcbkbd.ttf";
		} else {
			newFontName = "censcbk.ttf";
		}
	} else if (fontName.matchString("times new roman", true)) {
		if (bold) {
			newFontName = "courbd.ttf";
		} else {
			newFontName = "cour.ttf";
		}
	} else {
		debug("Could not identify font: %s. Reverting to Arial", fontName.c_str());
		if (bold) {
			newFontName = "zorknorm.ttf";
		} else {
			newFontName = "arial.ttf";
		}
	}

	// Push an empty TextFragment onto the end of the list
	// Creating the TextFragement before filling it prevents extra data copying during creation
	_inGameText->fragments.push_back(TextFragment());
	TextFragment *fragment = &_inGameText->fragments.back();

	Common::String fontKey = Common::String::format("%s-&d", newFontName.c_str(), point);
	if (_fonts.contains(fontKey)) {
		fragment->style.font = _fonts[fontKey];
	} else {
		fragment->style.font = new TruetypeFont(_engine, point);
		fragment->style.font->loadFile(newFontName);
	}

	fragment->style.align = align;
	fragment->style.color = _pixelFormat565.ARGBToColor(0, red, green, blue);
	fragment->text = textString;
}

Common::String StringManager::wideToASCII(const char *wideStr, uint arrayLength) {
	// TODO: Contemplate using a largish static buffer instead of a dynamic heap buffer
	uint newSize = arrayLength / 2;
	char *asciiString = new char[newSize];

	// Don't spam the user with warnings about UTF-16 support.
	// Just do one warning per String
	bool charOverflowWarning = false;
	// Crush each octet pair to a single octet with a simple cast
	for (uint i = 0; i < newSize; i++) {
		uint16 value = READ_LE_UINT16(wideStr + (i * 2));
		if (value > 255) {
			charOverflowWarning = true;
			value = 255;
		}
		asciiString[i] = (char)value;
	}

	if (charOverflowWarning) {
		warning("UTF-16 is not supported. Characters greater than 255 are clamped to 255");
	}

	Common::String returnString(asciiString, newSize);
	// Cleanup. Common::String constructor does a memmove() internally so we can safely delete
	delete[] asciiString;

	return returnString;
}

StringManager::TextStyle StringManager::getTextStyle(uint stringNumber) {
	return _inGameText[stringNumber].fragments.front().style;
}

} // End of namespace ZVision
