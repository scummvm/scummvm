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

TTstring::TTstring(const TTstring &str) {
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
	if (&str == this)
		// Trying to assign string to itself
		return;

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

bool TTstring::operator==(const TTstring &str) const {
	return _data && str._data && _data->_string == str._data->_string;
}

bool TTstring::operator==(const char *str) const {
	return _data && _data->_string == str;
}

void TTstring::save(SimpleFile *file) const {
	file->writeFormat("%s", c_str());
}

TTstring TTstring::tokenize(const char *delim) {
	const char *strP = _data->_string.c_str();
	const char *splitP = nullptr, *chP;

	for (const char *d = delim; *d; ++d) {
		chP = strchr(strP, *d);
		if (chP && (splitP == nullptr || chP < splitP))
			splitP = chP;
	}

	if (splitP) {
		TTstring result(CString(strP, splitP));
		_data->_string = CString(splitP + 1);
		return result;
	} else {
		TTstring result(strP);
		_data->_string = CString();
		return result;
	}
}

int TTstring::deletePrefix(int count) {
	int strSize = size();
	if (count > strSize)
		count = strSize;

	if (_data->_referenceCount == 1) {
		// No other references to this string, so we can just directly modify it
		_data->_string = CString(_data->_string.c_str() + count);
	} else {
		// Detach string from current shared data, and create a new one with the substring
		_data->_referenceCount--;
		_data = new TTstringData(_data->_string.c_str() + count);
	}

	return 1;
}

int TTstring::deleteSuffix(int count) {
	int strSize = size();
	if (count > strSize)
		count = strSize;

	CString newStr(_data->_string.c_str(), _data->_string.c_str() + strSize - count);
	if (_data->_referenceCount == 1) {
		// No other references to this string, so we can just directly modify it
		_data->_string = newStr;
	} else {
		// Detach string from current shared data, and create a new one with the substring
		_data->_referenceCount--;
		_data = new TTstringData(newStr);
	}

	return 1;
}

} // End of namespace Titanic
