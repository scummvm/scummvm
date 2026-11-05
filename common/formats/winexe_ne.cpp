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

#include "common/formats/winexe_ne.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/stream.h"

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
	_nameTable.clear();
}

bool NEResources::loadFromEXE(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle) {
	clear();

	if (!stream)
		return false;

	_exe = stream;
	_disposeFileHandle = disposeFileHandle;

	uint32 offsetResourceTable = getResourceTableOffset();
	if (offsetResourceTable == kInvalidAddress)
		return false;
	if (offsetResourceTable == 0)
		return true;

	if (!readResourceTable(offsetResourceTable))
		return false;

	return true;
}

uint32 NEResources::getResourceTableOffset() {
	if (!_exe)
		return kInvalidAddress;

	if (!_exe->seek(0))
		return kInvalidAddress;

	//                          'MZ'
	if (_exe->readUint16BE() != 0x4D5A)
		return kInvalidAddress;

	if (!_exe->seek(60))
		return kInvalidAddress;

	uint32 offsetSegmentEXE = _exe->readUint16LE();
	if (!_exe->seek(offsetSegmentEXE))
		return kInvalidAddress;

	//                          'NE'
	if (_exe->readUint16BE() != 0x4E45)
		return kInvalidAddress;

	// Reading automatic segment index
	if (!_exe->seek(offsetSegmentEXE + 0x0e))
		return kInvalidAddress;
	_autoSegmentIndex = _exe->readUint16LE();

	// number of NE segments
	if (!_exe->seek(offsetSegmentEXE + 0x001c))
		return kInvalidAddress;
	auto numSegments = _exe->readUint16LE();

	// segment table offset
	if (!_exe->seek(offsetSegmentEXE + 0x0022))
		return kInvalidAddress;
	auto segmentTableOffset = _exe->readUint16LE();

	// segment address alignment (page size)
	if (!_exe->seek(offsetSegmentEXE + 0x0032))
		return kInvalidAddress;
	auto segmentAddrShift = _exe->readUint16LE();

	if (!_exe->seek(offsetSegmentEXE + segmentTableOffset))
		return kInvalidAddress;
	_segments.reserve(numSegments);
	for (unsigned idx = 0; idx != numSegments; ++idx) {
		uint32 segmentOffset = _exe->readUint16LE();
		segmentOffset <<= segmentAddrShift;
		uint32 segmentSize = _exe->readUint16LE();
		if (segmentSize == 0)
			segmentSize = 0x10000;
		auto segmentFlags = _exe->readUint16LE();
		_exe->skip(2);
		_segments.push_back(NESegment{segmentOffset, segmentSize, segmentFlags});
	}

	if (!_exe->seek(offsetSegmentEXE + 36))
		return kInvalidAddress;

	uint32 offsetResourceTable = _exe->readUint16LE();
	if (offsetResourceTable == 0)
		// No resource table
		return 0;

	// Offset relative to the segment _exe header
	offsetResourceTable += offsetSegmentEXE;
	if (!_exe->seek(offsetResourceTable))
		return kInvalidAddress;

	return offsetResourceTable;
}

/**
 * A "resource ID -> name" mapping found in some NE executables, stored as a resource.
 * Not described by any documentation I could find, but the corresponding #define can be encountered in some old header files.
 * Mentioned by this blog post https://www.toptensoftware.com/blog/win3mu-5/ (section "The Mysterious Resource Type #15")
 *
 * The two-string layout is documented in the Windows 3.1 SDK, Vol. 4, section 7.11.1 (pp. 97-98).
 * https://archive.org/details/bitsavers_microsoftwindows3.1SDK1992PC302110492ProgrammersRe_14016743/page/97/mode/2up
 *
 * The name table records have the following layout:
 *
 * struct
 * {
 *  WORD lengthEntry;        // length of this entry (including this field)
 *  WORD resourceType;       // type ordinal; high bit indicates a name alias
 *  WORD resourceId;         // resource ordinal; high bit indicates a name alias
 *  CHAR typeName[];         // null-terminated type name, or an empty string
 *  CHAR resourceName[];     // null-terminated resource name, or an empty string
 * }
 *
 */

