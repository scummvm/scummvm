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

#include "common/debug.h"

#include "sludge/allfiles.h"
#include "sludge/newfatal.h"
#include "sludge/moreio.h"
#include "sludge/language.h"
#include "sludge/sludge.h"
#include "sludge/CommonCode/version.h"

namespace Sludge {

uint *languageTable;
Common::String *languageName;
settingsStruct gameSettings;

void makeLanguageTable(Common::File *table) {
	languageTable = new uint[gameSettings.numLanguages + 1];
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
