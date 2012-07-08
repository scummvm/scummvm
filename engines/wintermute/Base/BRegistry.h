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

#ifndef WINTERMUTE_BREGISTRY_H
#define WINTERMUTE_BREGISTRY_H

#include "engines/wintermute/Base/BBase.h"

namespace WinterMute {

class CBRegistry : public CBBase {
public:
	void setIniName(const char *name);
	char *getIniName();
	bool writeBool(const AnsiString &subKey, const AnsiString &key, bool Value);
	bool readBool(const AnsiString &subKey, const AnsiString &key, bool init = false);
	bool writeInt(const AnsiString &subKey, const AnsiString &key, int value);
	int readInt(const AnsiString &subKey, const AnsiString &key, int init = 0);
	bool writeString(const AnsiString &subKey, const AnsiString &key, const AnsiString &value);
	AnsiString readString(const AnsiString &subKey, const AnsiString &key, const AnsiString &init = "");
	CBRegistry(CBGame *inGame);
	virtual ~CBRegistry();

	void setBasePath(const char *basePath);
	AnsiString getBasePath() const {
		return _basePath;
	}

	void loadValues(bool local);
	void saveValues();

private:
	char *_iniName;

	typedef Common::HashMap<AnsiString, AnsiString> KeyValuePair;
	typedef Common::HashMap<AnsiString, KeyValuePair> PathValueMap;

	PathValueMap _localValues;
	PathValueMap _values;

	AnsiString _basePath;

	void loadXml(const AnsiString fileName, PathValueMap &values);
	void saveXml(const AnsiString fileName, PathValueMap &values);

	AnsiString getValue(PathValueMap &values, const AnsiString path, const AnsiString &key, bool &found);
};

} // end of namespace WinterMute

#endif
