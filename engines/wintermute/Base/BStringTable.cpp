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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "common/str.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBStringTable::CBStringTable(CBGame *inGame): CBBase(inGame) {

}


//////////////////////////////////////////////////////////////////////////
CBStringTable::~CBStringTable() {
	// delete strings
	_strings.clear();

}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBStringTable::addString(const char *key, const char *val, bool reportDuplicities) {
	if (key == NULL || val == NULL) return STATUS_FAILED;

	if (scumm_stricmp(key, "@right-to-left") == 0) {
		Game->_textRTL = true;
		return STATUS_OK;
	}

	Common::String final_key = key;
	StringUtil::toLowerCase(final_key);

	_stringsIter = _strings.find(final_key);
	if (_stringsIter != _strings.end() && reportDuplicities) Game->LOG(0, "  Warning: Duplicate definition of string '%s'.", final_key.c_str());

	_strings[final_key] = val;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
char *CBStringTable::getKey(const char *str) {
	if (str == NULL || str[0] != '/') return NULL;

	const char *value = strchr(str + 1, '/');
	if (value == NULL) return NULL;

	char *key = new char[value - str];
	strncpy(key, str + 1, value - str - 1);
	key[value - str - 1] = '\0';
	CBPlatform::strlwr(key);

	char *new_str;

	_stringsIter = _strings.find(key);
	if (_stringsIter != _strings.end()) {
		new_str = new char[_stringsIter->_value.size() + 1];
		strcpy(new_str, _stringsIter->_value.c_str());
		if (strlen(new_str) > 0 && new_str[0] == '/' && strchr(new_str + 1, '/')) {
			delete [] key;
			char *Ret = getKey(new_str);
			delete [] new_str;
			return Ret;
		} else {
			delete [] new_str;
			return key;
		}
	} else {
		return key;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBStringTable::expand(char **str, bool forceExpand) {
	if (Game->_doNotExpandStrings && !forceExpand) return;

	if (str == NULL || *str == NULL || *str[0] != '/') return;

	char *value = strchr(*str + 1, '/');
	if (value == NULL) return;

	char *key = new char[value - *str];
	strncpy(key, *str + 1, value - *str - 1);
	key[value - *str - 1] = '\0';
	CBPlatform::strlwr(key);

	value++;

	char *new_str;

	_stringsIter = _strings.find(key);
	if (_stringsIter != _strings.end()) {
		new_str = new char[_stringsIter->_value.size() + 1];
		strcpy(new_str, _stringsIter->_value.c_str());
	} else {
		new_str = new char[strlen(value) + 1];
		strcpy(new_str, value);
	}

	delete [] key;
	delete [] *str;
	*str = new_str;

	if (strlen(*str) > 0 && *str[0] == '/') expand(str, forceExpand);
}


//////////////////////////////////////////////////////////////////////////
const char *CBStringTable::expandStatic(const char *string, bool forceExpand) {
	if (Game->_doNotExpandStrings && !forceExpand) return string;

	if (string == NULL || string[0] == '\0' || string[0] != '/') return string;

	const char *value = strchr(string + 1, '/');
	if (value == NULL) return string;

	char *key = new char[value - string];
	strncpy(key, string + 1, value - string - 1);
	key[value - string - 1] = '\0';
	CBPlatform::strlwr(key);

	value++;

	const char *new_str;

	_stringsIter = _strings.find(key);
	if (_stringsIter != _strings.end()) {
		new_str = _stringsIter->_value.c_str();
	} else {
		new_str = value;
	}

	delete [] key;

	if (strlen(new_str) > 0 && new_str[0] == '/') return expandStatic(new_str, forceExpand);
	else return new_str;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBStringTable::loadFile(const char *filename, bool clearOld) {
	Game->LOG(0, "Loading string table...");

	if (clearOld) _strings.clear();

	uint32 size;
	byte *buffer = Game->_fileManager->readWholeFile(filename, &size);
	if (buffer == NULL) {
		Game->LOG(0, "CBStringTable::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	uint32 pos = 0;

	if (size > 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
		pos += 3;
		if (Game->_textEncoding != TEXT_UTF8) {
			Game->_textEncoding = TEXT_UTF8;
			//Game->_textEncoding = TEXT_ANSI;
			Game->LOG(0, "  UTF8 file detected, switching to UTF8 text encoding");
		}
	} else Game->_textEncoding = TEXT_ANSI;

	uint32 lineLength = 0;
	while (pos < size) {
		lineLength = 0;
		while (pos + lineLength < size && buffer[pos + lineLength] != '\n' && buffer[pos + lineLength] != '\0') lineLength++;

		uint32 realLength = lineLength - (pos + lineLength >= size ? 0 : 1);
		char *line = new char[realLength + 1];
		strncpy(line, (char *)&buffer[pos], realLength);
		line[realLength] = '\0';
		char *value = strchr(line, '\t');
		if (value == NULL) value = strchr(line, ' ');

		if (line[0] != ';') {
			if (value != NULL) {
				value[0] = '\0';
				value++;
				for (uint32 i = 0; i < strlen(value); i++) {
					if (value[i] == '|') value[i] = '\n';
				}
				addString(line, value, clearOld);
			} else if (line[0] != '\0') addString(line, "", clearOld);
		}

		delete [] line;
		pos += lineLength + 1;
	}

	delete [] buffer;

	Game->LOG(0, "  %d strings loaded", _strings.size());

	return STATUS_OK;
}

} // end of namespace WinterMute
