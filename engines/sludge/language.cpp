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

#include "sludge/allfiles.h"
#include "sludge/newfatal.h"
#include "sludge/moreio.h"
#include "sludge/language.h"
#include "sludge/sludge.h"
#include "sludge/CommonCode/version.h"

#include "common/debug.h"

namespace Sludge {

int *languageTable;
Common::String *languageName;
settingsStruct gameSettings;

Common::String getPrefsFilename(Common::String filename) {
	// Yes, this trashes the original string, but
	// we also free it at the end (warning!)...

	int n = filename.size();

	if (n > 4 && filename[n - 4] == '.') {
		filename.setChar(0, n - 4);
	}

	// get file name from dir
	int pos = 0;
	for (int i = 0; i < n; i++) {
		if (filename[i] == '/')
			pos = i + 1;
	}

	Common::String f = filename.c_str() + pos;
	Common::String joined = f + ".ini";

	return joined;
}

void readIniFile(const Common::String &filename) {

	Common::String langName = getPrefsFilename(filename);

	Common::File fd;
	if (!fd.open(langName)) {
		debug(kSludgeDebugDataLoad, "Fail to open language file : %s", langName.c_str());
		return;
	}

	gameSettings.languageID = 0;
	gameSettings.userFullScreen = false; //defaultUserFullScreen(); TODO: false value
	gameSettings.refreshRate = 0;
	gameSettings.antiAlias = 1;
	gameSettings.fixedPixels = false;
	gameSettings.noStartWindow = false;
	gameSettings.debugMode = false;

	Common::String lineSoFar = "";
	Common::String secondSoFar = "";
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
					if (lineSoFar == "LANGUAGE") {
						gameSettings.languageID = (uint)secondSoFar.asUint64();
					} else if (lineSoFar == "WINDOW") {
						gameSettings.userFullScreen = !secondSoFar.asUint64();
					} else if (lineSoFar == "REFRESH") {
						gameSettings.refreshRate = (uint)secondSoFar.asUint64();
					} else if (lineSoFar == "ANTIALIAS") {
						gameSettings.antiAlias = (int)secondSoFar.asUint64();
					} else if (lineSoFar == "FIXEDPIXELS") {
						gameSettings.fixedPixels = secondSoFar.asUint64();
					} else if (lineSoFar == "NOSTARTWINDOW") {
						gameSettings.noStartWindow = secondSoFar.asUint64();
					} else if (lineSoFar == "DEBUGMODE") {
						gameSettings.debugMode = secondSoFar.asUint64();
					}
				}
				doingSecond = false;
				lineSoFar.clear();
				secondSoFar.clear();
				break;

			case '=':
				doingSecond = true;
				break;

			default:
				if (doingSecond) {
					secondSoFar += readChar;
				} else {
					lineSoFar += readChar;
				}
				break;
		}
	} while (keepGoing);

	fd.close();
}

void saveIniFile(const Common::String &filename) {
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

	languageName = new Common::String[gameSettings.numLanguages + 1];
	if (!checkNew(languageName))
		return;

	for (uint i = 0; i <= gameSettings.numLanguages; i++) {
		languageTable[i] = i ? table->readUint16BE() : 0;
		debug(kSludgeDebugDataLoad, "languageTable %i: %i", i, languageTable[i]);
		languageName[i].clear();
		if (gameVersion >= VERSION(2, 0)) {
			if (gameSettings.numLanguages) {
				languageName[i] = readString(table);
				debug(kSludgeDebugDataLoad, "languageName %i: %s\n", i, languageName[i].c_str());
			}
		}
	}
}

int getLanguageForFileB() {
	int indexNum = -1;

	for (uint i = 0; i <= gameSettings.numLanguages; i++) {
		if (languageTable[i] == gameSettings.languageID)
			indexNum = i;
	}

	return indexNum;
}

} // End of namespace Sludge
