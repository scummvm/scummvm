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

uint8 Util::convertKeyDOSToHAN(char &c, uint16 &dest) {
	struct DOS2HanEntry {
		char key;
		uint16 hanChar;
		uint8 flags;
	};

	static const DOS2HanEntry hanConvTable[] = {
		{ 'A', 0x2041, 0x11 }, { 'B', 0x0741, 0x04 }, { 'C', 0x4041, 0x11 }, { 'D', 0x3441, 0x11 },
		{ 'E', 0x1841, 0x01 }, { 'F', 0x1c41, 0x11 }, { 'G', 0x5041, 0x11 }, { 'H', 0x05a1, 0x04 },
		{ 'I', 0x04a1, 0x02 }, { 'J', 0x04e1, 0x02 }, { 'K', 0x0461, 0x02 }, { 'L', 0x07a1, 0x02 },
		{ 'M', 0x0761, 0x04 }, { 'N', 0x0681, 0x04 }, { 'O', 0x04c1, 0x02 }, { 'P', 0x0581, 0x02 },
		{ 'Q', 0x2841, 0x01 }, { 'R', 0x0c41, 0x11 }, { 'S', 0x1041, 0x11 }, { 'T', 0x3041, 0x11 },
		{ 'U', 0x0561, 0x02 }, { 'V', 0x4c41, 0x01 }, { 'W', 0x3c41, 0x01 }, { 'X', 0x4841, 0x11 },
		{ 'Y', 0x0661, 0x04 }, { 'Z', 0x4441, 0x11 }, { 'a', 0x2041, 0x11 }, { 'b', 0x0741, 0x04 },
		{ 'c', 0x4041, 0x11 }, { 'd', 0x3441, 0x11 }, { 'e', 0x1441, 0x11 }, { 'f', 0x1c41, 0x11 },
		{ 'g', 0x5041, 0x11 }, { 'h', 0x05a1, 0x04 }, { 'i', 0x04a1, 0x02 }, { 'j', 0x04e1, 0x02 },
		{ 'k', 0x0461, 0x02 }, { 'l', 0x07a1, 0x02 }, { 'm', 0x0761, 0x04 }, { 'n', 0x0681, 0x04 },
		{ 'o', 0x0481, 0x02 }, { 'p', 0x0541, 0x02 }, { 'q', 0x2441, 0x11 }, { 'r', 0x0841, 0x11 },
		{ 's', 0x1041, 0x11 }, { 't', 0x2c41, 0x11 }, { 'u', 0x0561, 0x02 }, { 'v', 0x4c41, 0x11 },
		{ 'w', 0x3841, 0x11 }, { 'x', 0x4841, 0x11 }, { 'y', 0x0661, 0x04 }, { 'z', 0x4441, 0x11 }
	};

	dest = 0;
	if (!((c >= (uint8)'A' && c <= (uint8)'Z') || (c >= (uint8)'a' && c <= (uint8)'z')))
		return 0;

	uint16 res = 0;
	for (uint16 lml = 0, lmu = 51;lml <= lmu; ) {
		int16 a = (lml + lmu);
		if (a < 0)
			++a;
		res = a >> 1;
		if (hanConvTable[res].key > c)
			lmu = res - 1;
		else
			lml = res + 1;
		if (hanConvTable[res].key == c)
			break;
	}

	uint8 flags = 0;
	if (hanConvTable[res].key == c) {
		dest = hanConvTable[res].hanChar;
		flags = hanConvTable[res].flags;
	}

	return flags;
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
