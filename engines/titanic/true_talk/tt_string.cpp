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
#include "titanic/support/simple_file.h"

namespace Titanic {

TTstring::TTstring() : _status(SS_VALID) {
	_data = new TTstringData();
}

TTstring::TTstring(const char *str) : _status(SS_VALID) {
	_data = new TTstringData(str);
}

TTstring::TTstring(const CString &str) {
	_status = SS_VALID;
	_data = new TTstringData(str);
}

TTstring::TTstring(TTstring &str) {
	if (str._status != SS_VALID) {
		_status = SS_5;
		_data = nullptr;
	} else {
		_status = SS_VALID;
		_data = str._data;
		_data->_referenceCount++;
	}
}

TTstring::~TTstring() {
	if (_data && --_data->_referenceCount == 0)
		delete _data;
}

void TTstring::operator=(const TTstring &str) {
	// Delete old string reference, if any
	if (_data && --_data->_referenceCount == 0)
		delete _data;

	// Copy source string data
	_status = str._status;
	_data = str._data;
	if (_data)
		_data->_referenceCount++;
}

void TTstring::operator=(const CString &str) {
	operator=(str.c_str());
}

void TTstring::operator=(const char *str) {
	// Delete old string reference, if any
	if (_data && --_data->_referenceCount == 0)
		delete _data;

	// Create new string data
	_data = new TTstringData(str);
	_status = SS_VALID;
}

TTstring &TTstring::operator+=(const char *str) {
	_data->_string += str;
	return *this;
}

TTstring &TTstring::operator+=(const TTstring &str) {
	_data->_string += str;
	return *this;
}

TTstring &TTstring::operator+=(char c) {
	_data->_string += c;
	return *this;
}

const char &TTstring::operator[](uint index) {
	return *(c_str() + index);
}

bool TTstring::empty() const {
	return _data->_string.empty();
}

char TTstring::firstChar() const {
	return _data->_string.firstChar();
}

char TTstring::lastChar() const {
	return _data->_string.lastChar();
}

int TTstring::size() const {
	return _data->_string.size();
}

TTstring *TTstring::copy() const {
	return new TTstring(c_str());
}

bool TTstring::isValid() const {
	return _status == SS_VALID;
}

void TTstring::save(SimpleFile *file) const {
	file->writeFormat("%s", c_str());
}

} // End of namespace Titanic
