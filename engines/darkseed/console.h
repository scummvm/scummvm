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

#include "darkseed/gamefont.h"
#include "darkseed/sound.h"
#include "darkseed/nsp.h"
#include "darkseed/tostext.h"

namespace Darkseed {

class Console  {
private:
	TosText *_tosText;
	GameFont _font;
	Sound *_sound;

	Common::StringArray _text;
	int _startIdx = 0;
	bool _redrawRequired = false;

public:
	Console(TosText *tostext, Sound *sound);

	void printTosText(int tosIndex);
	void addTextLine(const Common::String &text);
	void addToCurrentLine(const Common::String &text);

	void draw();

private:
	void addLine(const Common::String &line);
};

} // End of namespace Darkseed

#endif // DARKSEED_CONSOLE_H
