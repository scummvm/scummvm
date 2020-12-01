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

#include "common/str.h"
#include "common/ustr.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/error.h"
#include "common/system.h"
#include "common/enc-internal.h"
#include "common/file.h"

namespace Common {

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 2. Original version returns a result code. This version does NOT
//    insert 'FFFD' on errors & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
void U32String::decodeUTF8(const char *src, uint32 len) {
	ensureCapacity(len, false);

	// The String class, and therefore the Font class as well, assume one
	// character is one byte, but in this case it's actually an UTF-8
	// string with up to 4 bytes per character. To work around this,
	// convert it to an U32String before drawing it, because our Font class
	// can handle that.
	for (uint i = 0; i < len;) {
		uint32 chr = 0;
		uint num = 1;

		if ((src[i] & 0xF8) == 0xF0) {
			num = 4;
		} else if ((src[i] & 0xF0) == 0xE0) {
			num = 3;
		} else if ((src[i] & 0xE0) == 0xC0) {
			num = 2;
		}

		if (len - i >= num) {
			switch (num) {
			case 4:
				chr |= (src[i++] & 0x07) << 18;
				chr |= (src[i++] & 0x3F) << 12;
				chr |= (src[i++] & 0x3F) << 6;
				chr |= (src[i++] & 0x3F);
				break;

			case 3:
				chr |= (src[i++] & 0x0F) << 12;
				chr |= (src[i++] & 0x3F) << 6;
				chr |= (src[i++] & 0x3F);
				break;

			case 2:
				chr |= (src[i++] & 0x1F) << 6;
				chr |= (src[i++] & 0x3F);
				break;

			default:
				chr = (src[i++] & 0x7F);
				break;
			}
		} else {
			break;
		}

		operator+=(chr);
	}
}

const uint16 invalidCode = 0xFFFD;

static bool cjk_tables_loaded = false;
static const uint16 *windows932ConversionTable;
static const uint16 *windows949ConversionTable;
static const uint16 *windows950ConversionTable;

static const uint16 *loadCJKTable(File &f, int idx, size_t sz) {
	f.seek(16 + idx * 4);
	uint32 off = f.readUint32LE();
	f.seek(off);
	uint16 *res = new uint16[sz];
	f.read(res, 2 * sz);
#ifndef SCUMM_LITTLE_ENDIAN
	for (uint i = 0; i < sz; i++)
		res[i] = FROM_LE_16(res[i]);
#endif
	return res;
}

static void loadCJKTables() {
	File f;

	cjk_tables_loaded = true;

	if (!f.open("encoding.dat")) {
		warning("encoding.dat is not found. Support for CJK is disabled");
		return;
	}

	if (f.size() < 16 + 3 * 4) {
		warning("encoding.dat is invalid. Support for CJK is disabled");
		return;
	}

	if (f.readUint32BE() != MKTAG('S', 'C', 'V', 'M')
	    || f.readUint32BE() != MKTAG('E', 'N', 'C', 'D')) {
		warning("encoding.dat is invalid. Support for CJK is disabled");
		return;
	}

	// Version and number of tables.
	if (f.readUint32LE() != 0 || f.readUint32LE() < 3) {
		warning("encoding.dat is of incompatible version. Support for CJK is disabled");
		return;
	}

	windows932ConversionTable = loadCJKTable(f, 0, 47 * 192);
	windows949ConversionTable = loadCJKTable(f, 1, 0x7e * 0xb2);
	windows950ConversionTable = loadCJKTable(f, 2, 89 * 157);
}

void U32String::decodeWindows932(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		// Katakana
		if (high >= 0xa1 && high <= 0xdf) {
			operator+=(high - 0xa1 + 0xFF61);
			continue;
		}

		if (i >= len) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		if (low < 0x40) {
			operator+=(invalidCode);
			continue;
		}
		uint8 lowidx = low - 0x40;
		uint8 highidx;

		if (high >= 0x81 && high <= 0x84)
			highidx = high - 0x81;
		else if (high >= 0x87 && high <= 0x9f)
			highidx = high - 0x87 + 4;
		else if (high >= 0xe0 && high <= 0xee)
			highidx = high - 0xe0 + 29;
		else {
			operator+=(invalidCode);
			continue;
		}

		if (!windows932ConversionTable) {
			operator+=(invalidCode);
			continue;
		}

		// Main range
		uint16 val = windows932ConversionTable[highidx * 192 + lowidx];
		operator+=(val ? val : invalidCode);
	}
}

