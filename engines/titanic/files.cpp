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

#include "titanic/files.h"
#include "common/util.h"

namespace Titanic {

SimpleFile::SimpleFile() {
}

void SimpleFile::open(const Common::String &name, FileMode mode) {
	assert(mode == FILE_READ);
	if (!_file.open(name))
		error("Could not find file - %s", name.c_str());
}

void SimpleFile::close() {
	_file.close();
}

void SimpleFile::safeRead(void *dst, size_t count) {
	if (_file.read(dst, count) != count)
		error("Could not read %d bytes", count);
}

int SimpleFile::unsafeRead(void *dst, size_t count) {
	return _file.read(dst, count);
}

CString SimpleFile::readString() {
	char c;
	CString result;
	bool backslashFlag = false;

	// First skip any spaces
	do {
		safeRead(&c, 1);
	} while (Common::isSpace(c));

	// Ensure we've found a starting quote for the string
	if (c != '"')
		error("Could not find starting quote");

	bool endFlag = false;
	while (!endFlag) {
		// Read the next character
		safeRead(&c, 1);

		if (backslashFlag) {
			backslashFlag = false;
			switch (c) {
			case 'n':
				result += '\n';
				break;
			case 'r':
				result += '\r';
				break;
			case '\t':
				result += '\t';
				break;
			default:
				result += c;
				break;
			}
		} else {
			switch (c) {
			case '"':
				endFlag = true;
				break;
			case '\\':
				backslashFlag = true;
				break;
			default:
				result += c;
				break;
			}
		}
	}

	// Return the string
	return result;
}

int SimpleFile::readNumber() {
	char c;
	int result = 0;
	bool minusFlag = false;

	// First skip any spaces
	do {
		safeRead(&c, 1);
	} while (Common::isSpace(c));

	// Check for prefix sign
	if (c == '+' || c == '-') {
		minusFlag = c == '-';
		safeRead(&c, 1);
	}

	// Read in the number
	if (!Common::isDigit(c))
		error("Invalid number");

	while (Common::isDigit(c)) {
		result = result * 10 + (c - '0');
		safeRead(&c, 1);
	}

	// Finally, if it's a minus value, then negate it
	if (minusFlag)
		result = -result;

	return result;
}

double SimpleFile::readFloat() {
	char c;
	Common::String result;

	// First skip any spaces
	do {
		safeRead(&c, 1);
	} while (Common::isSpace(c));

	// Check for prefix sign
	if (c == '+' || c == '-') {
		result += c;
		safeRead(&c, 1);
	}

	// Read in the number
	if (!Common::isDigit(c))
		error("Invalid number");

	while (Common::isDigit(c) || c == '.') {
		result += c;
		safeRead(&c, 1);
	}

	// Convert to a float and return it
	float floatValue;
	sscanf(result.c_str(), "%f", &floatValue);

	return floatValue;
}

} // End of namespace Titanic
