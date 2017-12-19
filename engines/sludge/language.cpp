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
#include "sludge/fileset.h"
#include "sludge/language.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/version.h"

namespace Sludge {

LanguageManager::LanguageManager() {
	init();
}

LanguageManager::~LanguageManager() {
	kill();
}

void LanguageManager::init() {
	_languageID = 0;
	_languageIdx = -1;
	_numLanguages = 0;
	_languageTable = nullptr;
	_languageNames = nullptr;
}

void LanguageManager::kill() {
	if (_languageTable) {
		delete []_languageTable;
		_languageTable = nullptr;
	}

	if (_languageNames) {
		delete []_languageNames;
		_languageNames = nullptr;
	}
}

void LanguageManager::createTable(Common::File *fp) {
	// get number of languages
	_numLanguages =
				(gameVersion >= VERSION(1, 3)) ? (fp->readByte()) : 0;
	debugC(2, kSludgeDebugDataLoad, "numLanguages : %c", _numLanguages);

	// make language table
	_languageTable = new uint[_numLanguages + 1];
	if (!checkNew(_languageTable))
		return;

	_languageNames = new Common::String[_numLanguages + 1];
	if (!checkNew(_languageNames))
		return;

	for (uint i = 0; i <= _numLanguages; i++) {
		_languageTable[i] = i ? fp->readUint16BE() : 0;
		debugC(2, kSludgeDebugDataLoad, "languageTable %i: %i", i, _languageTable[i]);
		_languageNames[i].clear();
		if (gameVersion >= VERSION(2, 0)) {
			if (_numLanguages) {
				_languageNames[i] = readString(fp);
				debugC(2, kSludgeDebugDataLoad, "languageName %i: %s\n", i, _languageNames[i].c_str());
			}
		}
	}
}

void LanguageManager::setLanguageID(uint id) {
	_languageID = id;
	// get index of language
	setLanguageIndex(getLanguageForFileB());
}

int LanguageManager::getLanguageForFileB() {
	int indexNum = -1;
	for (uint i = 0; i <= _numLanguages; i++) {
		if (_languageTable[i] == _languageID)
			indexNum = i;
	}
	return indexNum;
}

void LanguageManager::saveLanguageSetting(Common::WriteStream *writeStream) {
	writeStream->writeByte(_numLanguages);
}

void LanguageManager::loadLanguageSetting(Common::SeekableReadStream *readStream) {
	uint languageIdx = readStream->readByte();
	setLanguageIndex(languageIdx);
}

void LanguageManager::setLanguageIndex(int idx) {
	if (idx < 0)
		fatal("Can't find the translation data specified!");

	if (idx != _languageIdx) {
		// Load the saved language!
		_languageIdx = idx;
		// Now set file indices properly to the chosen language.
		g_sludge->_resMan->setFileIndices(_numLanguages, _languageIdx);
	}
}

} // End of namespace Sludge
