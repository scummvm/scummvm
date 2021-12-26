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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/memstream.h"
#include "common/file.h"
#include "trecision/trecision.h"
#include "trecision/fastfile.h"
#include "trecision/video.h"

namespace Trecision {

FastFile::FastFile() : Common::Archive(), _stream(nullptr), _compStream(nullptr), _compBuffer(nullptr) {
}

FastFile::~FastFile() {
	close();
}

const FileEntry *FastFile::getEntry(const Common::String &name) const {
	for (Common::Array<FileEntry>::const_iterator it = _fileEntries.begin(); it != _fileEntries.end(); ++it) {
		if (it->name.equalsIgnoreCase(name))
			return it;
	}

	return nullptr;
}

bool FastFile::open(TrecisionEngine *vm, const Common::String &name) {
	close();

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(name);
	if (!stream)
		return false;
	_stream = vm->readEndian(stream);

	int numFiles = _stream->readUint32();
	_fileEntries.resize(numFiles);
	for (int i = 0; i < numFiles; ++i) {
		FileEntry *entry = &_fileEntries[i];
		entry->name = _stream->readString(0, 12);
		entry->offset = _stream->readUint32();
	}

	return true;
}

void FastFile::close() {
	delete _stream;
	_stream = nullptr;
	delete _compStream;
	_compStream = nullptr;
	_fileEntries.clear();
}

bool FastFile::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	const FileEntry *entry = getEntry(name);
	return entry != nullptr;
}

int FastFile::listMembers(Common::ArchiveMemberList &list) const {
	list.clear();
	for (Common::Array<FileEntry>::const_iterator it = _fileEntries.begin(); it != _fileEntries.end(); ++it)
		list.push_back(getMember(it->name));

	return list.size();
}

const Common::ArchiveMemberPtr FastFile::getMember(const Common::Path &path) const {
	Common::String name = path.toString();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *FastFile::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	if (!_stream)
		return nullptr;

	Common::SeekableReadStream *stream = nullptr;
	const FileEntry *entry = getEntry(name);
	if (entry) {
		uint32 size = (entry + 1)->offset - entry->offset;
		if ((int32)(entry->offset + size) <= _stream->size()) {
			// Load data from fast file
			stream = new Common::SeekableSubReadStream(_stream, entry->offset, entry->offset + size);
		}
	}
	if (!stream) {
		// Load data from external file
		Common::File *file = new Common::File();
		if (file->open(name)) {
			stream = file;
		} else {
			delete file;
		}
	}
	if (!stream) {
		warning("FastFile - %s not found", name.c_str());
	}
	return stream;
}

void FastFile::decompress(const uint8 *src, uint32 srcSize, uint8 *dst, uint32 decompSize) {
	const uint16 *sw = (const uint16 *)(src + srcSize);
	uint8 *d = dst;
	uint32 bytesWritten = 0;
	const uint8 *s = src;
	unsigned short ctrl = 0, ctrl_cnt = 1;

	while (s < (const uint8 *)sw) {
		if (!--ctrl_cnt) {
			ctrl = READ_LE_UINT16(--sw);
			ctrl_cnt = 16;
		} else {
			ctrl <<= 1;
		}

		if (ctrl & 0x8000) {
			uint16 foo = READ_LE_UINT16(--sw);
			const uint8 *cs = d - (foo >> 4);

			uint16 num = 16 - (foo & 0xF);

			for (uint16 i = 0; i < num; ++i) {
				*d++ = *cs++;
				++bytesWritten;
				assert(bytesWritten <= decompSize);
			}

			*d++ = *cs++;
			*d++ = *cs;
			bytesWritten += 2;
			assert(bytesWritten <= decompSize);
		} else {
			*d++ = *s++;
			++bytesWritten;
			assert(bytesWritten <= decompSize);
		}
	}
}

#define FAST_COOKIE 0xFA57F00D
Common::SeekableReadStream *FastFile::createReadStreamForCompressedMember(const Common::String &name) {
	Common::SeekableReadStream *ff = createReadStreamForMember(name);
	if (ff == nullptr)
		error("createReadStreamForCompressedMember - File not found %s", name.c_str());

	const int32 dataSize = ff->size() - 8;

	const uint32 signature = ff->readUint32LE();
	if (signature != FAST_COOKIE)
		error("createReadStreamForCompressedMember - %s has a bad signature and can't be loaded", name.c_str());

	const int32 decompSize = ff->readSint32LE();

	uint8 *ibuf = new uint8[dataSize];
	const int32 realSize = MAX(dataSize, decompSize) + 8 + 100; // add extra padding for the decompressor

	delete _compStream;
	_compBuffer = (uint8 *) malloc (realSize);

	ff->read(ibuf, dataSize);
	delete ff;

	if (dataSize < decompSize)
		decompress(ibuf, dataSize, _compBuffer, realSize);
	else
		memcpy(_compBuffer, ibuf, dataSize);

	delete[] ibuf;

	_compStream = new Common::MemoryReadStream(_compBuffer, realSize, DisposeAfterUse::YES);
	return _compStream;
}
} // End of namespace Trecision