static uint16 convertUHCToUCSReal(uint8 high, uint8 low) {
	uint lowidx = 0;
	if (low >= 0x41 && low <= 0x5a)
		lowidx = low - 0x41;
	else if (low >= 0x61 && low <= 0x7a)
		lowidx = low - 0x61 + 0x1a;
	else if (low >= 0x81 && low <= 0xfe)
		lowidx = low - 0x81 + 0x1a * 2;
	else
		return 0;
	if (!windows949ConversionTable)
		return 0;
	uint16 idx = (high - 0x81) * 0xb2 + lowidx;
	return windows949ConversionTable[idx];
}

uint16 convertUHCToUCS(uint8 high, uint8 low) {
	if (!cjk_tables_loaded)
		loadCJKTables();

	return convertUHCToUCSReal(high, low);
}


void U32String::decodeWindows949(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		if (high == 0x80 || high == 0xff) {
			operator+=(invalidCode);
			continue;
		}

		if (i >= len) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		uint16 val = convertUHCToUCSReal(high, low);

		operator+=(val ? val : invalidCode);
	}
}

void U32String::decodeWindows950(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		// Euro symbol
		if (high == 0x80) {
			operator+=(0x20ac);
			continue;
		}

		if (high == 0xff) {
			operator+=(invalidCode);
			continue;
		}

		if (i >= len) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		uint8 lowidx = low < 0x80 ? low - 0x40 : low - 0x62;

		// Main range
		if (high >= 0xa1 && high < 0xfa) {
			uint16 val = windows950ConversionTable ?
				windows950ConversionTable[(high - 0xa1) * 157 + lowidx] : 0;
			operator+=(val ? val : invalidCode);
			continue;
		}

		// PUA range
		if (high <= 0x8d) {
			operator+=(0xeeb8 + 157 * (high-0x81) + lowidx);
			continue;
		}
		if (high <= 0xa0) {
			operator+=(0xe311 + (157 * (high-0x8e)) + lowidx);
			continue;
		}
		if (high >= 0xfa) {
			operator+=(0xe000 + (157 * (high-0xfa)) + lowidx);
			continue;
		}
	}
}

void String::encodeWindows932(const U32String &src) {
	static uint16 *reverseTable;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!reverseTable && windows932ConversionTable) {
		uint16 *rt = new uint16[0x10000];
		memset(rt, 0, sizeof(rt[0]) * 0x10000);
		for (uint highidx = 0; highidx < 58; highidx++)
			for (uint lowidx = 0; lowidx < 192; lowidx++) {
				uint8 high = 0;
				uint8 low = lowidx + 0x40;
				uint16 unicode = windows932ConversionTable[highidx * 192 + lowidx];

				if (highidx < 4)
					high = highidx + 0x81;
				else if (highidx < 29)
					high = highidx + 0x87 - 4;
				else
					high = highidx + 0xe0 - 29;

				rt[unicode] = (high << 8) | low;
			}

		reverseTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		// Katakana
		if (point >= 0xff61 && point <= 0xff9f) {
			operator+=(0xa1 + (point - 0xFF61));
			continue;
		}

		if (point > 0x10000) {
			operator+=('?');
			continue;
		}

		if (!reverseTable) {
			operator+=('?');
			continue;
		}

		uint16 rev = reverseTable[point];
		if (rev != 0) {
			operator+=(rev >> 8);
			operator+=(rev & 0xff);
			continue;
		}

		// This codepage contains cyrillic, so no need to transliterate

		operator+=('?');
		continue;
	}
}

void String::encodeWindows949(const U32String &src) {
	static const uint16 *reverseTable;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!reverseTable && windows949ConversionTable) {
		uint16 *rt = new uint16[0x10000];
		memset(rt, 0, sizeof(rt[0]) * 0x10000);

		for (uint highidx = 0; highidx < 0x7e; highidx++)
			for (uint lowidx = 0; lowidx < 0xb2; lowidx++) {
				uint8 high = highidx + 0x81;
				uint8 low = 0;
				uint16 unicode = windows949ConversionTable[highidx * 0xb2 + lowidx];

				if (lowidx < 0x1a)
					low = 0x41 + lowidx;
				else if (lowidx < 0x1a * 2)
					low = 0x61 + lowidx - 0x1a;
				else
					low = 0x81 + lowidx - 0x1a * 2;
				rt[unicode] = (high << 8) | low;
			}

		reverseTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		if (point > 0x10000 || !reverseTable) {
			operator+=('?');
			continue;
		}

		uint16 rev = reverseTable[point];
		if (rev == 0) {
			// This codepage contains cyrillic, so no need to transliterate
			operator+=('?');
			continue;
		}

		operator+=(rev >> 8);
		operator+=(rev & 0xff);
	}
}

