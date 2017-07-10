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
#if 0
#include <errno.h>
#include <stdint.h>
#include <string.h>
// For unicode conversion
#include <iconv.h>
#endif

#include "debug.h"
#include "stringy.h"
#include "allfiles.h"
#include "moreio.h"
#include "newfatal.h"
#include "CommonCode/version.h"

#include "common/file.h"

namespace Sludge {

bool sliceBusy = true;

Common::File *bigDataFile = NULL;

uint32_t startOfDataIndex, startOfTextIndex, startOfSubIndex,
		startOfObjectIndex;

bool openSubSlice(int num) {
//	FILE * dbug = fopen ("debuggy.txt", "at");

//	fprintf (dbug, "\nTrying to open sub %i\n", num);

	if (sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return false;
	}
//	fprintf (dbug, "Going to position %li\n", startOfSubIndex + (num << 2));
	bigDataFile->seek(startOfSubIndex + (num << 2), 0);
	bigDataFile->seek(get4bytes(bigDataFile), 0);
//	fprintf (dbug, "Told to skip forward to %li\n", ftell (bigDataFile));
//	fclose (dbug);

	return sliceBusy = true;
}

bool openObjectSlice(int num) {
//	FILE * dbug = fopen ("debuggy.txt", "at");

//	fprintf (dbug, "\nTrying to open object %i\n", num);

	if (sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return false;
	}

//	fprintf (dbug, "Going to position %li\n", startOfObjectIndex + (num << 2));
	bigDataFile->seek(startOfObjectIndex + (num << 2), 0);
	bigDataFile->seek(get4bytes(bigDataFile), 0);
//	fprintf (dbug, "Told to skip forward to %li\n", ftell (bigDataFile));
//	fclose (dbug);
	return sliceBusy = true;
}

unsigned int openFileFromNum(int num) {
//	FILE * dbug = fopen ("debuggy.txt", "at");

	if (sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return 0;
	}

//	fprintf (dbug, "\nTrying to open file %i\n", num);
//	fprintf (dbug, "Jumping to %li (for index) \n", startOfDataIndex + (num << 2));
	bigDataFile->seek(startOfDataIndex + (num << 2), 0);
	bigDataFile->seek(get4bytes(bigDataFile), 1);
//	fprintf (dbug, "Jumping to %li (for data) \n", ftell (bigDataFile));
	sliceBusy = true;
//	fclose (dbug);

	return get4bytes(bigDataFile);
}

// Converts a string from Windows CP-1252 to UTF-8.
// This is needed for old games.
char *convertString(char *s) {
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

	size_t len1 = strlen(s) + 1;
	size_t len2 = 65535;
	size_t iconv_value =
#ifdef _WIN32
	iconv(convert, (const char **) tmp1, &len1, tmp2, &len2);
#else
	iconv(convert, (char **) tmp1, &len1, tmp2, &len2);
#endif

	if (iconv_value == (size_t) - 1) {
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

char *getNumberedString(int value) {

	if (sliceBusy) {
		fatal("Can't read from data file", "I'm already reading something");
		return NULL;
	}

	bigDataFile->seek((value << 2) + startOfTextIndex, 0);
	value = get4bytes(bigDataFile);
	bigDataFile->seek(value, 0);

	char *s = readString(bigDataFile);

	if (gameVersion < VERSION(2, 2)) {
		// This is an older game - We need to convert the string to UTF-8
		s = convertString(s);
	}

	return s;
}

bool startAccess() {
	int wasBusy = sliceBusy;
	sliceBusy = true;
	return wasBusy;
}
void finishAccess() {
	sliceBusy = false;
}

int32_t startIndex;

void setFileIndices(Common::File *fp, int numLanguages,
		unsigned int skipBefore) {
	if (fp) {
		// Keep hold of the file handle, and let things get at it
		bigDataFile = fp;
		startIndex = fp->pos();
	} else {
		// No file pointer - this means that we reuse the bigDataFile
		fp = bigDataFile;
		fp->seek(startIndex, SEEK_SET);
	}
	sliceBusy = false;

	if (skipBefore > numLanguages) {
		warning("Not a valid language ID! Using default instead.");
		skipBefore = 0;
	}

	// STRINGS
	int skipAfter = numLanguages - skipBefore;
	while (skipBefore) {
		fp->seek(get4bytes(fp), SEEK_SET);
		skipBefore--;
	}
	startOfTextIndex = fp->pos() + 4;

	fp->seek(get4bytes(fp), SEEK_SET);

	while (skipAfter) {
		fp->seek(get4bytes(fp), SEEK_SET);
		skipAfter--;
	}

	startOfSubIndex = fp->pos() + 4;
	fp->seek(get4bytes(fp), SEEK_CUR);

	startOfObjectIndex = fp->pos() + 4;
	fp->seek(get4bytes(fp), SEEK_CUR);

	// Remember that the data section starts here
	startOfDataIndex = fp->pos();
}

} // End of namespace Sludge
