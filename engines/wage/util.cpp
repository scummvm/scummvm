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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/stream.h"

#include "wage/wage.h"

namespace Wage {

Common::Rect *readRect(Common::SeekableReadStream *in) {
	int x1, y1, x2, y2;

	y1 = in->readSint16BE();
	x1 = in->readSint16BE();
	y2 = in->readSint16BE() + 4;
	x2 = in->readSint16BE() + 4;

	bool normalized = false;

	if (x1 > x2) {
		SWAP(x1, x2);
		normalized = true;
	}

	if (y1 > y2) {
		SWAP(y1, y2);
		normalized = true;
	}

	debug(9, "readRect: %s%d, %d, %d, %d", normalized ? "norm " : "", x1, y1, x2, y2);

	return new Common::Rect(x1, y1, x2, y2);
}

const char *getIndefiniteArticle(const Common::String &word) {
	switch (word[0]) {
	case 'a': case 'A':
	case 'e': case 'E':
	case 'i': case 'I':
	case 'o': case 'O':
	case 'u': case 'U':
		return "an ";
	default:
		break;
	}
	return "a ";
}

enum {
	GENDER_MALE = 0,
	GENDER_FEMALE = 1,
	GENDER_NEUTRAL = 2
};

const char *prependGenderSpecificPronoun(int gender) {
	if (gender == GENDER_MALE)
		return "his ";
	else if (gender == GENDER_FEMALE)
		return "her ";
	else
		return "its ";
}

const char *getGenderSpecificPronoun(int gender, bool capitalize) {
	if (gender == GENDER_MALE)
		return capitalize ? "He" : "he";
	else if (gender == GENDER_FEMALE)
		return capitalize ? "She" : "she";
	else
		return capitalize ? "It" : "it";
}

bool isStorageScene(const Common::String &name) {
	if (name.equalsIgnoreCase(STORAGESCENE))
		return true;

	if (name.equalsIgnoreCase("STROAGE@")) // Jumble
		return true;

	if (name.equalsIgnoreCase("STORAGE@@")) // Jumble
		return true;

	return false;
}

} // End of namespace Wage
