/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCUMM_HE_FONT_HE_H
#define SCUMM_HE_FONT_HE_H

#include "common/array.h"
#include "common/list.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "graphics/font.h"

namespace Scumm {

class ScummEngine_v99he;

// Font style flags, taken from the original
enum HEFontStyle {
	kHEFontStyleItalic = 0x01,
	kHEFontStyleUnderline = 0x02,
	kHEFontStyleBold = 0x04,
	kHEFontStyleCenter = 0x08,
	kHEFontStyleLeft = 0x10,
	kHEFontStyleRight = 0x20
};

struct HEFontContextElement {
	HEFontContextElement() : imageNumber(0),
							 font(nullptr),
							 fgColor(0),
							 bgColor(0),
							 transparentBg(false),
							 align(Graphics::kTextAlignCenter) {}

	~HEFontContextElement() {
		delete font;
		font = nullptr;
	}

	int imageNumber;
	Graphics::Font *font;
	int fgColor;
	int bgColor;
	bool transparentBg;
	Graphics::TextAlign align;
};

struct HEFontEntry {
	Common::String fileName; // Filename without extension (e.g., "LiberationSans-Bold")
	Common::String fontName; // Actual font name from metadata (e.g., "Liberation Sans")
};

class HEFont {
public:
	HEFont(ScummEngine_v99he *vm);
	~HEFont();

	bool startFont(int imageNum);
	bool endFont(int imageNum);
	bool createFont(int imageNum, const char *fontName, int fgColor, int bgColor, int style, int size);
	bool renderString(int imageNum, int imageState, int xPos, int yPos, const char *string);
	int getStringWidth(int imageNum, const char *string);
	int getStringHeight(int imageNum, const char *string);
	int enumInit();
	void enumDestroy();
	const char *enumGet(int index);
	int enumFind(const char *fontName);

private:
	ScummEngine_v99he *_vm;

	Common::List<HEFontContextElement *> _fontContextList;

	Common::Array<HEFontEntry> _fontEntries;
	bool _fontsEnumerated;

	HEFontContextElement *findFontContext(int imageNum);
	void enumerateFonts();
};

} // End of namespace Scumm

#endif // SCUMM_HE_FONT_HE_H
