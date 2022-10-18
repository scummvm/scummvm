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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_string_table.h"
#include "common/str.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseStringTable::BaseStringTable(BaseGame *inGame) : BaseClass(inGame) {

}


//////////////////////////////////////////////////////////////////////////
BaseStringTable::~BaseStringTable() {
	// delete strings
	_strings.clear();

}


//////////////////////////////////////////////////////////////////////////
bool BaseStringTable::addString(const char *key, const char *val, bool reportDuplicities) {
	if (key == nullptr || val == nullptr) {
		return STATUS_FAILED;
	}

	if (scumm_stricmp(key, "@right-to-left") == 0) {
		_gameRef->_textRTL = true;
		return STATUS_OK;
	}

	Common::String finalKey = key;
	finalKey.toLowercase();

	StringsIter it = _strings.find(finalKey);
	if (it != _strings.end() && reportDuplicities) {
		BaseEngine::LOG(0, "  Warning: Duplicate definition of string '%s'.", finalKey.c_str());
	}

	_strings[finalKey] = val;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
char *BaseStringTable::getKey(const char *str) const {
	if (str == nullptr || str[0] != '/') {
		return nullptr;
	}

	const char *value = strchr(str + 1, '/');
	if (value == nullptr) {
		return nullptr;
	}

	char *key = new char[value - str];
	Common::strlcpy(key, str + 1, (size_t)(value - str));

	BasePlatform::strlwr(key);

	char *newStr;

	StringsIter it = _strings.find(key);
	if (it != _strings.end()) {
		newStr = new char[it->_value.size() + 1];
		strcpy(newStr, it->_value.c_str());
		if (strlen(newStr) > 0 && newStr[0] == '/' && strchr(newStr + 1, '/')) {
			delete[] key;
			char *ret = getKey(newStr);
			delete[] newStr;
			return ret;
		} else {
			delete[] newStr;
			return key;
		}
	} else {
		return key;
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseStringTable::expand(char **str) const {
	if (str == nullptr || *str == nullptr || *str[0] != '/') {
		return;
	}

	char *value = strchr(*str + 1, '/');
	if (value == nullptr) {
		return;
	}

	char *key = new char[value - *str];
	Common::strlcpy(key, *str + 1, (size_t)(value - *str));

	BasePlatform::strlwr(key);

	value++;

	char *newStr;

	StringsIter it = _strings.find(key);
	if (it != _strings.end()) {
		newStr = new char[it->_value.size() + 1];
		strcpy(newStr, it->_value.c_str());
	} else {
		newStr = new char[strlen(value) + 1];
		strcpy(newStr, value);
	}

	delete[] key;
	delete[] *str;
	*str = newStr;

	if (strlen(*str) > 0 && *str[0] == '/') {
		expand(str);
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseStringTable::expand(Common::String &str) const {
	char *tmp = new char[str.size() + 1];
	strcpy(tmp, str.c_str());
	expand(&tmp);
	str = tmp;
	delete[] tmp;
}


//////////////////////////////////////////////////////////////////////////
const char *BaseStringTable::expandStatic(const char *string) const {
	if (string == nullptr || string[0] == '\0' || string[0] != '/') {
		return string;
	}

	const char *value = strchr(string + 1, '/');
	if (value == nullptr) {
		return string;
	}

	char *key = new char[value - string];
	Common::strlcpy(key, string + 1, (size_t)(value - string - 1));
	BasePlatform::strlwr(key);

	value++;

	const char *newStr;

	StringsIter it = _strings.find(key);
	if (it != _strings.end()) {
		newStr = it->_value.c_str();
	} else {
		newStr = value;
	}

	delete[] key;

	if (strlen(newStr) > 0 && newStr[0] == '/') {
		return expandStatic(newStr);
	} else {
		return newStr;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseStringTable::loadFile(const char *filename, bool clearOld) {
	BaseEngine::LOG(0, "Loading string table...");

	if (clearOld) {
		_filenames.clear();
		_strings.clear();
	}
	_filenames.push_back(Common::String(filename));

	uint32 size;
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &size);
	if (buffer == nullptr) {
		BaseEngine::LOG(0, "BaseStringTable::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	uint32 pos = 0;

	if (size > 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
		pos += 3;
		if (_gameRef->_textEncoding != TEXT_UTF8) {
			_gameRef->_textEncoding = TEXT_UTF8;
			//_gameRef->_textEncoding = TEXT_ANSI;
			BaseEngine::LOG(0, "  UTF8 file detected, switching to UTF8 text encoding");
		}
	} else {
		_gameRef->_textEncoding = TEXT_ANSI;
	}

	uint32 lineLength = 0;
	while (pos < size) {
		lineLength = 0;
		while (pos + lineLength < size && buffer[pos + lineLength] != '\n' && buffer[pos + lineLength] != '\0') {
			lineLength++;
		}

		uint32 realLength = lineLength - (pos + lineLength >= size ? 0 : 1);
		char *line = new char[realLength + 1];
		Common::strlcpy(line, (char *)&buffer[pos], realLength + 1);
		char *value = strchr(line, '\t');
		if (value == nullptr) {
			value = strchr(line, ' ');
		}

		if (line[0] != ';') {
			if (value != nullptr) {
				value[0] = '\0';
				value++;
				for (uint32 i = 0; i < strlen(value); i++) {
					if (value[i] == '|') {
						value[i] = '\n';
					}
				}
				addString(line, value, clearOld);
			} else if (line[0] != '\0') {
				addString(line, "", clearOld);
			}
		}

		delete[] line;
		pos += lineLength + 1;
	}

	delete[] buffer;

	BaseEngine::LOG(0, "  %d strings loaded", _strings.size());

	return STATUS_OK;
}

bool BaseStringTable::persist(BasePersistenceManager *persistMgr) {
	// Do nothing if the save game is too old.
	if (!persistMgr->checkVersion(1, 3, 1)) {
		return true;
	}
	uint32 numFiles = _filenames.size();
	persistMgr->transferUint32("NumFiles", &numFiles);
	if (persistMgr->getIsSaving()) {
		for (uint i = 0; i < numFiles; i++) {
			persistMgr->transferString("Filename", &_filenames[i]);
		}
	} else {
		_strings.clear();
		_filenames.clear();
		for (uint i = 0; i < numFiles; i++) {
			Common::String filename = "";
			persistMgr->transferString("Filename", &filename);
			loadFile(filename.c_str(), false);
		}
	}
	return true;
}

} // End of namespace Wintermute
