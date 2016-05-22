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

#include "titanic/true_talk/tt_pronoun.h"

namespace Titanic {

bool TTpronoun::_staticFlag;

TTpronoun::TTpronoun(TTstring &str, WordClass wordClass, int val2, int val3, int val4) :
		TTmajorWord(str, wordClass, val2, val3), _field30(val4) {
}

TTpronoun::TTpronoun(const TTpronoun *src) : TTmajorWord(src) {
	if (src->getStatus()) {
		_field30 = 0;
		_status = SS_5;
	} else {
		_field30 = src->_field30;
	}
}

int TTpronoun::load(SimpleFile *file) {
	int val;

	if (!TTword::load(file, WC_PRONOUN) && file->scanf("%d", &val)) {
		if (val >= 0 && val <= 12) {
			_field30 = val;
			return 0;
		} else {
			return 5;
		}
	} else {
		return 8;
	}
}

TTword *TTpronoun::copy() const {
	TTpronoun *returnWordP = new TTpronoun(this);
	returnWordP->_status = _status;
	if (!_status) {
		_staticFlag = false;
		return returnWordP;
	} else if (_status == SS_13 && !_staticFlag) {
		_staticFlag = true;
		delete returnWordP;
		return copy();
	} else {
		delete returnWordP;
		return nullptr;
	}
}

} // End of namespace Titanic
