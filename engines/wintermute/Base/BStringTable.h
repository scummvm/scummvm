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

#ifndef WINTERMUTE_BSTRINGTABLE_H
#define WINTERMUTE_BSTRINGTABLE_H


#include "common/hashmap.h"
#include "engines/wintermute/Base/BBase.h"

namespace WinterMute {

class CBStringTable : public CBBase {
public:
	const char *expandStatic(const char *string, bool forceExpand = false);
	ERRORCODE loadFile(const char *filename, bool deleteAll = true);
	void expand(char **str, bool forceExpand = false);
	ERRORCODE addString(const char *key, const char *val, bool reportDuplicities = true);
	CBStringTable(CBGame *inGame);
	virtual ~CBStringTable();
	Common::HashMap<Common::String, Common::String> _strings;
	char *getKey(const char *str);
private:
	Common::HashMap<Common::String, Common::String>::iterator _stringsIter;

};

} // end of namespace WinterMute

#endif
