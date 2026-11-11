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
#include "common/textconsole.h"

#include "tesoro/core/hovertext.h"

namespace Tesoro {

HoverText::HoverText() : _colour(0) {
	memset(_text, 0, sizeof(_text));
}

void HoverText::set(const char *text, int colour) {
	if (!text) {
		_text[0] = '\0';
	} else {
		const uint length = strlen(text);
		if (length >= kSize) {
			warning("HoverText: text of %u characters does not fit", length);
			memcpy(_text, text, kSize - 1);
			_text[kSize - 1] = '\0';
		} else {
			memcpy(_text, text, length + 1);
		}
	}

	if (colour > 0)
		_colour = (uint16)colour;
}

} // End of namespace Tesoro
