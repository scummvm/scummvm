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
#include "common/compression/gzio.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/substream.h"
#include "common/memstream.h"
#include "common/compression/rnc_deco.h"
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

bool RNCAArchive::hasFile(const Common::Path &path) const {
	return _files.contains(translatePath(path));
}

int RNCAArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator i = _files.begin(), end = _files.end(); i != end; ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
	}

	return _files.size();
}

const Common::ArchiveMemberPtr RNCAArchive::getMember(const Common::Path &path) const {
	Common::String translated = translatePath(path);
	if (!_files.contains(translated))
		return nullptr;

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(_files.getVal(translated)._fileName, *this));
}

Common::SharedArchiveContents RNCAArchive::readContentsForPath(const Common::String& translated) const {
	if (!_files.contains(translated))
		return Common::SharedArchiveContents();
	const RNCAFileDescriptor& desc = _files.getVal(translated);
	_stream->seek(desc._fileDataOffset);

	if (_stream->readUint32BE() != Common::RncDecoder::kRnc1Signature) {
		return Common::SharedArchiveContents();
	}

	// Read unpacked/packed file length
	uint32 unpackLen = _stream->readUint32BE();
	uint32 packLen = _stream->readUint32BE();

	if (unpackLen > 0x7ffff000 || packLen > 0x7ffff000) {
		debug("Header error for %s", desc._fileName.c_str());
		return Common::SharedArchiveContents();
	}

	// Rewind back the header
	_stream->seek(desc._fileDataOffset);
	packLen += 0x12;

	byte *compressedBuffer = new byte[packLen];
	if (_stream->read(compressedBuffer, packLen) != packLen) {
		debug("Read error for %s", desc._fileName.c_str());
		return Common::SharedArchiveContents();
	}
	byte *uncompressedBuffer = new byte[unpackLen];

	Common::RncDecoder rnc;

	if (rnc.unpackM1(compressedBuffer, packLen, uncompressedBuffer) != (int32) unpackLen) {
		debug("Unpacking error for %s", desc._fileName.c_str());
		return Common::SharedArchiveContents();
	}

	byte b = 0;
	for (byte *ptr = uncompressedBuffer; ptr < uncompressedBuffer + unpackLen; ptr++) {
		b += *ptr;
		*ptr = b;
	}

	return Common::SharedArchiveContents(uncompressedBuffer, unpackLen);
}

} // End of namespace DreamWeb
