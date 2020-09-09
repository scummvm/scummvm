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

#include "ultima/shared/engine/resources.h"
#include "ultima/shared/early/font_resources.h"
#include "common/endian.h"

#ifndef RELEASE_BUILD
#include "ultima/ultima1/core/resources.h"
#endif

namespace Ultima {
namespace Shared {

/*-------------------------------------------------------------------*/

bool Resources::open() {
	// Save locally constructred resources to the archive manager for access
	Shared::FontResources sharedFonts(this);
	sharedFonts.save();
#ifndef RELEASE_BUILD
	Ultima1::GameResources u1Data(this);
	u1Data.save();
#endif

	return true;
}

void Resources::addResource(const Common::String &name, const byte *data, size_t size) {
	// Add a new entry to the local resources list for the passed data
	_localResources.push_back(LocalResource());
	LocalResource &lr = _localResources[_localResources.size() - 1];

	lr._name = name;
	lr._data.resize(size);
	Common::copy(data, data + size, &lr._data[0]);
}

bool Resources::hasFile(const Common::String &name) const {
	for (uint idx = 0; idx < _localResources.size(); ++idx)
		if (!_localResources[idx]._name.compareToIgnoreCase(name))
			return true;

	return false;
}

int Resources::listMembers(Common::ArchiveMemberList &list) const {
	for (uint idx = 0; idx < _localResources.size(); ++idx) {
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(_localResources[idx]._name, this)));
	}

	return _localResources.size();
}

const Common::ArchiveMemberPtr Resources::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *Resources::createReadStreamForMember(const Common::String &name) const {
	for (uint idx = 0; idx < _localResources.size(); ++idx) {
		const LocalResource &lr = _localResources[idx];
		if (!lr._name.compareToIgnoreCase(name))
			return new Common::MemoryReadStream(&lr._data[0], lr._data.size());
	}

	return nullptr;
}

/*-------------------------------------------------------------------*/

void Resources::FileResource::load(File &f) {
	_name = f.readString();
	_offset = f.readUint32LE();
	_size = f.readUint16LE();
}

/*-------------------------------------------------------------------*/

ResourceFile::ResourceFile(const Common::String &filename) : _filename(filename), _bufferP(_buffer) {
	Common::fill(_buffer, _buffer + STRING_BUFFER_SIZE, 0);
}

void ResourceFile::load() {
	_file.open(_filename);
	synchronize();
	_file.close();
}

void ResourceFile::syncString(const char *&str) {
	str = _bufferP;
	do {
		*_bufferP = _file.readByte();
	} while (*_bufferP++);

	assert(_bufferP < (_buffer + STRING_BUFFER_SIZE));
}

void ResourceFile::syncStrings(const char **str, size_t count) {
	uint tag = _file.readUint32LE();
	assert(tag == MKTAG(count, 0, 0, 0));

	for (size_t idx = 0; idx < count; ++idx)
		syncString(str[idx]);
}

void ResourceFile::syncStrings2D(const char **str, size_t count1, size_t count2) {
	uint tag = _file.readUint32LE();
	assert(tag == MKTAG(count1, count2, 0, 0));

	for (size_t idx = 0; idx < count1 * count2; ++idx)
		syncString(str[idx]);
}

void ResourceFile::syncNumber(int &val) {
	val = _file.readSint32LE();
}

void ResourceFile::syncNumbers(int *vals, size_t count) {
	uint tag = _file.readUint32LE();
	assert(tag == MKTAG(count, 0, 0, 0));
	for (size_t idx = 0; idx < count; ++idx)
		vals[idx] = _file.readSint32LE();
}

void ResourceFile::syncNumbers2D(int *vals, size_t count1, size_t count2) {
	uint tag = _file.readUint32LE();
	assert(tag == MKTAG(count1, count2, 0, 0));
	for (size_t idx = 0; idx < count1 * count2; ++idx)
		vals[idx] = _file.readSint32LE();
}

