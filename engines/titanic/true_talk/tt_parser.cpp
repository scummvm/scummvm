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

#include "titanic/true_talk/tt_parser.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/tt_input.h"

namespace Titanic {

int TTparser::processInput(TTinput *input) {
	_input = input;
	if (normalize(input))
		return 0;

	warning("TODO: TTparser::processInput");
	return 0;
}

int TTparser::normalize(TTinput *input) {
	TTstring *line = new TTstring();

	for (const char *lineP = input->_line.c_str(); lineP; ++lineP) {
		char c = *lineP;
		if (Common::isLower(c)) {
			(*line) += c;
		} else if (Common::isSpace(c)) {
			if (!line->empty() && line->lastChar() != ' ')
				(*line) += ' ';
		} else if (Common::isUpper(c)) {
			(*line) += toupper(c);
		} else if (Common::isDigit(c)) {
			// TODO: num handling
		}
		// TODO other cases
	}

	return 0;
}

} // End of namespace Titanic
