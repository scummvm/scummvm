/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/file.h"
#include "common/substream.h"
#include "common/memstream.h"

#include "engines/grim/grim.h"
#include "engines/grim/lab.h"

namespace Grim {

LabEntry::LabEntry(const Common::String &name, uint32 offset, uint32 len, Lab *parent) :
		_offset(offset), _len(len), _parent(parent), _name(name) {
	_name.toLowercase();
}

Common::SeekableReadStream *LabEntry::createReadStream() const {
	return _parent->createReadStreamForMember(_name);
}

Lab::Lab() {
	_stream = nullptr;
}

Lab::~Lab() {
	delete _stream;
}

bool Lab::open(const Common::String &filename, bool keepStream) {
	_labFileName = filename;

	bool result = true;

	Common::File *file = new Common::File();
	if (!file->open(filename) || file->readUint32BE() != MKTAG('L','A','B','N')) {
		result = false;
	} else {
		file->readUint32LE(); // version

		if (g_grim->getGameType() == GType_GRIM)
			parseGrimFileTable(file);
		else
			parseMonkey4FileTable(file);
	}
	if (result && keepStream) {
		file->seek(0, SEEK_SET);
		byte *data = static_cast<byte*>(malloc(sizeof(byte) * file->size()));
		file->read(data, file->size());
		_stream = new Common::MemoryReadStream(data, file->size(), DisposeAfterUse::YES);
	}
	delete file;

	return result;
}

void Lab::parseGrimFileTable(Common::File *file) {
	uint32 entryCount = file->readUint32LE();
	uint32 stringTableSize = file->readUint32LE();

	char *stringTable = new char[stringTableSize];
	file->seek(16 * (entryCount + 1));
	file->read(stringTable, stringTableSize);
	file->seek(16);

	int32 filesize = file->size();

	for (uint32 i = 0; i < entryCount; i++) {
		int fnameOffset = file->readUint32LE();
		int start = file->readUint32LE();
		int size = file->readUint32LE();
		file->readUint32LE();

		Common::String fname = stringTable + fnameOffset;
		fname.toLowercase();

		if (start + size > filesize)
			error("File \"%s\" past the end of lab \"%s\". Your game files may be corrupt.", fname.c_str(), _labFileName.c_str());

		LabEntry *entry = new LabEntry(fname, start, size, this);
		_entries[fname] = LabEntryPtr(entry);
	}

	delete[] stringTable;
}

void Lab::parseMonkey4FileTable(Common::File *file) {
	uint32 entryCount = file->readUint32LE();
	uint32 stringTableSize = file->readUint32LE();
	uint32 stringTableOffset = file->readUint32LE() - 0x13d0f;

	char *stringTable = new char[stringTableSize];
	file->seek(stringTableOffset);
	file->read(stringTable, stringTableSize);
	file->seek(20);

	int32 filesize = file->size();

	// Decrypt the string table
	for (uint32 i = 0; i < stringTableSize; i++)
		if (stringTable[i] != 0)
			stringTable[i] ^= 0x96;

	for (uint32 i = 0; i < entryCount; i++) {
		int fnameOffset = file->readUint32LE();
		int start = file->readUint32LE();
		int size = file->readUint32LE();
		file->readUint32LE();

		char *str = stringTable + fnameOffset;
		int len = strlen(str);

		for (int l = 0; l < len; ++l) {
			if (str[l] == '\\')
				str[l] = '/';
		}
		Common::String fname = str;
		fname.toLowercase();

		if (start + size > filesize)
			error("File \"%s\" past the end of lab \"%s\". Your game files may be corrupt.", fname.c_str(), _labFileName.c_str());

		LabEntry *entry = new LabEntry(fname, start, size, this);
		_entries[fname] = LabEntryPtr(entry);
	}

	delete[] stringTable;
}

bool Lab::hasFile(const Common::String &filename) const {
	Common::String fname(filename);
	fname.toLowercase();
	return _entries.contains(fname);
}

int Lab::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (LabMap::const_iterator i = _entries.begin(); i != _entries.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(i->_value));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr Lab::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	Common::String fname(name);
	fname.toLowercase();
	return _entries[fname];
}

Common::SeekableReadStream *Lab::createReadStreamForMember(const Common::String &filename) const {
	if (!hasFile(filename))
		return nullptr;

	Common::String fname(filename);
	fname.toLowercase();
	LabEntryPtr i = _entries[fname];

	if (!_stream) {
		Common::File *file = new Common::File();
		file->open(_labFileName);
		return new Common::SeekableSubReadStream(file, i->_offset, i->_offset + i->_len, DisposeAfterUse::YES);
	} else {
		byte *data = static_cast<byte*>(malloc(sizeof(byte) * i->_len));
		_stream->seek(i->_offset, SEEK_SET);
		_stream->read(data, i->_len);
		return new Common::MemoryReadStream(data, i->_len, DisposeAfterUse::YES);
	}
}

} // end of namespace Grim
