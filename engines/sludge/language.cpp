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
#include "allfiles.h"
#include "stringy.h"
#include "newfatal.h"
#include "moreio.h"
#include "language.h"
#include "CommonCode/version.h"
#include "platform-dependent.h"

#include "sludge.h"

#include "common/debug.h"

namespace Sludge {

int *languageTable;
char **languageName;
settingsStruct gameSettings;

unsigned int stringToInt(char *s) {
	int i = 0;
	bool negative = false;
	for (;;) {
		if (*s >= '0' && *s <= '9') {
			i *= 10;
			i += *s - '0';
			s++;
		} else if (*s == '-') {
			negative = !negative;
			s++;
		} else {
			if (negative)
				return -i;
			return i;
		}
	}
}

char *getPrefsFilename(char *filename) {
	// Yes, this trashes the original string, but
	// we also free it at the end (warning!)...

	int n, i;

	n = strlen(filename);

	if (n > 4 && filename[n - 4] == '.') {
		filename[n - 4] = 0;
	}

	char *f = filename;
	for (i = 0; i < n; i++) {
#ifdef _WIN32
		if (filename[i] == '\\')
#else
		if (filename[i] == '/')
#endif
			f = filename + i + 1;
	}

	char *joined = joinStrings(f, ".ini");

	delete filename;
	filename = NULL;
	return joined;
}

void readIniFile(char *filename) {

	char *langName = getPrefsFilename(copyString(filename));

	Common::File fd;
	if (!fd.open(langName)) {
		debug(kSludgeDebugDataLoad, "Fail to open language file : %s",
				langName);
		return;
	}

	gameSettings.languageID = 0;
	gameSettings.userFullScreen = false; //defaultUserFullScreen(); TODO: false value
	gameSettings.refreshRate = 0;
	gameSettings.antiAlias = 1;
	gameSettings.fixedPixels = false;
	gameSettings.noStartWindow = false;
	gameSettings.debugMode = false;

	delete langName;
	langName = NULL;

	char lineSoFar[257] = "";
	char secondSoFar[257] = "";
	unsigned char here = 0;
	char readChar = ' ';
	bool keepGoing = true;
	bool doingSecond = false;

	do {
		readChar = fd.readByte();
		if (fd.eos()) {
			readChar = '\n';
			keepGoing = false;
		}
		switch (readChar) {
		case '\n':
		case '\r':
			if (doingSecond) {
				if (strcmp(lineSoFar, "LANGUAGE") == 0) {
					gameSettings.languageID = stringToInt(secondSoFar);
				} else if (strcmp(lineSoFar, "WINDOW") == 0) {
					gameSettings.userFullScreen = !stringToInt(secondSoFar);
				} else if (strcmp(lineSoFar, "REFRESH") == 0) {
					gameSettings.refreshRate = stringToInt(secondSoFar);
				} else if (strcmp(lineSoFar, "ANTIALIAS") == 0) {
					gameSettings.antiAlias = stringToInt(secondSoFar);
				} else if (strcmp(lineSoFar, "FIXEDPIXELS") == 0) {
					gameSettings.fixedPixels = stringToInt(secondSoFar);
				} else if (strcmp(lineSoFar, "NOSTARTWINDOW") == 0) {
					gameSettings.noStartWindow = stringToInt(secondSoFar);
				} else if (strcmp(lineSoFar, "DEBUGMODE") == 0) {
					gameSettings.debugMode = stringToInt(secondSoFar);
				}
			}
			here = 0;
			doingSecond = false;
			lineSoFar[0] = 0;
			secondSoFar[0] = 0;
			break;

		case '=':
			doingSecond = true;
			here = 0;
			break;

		default:
			if (doingSecond) {
				secondSoFar[here++] = readChar;
				secondSoFar[here] = 0;
			} else {
				lineSoFar[here++] = readChar;
				lineSoFar[here] = 0;
			}
			break;
		}
	} while (keepGoing);

	fd.close();
}

void saveIniFile(char *filename) {
#if 0
	char *langName = getPrefsFilename(copyString(filename));
	FILE *fp = fopen(langName, "wt");
	delete langName;

	fprintf(fp, "LANGUAGE=%d\n", gameSettings.languageID);
	fprintf(fp, "WINDOW=%d\n", ! gameSettings.userFullScreen);
	fprintf(fp, "ANTIALIAS=%d\n", gameSettings.antiAlias);
	fprintf(fp, "FIXEDPIXELS=%d\n", gameSettings.fixedPixels);
	fprintf(fp, "NOSTARTWINDOW=%d\n", gameSettings.noStartWindow);
	fprintf(fp, "DEBUGMODE=%d\n", gameSettings.debugMode);

	fclose(fp);
#endif
}

void makeLanguageTable(Common::File *table) {
	languageTable = new int[gameSettings.numLanguages + 1];
	if (!checkNew(languageTable))
		return;

	languageName = new char *[gameSettings.numLanguages + 1];
	if (!checkNew(languageName))
		return;

	for (unsigned int i = 0; i <= gameSettings.numLanguages; i++) {
		languageTable[i] = i ? table->readUint16BE() : 0;
		debug(kSludgeDebugDataLoad, "languageTable %i: %i", i,
				languageTable[i]);
		languageName[i] = 0;
		if (gameVersion >= VERSION(2, 0)) {
			if (gameSettings.numLanguages) {
				languageName[i] = readString(table);
				debug(kSludgeDebugDataLoad, "languageName %i: %s\n", i,
						languageName[i]);
			}
		}
	}
}

int getLanguageForFileB() {
	int indexNum = -1;

	for (unsigned int i = 0; i <= gameSettings.numLanguages; i++) {
		if (languageTable[i] == gameSettings.languageID)
			indexNum = i;
	}

	return indexNum;
}

} // End of namespace Sludge
