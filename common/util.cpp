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

#define FORBIDDEN_SYMBOL_EXCEPTION_ctype_h

#include "common/util.h"
#include "common/debug.h"
#include "common/translation.h"

namespace Common {

//
// Print hexdump of the data passed in
//
void hexdump(const byte *data, int len, int bytesPerLine, int startOffset) {
	assert(1 <= bytesPerLine && bytesPerLine <= 32);
	int i;
	byte c;
	int offset = startOffset;
	while (len >= bytesPerLine) {
		debugN("%06x: ", offset);
		for (i = 0; i < bytesPerLine; i++) {
			debugN("%02x ", data[i]);
			if (i % 4 == 3)
				debugN(" ");
		}
		debugN(" |");
		for (i = 0; i < bytesPerLine; i++) {
			c = data[i];
			if (c < 32 || c >= 127)
				c = '.';
			debugN("%c", c);
		}
		debugN("|\n");
		data += bytesPerLine;
		len -= bytesPerLine;
		offset += bytesPerLine;
	}

	if (len <= 0)
		return;

	debugN("%06x: ", offset);
	for (i = 0; i < bytesPerLine; i++) {
		if (i < len)
			debugN("%02x ", data[i]);
		else
			debugN("   ");
		if (i % 4 == 3)
			debugN(" ");
	}
	debugN(" |");
	for (i = 0; i < len; i++) {
		c = data[i];
		if (c < 32 || c >= 127)
			c = '.';
		debugN("%c", c);
	}
	for (; i < bytesPerLine; i++)
		debugN(" ");
	debugN("|\n");
}


#pragma mark -


bool parseBool(const String &val, bool &valAsBool) {
	if (val.equalsIgnoreCase("true") ||
		val.equalsIgnoreCase("yes") ||
		val.equals("1")) {
		valAsBool = true;
		return true;
	}
	if (val.equalsIgnoreCase("false") ||
		val.equalsIgnoreCase("no") ||
		val.equals("0") ||
		val.empty()) {
		valAsBool = false;
		return true;
	}

	return false;
}


#pragma mark -


#define ENSURE_ASCII_CHAR(c) \
		if (c < 0 || c > 127) \
			return false

bool isAscii(int c) {
	ENSURE_ASCII_CHAR(c);
	return true;
}

bool isAlnum(int c) {
	ENSURE_ASCII_CHAR(c);
	return isalnum((byte)c);
}

bool isAlpha(int c) {
	ENSURE_ASCII_CHAR(c);
	return isalpha((byte)c);
}

bool isDigit(int c) {
	ENSURE_ASCII_CHAR(c);
	return isdigit((byte)c);
}

bool isXDigit(int c) {
	ENSURE_ASCII_CHAR(c);
	return isxdigit((byte)c);
}

bool isLower(int c) {
	ENSURE_ASCII_CHAR(c);
	return islower((byte)c);
}

bool isSpace(int c) {
	ENSURE_ASCII_CHAR(c);
	return isspace((byte)c);
}

bool isUpper(int c) {
	ENSURE_ASCII_CHAR(c);
	return isupper((byte)c);
}

bool isPrint(int c) {
	ENSURE_ASCII_CHAR(c);
	return isprint((byte)c);
}

bool isPunct(int c) {
	ENSURE_ASCII_CHAR(c);
	return ispunct((byte)c);
}

bool isCntrl(int c) {
	ENSURE_ASCII_CHAR(c);
	return iscntrl((byte)c);
}

bool isGraph(int c) {
	ENSURE_ASCII_CHAR(c);
	return isgraph((byte)c);
}

bool isBlank(int c) {
	return c == ' ' || c == '\t';
}


#pragma mark -


Common::String getHumanReadableBytes(uint64 bytes, const char *&unitsOut) {
	if (bytes < 1024) {
		// I18N: Abbreviation for 'bytes' as data size
		unitsOut = _s("B");
		return Common::String::format("%lu", (unsigned long int)bytes);
	}

	double floating = bytes / 1024.0;
	// I18N: Abbreviation for 'kilobytes' as data size
	unitsOut = _s("KB");

	if (floating >= 1024) {
		floating /= 1024.0;
		// I18N: Abbreviation for 'megabytes' as data size
		unitsOut = _s("MB");
	}

	if (floating >= 1024) {
		floating /= 1024.0;
		// I18N: Abbreviation for 'gigabytes' as data size
		unitsOut = _s("GB");
	}

	if (floating >= 1024) { // woah
		floating /= 1024.0;
		// I18N: Abbreviation for 'terabytes' as data size
		unitsOut = _s("TB");
	}

	// print one digit after floating point
	return Common::String::format("%.1f", floating);
}

} // End of namespace Common
