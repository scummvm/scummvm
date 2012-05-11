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

#include "PlatformSDL.h"
#include "BFileManager.h"
#include "BGame.h"
#include "BStringTable.h"
#include "StringUtil.h"
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
HRESULT CBStringTable::AddString(const char *Key, const char *Val, bool ReportDuplicities) {
	if (Key == NULL || Val == NULL) return E_FAIL;

	if (scumm_stricmp(Key, "@right-to-left") == 0) {
		Game->_textRTL = true;
		return S_OK;
	}

	Common::String final_key = Key;
	StringUtil::ToLowerCase(final_key);

	_stringsIter = _strings.find(final_key);
	if (_stringsIter != _strings.end() && ReportDuplicities) Game->LOG(0, "  Warning: Duplicate definition of string '%s'.", final_key.c_str());

	_strings[final_key] = Val;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
char *CBStringTable::GetKey(const char *Str) {
	if (Str == NULL || Str[0] != '/') return NULL;

	char *value = strchr((char *)Str + 1, '/');
	if (value == NULL) return NULL;

	char *key = new char[value - Str];
	strncpy(key, Str + 1, value - Str - 1);
	key[value - Str - 1] = '\0';
	CBPlatform::strlwr(key);

	char *new_str;

	_stringsIter = _strings.find(key);
	if (_stringsIter != _strings.end()) {
		new_str = new char[_stringsIter->_value.size() + 1];
		strcpy(new_str, _stringsIter->_value.c_str());
		if (strlen(new_str) > 0 && new_str[0] == '/' && strchr(new_str + 1, '/')) {
			delete [] key;
			char *Ret = GetKey(new_str);
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
void CBStringTable::Expand(char **Str, bool ForceExpand) {
	if (Game->_doNotExpandStrings && !ForceExpand) return;

	if (Str == NULL || *Str == NULL || *Str[0] != '/') return;

	char *value = strchr(*Str + 1, '/');
	if (value == NULL) return;

	char *key = new char[value - *Str];
	strncpy(key, *Str + 1, value - *Str - 1);
	key[value - *Str - 1] = '\0';
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
	delete [] *Str;
	*Str = new_str;

	if (strlen(*Str) > 0 && *Str[0] == '/') Expand(Str, ForceExpand);
}


//////////////////////////////////////////////////////////////////////////
const char *CBStringTable::ExpandStatic(const char *String, bool ForceExpand) {
	if (Game->_doNotExpandStrings && !ForceExpand) return String;

	if (String == NULL || String[0] == '\0' || String[0] != '/') return String;

	const char *value = strchr(String + 1, '/');
	if (value == NULL) return String;

	char *key = new char[value - String];
	strncpy(key, String + 1, value - String - 1);
	key[value - String - 1] = '\0';
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

	if (strlen(new_str) > 0 && new_str[0] == '/') return ExpandStatic(new_str, ForceExpand);
	else return new_str;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBStringTable::LoadFile(const char *Filename, bool ClearOld) {
	Game->LOG(0, "Loading string table...");

	if (ClearOld) _strings.clear();

	uint32 Size;
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename, &Size);
	if (Buffer == NULL) {
		Game->LOG(0, "CBStringTable::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	int Pos = 0;

	if (Size > 3 && Buffer[0] == 0xEF && Buffer[1] == 0xBB && Buffer[2] == 0xBF) {
		Pos += 3;
		if (Game->_textEncoding != TEXT_UTF8) {
			Game->_textEncoding = TEXT_UTF8;
			//Game->_textEncoding = TEXT_ANSI;
			Game->LOG(0, "  UTF8 file detected, switching to UTF8 text encoding");
		}
	} else Game->_textEncoding = TEXT_ANSI;

	int LineLength = 0;
	while (Pos < Size) {
		LineLength = 0;
		while (Pos + LineLength < Size && Buffer[Pos + LineLength] != '\n' && Buffer[Pos + LineLength] != '\0') LineLength++;

		int RealLength = LineLength - (Pos + LineLength >= Size ? 0 : 1);
		char *line = new char[RealLength + 1];
		strncpy(line, (char *)&Buffer[Pos], RealLength);
		line[RealLength] = '\0';
		char *value = strchr(line, '\t');
		if (value == NULL) value = strchr(line, ' ');

		if (line[0] != ';') {
			if (value != NULL) {
				value[0] = '\0';
				value++;
				for (int i = 0; i < strlen(value); i++) {
					if (value[i] == '|') value[i] = '\n';
				}
				AddString(line, value, ClearOld);
			} else if (line[0] != '\0') AddString(line, "", ClearOld);
		}

		delete [] line;
		Pos += LineLength + 1;
	}

	delete [] Buffer;

	Game->LOG(0, "  %d strings loaded", _strings.size());

	return S_OK;
}

} // end of namespace WinterMute
