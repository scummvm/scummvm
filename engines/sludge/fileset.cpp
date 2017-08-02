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

#include "sludge/allfiles.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/version.h"

namespace Sludge {

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

// Converts a string from Windows CP-1252 to UTF-8.
// This is needed for old games.
Common::String ResourceManager::convertString(const Common::String &s) {
#if 0
	static char *buf = NULL;

	if (! buf) {
		buf = new char [65536];
		if (! checkNew(buf)) return NULL;
	}

	char **tmp1 = (char **) &s;
	char **tmp2 = (char **) &buf;
	char *sOrig = s;
	char *bufOrig = buf;
#if defined __unix__ && !(defined __APPLE__)
	iconv_t convert = iconv_open("UTF-8", "ISO8859-2");
#else
	iconv_t convert = iconv_open("UTF-8", "CP1250");
#endif

	if (convert == (iconv_t) - 1) {
		switch (errno) {
			case EINVAL:
			fprintf(stderr, "Error: Encoding not supported by iconv.\n");
			break;
			default:
			fprintf(stderr, "Error: Could not open iconv conversion descriptor.\n");
		}
	}

	uint len1 = strlen(s) + 1;
	uint len2 = 65535;
	uint iconv_value =
#ifdef _WIN32
	iconv(convert, (const char **) tmp1, &len1, tmp2, &len2);
#else
	iconv(convert, (char **) tmp1, &len1, tmp2, &len2);
#endif

	if (iconv_value == (uint) - 1) {
		switch (errno) {
			/* See "man 3 iconv" for an explanation. */
			case EILSEQ:
			fprintf(stderr, "Invalid multibyte sequence.\n");
			break;
			case EINVAL:
			fprintf(stderr, "Incomplete multibyte sequence.\n");
			break;
			case E2BIG:
			fprintf(stderr, "No more room.\n");
			break;
			default:
			fprintf(stderr, "Error: %s.\n", strerror(errno));
		}
		fatal("Conversion to Unicode failed. This can be fixed by recompiling the game in a current version of the SLUDGE Development Kit, but this error should never happen. Congratulations, you've found a bug in the SLUDGE engine! Please let us know about it.");
	}
	iconv_close(convert);

	delete [] sOrig;
	return copyString(buf = bufOrig);
#endif
	return s; //TODO: false value
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
