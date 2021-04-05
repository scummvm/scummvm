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

#include "common/scummsys.h"
#include "trecision/trecision.h"
#include "trecision/nl/sys/ffile.h"

#include "common/str.h"
#include "common/file.h"
#include "common/substream.h"
#include "trecision/video.h"

namespace Trecision {

/* -----------------19/01/98 17.12-------------------
 * Compare
 *
 * bsearch comparison routine
 * --------------------------------------------------*/
int Compare(const void *p1, const void *p2) {
	SFileEntry *p1c = (SFileEntry *)p1, *p2c = (SFileEntry*)p2;
	return (scumm_stricmp((p1c)->name, (p2c)->name));
}

FastFile::FastFile() : Common::Archive(), _stream(nullptr) {
}

FastFile::~FastFile() {
	close();
}

const FastFile::FileEntry *FastFile::getEntry(const Common::String &name) const {
	FileEntry key;
	strncpy(key.name, name.c_str(), ARRAYSIZE(key.name));

	FileEntry *entry = (FileEntry *)bsearch(&key, &_fileEntries[0], _fileEntries.size(), sizeof(FileEntry), Compare);
	return entry;
}

bool FastFile::open(const Common::String &name) {
	close();

	_stream = SearchMan.createReadStreamForMember(name);
	if (!_stream)
		return false;

	int numFiles = _stream->readUint32LE();
	_fileEntries.resize(numFiles);
	for (int i = 0; i < numFiles; i++) {
		FileEntry *entry = &_fileEntries[i];
		_stream->read(entry->name, ARRAYSIZE(entry->name));
		entry->offset = _stream->readUint32LE();
	}

	return true;
}

void FastFile::close() {
	delete _stream;
	_stream = nullptr;
	_fileEntries.clear();
}

bool FastFile::hasFile(const Common::String &name) const {
	const FileEntry *entry = getEntry(name);
	return entry != nullptr;
}

int FastFile::listMembers(Common::ArchiveMemberList &list) const {
	list.clear();
	for (Common::Array<FileEntry>::const_iterator i = _fileEntries.begin(); i != _fileEntries.end(); i++)
		list.push_back(getMember(i->name));

	return list.size();
}

const Common::ArchiveMemberPtr FastFile::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *FastFile::createReadStreamForMember(const Common::String &name) const {
	if (!_stream)
		return nullptr;

	const FileEntry *entry = getEntry(name);
	if (entry) {
		uint32 size = (entry + 1)->offset - entry->offset;
		return new Common::SeekableSubReadStream(_stream, entry->offset, entry->offset + size);
	}

	return nullptr;
}

} // End of namespace Trecision
