/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/endian.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/memstream.h"

#include "engines/grim/grim.h"
#include "engines/grim/lab.h"

namespace Grim {

LabEntry::LabEntry()
	: _name(Common::String()), _offset(0), _len(0), _parent(NULL) {
}

LabEntry::LabEntry(Common::String name, uint32 offset, uint32 len, Lab *parent)
	: _offset(offset), _len(len), _parent(parent) {
	_name = name;
	_name.toLowercase();
}

Common::SeekableReadStream *LabEntry::createReadStream() const {
	return _parent->createReadStreamForMember(_name);
}

bool Lab::open(const Block *lab) {
	_f = new Common::MemoryReadStream((byte *)lab->getData(), lab->getLen());
	_labFileName = "";
	_memLab = lab;

	return loadLab();
}

bool Lab::open(const Common::String &filename) {
	_labFileName = filename;

	close();

	Common::File *file = new Common::File();
	if (!file->open(filename)) {
		delete file;
		return false;
	}

	_f = (Common::SeekableReadStream *)file;
	return loadLab();
}

bool Lab::loadLab() {
	if (_f->readUint32BE() != MKTAG('L','A','B','N')) {
		close();
		return false;
	}

	_f->readUint32LE(); // version

	if (g_grim->getGameType() == GType_GRIM)
		parseGrimFileTable();
	else
		parseMonkey4FileTable();

	return true;
}

void Lab::parseGrimFileTable() {
	uint32 entryCount = _f->readUint32LE();
	uint32 stringTableSize = _f->readUint32LE();

	char *stringTable = new char[stringTableSize];
	_f->seek(16 * (entryCount + 1));
	_f->read(stringTable, stringTableSize);
	_f->seek(16);

	for (uint32 i = 0; i < entryCount; i++) {
		int fnameOffset = _f->readUint32LE();
		int start = _f->readUint32LE();
		int size = _f->readUint32LE();
		_f->readUint32LE();

		Common::String fname = stringTable + fnameOffset;
		fname.toLowercase();

		LabEntry *entry = new LabEntry(fname, start, size, this);
		_entries[fname] = LabEntryPtr(entry);
	}

	delete[] stringTable;
}

void Lab::parseMonkey4FileTable() {
	uint32 entryCount = _f->readUint32LE();
	uint32 stringTableSize = _f->readUint32LE();
	uint32 stringTableOffset = _f->readUint32LE() - 0x13d0f;

	char *stringTable = new char[stringTableSize];
	_f->seek(stringTableOffset);
	_f->read(stringTable, stringTableSize);
	_f->seek(20);

	// Decrypt the string table
	for (uint32 i = 0; i < stringTableSize; i++)
		if (stringTable[i] != 0)
			stringTable[i] ^= 0x96;

	for (uint32 i = 0; i < entryCount; i++) {
		int fnameOffset = _f->readUint32LE();
		int start = _f->readUint32LE();
		int size = _f->readUint32LE();
		_f->readUint32LE();

		char *str = stringTable + fnameOffset;
		int len = strlen(str);

		for (int l = 0; l < len; ++l) {
			if (str[l] == '\\')
				str[l] = '/';
		}
		Common::String fname = str;
		fname.toLowercase();

		LabEntry *entry = new LabEntry(fname, start, size, this);
		_entries[fname] = LabEntryPtr(entry);
	}

	delete[] stringTable;
}

bool Lab::hasFile(const Common::String &filename) {
	Common::String fname(filename);
	fname.toLowercase();
	return _entries.contains(fname);
}

bool Lab::hasFile(const Common::String &filename) const {
	Common::String fname(filename);
	fname.toLowercase();
	return _entries.contains(fname);
}

int Lab::listMembers(Common::ArchiveMemberList &list) {
	int count = 0;

	for (LabMap::const_iterator i = _entries.begin(); i != _entries.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(i->_value));
		++count;
	}

	return count;
}

Common::ArchiveMemberPtr Lab::getMember(const Common::String &name) {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	Common::String fname(name);
	fname.toLowercase();
	return _entries[fname];
}

Block *Lab::getFileBlock(const Common::String &filename) const {
	if (!hasFile(filename))
		return 0;

	LabEntryPtr i = _entries[filename];
	Block *blockData;

	/*If the whole Lab has been loaded into ram, we return a pointer
	of requested data directly, without copying them. Otherwise read them
	from the disk.*/
	if(_memLab) {
		const char *data = _memLab->getData() + i->_offset;
		blockData = new Block(data, i->_len, DisposeAfterUse::NO);
	} else {
		_f->seek(i->_offset, SEEK_SET);
		char *data = new char[i->_len];
		_f->read(data, i->_len);
		blockData = new Block(data, i->_len, DisposeAfterUse::YES);
	}

	return blockData;
}

Common::SeekableReadStream *Lab::createReadStreamForMember(const Common::String &filename) const {
	if (!hasFile(filename))
		return 0;

	Common::String fname(filename);
	fname.toLowercase();
	LabEntryPtr i = _entries[fname];

	/*If the whole Lab has been loaded into ram, we return a MemoryReadStream
	that map requested data directly, without copying them. Otherwise open a new
	stream from disk.*/
	if(_memLab)
		return new Common::MemoryReadStream((byte*)(_memLab->getData() + i->_offset), i->_len, DisposeAfterUse::NO);

	Common::File *file = new Common::File();
	file->open(_labFileName);
	return new Common::SeekableSubReadStream(file, i->_offset, i->_offset + i->_len, DisposeAfterUse::YES );
}

uint32 Lab::getFileLength(const Common::String &filename) const {
	if(!hasFile(filename))
		return 0;

	return _entries[filename]->_len;
}

void Lab::close() {
	delete _f;
	_f = NULL;

	if(_memLab)
		delete _memLab;

	_entries.clear();
}

} // end of namespace Grim
