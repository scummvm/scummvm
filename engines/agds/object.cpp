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

#include "agds/object.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace AGDS {

Object::Object(const Common::String &name, Common::SeekableReadStream * stream) : _name(name), _stringTableLoaded(false) {
	stream->skip(2);
	uint16 dataSize = stream->readUint16LE();
	if (dataSize != 0)
		error("implement me: object with data (%u)", dataSize);
	uint16 codeSize = stream->readUint16LE();
	uint8 flags = stream->readByte();
	if (flags != 1)
		error("implement me: no flags handling yet");

	debug("object code size %u", codeSize);
	_code.resize(codeSize);
	stream->read(_code.data(), _code.size());
}

void Object::readStringTable(unsigned resOffset, uint16 resCount) {
	if (_stringTableLoaded)
		return;

	resOffset += 5 /*instruction*/ + 0x11 /*another header*/;
	if (resOffset >= _code.size())
		error("invalid resource table offset");

	//debug("resource table at %08x", resOffset);
	Common::MemoryReadStream stream(_code.data() + resOffset, _code.size() - resOffset);
	for(uint16 i = 0; i < resCount; ++i) {
		uint16 offset = stream.readUint16LE();
		uint16 flags = stream.readUint16LE();

		unsigned nameOffset = resOffset + offset;
		if (nameOffset > _code.size())
			error("invalid resource name offset");

		const char * nameBegin = reinterpret_cast<const char *>(_code.data() + nameOffset);
		const char * codeEnd = reinterpret_cast<const char *>(_code.data() + _code.size());
		const char * nameEnd = Common::find(nameBegin, codeEnd, 0);

		Common::String name(nameBegin, nameEnd - nameBegin);

		//debug("resource table 1[%04u]: 0x%04x %s", i, flags, name.c_str());
		_stringTable[i] = StringEntry(name, flags);
	}
	_stringTableLoaded = true;
}

const Object::StringEntry & Object::getString(uint16 index) const {
	if (!_stringTableLoaded)
		error("no string table loaded");

	StringTableType::const_iterator i = _stringTable.find(index);
	if (i == _stringTable.end())
		error("no resource name with id %u", index);

	return i->_value;
}

}
