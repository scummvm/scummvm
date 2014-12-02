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

#ifndef ZVISION_STRING_MANAGER_H
#define ZVISION_STRING_MANAGER_H

#include "zvision/detection.h"
#include "zvision/fonts/truetype_font.h"


namespace Graphics {
class FontManager;
}

namespace ZVision {

class ZVision;

class StringManager {
public:
	StringManager(ZVision *engine);
	~StringManager();

public:
	struct TextStyle {
		TruetypeFont *font;
		uint16 color; // In RBG 565
		Graphics::TextAlign align;
	};

	struct TextFragment {
		TextStyle style;
		Common::String text;
	};

	enum {
		ZVISION_STR_SAVEEXIST = 23,
		ZVISION_STR_SAVED = 4,
		ZVISION_STR_SAVEEMPTY = 21,
		ZVISION_STR_EXITPROMT = 6
	};

private:
	struct InGameText {
		Common::List<TextFragment> fragments;
	};

	enum {
		NUM_TEXT_LINES = 56 // Max number of lines in a .str file. We hardcode this number because we know ZNem uses 42 strings and ZGI uses 56
	};

private:
	ZVision *_engine;
	Common::String _lines[NUM_TEXT_LINES];

	InGameText _inGameText[NUM_TEXT_LINES];
	Common::HashMap<Common::String, TruetypeFont *> _fonts;

	TextStyle _lastStyle;

public:
	void initialize(ZVisionGameId gameId);
	StringManager::TextStyle getTextStyle(uint stringNumber);
	const Common::String getTextLine(uint stringNumber);

private:
	void loadStrFile(const Common::String &fileName);
	void parseStrFile(const Common::String &fileName);
	void parseTag(const Common::String &tagString, uint lineNumber);

	static Common::String readWideLine(Common::SeekableReadStream &stream);
};

} // End of namespace ZVision

#endif
