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

#ifndef DARKSEED_CONSOLE_H
#define DARKSEED_CONSOLE_H

#include "graphics/font.h"
#include "darkseed/sound.h"
#include "darkseed/tostext.h"
#include "darkseed/langtext.h"

namespace Darkseed {

class Console {
private:
	TosText *_tosText;
	Graphics::Font *_font;
	Sound *_sound;

	Common::U32StringArray _text;
	int _startIdx = 0;
	bool _redrawRequired = false;
	int _numLines = 4;
	int _lineHeight = 11;
	bool _isCJKLanguage = false;

public:
	Console(TosText *tostext, Sound *sound);
	~Console();

	void clear();

	void printTosText(int tosIndex, bool shouldAddToCurrentLine = false);
	void addTextLine(const Common::U32String &text);
	void addToCurrentLine(const Common::String &text);
	void addToCurrentLineU32(const Common::U32String &text);

	void addI18NText(const I18nText &text);

	void draw(bool forceRedraw = false);
	void drawStringAt(int x, int y, const Common::U32String &text) const;

private:
	void addLine(const Common::U32String &line);
};

} // End of namespace Darkseed

#endif // DARKSEED_CONSOLE_H
