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
#include "common/file.h"

namespace Chewy {

TempFileArchive::Entry::Entry(const Common::String &name) :
		_name(name), _data(nullptr), _size(0) {
}

TempFileArchive::Entry::~Entry() {
	delete[] _data;
}

void TempFileArchive::Entry::allocate(size_t maxSize) {
	_data = new byte[maxSize];
	_size = maxSize;
}

bool TempFileArchive::FileProxy::eos() const {
	return _pos >= _size;
}

uint32 TempFileArchive::FileProxy::read(void *dataPtr, uint32 dataSize) {
	int bytesRemaining = MIN(_size - _pos, (int32)dataSize);
	Common::copy(_ptr + _pos, _ptr + _pos + bytesRemaining, (byte *)dataPtr);
	_pos += bytesRemaining;
	assert(_pos >= 0 && _pos <= 0xffff);

	return bytesRemaining;
}

uint32 TempFileArchive::FileProxy::write(const void *dataPtr, uint32 dataSize) {
	assert((int32)(_pos + dataSize) <= _size);
	Common::copy((const byte *)dataPtr, (const byte *)dataPtr + dataSize, _ptr + _pos);
	_pos += dataSize;
	assert(_pos >= 0 && _pos <= 0xffff);

	return dataSize;
}

int64 TempFileArchive::FileProxy::pos() const {
	return _pos;
}

int64 TempFileArchive::FileProxy::size() const {
	return _size;
}

bool TempFileArchive::FileProxy::seek(int64 offset, int whence) {
	switch (whence) {
	case SEEK_END:
		offset = size() + offset;
		// Fall through
	case SEEK_SET:
		// Fall through
	default:
		_pos = offset;
		break;
	case SEEK_CUR:
		_pos += offset;
		break;
	}

	assert(_pos >= 0 && _pos <= 0xffff);
	return true;
}

bool TempFileArchive::FileProxy::flush() {
	return true;
}

void TempFileArchive::FileProxy::finalize() {
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

	return new FileProxy(entry->_data, entry->_size);
}

Common::SeekableWriteStream *TempFileArchive::createWriteStreamForMember(const Common::Path &path) {
	const Entry *entry = getEntry(path.toString());
	if (!entry)
		return nullptr;

	return new FileProxy(entry->_data, entry->_size);
}

} // namespace Chewy
