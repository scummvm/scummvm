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

#include "common/language.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Kyra {

class Util {
public:
	static int decodeString1(const char *src, char *dst);
	static void decodeString2(const char *src, char *dst);
	static Common::String decodeString1(const Common::String &src);
	static Common::String decodeString2(const Common::String &src);

	static void convertString_KYRAtoGUI(char *str, int bufferSize, Common::CodePage srcCP = Common::kDos850);
	static void convertString_GUItoKYRA(char *str, int bufferSize, Common::CodePage dstCP = Common::kDos850);
	static Common::String convertString_GUItoKYRA(Common::String &str, Common::CodePage dstCP = Common::kDos850);
	static Common::String convertISOToUTF8(Common::String &str);
	static void convertISOToDOS(char &c);

	static uint16 convertDOSToJohab(char c, uint8 *mergeFlags = 0);
	// This method does not only need a ref to the new character, but also to the one before that, since
	// both of these will be used to calculate the output and also both of these may get modfied. The
	// reset parameter will reset the internal merge state (useful when the build-up process is broken,
	// e. g. when typing a one-byte character, like a digit).
	static void mergeUpdateJohabChars(uint16 &destJohabChar0, uint16 &destJohabChar1, char asciiInput, bool reset);

	static Common::String findMacResourceFile(const char *baseName);

private:
	struct DOS2JOHABEntry {
		char key;
		uint16 johabChar;
		uint8 flags;
	};

	static const DOS2JOHABEntry _johabConvTable[52];
};

} // End of namespace Kyra

#endif
