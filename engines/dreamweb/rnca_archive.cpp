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

#include "common/array.h"
#include "common/gzio.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/substream.h"
#include "common/memstream.h"
#include "common/rnc_deco.h"
#include "common/file.h"

#include "dreamweb/rnca_archive.h"

namespace DreamWeb {

RNCAArchive* RNCAArchive::open(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose) {
	FileMap files;

	if (stream->readUint32BE() != 0x524e4341)
		return nullptr;

	uint16 metadataSize1 = stream->readUint16BE();
	stream->readUint16BE(); // No idea
	uint16 metadataSize2 = stream->readUint16BE();
	stream->readByte(); // Always zero

	if (metadataSize1 != metadataSize2 || metadataSize1 < 15)
		return nullptr;

	int headerlessMetadataSize = metadataSize1 - 11;
	byte *metadata = new byte[headerlessMetadataSize];
	stream->read(metadata, headerlessMetadataSize);
	const byte *eptr = metadata;

	while (eptr < metadata + headerlessMetadataSize - 5) {
		const byte *ptr = eptr;
		while (*ptr)
			ptr++;
		Common::String fileName((const char *) eptr, ptr - eptr);
		ptr++;
		uint32 off = READ_BE_UINT32(ptr);
		eptr = ptr + 4;
		files[fileName] = RNCAFileDescriptor(fileName, off);
	}

	delete[] metadata;

	return new RNCAArchive(files, stream, dispose);
}

static Common::String translateName(const Common::Path &path) {
	return Common::normalizePath(path.toString('\\'), '\\');
}

bool RNCAArchive::hasFile(const Common::Path &path) const {
	return _files.contains(translateName(path));
}

int RNCAArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator i = _files.begin(), end = _files.end(); i != end; ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, this)));
	}

	return _files.size();
}

const Common::ArchiveMemberPtr RNCAArchive::getMember(const Common::Path &path) const {
	Common::String translated = translateName(path);
	if (!_files.contains(translated))
		return nullptr;

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(_files.getVal(translated)._fileName, this));
}

// TODO: Make streams stay valid after destruction of archive
Common::SeekableReadStream *RNCAArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String translated = translateName(path);
	if (!_files.contains(translated))
		return nullptr;
	const RNCAFileDescriptor& desc = _files.getVal(translated);
	if (_cache.contains(desc._fileName)) {
		const Common::SharedPtr<CacheEntry> &entry = _cache[desc._fileName];
		if (entry->is_error) {
			return nullptr;
		}
		return new Common::MemoryReadStream(entry->contents, entry->size, DisposeAfterUse::NO);
	}

	_stream->seek(desc._fileDataOffset);

	if (_stream->readUint32BE() != Common::RncDecoder::kRnc1Signature) {
		_cache[desc._fileName] = CacheEntry::error();
		return nullptr;
	}

	// Read unpacked/packed file length
	uint32 unpackLen = _stream->readUint32BE();
	uint32 packLen = _stream->readUint32BE();

	if (unpackLen > 0x7ffff000 || packLen > 0x7ffff000) {
		_cache[desc._fileName] = CacheEntry::error();
		debug("Header error for %s", desc._fileName.c_str());
		return nullptr;
	}

	// Rewind back the header
	_stream->seek(desc._fileDataOffset);
	packLen += 0x12;

	byte *compressedBuffer = new byte[packLen];
	if (_stream->read(compressedBuffer, packLen) != packLen) {
		_cache[desc._fileName] = CacheEntry::error();
		debug("Read error for %s", desc._fileName.c_str());
		return nullptr;		
	}
	byte *uncompressedBuffer = new byte[unpackLen];

	Common::RncDecoder rnc;
	
	if (rnc.unpackM1(compressedBuffer, packLen, uncompressedBuffer) != (int32) unpackLen) {
		_cache[desc._fileName] = CacheEntry::error();
		debug("Unpacking error for %s", desc._fileName.c_str());
		return nullptr;
	}

	byte b = 0;
	for (byte *ptr = uncompressedBuffer; ptr < uncompressedBuffer + unpackLen; ptr++) {
		b += *ptr;
		*ptr = b;
	}
	
	_cache[desc._fileName].reset(new CacheEntry);
	_cache[desc._fileName]->size = unpackLen;
	_cache[desc._fileName]->is_error = false;
	_cache[desc._fileName]->contents = uncompressedBuffer;

	return new Common::MemoryReadStream(uncompressedBuffer, unpackLen, DisposeAfterUse::NO);
}

} // End of namespace DreamWeb