static const char g_cyrillicTransliterationTable[] = {
	' ', 'E', 'D', 'G', 'E', 'Z', 'I', 'I', 'J', 'L', 'N', 'C', 'K', 'I', 'U', 'D',
	'A', 'B', 'V', 'G', 'D', 'E', 'Z', 'Z', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'R', 'S', 'T', 'U', 'F', 'H', 'C', 'C', 'S', 'S', '\"', 'Y', '\'', 'E', 'U', 'A',
	'a', 'b', 'v', 'g', 'd', 'e', 'z', 'z', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'r', 's', 't', 'u', 'f', 'h', 'c', 'c', 's', 's', '\"', 'y', '\'', 'e', 'u', 'a',
	'e', 'e', 'd', 'g', 'e', 'z', 'i', 'i', 'j', 'l', 'n', 'c', 'k', 'i', 'u', 'd',
};

void String::translitChar(U32String::value_type point) {
	if (point == 0xa0) {
		operator+=(' ');
		return;
	}

	if (point == 0xad) {
		operator+=('-');
		return;
	}

	if (point == 0x2116) {
		operator+=('N');
		return;
	}

	if (point >= 0x401 && point <= 0x45f) {
		operator+=(g_cyrillicTransliterationTable[point - 0x400]);
		return;
	}

	operator+=('?');
}

void String::encodeWindows950(const U32String &src, bool transliterate) {
	static uint16 *reverseTable;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!reverseTable && windows950ConversionTable) {
		uint16 *rt = new uint16[0x10000];
		memset(rt, 0, sizeof(rt[0]) * 0x10000);

		for (uint highidx = 0; highidx < 90; highidx++)
			for (uint lowidx = 0; lowidx < 157; lowidx++) {
				uint8 high = highidx + 0xa1;
				uint8 low = 0;
				uint16 unicode = windows950ConversionTable[highidx * 157 + lowidx];

				if (lowidx <= 0x3e)
					low = 0x40 + lowidx;
				else
					low = 0x62 + lowidx;
				rt[unicode] = (high << 8) | low;
			}

		reverseTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		if (point > 0x10000) {
			operator+=('?');
			continue;
		}

		// Euro symbol
		if (point == 0x20ac) {
			operator+=((char) 0x80);
			continue;
		}

		if (!reverseTable) {
			operator+=('?');
			continue;
		}

		uint16 rev = reverseTable[point];
		if (rev != 0) {
			operator+=(rev >> 8);
			operator+=(rev & 0xff);
			continue;
		}

		// PUA range
		if (point >= 0xe000 && point <= 0xf848) {
			byte lowidx = 0, high = 0, low = 0;
			if (point <= 0xe310) {
				high = (point - 0xe000) / 157 + 0xfa;
				lowidx = (point - 0xe000) % 157;
			} else if (point <= 0xeeb7) {
				high = (point - 0xe311) / 157 + 0x8e;
				lowidx = (point - 0xe311) % 157;
			} else if (point <= 0xf6b0) {
				high = (point - 0xeeb8) / 157 + 0x81;
				lowidx = (point - 0xeeb8) % 157;
			} else {
				high = (point - 0xf672) / 157 + 0xc6;
				lowidx = (point - 0xf672) % 157;
			}

			if (lowidx <= 0x3e)
				low = 0x40 + lowidx;
			else
				low = 0x62 + lowidx;

			operator+=(high);
			operator+=(low);
			reverseTable[point] = (high << 8) | low;
			continue;
		}

		if (transliterate) {
			translitChar(point);
			continue;
		}

		operator+=('?');
		continue;
	}
}

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 2. Original version returns a result code. This version inserts '0xFFFD' if
//    character does not fit in 4 bytes & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
void String::encodeUTF8(const U32String &src) {
	ensureCapacity(src.size(), false);
	static const uint8 firstByteMark[5] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };
	char writingBytes[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	uint i = 0;
	while (i < src.size()) {
		unsigned short bytesToWrite = 0;
		const uint32 byteMask = 0xBF;
		const uint32 byteMark = 0x80;

		uint32 ch = src[i++];
		if (ch < (uint32)0x80) {
			bytesToWrite = 1;
		} else if (ch < (uint32)0x800) {
			bytesToWrite = 2;
		} else if (ch < (uint32)0x10000) {
			bytesToWrite = 3;
		} else if (ch <= 0x0010FFFF) {
			bytesToWrite = 4;
		} else {
			bytesToWrite = 3;
			ch = invalidCode;
		}

		char *pBytes = writingBytes + (4 - bytesToWrite);

		switch (bytesToWrite) {
		case 4:
			pBytes[3] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 3:
			pBytes[2] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 2:
			pBytes[1] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 1:
			pBytes[0] = (char)(ch | firstByteMark[bytesToWrite]);
			break;
		default:
			break;
		}

		operator+=(pBytes);
	}
}

