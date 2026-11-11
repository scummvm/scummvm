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

#ifndef TESORO_CORE_HOVERTEXT_H
#define TESORO_CORE_HOVERTEXT_H

#include "common/scummsys.h"

namespace Tesoro {

class HoverText {
public:
	static const uint kSize = 160;

	HoverText();

	void set(const char *text, int colour);

	const char *text() const { return _text; }
	bool empty() const { return _text[0] == '\0'; }
	uint16 colour() const { return _colour; }

	const char *buffer() const { return _text; }

private:
	char _text[kSize];
	uint16 _colour;
};

} // End of namespace Tesoro

#endif
