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

#include "chewy/temp_file.h"

namespace Chewy {

bool TempFileArchive::FileProxy::eos() const {
	return _src->eos();
}

uint32 TempFileArchive::FileProxy::read(void *dataPtr, uint32 dataSize) {
	return _src->read(dataPtr, dataSize);
}

int64 TempFileArchive::FileProxy::pos() const {
	return _src->pos();
}

int64 TempFileArchive::FileProxy::size() const {
	return _src->size();
}

bool TempFileArchive::FileProxy::seek(int64 offset, int whence) {
	return _src->seek(offset, whence);
}

uint32 TempFileArchive::FileProxy::write(const void *dataPtr, uint32 dataSize) {
	return _src->write(dataPtr, dataSize);
}

bool TempFileArchive::FileProxy::flush() {
	return _src->flush();
}

void TempFileArchive::FileProxy::finalize() {
	_src->finalize();
}


TempFileArchive::TempFileArchive() {
}

const TempFileArchive::Entry *TempFileArchive::getEntry(
		const Common::String &name) const {
	for (EntryList::const_iterator it = _files.begin(); it != _files.end(); ++it) {
		const Entry &e = *it;
		if (e._name.equalsIgnoreCase(name))
			return &e;
	}

	return nullptr;
}

bool TempFileArchive::hasFile(const Common::Path &path) const {
	return getEntry(path.toString()) != nullptr;
}

int TempFileArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;
	for (EntryList::const_iterator it = _files.begin(); it != _files.end(); ++it) {
		const Entry &e = *it;
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(e._name, this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr TempFileArchive::getMember(const Common::Path &path) const {
	Common::String name = path.toString();
	if (getEntry(name))
		return Common::ArchiveMemberPtr(
			new Common::GenericArchiveMember(name, this));

	return Common::ArchiveMemberPtr();
}

Common::SeekableReadStream *TempFileArchive::createReadStreamForMember(
		const Common::Path &path) const {
	const Entry *entry = getEntry(path.toString());
	if (!entry)
		return nullptr;

	Common::MemoryReadWriteStream *stream =
		const_cast<Common::MemoryReadWriteStream *>(&entry->_stream);
	stream->seek(0, SEEK_SET);
	return new FileProxy(stream);
}

Common::SeekableWriteStream *TempFileArchive::createWriteStreamForMember(const Common::Path &path) {
	const Entry *entry = getEntry(path.toString());
	if (!entry)
		return nullptr;

	Common::MemoryReadWriteStream *stream =
		const_cast<Common::MemoryReadWriteStream *>(&entry->_stream);
	stream->seek(0, SEEK_SET);
	return new FileProxy(stream);
}

}
