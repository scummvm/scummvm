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

#include "common/debug.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/winexe_ne.h"

namespace Common {

NEResources::NEResources() {
	_exe = nullptr;
}

NEResources::~NEResources() {
	clear();
}

void NEResources::clear() {
	if (_exe) {
		if (_disposeFileHandle == DisposeAfterUse::YES)
			delete _exe;
		_exe = nullptr;
	}

	_resources.clear();
}

bool NEResources::loadFromEXE(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
	clear();

	if (!stream)
		return false;

	_exe = stream;
	_disposeFileHandle = disposeFileHandle;

	uint32 offsetResourceTable = getResourceTableOffset();
	if (offsetResourceTable == 0xFFFFFFFF)
		return false;
	if (offsetResourceTable == 0)
		return true;

	if (!readResourceTable(offsetResourceTable))
		return false;

	return true;
}

uint32 NEResources::getResourceTableOffset() {
	if (!_exe)
		return 0xFFFFFFFF;

	if (!_exe->seek(0))
		return 0xFFFFFFFF;

	//                          'MZ'
	if (_exe->readUint16BE() != 0x4D5A)
		return 0xFFFFFFFF;

	if (!_exe->seek(60))
		return 0xFFFFFFFF;

	uint32 offsetSegmentEXE = _exe->readUint16LE();
	if (!_exe->seek(offsetSegmentEXE))
		return 0xFFFFFFFF;

	//                          'NE'
	if (_exe->readUint16BE() != 0x4E45)
		return 0xFFFFFFFF;

	if (!_exe->seek(offsetSegmentEXE + 36))
		return 0xFFFFFFFF;

	uint32 offsetResourceTable = _exe->readUint16LE();
	if (offsetResourceTable == 0)
		// No resource table
		return 0;

	// Offset relative to the segment _exe header
	offsetResourceTable += offsetSegmentEXE;
	if (!_exe->seek(offsetResourceTable))
		return 0xFFFFFFFF;

	return offsetResourceTable;
}

static const char *s_resTypeNames[] = {
	"", "cursor", "bitmap", "icon", "menu", "dialog", "string",
	"font_dir", "font", "accelerator", "rc_data", "msg_table",
	"group_cursor", "", "group_icon", "", "version", "dlg_include",
	"", "plug_play", "vxd", "ani_cursor", "ani_icon", "html",
	"manifest"
};

bool NEResources::readResourceTable(uint32 offset) {
	if (!_exe)
		return false;

	if (!_exe->seek(offset))
		return false;

	uint32 align = 1 << _exe->readUint16LE();
	uint16 typeID = _exe->readUint16LE();

	while (typeID != 0) {
		// High bit of the type means integer type
		WinResourceID type;
		if (typeID & 0x8000)
			type = typeID & 0x7FFF;
		else
			type = getResourceString(*_exe, offset + typeID);

		uint16 resCount = _exe->readUint16LE();

		_exe->skip(4); // reserved

		for (int i = 0; i < resCount; i++) {
			Resource res;

			// Resource properties
			res.offset = _exe->readUint16LE() * align;
			res.size   = _exe->readUint16LE() * align;
			res.flags  = _exe->readUint16LE();
			uint16 id  = _exe->readUint16LE();
			res.handle = _exe->readUint16LE();
			res.usage  = _exe->readUint16LE();

			res.type = type;

			// High bit means integer type
			if (id & 0x8000)
				res.id = id & 0x7FFF;
			else
				res.id = getResourceString(*_exe, offset + id);

			if (typeID & 0x8000 && ((typeID & 0x7FFF) < ARRAYSIZE(s_resTypeNames)) && s_resTypeNames[typeID & 0x7FFF][0] != 0)
				debug(2, "Found resource %s %s", s_resTypeNames[typeID & 0x7FFF], res.id.toString().c_str());
			else
				debug(2, "Found resource %s %s", type.toString().c_str(), res.id.toString().c_str());

			_resources.push_back(res);
		}

		typeID = _exe->readUint16LE();
	}

	return true;
}

String NEResources::getResourceString(SeekableReadStream &exe, uint32 offset) {
	uint32 curPos = exe.pos();

	if (!exe.seek(offset)) {
		exe.seek(curPos);
		return "";
	}

	uint8 length = exe.readByte();

	String string;
	for (uint16 i = 0; i < length; i++)
		string += (char)exe.readByte();

	exe.seek(curPos);
	return string;
}

const NEResources::Resource *NEResources::findResource(const WinResourceID &type, const WinResourceID &id) const {
	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it)
		if (it->type == type && it->id == id)
			return &*it;

	return nullptr;
}

SeekableReadStream *NEResources::getResource(const WinResourceID &type, const WinResourceID &id) {
	const Resource *res = findResource(type, id);

	if (!res)
		return nullptr;

	_exe->seek(res->offset);
	return _exe->readStream(res->size);
}

const Array<WinResourceID> NEResources::getIDList(const WinResourceID &type) const {
	Array<WinResourceID> idArray;

	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it)
		if (it->type == type)
			idArray.push_back(it->id);

	return idArray;
}

String NEResources::loadString(uint32 stringID) {
	// This is how the resource ID is calculated
	String string;
	SeekableReadStream *stream = getResource(kWinString, (stringID >> 4) + 1);

	if (!stream)
		return string;

	// Skip over strings we don't care about
	uint32 startString = stringID & ~0xF;

	for (uint32 i = startString; i < stringID; i++)
		stream->skip(stream->readByte());

	byte size = stream->readByte();
	while (size--)
		string += (char)stream->readByte();

	delete stream;
	return string;
}

WinResources::VersionInfo *NEResources::parseVersionInfo(SeekableReadStream *res) {
	VersionInfo *info = new VersionInfo;

	while (res->pos() < res->size() && !res->eos()) {
		while (res->pos() % 4 && !res->eos()) // Pad to 4
			res->readByte();

		/* uint16 len = */ res->readUint16LE();
		/* uint16 valLen = */ res->readUint16LE();
		uint16 c;

		Common::String key;
		while ((c = res->readByte()) != 0 && !res->eos())
			key += c;

		while (res->pos() % 4 && !res->eos()) // Pad to 4
			res->readByte();

		if (res->eos())
			break;

		if (key == "VS_VERSION_INFO") {
			if (!info->readVSVersionInfo(res))
				return info;
		}
	}

	return info;
}

} // End of namespace Common
