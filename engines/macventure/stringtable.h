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
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_STRINGTABLE_H
#define MACVENTURE_STRINGTABLE_H

#include "macventure/macventure.h"

#include "common/file.h"

namespace MacVenture {

extern void toASCII(Common::String &str);

enum StringTableID {
	kErrorStringTableID = 0x80,
	kFilenamesStringTableID = 0x81,
	kCommonArticlesStringTableID = 0x82,
	kNamingArticlesStringTableID = 0x83,
	kIndirectArticlesStringTableID = 0x84
};

class StringTable {
public:
	StringTable(MacVentureEngine *engine, Common::MacResManager *resMan, StringTableID id) {
		_engine = engine;
		_resourceManager = resMan;
		_id = id;

		if (!loadStrings())
			error("ENGINE: Could not load string table %x", id);
	}

	~StringTable() {

	}

	const Common::Array<Common::String> &getStrings() {
		return _strings;
	}

	Common::String getString(uint ndx) {
		assert(ndx < _strings.size());
		return _strings[ndx];
	}

private:

	bool loadStrings() {
		Common::MacResIDArray resArray;
		Common::SeekableReadStream *res;

		if ((resArray = _resourceManager->getResIDArray(MKTAG('S', 'T', 'R', '#'))).size() == 0)
			return false;

		res = _resourceManager->getResource(MKTAG('S', 'T', 'R', '#'), _id);

		_strings.push_back("dummy"); // String tables are 1-indexed
		uint16 numStrings = res->readUint16BE();
		uint8 strLength = 0;
		for (uint i = 0; i < numStrings; ++i) {
			strLength = res->readByte();
			char *str = new char[strLength + 1];
			res->read(str, strLength);
			str[strLength] = '\0';
			// HACK until a proper special char implementation is found, this will have to do.
			Common::String result = Common::String(str);
			toASCII(result);
			debugC(4, kMVDebugText, "Loaded string %s", str);
			_strings.push_back(Common::String(result));
			delete[] str;
		}

		delete res;
		return true;
	}

private:

	MacVentureEngine *_engine;
	Common::MacResManager *_resourceManager;

	StringTableID _id;

	Common::Array<Common::String> _strings;
};

} // End of namespace MacVenture

#endif