bool NEResources::readNameTable(uint32 offset, uint32 size) {
	if (!_exe)
		return false;

	uint32 curPos = _exe->pos();
	if (!_exe->seek(offset)) {
		_exe->seek(curPos);
		return false;
	}

	uint32 offsetEnd = offset + size;
	while (_exe->pos() < offsetEnd) {
		uint16 lengthEntry = _exe->readUint16LE();
		if (lengthEntry == 0)
			break;

		uint16 rawResourceType = _exe->readUint16LE();
		uint16 rawResourceId = _exe->readUint16LE();

		// The high bits indicate name aliases and must be checked before masking.
		// Only the numeric ordinals are stored as map keys.
		bool hasResourceTypeNameAlias = (rawResourceType & 0x8000) != 0;
		uint16 resourceType = rawResourceType & 0x7FFF;
		bool hasResourceIdNameAlias = (rawResourceId & 0x8000) != 0;
		uint16 resourceId = rawResourceId & 0x7FFF;

		// Maunal seeking to the correct position is necessary, as bounded readString always fully consumes the given length.
		uint32 typeNamePos = _exe->pos();
		String typeName = _exe->readString('\0', lengthEntry - 7);
		_exe->seek(typeNamePos + typeName.size() + 1);

		String resourceName = _exe->readString('\0', lengthEntry - typeName.size() - 7);
		if (typeName.size() + resourceName.size() + 8 != lengthEntry) {
			warning("NEResources::readNameTable(): unexpected string length in name table (expected = %d, found = %d, typeName = %s, resourceName = %s)",
					lengthEntry - 6, typeName.size() + resourceName.size() + 2, typeName.c_str(), resourceName.c_str());
		}

		if (hasResourceTypeNameAlias && !typeName.empty())
			_nameTable[resourceType].typeName = typeName;
		if (hasResourceIdNameAlias && !resourceName.empty())
			_nameTable[resourceType].resourceNames[resourceId] = resourceName;
		debug(2, "NEResources::readNameTable(): resourceType = %d, resourceId = %d, typeName = %s, resourceName = %s",
			  rawResourceType, rawResourceId, typeName.c_str(), resourceName.c_str());
	}

	_exe->seek(curPos);
	return true;
}

static const char *const s_resTypeNames[] = {
	"", "cursor", "bitmap", "icon", "menu", "dialog", "string",
	"font_dir", "font", "accelerator", "rc_data", "msg_table",
	"group_cursor", "", "group_icon", "name_table", "version", "dlg_include",
	"", "plug_play", "vxd", "ani_cursor", "ani_icon", "html",
	"manifest"};

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

		debug(2, "%d resources in table", resCount);

		_exe->skip(4); // reserved

		// WORD TYPEINFO::rtResourceCount does not have a limit in the official docs (SDK Vol. 4, section 6.2.3.1, p. 76).
		// https://archive.org/details/bitsavers_microsoftwindows3.1SDK1992PC302110492ProgrammersRe_14016743/page/75/mode/2up
		// 1024 is an arbitrary bound for safeguard; Increase the bound if any valid NE file with more than 1024 entries are found.
		if (resCount > 1024) {
			warning("NEResources::readResourceTable(): resource table looks malformed, %d entries > 1024", resCount);
			resCount = 1024;
		}

		for (int i = 0; i < resCount; i++) {
			Resource res;

			// Resource properties
			res.offset = _exe->readUint16LE() * align;
			res.size = _exe->readUint16LE() * align;
			res.flags = _exe->readUint16LE();
			uint16 id = _exe->readUint16LE();
			res.handle = _exe->readUint16LE();
			res.usage = _exe->readUint16LE();

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
			if (type == kWinNameTable)
				readNameTable(res.offset, res.size);
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
	for (uint16 i = 0; i < length; i++) {
		char b = (char)exe.readByte();

		// Do not read beyond end of string
		if (!b) {
			break;
		}

		string += b;
	}

	exe.seek(curPos);
	return string;
}

bool NEResources::matchesType(const WinResourceID &resourceType, const WinResourceID &queryType) const {
	// Match the stored type directly first; matching requires equality of both type and value.
	if (resourceType == queryType)
		return true;

	// A name-table alias can link an ordinal type to a named type in either direction.
	// With no aliases, only the direct comparison above can match.
	if (_nameTable.empty())
		return false;

	const bool queryHasString = queryType.hasString();
	const bool resourceHasString = resourceType.hasString();
	// Alias matching requires exactly one non-empty name.
	if (queryHasString == resourceHasString)
		return false;

	// NameTable keys are ordinals, so look up the side without a name.
	// Null and empty-string keys have no entry and yield an empty alias.
	const WinResourceID &ordinalType = queryHasString ? resourceType : queryType;
	const WinResourceID &namedType = queryHasString ? queryType : resourceType;
	const String &typeName = _nameTable.getValOrDefault(ordinalType).typeName;
	return !typeName.empty() && namedType == typeName;
}

const NEResources::Resource *NEResources::findResource(const WinResourceID &type, const WinResourceID &id) const {
	for (const auto &resource : _resources) {
		if (matchesType(resource.type, type) &&
			(resource.id == id ||
			 (!_nameTable.empty() &&
			  _nameTable.contains(resource.type) &&
			  _nameTable[resource.type].resourceNames.contains(resource.id) &&
			  _nameTable[resource.type].resourceNames[resource.id] == id.toString())))
			return &resource;
	}

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

	for (const auto &resource : _resources)
		if (matchesType(resource.type, type))
			idArray.push_back(resource.id);

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
