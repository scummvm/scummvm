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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_STRING_TABLE_H
#define WINTERMUTE_BASE_STRING_TABLE_H


#include "common/hashmap.h"
#include "engines/wintermute/base/base.h"

namespace Wintermute {

class BasePersistenceManager;

class BaseStringTable : public BaseClass {
public:
	bool loadFile(const char *filename, bool deleteAll = true);
	void expand(char **str) const;
	void expand(Common::String &str) const;
	const char *expandStatic(const char *string) const;
	bool addString(const char *key, const char *val, bool reportDuplicities = true);
	BaseStringTable(BaseGame *inGame);
	~BaseStringTable() override;
	char *getKey(const char *str) const;
	bool persist(BasePersistenceManager *persistMgr) override;
private:
	Common::HashMap<Common::String, Common::String> _strings;
	Common::Array<Common::String> _filenames;
	typedef Common::HashMap<Common::String, Common::String>::const_iterator StringsIter;

};

} // End of namespace Wintermute

#endif
