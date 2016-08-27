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

#include "titanic/true_talk/tt_picture.h"

namespace Titanic {

bool TTpicture::_staticFlag;

TTpicture::TTpicture(const TTstring &str, WordClass wordClass, int val2, int val3, int val4, int val5, int val6) :
		TTmajorWord(str, wordClass, val2, val4), _tag(val3), _field30(val5), _field3C(val6),
		_field38(0) {
}

TTpicture::TTpicture(const TTpicture *src) : TTmajorWord(src) {
	if (getStatus()) {
		_tag = 0;
		_field30 = 0;
		_field38 = 0;
		_field3C = 0;
		_status = SS_5;
	} else {
		_tag = src->_tag;
		_field30 = src->_field30;
		_field38 = src->_field38;
		_field3C = src->_field3C;
	}
}

int TTpicture::load(SimpleFile *file) {
	CString str;
	int val1, val2;

	if (!TTword::load(file, WC_THING) && file->scanf("%s %d %d", &str, &val1, &val2)) {
		_tag = readNumber(str.c_str());
		_field30 = val1;
		_field3C = val2;
		return 0;
	} else {
		return 3;
	}
}

TTword *TTpicture::copy() const {
	TTpicture *returnWordP = new TTpicture(this);
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

bool TTpicture::checkTag() const {
	return _tag == MKTAG('S', 'E', 'X', 'X') ||
		_tag == MKTAG('E', 'X', 'C', 'R') ||
		_tag == MKTAG('P', 'P', 'R', 'T') ||
		_tag == MKTAG('B', 'L', 'A', 'S');
}

bool TTpicture::compareTagTo(uint tag) const {
	return _tag == tag;
}

uint TTpicture::getTag() const {
	return _tag;
}

bool TTpicture::proc9(int val) const {
	return _field3C == val;
}

int TTpicture::proc10() const {
	return _field3C;
}

} // End of namespace Titanic