void ResourceFile::syncNumbers3D(int *vals, size_t count1, size_t count2, size_t count3) {
	uint tag = _file.readUint32LE();
	assert(tag == MKTAG(count1, count2, count3, 0));
	for (size_t idx = 0; idx < count1 * count2 * count3; ++idx)
		vals[idx] = _file.readSint32LE();
}

void ResourceFile::syncBytes(byte *vals, size_t count) {
	uint tag = _file.readUint32LE();
	assert(tag == MKTAG(count, 0, 0, 0));
	_file.read(vals, count);
}

void ResourceFile::syncBytes2D(byte *vals, size_t count1, size_t count2) {
	uint tag = _file.readUint32LE();
	assert(tag == MKTAG(count1, count2, 0, 0));
	_file.read(vals, count1 * count2);
}

/*-------------------------------------------------------------------*/

void LocalResourceFile::save() {
	synchronize();
	_file.finalize();
	_owner->addResource(_filename, _file.getData(), _file.size());
}

void LocalResourceFile::syncString(const char *&str) {
	if (!_owner) {
		ResourceFile::syncString(str);
	} else {
		_file.writeString(str);
		_file.writeByte('\0');
	}
}

void LocalResourceFile::syncStrings(const char **str, size_t count) {
	if (!_owner) {
		ResourceFile::syncStrings(str, count);
	} else {
		_file.writeUint32LE(MKTAG(count, 0, 0, 0));
		for (size_t idx = 0; idx < count; ++idx)
			syncString(str[idx]);
	}
}

void LocalResourceFile::syncStrings2D(const char **str, size_t count1, size_t count2) {
	if (!_owner) {
		ResourceFile::syncStrings2D(str, count1, count2);
	} else {
		_file.writeUint32LE(MKTAG(count1, count2, 0, 0));
		for (size_t idx = 0; idx < count1 * count2; ++idx)
			syncString(str[idx]);
	}
}

void LocalResourceFile::syncNumber(int &val) {
	if (!_owner)
		ResourceFile::syncNumber(val);
	else
		_file.writeUint32LE(val);
}

void LocalResourceFile::syncNumbers(int *vals, size_t count) {
	if (!_owner) {
		ResourceFile::syncNumbers(vals, count);
	} else {
		_file.writeUint32LE(MKTAG(count, 0, 0, 0));
		for (size_t idx = 0; idx < count; ++idx)
			_file.writeUint32LE(vals[idx]);
	}
}

void LocalResourceFile::syncNumbers2D(int *vals, size_t count1, size_t count2) {
	if (!_owner) {
		ResourceFile::syncNumbers2D(vals, count1, count2);
	} else {
		_file.writeUint32LE(MKTAG(count1, count2, 0, 0));
		for (size_t idx = 0; idx < count1 * count2; ++idx)
			_file.writeUint32LE(vals[idx]);
	}
}

void LocalResourceFile::syncNumbers3D(int *vals, size_t count1, size_t count2, size_t count3) {
	if (!_owner) {
		ResourceFile::syncNumbers3D(vals, count1, count2, count3);
	} else {
		_file.writeUint32LE(MKTAG(count1, count2, count3, 0));
		for (size_t idx = 0; idx < count1 * count2 * count3; ++idx)
			_file.writeUint32LE(vals[idx]);
	}
}

void LocalResourceFile::syncBytes(byte *vals, size_t count) {
	if (!_owner) {
		ResourceFile::syncBytes(vals, count);
	} else {
		_file.writeUint32LE(MKTAG(count, 0, 0, 0));
		_file.write(vals, count);
	}
}

void LocalResourceFile::syncBytes2D(byte *vals, size_t count1, size_t count2) {
	if (!_owner) {
		ResourceFile::syncBytes2D(vals, count1, count2);
	} else {
		_file.writeUint32LE(MKTAG(count1, count2, 0, 0));
		_file.write(vals, count1 * count2);
	}
}

} // End of namespace Shared
} // End of namespace Ultima