#define decodeUTF16Template(suffix, read)				\
Common::U32String U32String::decodeUTF16 ## suffix (const uint16 *start, uint len) {	\
	const uint16 *ptr = start;					\
	Common::U32String dst;						\
	dst.ensureCapacity(len, false);					\
									\
	while (len > 0) {						\
		uint16 c = read(ptr++);					\
		len--;							\
		if (c >= 0xD800 && c <= 0xDBFF && len > 0) {		\
			uint16 low = read(ptr);				\
			if (low >= 0xDC00 && low <= 0xDFFF) {		\
				/* low is OK, we can advance pointer */	\
				ptr++; len--;				\
				dst += ((c & 0x3ff) << 10)		\
					| (low & 0x3ff);		\
			} else {					\
				dst += invalidCode;			\
			}						\
			continue;					\
		}							\
									\
		if (c >= 0xD800 && c <= 0xDFFF) {			\
			dst += invalidCode;				\
			continue;					\
		}							\
		dst += c;						\
	}								\
									\
	return dst;							\
}

decodeUTF16Template(BE, READ_BE_UINT16)
decodeUTF16Template(LE, READ_LE_UINT16)
decodeUTF16Template(Native, READ_UINT16)

#define encodeUTF16Template(suffix, write)				\
uint16 *U32String::encodeUTF16 ## suffix (uint *len) const {		\
	uint16 *out = new uint16[_size * 2 + 1];			\
	uint16 *ptr = out;						\
									\
	for (uint i = 0; i < _size; i++) {				\
		uint32 c = _str[i];					\
		if (c < 0x10000) {					\
			write(ptr++, c);				\
			continue;					\
		}							\
		write (ptr++, 0xD800 | ((c >> 10) & 0x3ff));		\
		write (ptr++, 0xDC00 | (c & 0x3ff));			\
	}								\
									\
	write(ptr, 0);							\
	if (len)							\
		*len = ptr - out;					\
									\
	return out;							\
}

encodeUTF16Template(BE, WRITE_BE_UINT16)
encodeUTF16Template(LE, WRITE_LE_UINT16)
encodeUTF16Template(Native, WRITE_UINT16)

// Upper bound on unicode codepoint in any single-byte encoding. Must be divisible by 0x100 and be strictly above large codepoint
static const int kMaxCharSingleByte = 0x3000;


static const uint16 *
getConversionTable(CodePage page) {
	switch (page) {
	case kWindows1250:
		return kWindows1250ConversionTable;
	case kWindows1251:
		return kWindows1251ConversionTable;
	case kWindows1252:
		return kWindows1252ConversionTable;
	case kWindows1253:
		return kWindows1253ConversionTable;
	case kWindows1254:
		return kWindows1254ConversionTable;
	case kWindows1255:
		return kWindows1255ConversionTable;
	case kWindows1256:
		return kWindows1256ConversionTable;
	case kWindows1257:
		return kWindows1257ConversionTable;
	case kMacCentralEurope:
		return kMacCentralEuropeConversionTable;
	case kISO8859_1:
		return kLatin1ConversionTable;
	case kISO8859_2:
		return kLatin2ConversionTable;
	case kISO8859_5:
		return kISO5ConversionTable;
	case kDos850:
		return kDos850ConversionTable;
	case kDos866:
		return kDos866ConversionTable;
	case kASCII:
		return kASCIIConversionTable;

	case kCodePageInvalid:
	// Multibyte encodings. Can't be represented in simple table way
	case kUtf8:
	case kWindows932:
	case kWindows949:
	case kWindows950:
		return nullptr;
	}
	return nullptr;
}

struct ReverseTablePrefixTreeLevel1 {
	struct ReverseTablePrefixTreeLevel2 *next[kMaxCharSingleByte / 0x100];
	bool valid;
};

struct ReverseTablePrefixTreeLevel2 {
	uint8 end[256];

