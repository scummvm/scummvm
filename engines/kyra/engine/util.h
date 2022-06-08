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

#ifndef KYRA_UTIL_H
#define KYRA_UTIL_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Kyra {

class Util {
public:
	static int decodeString1(const char *src, char *dst);
	static void decodeString2(const char *src, char *dst);
	static Common::String decodeString1(const Common::String &src);
	static Common::String decodeString2(const Common::String &src);

	static void convertDOSToUTF8(char *str, int bufferSize);
	static void convertUTF8ToDOS(char *str, int bufferSize);
	static Common::String convertUTF8ToDOS(Common::String &str);
	static Common::String convertISOToUTF8(Common::String &str);
	static void convertISOToDOS(char &c);

	static uint8 convertKeyDOSToHAN(char &c, uint16 &dest);

	static Common::String findMacResourceFile(const char *baseName);
};

} // End of namespace Kyra

#endif
