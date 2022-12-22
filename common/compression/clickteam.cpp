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
#include "common/compression/clickteam.h"
#include "common/compression/gzio.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/substream.h"
#include "common/memstream.h"

#define STUB_SIZE 0x16000
#define FLAG_COMPRESSED 1

namespace Common {

ClickteamInstaller::ClickteamFileDescriptor::ClickteamFileDescriptor(const ClickteamTag& contentsTag, uint32 off)
  : _fileDataOffset(0), _fileDescriptorOffset(0), _compressedSize(0), _uncompressedSize(0) {
	uint32 stringsOffset = 36;
	byte *tag = contentsTag._contents + off;
	uint32 lmax = contentsTag._size - off;
	if (lmax < 0x24)
		return;
	uint16 ls = READ_LE_UINT32(tag), l;
	if (ls < 0x24)
		return;

	l = MIN((uint32)ls, lmax);

	uint16 flags = READ_LE_UINT32(tag+22);
	if (flags & 6)
		stringsOffset += 0x12;
	if (flags & 8)
		stringsOffset += 0x18;
	if (stringsOffset >= l) {
		return;
	}
	_fileDataOffset = READ_LE_UINT32(tag + 6);
	_compressedSize = READ_LE_UINT32(tag + 10);
	_uncompressedSize = READ_LE_UINT32(tag + 14);
	_expectedCRC = READ_LE_UINT32(tag + 18);
	char *strings = (char *)tag + stringsOffset;
	char *p;
	for (p = strings; p < (char*)tag + lmax && *p; p++);
	_fileName = Common::String(strings, p - strings);
	_fileDescriptorOffset = off;
}

ClickteamInstaller::ClickteamTag* ClickteamInstaller::getTag(ClickteamTagId tagId) const {
	return _tags.getValOrDefault((uint16) tagId).get();
}

static uint32 computeCRC(byte *buf, uint32 sz, uint32 previous) {
	uint32 cur = previous;
	byte *ptr = buf;
	uint32 i;

	for (i = 0; i < (sz & ~3); i += 4, ptr += 4)
		cur = READ_LE_UINT32(ptr) + (cur >> 31) + (cur << 1);

	for (; i < sz; i++, ptr++)
		cur = (*ptr) + (cur >> 31) + (cur << 1);

	return cur;
}

bool checkStubAndComputeCRC1(Common::SeekableReadStream *stream, uint32 &crc) {
	if (stream->size() <= STUB_SIZE) {
		return false;
	}

	byte *stub = new byte[STUB_SIZE];
	static const byte BLOCK1_MAGIC_START[] = { 0x77, 0x77, 0x49, 0x4e, 0x53, 0x53 };
	static const byte BLOCK1_MAGIC_END[] = { 0x77, 0x77, 0x49, 0x4e, 0x53, 0x45 };

	stream->seek(0);
	stream->read(stub, STUB_SIZE);

	byte *ptr;

	for (ptr = stub; ptr < stub + STUB_SIZE - sizeof(BLOCK1_MAGIC_START); ptr++) {
		if (memcmp(ptr, BLOCK1_MAGIC_START, sizeof(BLOCK1_MAGIC_START)) == 0)
			break;
	}

	if (ptr == stub + STUB_SIZE - sizeof(BLOCK1_MAGIC_START)) {
		delete[] stub;
		return false;
	}

	byte *block1start = ptr;
	ptr += sizeof(BLOCK1_MAGIC_START);

	for (; ptr < stub + STUB_SIZE - sizeof(BLOCK1_MAGIC_END); ptr++) {
		if (memcmp(ptr, BLOCK1_MAGIC_END, sizeof(BLOCK1_MAGIC_END)) == 0)
			break;
	}

	if (ptr == stub + STUB_SIZE - sizeof(BLOCK1_MAGIC_END)) {
		delete[] stub;
		return false;
	}

	byte *block1end = ptr;
	crc = computeCRC(block1start, block1end - block1start, 0);
	delete[] stub;
	return true;
}

struct TagHead {
	uint16 id;
	uint16 flags;
	uint32 compressedLen;
};

ClickteamInstaller* ClickteamInstaller::open(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose) {
	Common::HashMap<Common::String, ClickteamFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> files;
	HashMap<uint16, Common::SharedPtr<ClickteamTag>> tags;
	uint32 crc_xor;

	if (!checkStubAndComputeCRC1(stream, crc_xor))
		return nullptr;

	int64 block3_offset = -1, block3_len = 0;

	while (!stream->eos()) {
		TagHead tagHead;
		stream->read(&tagHead, sizeof(tagHead));
		uint16 tagId = FROM_LE_16(tagHead.id);
		uint16 flags = FROM_LE_16(tagHead.flags);
		uint32 compressedPayloadLen = FROM_LE_32(tagHead.compressedLen);
		if (tagId == 0x7f7f) {
			stream->skip(4);
			block3_offset = stream->pos();
			block3_len = stream->readUint32LE();
			break;
		}
		if (compressedPayloadLen == 0) {
			break;
		}
		byte *compressedPayload = new byte[compressedPayloadLen];
		stream->read(compressedPayload, compressedPayloadLen);

		ClickteamTag *tag;

		if (flags & FLAG_COMPRESSED) {
			if (compressedPayloadLen < 4) {
				delete[] compressedPayload;
				continue;
			}
			uint32 uncompressedPayloadLen = READ_LE_UINT32(compressedPayload);
			byte *uncompressedPayload = new byte[uncompressedPayloadLen];
			int32 ret = Common::GzioReadStream::clickteamDecompress(uncompressedPayload,
										  uncompressedPayloadLen,
										  compressedPayload + 4,
										  compressedPayloadLen - 4);
			delete[] compressedPayload;
			if (ret < 0) {
				debug ("Decompression error");
				continue;
			}
			tag = new ClickteamTag(tagId, uncompressedPayload, uncompressedPayloadLen);
		} else {
			tag = new ClickteamTag(tagId, compressedPayload, compressedPayloadLen);
		}
		tags[tagId].reset(tag);
		switch (tag->_tagId) {
		case (uint16) ClickteamTagId::FILE_LIST: {
			if (tag->_size < 4) {
				return nullptr;
			}
			uint32 count = READ_LE_UINT32(tag->_contents);
			uint32 off = 4;
			for (unsigned i = 0; i < count && off + 0x24 < tag->_size; i++) {
				uint16 l = READ_LE_UINT16(tag->_contents + off);
				if (l < 0x24)
					break;
				ClickteamFileDescriptor desc(*tag, off);
				files[desc._fileName] = desc;
				off += l;
			}
			break;
		}
		case 0x1237: {
			byte *p;
			for (p = tag->_contents; p < tag->_contents + tag->_size; p++)
				if (!*p)
					break;
			crc_xor = computeCRC(tag->_contents, p - tag->_contents, crc_xor);
			break;
		}
		}
	}

	if (block3_offset <= 0 || block3_len <= 0)
		return nullptr;

	return new ClickteamInstaller(files, tags, crc_xor, block3_offset, block3_len, stream, dispose);
}

bool ClickteamInstaller::hasFile(const Path &path) const {
	return _files.contains(translatePath(path));
}

int ClickteamInstaller::listMembers(ArchiveMemberList &list) const {
	int members = 0;

	for (Common::HashMap<Common::String, ClickteamFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::const_iterator i = _files.begin(), end = _files.end();
	     i != end; ++i) {
		list.push_back(ArchiveMemberList::value_type(new GenericArchiveMember(i->_key, this)));
		++members;
	}

	return members;
}

const ArchiveMemberPtr ClickteamInstaller::getMember(const Path &path) const {
	Common::String translated = translatePath(path);
	if (!_files.contains(translated))
		return nullptr;

	return Common::SharedPtr<Common::ArchiveMember>(new GenericArchiveMember(_files.getVal(translated)._fileName, this));
}

Common::SharedArchiveContents ClickteamInstaller::readContentsForPath(const Common::String& translated) const {
	if (!_files.contains(translated))
		return Common::SharedArchiveContents();
	ClickteamFileDescriptor desc = _files.getVal(translated);
	Common::SeekableReadStream *subStream = new Common::SeekableSubReadStream(_stream.get(), _block3Offset + desc._fileDataOffset,
										  _block3Offset + desc._fileDataOffset + desc._compressedSize);
	if (!subStream) {
		debug("Decompression error");
		return Common::SharedArchiveContents();
	}
	Common::ScopedPtr<Common::SeekableReadStream> uncStream(GzioReadStream::openClickteam(subStream, desc._uncompressedSize, DisposeAfterUse::YES));
	if (!uncStream) {
		debug("Decompression error");
		return Common::SharedArchiveContents();
	}

	byte *uncompressedBuffer = new byte[desc._uncompressedSize];

	int64 ret = uncStream->read(uncompressedBuffer, desc._uncompressedSize);
	if (ret < 0 || ret < desc._uncompressedSize) {
		debug ("Decompression error");
		delete[] uncompressedBuffer;
		return Common::SharedArchiveContents();
	}

	if (desc._expectedCRC != 0 || !desc._fileName.equalsIgnoreCase("Uninstal.exe")) {
		uint32 expectedCrc = desc._expectedCRC ^ _crcXor;
		uint32 actualCrc = computeCRC(uncompressedBuffer, desc._uncompressedSize, 0);

		if (actualCrc != expectedCrc) {
			debug("CRC mismatch for %s: expected=%08x (obfuscated %08x), actual=%08x", desc._fileName.c_str(), expectedCrc, desc._expectedCRC, actualCrc);
			delete[] uncompressedBuffer;
			return Common::SharedArchiveContents();
		}
	}

	// TODO: Make it configurable to use a uncompressing substream instead
	return Common::SharedArchiveContents(uncompressedBuffer, desc._uncompressedSize);
}

}