	ReverseTablePrefixTreeLevel2() {
		memset(end, 0, sizeof(end));
	}
};

ReverseTablePrefixTreeLevel1 reverseTables[kLastEncoding + 1];

static const ReverseTablePrefixTreeLevel1 *
getReverseConversionTable(CodePage page) {
	if (reverseTables[page].valid)
		return &reverseTables[page];
	const uint16 *conversionTable = getConversionTable(page);
	if (!conversionTable)
		return nullptr;
	reverseTables[page].valid = true;
	for (uint i = 0; i < 0x80; i++) {
		uint32 c = conversionTable[i];
		if (c == 0 || c >= kMaxCharSingleByte)
			continue;
		if (!reverseTables[page].next[c >> 8]) {
			reverseTables[page].next[c >> 8] = new ReverseTablePrefixTreeLevel2();
		}

		reverseTables[page].next[c >> 8]->end[c&0xff] = i | 0x80;
	}

	return &reverseTables[page];
}

void U32String::decodeOneByte(const char *src, uint32 len, CodePage page) {
    	const uint16 *conversionTable = getConversionTable(page);

	if (conversionTable == nullptr) {
		conversionTable = kASCIIConversionTable;
	}

	ensureCapacity(len, false);

	for (uint i = 0; i < len; ++i) {
		if ((src[i] & 0x80) == 0) {
			operator+=(src[i]);
			continue;
		}

		uint16 val = conversionTable[src[i] & 0x7f];
		operator+=(val ? val : invalidCode);
	}
}

void String::encodeOneByte(const U32String &src, CodePage page, bool transliterate) {
	const ReverseTablePrefixTreeLevel1 *conversionTable = 
		getReverseConversionTable(page);

	ensureCapacity(src.size(), false);

	if (conversionTable == nullptr) {
		for (uint i = 0; i < src.size(); ++i) {
			uint32 c = src[i];
			if (c <= 0x7F) {
				operator+=((char)c);
				continue;
			}

			if (transliterate) {
				translitChar(c);
			} else
				operator+=('?');
		}
		return;
	}

	for (uint i = 0; i < src.size(); ++i) {
		uint32 c = src[i];
		if (c <= 0x7F) {
			operator+=((char)c);
			continue;
		}

		if (c >= kMaxCharSingleByte)
			continue;
		ReverseTablePrefixTreeLevel2 *l2 = conversionTable->next[c>>8];
		unsigned char uc = l2 ? l2->end[c&0xff] : 0;
		if (uc != 0) {
			operator+=((char)uc);
			continue;
		}

		if (transliterate) {
			translitChar(c);
		} else
			operator+=('?');
	}
}

void String::encodeInternal(const U32String &src, CodePage page) {
	switch(page) {
	case kUtf8:
		encodeUTF8(src);
		break;
	case kWindows932:
		encodeWindows932(src);
		break;		
	case kWindows949:
		encodeWindows949(src);
		break;
	case kWindows950:
		encodeWindows950(src);
		break;
	default:
		encodeOneByte(src, page);
		break;
	}
}

U32String convertToU32String(const char *str, CodePage page) {
	return String(str).decode(page);
}

U32String convertUtf8ToUtf32(const String &str) {
	return str.decode(kUtf8);
}

String convertFromU32String(const U32String &string, CodePage page) {
	return string.encode(page);
}

String convertUtf32ToUtf8(const U32String &u32str) {
	return u32str.encode(kUtf8);
}

void U32String::decodeInternal(const char *str, uint32 len, CodePage page) {
	assert(str);

	_storage[0] = 0;
	_size = 0;

	switch(page) {
	case kUtf8:
		decodeUTF8(str, len);
		break;
	case kWindows932:
		decodeWindows932(str, len);
		break;
	case kWindows949:
		decodeWindows949(str, len);
		break;
	case kWindows950:
		decodeWindows950(str, len);
		break;
	default:
		decodeOneByte(str, len, page);
		break;
	}
}

U32String String::decode(CodePage page) const {
	if (page == kCodePageInvalid ||
			page > kLastEncoding) {
		error("Invalid codepage");
	}

	U32String unicodeString;
	unicodeString.decodeInternal(_str, _size, page);
	return unicodeString;
}

String U32String::encode(CodePage page) const {
	if (page == kCodePageInvalid ||
			page > kLastEncoding) {
		error("Invalid codepage");
	}

	String string;
	string.encodeInternal(*this, page);
	return string;
}

} // End of namespace Common
