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

#include "common/macresman.h"
#include "common/punycode.h"
#include "kyra/engine/util.h"

namespace Kyra {

int Util::decodeString1(const char *src, char *dst) {
	static const uint8 decodeTable1[] = {
		0x20, 0x65, 0x74, 0x61, 0x69, 0x6E, 0x6F, 0x73, 0x72, 0x6C, 0x68,
		0x63, 0x64, 0x75, 0x70, 0x6D
	};

	static const uint8 decodeTable2[] = {
		0x74, 0x61, 0x73, 0x69, 0x6F, 0x20, 0x77, 0x62, 0x20, 0x72, 0x6E,
		0x73, 0x64, 0x61, 0x6C, 0x6D, 0x68, 0x20, 0x69, 0x65, 0x6F, 0x72,
		0x61, 0x73, 0x6E, 0x72, 0x74, 0x6C, 0x63, 0x20, 0x73, 0x79, 0x6E,
		0x73, 0x74, 0x63, 0x6C, 0x6F, 0x65, 0x72, 0x20, 0x64, 0x74, 0x67,
		0x65, 0x73, 0x69, 0x6F, 0x6E, 0x72, 0x20, 0x75, 0x66, 0x6D, 0x73,
		0x77, 0x20, 0x74, 0x65, 0x70, 0x2E, 0x69, 0x63, 0x61, 0x65, 0x20,
		0x6F, 0x69, 0x61, 0x64, 0x75, 0x72, 0x20, 0x6C, 0x61, 0x65, 0x69,
		0x79, 0x6F, 0x64, 0x65, 0x69, 0x61, 0x20, 0x6F, 0x74, 0x72, 0x75,
		0x65, 0x74, 0x6F, 0x61, 0x6B, 0x68, 0x6C, 0x72, 0x20, 0x65, 0x69,
		0x75, 0x2C, 0x2E, 0x6F, 0x61, 0x6E, 0x73, 0x72, 0x63, 0x74, 0x6C,
		0x61, 0x69, 0x6C, 0x65, 0x6F, 0x69, 0x72, 0x61, 0x74, 0x70, 0x65,
		0x61, 0x6F, 0x69, 0x70, 0x20, 0x62, 0x6D
	};

	int size = 0;
	uint cChar = 0;
	while ((cChar = *src++) != 0) {
		if (cChar & 0x80) {
			cChar &= 0x7F;
			int index = (cChar & 0x78) >> 3;
			*dst++ = decodeTable1[index];
			++size;
			assert(cChar < sizeof(decodeTable2));
			cChar = decodeTable2[cChar];
		}

		*dst++ = cChar;
		++size;
	}

	*dst++ = 0;
	return size;
}

void Util::decodeString2(const char *src, char *dst) {
	if (!src || !dst)
		return;

	char out = 0;
	while ((out = *src) != 0) {
		if (*src == 0x1B) {
			++src;
			out = *src + 0x7F;
		}
		*dst++ = out;
		++src;
	}

	*dst = 0;
}

void Util::convertDOSToUTF8(char *str, int bufferSize) {
	Common::strlcpy(str, Common::U32String(str, Common::kDos850).encode(Common::kUtf8).c_str(), bufferSize);
}

void Util::convertUTF8ToDOS(char *str, int bufferSize) {
	Common::strlcpy(str, Common::U32String(str, Common::kUtf8).encode(Common::kDos850).c_str(), bufferSize);
}

Common::String Util::convertUTF8ToDOS(Common::String &str) {
	return Common::U32String(str.c_str(), Common::kUtf8).encode(Common::kDos850);
}

Common::String Util::convertISOToUTF8(Common::String &str) {
	return Common::U32String(str.c_str(), Common::kISO8859_1).encode(Common::kUtf8);
}

void Util::convertISOToDOS(char &c) {
	if (c)
		c = Common::U32String(Common::String::format("%c", c), Common::kISO8859_1).encode(Common::kDos850).firstChar();
}

Common::String Util::decodeString1(const Common::String &src) {
	char *tmp = new char[src.size() * 2 + 2];
	Util::decodeString1(src.c_str(), tmp);
	return tmp;
}

Common::String Util::decodeString2(const Common::String &src) {
	char *tmp = new char[src.size() * 2 + 2];
	Util::decodeString2(src.c_str(), tmp);
	return tmp;
}

Common::String Util::findMacResourceFile(const char *baseName) {
	// The original executable has a TM char as its last character (character
	// 0xAA from Mac code page). Depending on the emulator or platform used to
	// copy the file it might have been reencoded to something else. So I look
	// for multiple versions, also for punycode encoded files and also for the
	// case where the user might have just removed the last character by
	// renaming the file.

	const Common::CodePage tryCodePages[] = {
		Common::kMacRoman,
		Common::kISO8859_1
	};

	Common::MacResManager resource;
	Common::String tryName(baseName);
	Common::String fileName;

	for (int i = 0; i < 2; ++i) {
		for (int ii = 0; ii < ARRAYSIZE(tryCodePages); ++ii) {
			Common::U32String fn(tryName, tryCodePages[ii]);
			fileName = fn.encode(Common::kUtf8);
			if (resource.exists(fileName))
				return fileName;
			fileName = Common::punycode_encodefilename(fn);
			if (resource.exists(fileName))
				return fileName;
		}
		tryName += "\xAA";
	}

	fileName.clear();
	return fileName;
}

} // End of namespace Kyra
