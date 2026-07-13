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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/resources.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/substream.h"

#include "ripper/detection.h"

namespace Ripper {

static const uint32 kModernLibraryMagic = 0x4c494232;

AssetLibrary::AssetLibrary() : _modernFormat(false) {
}

Common::String AssetLibrary::readFixedString(const char *data, uint length) {
	uint actualLength = 0;
	while (actualLength < length && data[actualLength] != '\0')
		++actualLength;
	return Common::String(data, data + actualLength);
}

Common::String AssetLibrary::normalizeMemberName(const Common::String &memberName, bool includeExtension) {
	uint baseStart = 0;
	for (uint i = 0; i < memberName.size(); ++i) {
		if (memberName[i] == '/' || memberName[i] == '\\' || memberName[i] == ':')
			baseStart = i + 1;
	}

	uint dot = memberName.size();
	for (uint i = baseStart; i < memberName.size(); ++i) {
		if (memberName[i] == '.' && memberName.size() - i <= 4)
			dot = i;
	}

	const uint baseEnd = dot < memberName.size() ? dot : memberName.size();
	Common::String key;
	for (uint i = baseStart; i < baseEnd && key.size() < 8; ++i)
		key += memberName[i];

	if (includeExtension && dot < memberName.size()) {
		key += '.';
		for (uint i = dot + 1; i < memberName.size() && i < dot + 4; ++i)
			key += memberName[i];
	}

	key.toLowercase();
	return key;
}

bool AssetLibrary::open(const Common::Path &filename) {
	Common::File file;
	_entries.clear();
	_filename = filename;
	_modernFormat = false;

	if (!file.open(filename)) {
		warning("Ripper: could not open asset library '%s'", filename.toString().c_str());
		return false;
	}

	const int64 fileSize64 = file.size();
	if (fileSize64 < 6 || fileSize64 > 0xffffffffLL) {
		warning("Ripper: invalid asset library size %lld for '%s'", fileSize64, filename.toString().c_str());
		return false;
	}
	const uint32 fileSize = (uint32)fileSize64;
	const uint16 entryCount = file.readUint16LE();
	const uint32 directoryOffset = file.readUint32LE();
	uint32 payloadStart = 6;

	if (fileSize >= 10) {
		const uint32 magic = file.readUint32LE();
		if (magic == kModernLibraryMagic) {
			_modernFormat = true;
			payloadStart = 10;
		}
	}

	const uint32 recordSize = _modernFormat ? 20 : 12;
	const uint64 tableSize = (uint64)entryCount * recordSize;
	if (directoryOffset < payloadStart || (uint64)directoryOffset + tableSize > fileSize) {
		warning("Ripper: invalid directory range offset=%u count=%u recordSize=%u fileSize=%u in '%s'",
			directoryOffset, entryCount, recordSize, fileSize, filename.toString().c_str());
		return false;
	}

	file.seek(directoryOffset);
	for (uint i = 0; i < entryCount; ++i) {
		Entry entry;
		entry.offset = file.readUint32LE();
		char baseName[8];
		if (file.read(baseName, sizeof(baseName)) != sizeof(baseName)) {
			warning("Ripper: truncated directory entry %u in '%s'", i, filename.toString().c_str());
			return false;
		}

		Common::String name = readFixedString(baseName, sizeof(baseName));
		entry.timestamp = 0;
		if (_modernFormat) {
			char extension[4];
			if (file.read(extension, sizeof(extension)) != sizeof(extension))
				return false;
			const Common::String ext = readFixedString(extension, sizeof(extension));
			if (!ext.empty())
				name += "." + ext;
			entry.timestamp = file.readUint32LE();
		}

		entry.key = normalizeMemberName(name, _modernFormat);
		entry.size = 0;
		if (entry.key.empty() || entry.offset < payloadStart || entry.offset > directoryOffset) {
			warning("Ripper: invalid directory entry %u key='%s' offset=%u in '%s'",
				i, entry.key.c_str(), entry.offset, filename.toString().c_str());
			return false;
		}
		if (!_entries.empty() && entry.offset < _entries.back().offset) {
			warning("Ripper: non-monotonic directory entry %u in '%s'", i, filename.toString().c_str());
			return false;
		}
		for (uint existing = 0; existing < _entries.size(); ++existing) {
			if (_entries[existing].key == entry.key) {
				warning("Ripper: duplicate member '%s' in '%s'", entry.key.c_str(), filename.toString().c_str());
				return false;
			}
		}

		_entries.push_back(entry);
	}

	for (uint i = 0; i < _entries.size(); ++i) {
		const uint32 end = i + 1 < _entries.size() ? _entries[i + 1].offset : directoryOffset;
		if (end < _entries[i].offset) {
			warning("Ripper: invalid member range for '%s' in '%s'",
				_entries[i].key.c_str(), filename.toString().c_str());
			return false;
		}
		_entries[i].size = end - _entries[i].offset;
		debugC(3, kDebugResources,
			"Ripper: archive '%s' member=%u key='%s' offset=%u size=%u timestamp=0x%08x",
			filename.toString().c_str(), i, _entries[i].key.c_str(), _entries[i].offset,
			_entries[i].size, _entries[i].timestamp);
	}

	debugC(1, kDebugResources, "Ripper: opened %s asset library '%s' with %u entries",
		_modernFormat ? "2BIL" : "legacy", filename.toString().c_str(), _entries.size());
	return true;
}

const AssetLibrary::Entry *AssetLibrary::findEntry(const Common::String &memberName) const {
	const Common::String key = normalizeMemberName(memberName, _modernFormat);
	for (uint i = 0; i < _entries.size(); ++i) {
		if (_entries[i].key == key)
			return &_entries[i];
	}
	return nullptr;
}

bool AssetLibrary::hasMember(const Common::String &memberName) const {
	return findEntry(memberName) != nullptr;
}

Common::SeekableReadStream *AssetLibrary::createReadStreamForMember(const Common::String &memberName) const {
	const Entry *entry = findEntry(memberName);
	if (!entry) {
		warning("Ripper: member '%s' was not found in '%s'", memberName.c_str(), _filename.toString().c_str());
		return nullptr;
	}

	Common::File *file = new Common::File();
	if (!file->open(_filename)) {
		warning("Ripper: could not reopen asset library '%s' for member '%s'",
			_filename.toString().c_str(), memberName.c_str());
		delete file;
		return nullptr;
	}

	debugC(2, kDebugResources, "Ripper: opening member '%s' from '%s' offset=%u size=%u",
		memberName.c_str(), _filename.toString().c_str(), entry->offset, entry->size);
	return new Common::SeekableSubReadStream(file, entry->offset, entry->offset + entry->size,
		DisposeAfterUse::YES);
}

bool ResourceManager::initialize() {
	Common::File iniFile;
	Common::INIFile ini;
	if (!iniFile.open("ripper.ini") || !ini.loadFromStream(iniFile)) {
		warning("Ripper: could not load RIPPER.INI");
		return false;
	}

	Common::String scriptLibrary;
	Common::String interfaceLibrary;
	if (!ini.getKey("script", "files", scriptLibrary) ||
		!ini.getKey("interface", "files", interfaceLibrary)) {
		warning("Ripper: RIPPER.INI is missing FILES script or interface entries");
		return false;
	}

	debugC(2, kDebugResources, "Ripper: RIPPER.INI script='%s' interface='%s'",
		scriptLibrary.c_str(), interfaceLibrary.c_str());
	return _scripts.open(Common::Path(scriptLibrary)) && _interface.open(Common::Path(interfaceLibrary));
}

} // End of namespace Ripper
