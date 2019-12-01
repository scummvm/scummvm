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

#include "common/file.h"
#include "common/debug.h"
#include "common/ustr.h"

#include "sludge/allfiles.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/version.h"

namespace Sludge {

ResourceManager::ResourceManager() {
	init();
}

ResourceManager::~ResourceManager() {
	kill();
}

void ResourceManager::init() {
	_sliceBusy = true;
	_bigDataFile = nullptr;
	_startOfDataIndex = 0;
	_startOfTextIndex = 0;
	_startOfSubIndex = 0;
	_startOfObjectIndex = 0;
	_startIndex = 0;
	_allResourceNames.clear();
}
void ResourceManager::kill() {
	if (_bigDataFile) {
		delete _bigDataFile;
		_bigDataFile = nullptr;
	}
	_allResourceNames.clear();
}

bool ResourceManager::openSubSlice(int num) {
	if (_sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return false;
	}
	_bigDataFile->seek(_startOfSubIndex + (num << 2), 0);
	_bigDataFile->seek(_bigDataFile->readUint32LE(), 0);

	return _sliceBusy = true;
}

bool ResourceManager::openObjectSlice(int num) {
	if (_sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return false;
	}

	_bigDataFile->seek(_startOfObjectIndex + (num << 2), 0);
	_bigDataFile->seek(_bigDataFile->readUint32LE(), 0);
	return _sliceBusy = true;
}

uint ResourceManager::openFileFromNum(int num) {
	if (_sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return 0;
	}

	_bigDataFile->seek(_startOfDataIndex + (num << 2), 0);
	_bigDataFile->seek(_bigDataFile->readUint32LE(), 1);
	_sliceBusy = true;

	return _bigDataFile->readUint32LE();
}

uint32 ResourceManager::_cp1250ToUTF32[128] = {
  /* 0x80 */
  0x20ac, 0xfffd, 0x201a, 0xfffd, 0x201e, 0x2026, 0x2020, 0x2021,
  0xfffd, 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
  /* 0x90 */
  0xfffd, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0xfffd, 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
  /* 0xa0 */
  0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7,
  0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b,
  /* 0xb0 */
  0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
  0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
  /* 0xc0 */
  0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
  0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
  /* 0xd0 */
  0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
  0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
  /* 0xe0 */
  0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
  0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
  /* 0xf0 */
  0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
  0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
};

// Converts a string from ISO8859-2 or CP1250 to UTF8.
// This is needed for old games.
Common::String ResourceManager::convertString(const Common::String &s) {
	Common::String res;
	Common::U32String tmp;

	// Convert CP1250 to UTF32
	for (uint i = 0; i < s.size(); ++i) {
		const byte c = s[i];
		if (c < 0x80) {
			tmp += c;
		} else {
			uint32 utf32 = _cp1250ToUTF32[c - 0x80];
			if (utf32) {
				tmp += utf32;
			} else {
				// It's an invalid CP1250 character...
				return s;
			}
		}
	}

	// Convert UTF32 to UTF8
	for (uint i = 0; i < tmp.size(); ++i) {
		uint32 wc = tmp[i];
		int count;
		if (wc < 0x80)
			count = 1;
		else if (wc < 0x800)
			count = 2;
		else if (wc < 0x10000) {
			if (wc < 0xd800 || wc >= 0xe000) {
				count = 3;
			} else {
				// It's an invalid UTF32 character...
				return s;
			}
		} else if (wc < 0x110000) {
			count = 4;
		} else {
			// It's an invalid UTF32 character...
			return s;
		}
		Common::String r = "";
		switch (count) {
			case 4:
				r = (char)(0x80 | (wc & 0x3f)) + r;
				wc = wc >> 6;
				wc |= 0x10000;
				// falls through
			case 3:
				r = (char)(0x80 | (wc & 0x3f)) + r;
				wc = wc >> 6;
				wc |= 0x800;
				// falls through
			case 2:
				r = (char)(0x80 | (wc & 0x3f)) + r;
				wc = wc >> 6;
				wc |= 0xc0;
				// falls through
			case 1:
				r = wc + r;
				break;
			default:
				break;
		}
		res += r;
	}

	return res;
}

Common::String ResourceManager::getNumberedString(int value) {
	if (_sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return NULL;
	}

	_bigDataFile->seek((value << 2) + _startOfTextIndex, 0);
	value = _bigDataFile->readUint32LE();
	_bigDataFile->seek(value, 0);

	Common::String s = readString(_bigDataFile);

	if (gameVersion < VERSION(2, 2)) {
		// This is an older game - We need to convert the string to UTF-8
		s = convertString(s);
	}

	return s;
}

bool ResourceManager::startAccess() {
	int wasBusy = _sliceBusy;
	_sliceBusy = true;
	return wasBusy;
}
void ResourceManager::finishAccess() {
	_sliceBusy = false;
}

void ResourceManager::readResourceNames(Common::SeekableReadStream *readStream) {
	int numResourceNames = readStream->readUint16BE();
	debugC(2, kSludgeDebugDataLoad, "numResourceNames %i", numResourceNames);
	_allResourceNames.reserve(numResourceNames);

	for (int fn = 0; fn < numResourceNames; fn++) {
		_allResourceNames.push_back(readString(readStream));
		debugC(2, kSludgeDebugDataLoad, "Resource %i: %s", fn, _allResourceNames[fn].c_str());
	}
}

const Common::String ResourceManager::resourceNameFromNum(int i) {
	if (i == -1)
		return "";

	if (_allResourceNames.empty())
		return "RESOURCE";

	if (i < (int)_allResourceNames.size())
		return _allResourceNames[i];

	return "Unknown resource";
}

void ResourceManager::setData(Common::File *fp) {
	_bigDataFile = fp;
	_startIndex = fp->pos();
}

void ResourceManager::setFileIndices(uint numLanguages, uint skipBefore) {
	_bigDataFile->seek(_startIndex, SEEK_SET);
	_sliceBusy = false;

	if (skipBefore > numLanguages) {
		warning("Not a valid language ID! Using default instead.");
		skipBefore = 0;
	}

	// STRINGS
	int skipAfter = numLanguages - skipBefore;
	while (skipBefore) {
		_bigDataFile->seek(_bigDataFile->readUint32LE(), SEEK_SET);
		skipBefore--;
	}
	_startOfTextIndex = _bigDataFile->pos() + 4;
	debugC(2, kSludgeDebugDataLoad, "startOfTextIndex: %i", _startOfTextIndex);

	_bigDataFile->seek(_bigDataFile->readUint32LE(), SEEK_SET);

	while (skipAfter) {
		_bigDataFile->seek(_bigDataFile->readUint32LE(), SEEK_SET);
		skipAfter--;
	}

	_startOfSubIndex = _bigDataFile->pos() + 4;
	_bigDataFile->seek(_bigDataFile->readUint32LE(), SEEK_CUR);
	debugC(2, kSludgeDebugDataLoad, "startOfSubIndex: %i", _startOfSubIndex);

	_startOfObjectIndex = _bigDataFile->pos() + 4;
	_bigDataFile->seek(_bigDataFile->readUint32LE(), SEEK_CUR);
	debugC(2, kSludgeDebugDataLoad, "startOfObjectIndex: %i", _startOfObjectIndex);

	// Remember that the data section starts here
	_startOfDataIndex = _bigDataFile->pos();
	debugC(2, kSludgeDebugDataLoad, "startOfDataIndex: %i", _startOfDataIndex);
}

} // End of namespace Sludge
