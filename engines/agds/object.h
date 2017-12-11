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

#ifndef AGDS_OBJECT_H
#define AGDS_OBJECT_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace AGDS {

class Object {
public:
	typedef Common::Array<uint8> CodeType;

	struct StringEntry
	{
		Common::String	string;
		uint16			flags;

		StringEntry(): string(), flags() { }
		StringEntry(const Common::String &s, uint16 f): string(s), flags(f) { }
	};

private:
	typedef Common::Array<StringEntry> StringTableType;

	Common::String					_name;
	CodeType						_code;
	StringTableType					_stringTable;
	bool							_stringTableLoaded;
	const Graphics::Surface *		_picture;
	Common::Point					_pos;


public:
	Object(const Common::String &name, Common::SeekableReadStream * stream);

	void readStringTable(unsigned resOffset, uint16 resCount);
	const StringEntry & getString(uint16 index) const;
	uint getStringTableSize() const
	{ return _stringTable.size(); }

	const Common::String & getName() const
	{ return _name; }

	const CodeType & getCode() const {
		return _code;
	}

	void setPicture(const Graphics::Surface *);

	const Graphics::Surface *getPicture() const {
		return _picture;
	}

	void paint(Graphics::Surface &backbuffer);

	void move(Common::Point pos) {
		_pos = pos;
	}

	int getX() const {
		return _pos.x;
	}
	int getY() const {
		return _pos.y;
	}
};


} // End of namespace AGDS

#endif /* AGDS_OBJECT_H */
