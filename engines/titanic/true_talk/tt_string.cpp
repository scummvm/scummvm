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

#include "titanic/true_talk/tt_string.h"

namespace Titanic {

TTString::TTString() : _status(SS_VALID) {
	_data = new TTStringData();
}

TTString::TTString(const char *str) : _status(SS_VALID) {
	_data = new TTStringData(str);
}

TTString::TTString(const CString &str) {
	if (_status != SS_VALID) {
		_status = SS_5;
		_data = nullptr;
	} else {
		_status = SS_VALID;
		_data = new TTStringData(str);
	}
}

TTString::TTString(TTString &str) {
	if (_status != SS_VALID) {
		_status = SS_5;
		_data = nullptr;
	} else {
		_status = SS_VALID;
		_data = str._data;
		_data->_referenceCount++;
	}
}

TTString::~TTString() {
	if (--_data->_referenceCount == 0)
		delete _data;
}

bool TTString::isValid() const {
	return _status == SS_VALID;
}

} // End of namespace Titanic
