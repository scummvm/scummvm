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
#include "common/endian.h"
#include "common/config-manager.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "queen/resource.h"

namespace Queen {


const char *const Resource::_tableFilename = "queen.tbl";

static int compareResourceEntry(const void *a, const void *b) {
	const char *filename = (const char *)a;
	const ResourceEntry *entry = (const ResourceEntry *)b;
	return strcmp(filename, entry->filename);
}

Resource::Resource()
	: _resourceEntries(0), _resourceTable(nullptr) {
	memset(&_version, 0, sizeof(_version));

	_currentResourceFileNum = 1;
	if (!_resourceFile.open("queen.1c")) {
		if (!_resourceFile.open("queen.1")) {
			error("Could not open resource file 'queen.1[c]'");
		}
	}
	if (!detectVersion(&_version, &_resourceFile)) {
		error("Unable to detect game version");
	}

	if (_version.features & GF_REBUILT) {
		readTableEntries(&_resourceFile);
	} else {
		readTableFile(_version.queenTblVersion, _version.queenTblOffset);
	}

	checkJASVersion();
	debug(5, "Detected game version: %s, which has %d resource entries", _version.str, _resourceEntries);
}

Resource::~Resource() {
	_resourceFile.close();

	if (_resourceTable != _resourceTablePEM10)
		delete[] _resourceTable;
}

ResourceEntry *Resource::resourceEntry(const char *filename) const {
	assert(filename[0] && strlen(filename) < 14);

	Common::String entryName(filename);
	entryName.toUppercase();

	ResourceEntry *re = nullptr;
	re = (ResourceEntry *)bsearch(entryName.c_str(), _resourceTable, _resourceEntries, sizeof(ResourceEntry), compareResourceEntry);
	return re;
}

uint8 *Resource::loadFile(const char *filename, uint32 skipBytes, uint32 *size) {
	debug(7, "Resource::loadFile('%s')", filename);
	ResourceEntry *re = resourceEntry(filename);
	assert(re != nullptr);
	uint32 sz = re->size - skipBytes;
	if (size != nullptr) {
		*size = sz;
	}
	byte *dstBuf = new byte[sz];
	seekResourceFile(re->bundle, re->offset + skipBytes);
	_resourceFile.read(dstBuf, sz);
	return dstBuf;
}

void Resource::loadTextFile(const char *filename, Common::StringArray &stringList) {
	debug(7, "Resource::loadTextFile('%s')", filename);
	ResourceEntry *re = resourceEntry(filename);
	assert(re != nullptr);
	seekResourceFile(re->bundle, re->offset);
	Common::SeekableSubReadStream stream(&_resourceFile, re->offset, re->offset + re->size);
	while (true) {
		Common::String tmp = stream.readLine();
		if (stream.eos() || stream.err())
			break;
		stringList.push_back(tmp);
	}
}

void Resource::checkJASVersion() {
	if (_version.platform == Common::kPlatformAmiga) {
		// don't bother verifying the JAS version string with these versions,
		// it will be done at the end of Logic::readQueenJas, anyway
		return;
	}
	ResourceEntry *re = resourceEntry("QUEEN.JAS");
	assert(re != nullptr);
	uint32 offset = re->offset;
	if (isDemo())
		offset += JAS_VERSION_OFFSET_DEMO;
	else if (isInterview())
		offset += JAS_VERSION_OFFSET_INTV;
	else
		offset += JAS_VERSION_OFFSET_PC;
	seekResourceFile(re->bundle, offset);

	char versionStr[6];
	_resourceFile.read(versionStr, 6);
	if (strcmp(_version.str, versionStr))
		error("Verifying game version failed! (expected: '%s', found: '%s')", _version.str, versionStr);
}

void Resource::seekResourceFile(int num, uint32 offset) {
	if (_currentResourceFileNum != num) {
		debug(7, "Opening resource file %d, current %d", num, _currentResourceFileNum);
		_resourceFile.close();
		char name[20];
		Common::sprintf_s(name, "queen.%d", num);
		if (!_resourceFile.open(name)) {
			error("Could not open resource file '%s'", name);
		}
		_currentResourceFileNum = num;
	}
	_resourceFile.seek(offset);
}

void Resource::readTableFile(uint8 version, uint32 offset) {
	Common::File tableFile;
	tableFile.open(_tableFilename);
	if (tableFile.isOpen() && tableFile.readUint32BE() == MKTAG('Q','T','B','L')) {
		uint32 tableVersion = tableFile.readUint32BE();
		if (version > tableVersion) {
			error("The game you are trying to play requires version %d of queen.tbl, "
			      "you have version %d ; please update it", version, tableVersion);
		}
		tableFile.seek(offset);
		readTableEntries(&tableFile);
	} else {
		// check if it is the english floppy version, for which we have a hardcoded version of the table
		if (strcmp(_version.str, "PEM10") == 0) {
			_resourceEntries = 1076;
			_resourceTable = _resourceTablePEM10;
		} else {
			error("Could not find tablefile '%s'", _tableFilename);
		}
	}
}

void Resource::readTableEntries(Common::File *file) {
	_resourceEntries = file->readUint16BE();
	_resourceTable = new ResourceEntry[_resourceEntries];
	for (uint16 i = 0; i < _resourceEntries; ++i) {
		ResourceEntry *re = &_resourceTable[i];
		file->read(re->filename, 12);
		re->filename[12] = '\0';
		re->bundle = file->readByte();
		re->offset = file->readUint32BE();
		re->size = file->readUint32BE();
	}
}

Common::File *Resource::findSound(const char *filename, uint32 *size) {
	assert(strstr(filename, ".SB") != nullptr || strstr(filename, ".AMR") != nullptr || strstr(filename, ".INS") != nullptr);
	ResourceEntry *re = resourceEntry(filename);
	if (re) {
		*size = re->size;
		seekResourceFile(re->bundle, re->offset);
		return &_resourceFile;
	}
	return nullptr;
}

} // End of namespace Queen
