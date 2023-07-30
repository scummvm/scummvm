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
#include "common/compression/deflate.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/substream.h"
#include "common/memstream.h"

#define FLAG_COMPRESSED 1

namespace Common {

ClickteamInstaller::ClickteamFileDescriptor::ClickteamFileDescriptor(const ClickteamTag& contentsTag, uint32 off)
	: _fileDataOffset(0), _fileDescriptorOffset(0), _compressedSize(0), _uncompressedSize(0), _isReferenceMissing(false) {
	switch (contentsTag._tagId) {
	case (uint16)ClickteamTagId::FILE_LIST: {
		uint32 stringsOffset = 36;
		byte *tag = contentsTag._contents + off;
		uint32 lmax = contentsTag._size - off;
		if (lmax < 0x24)
			return;
		uint32 ls = READ_LE_UINT32(tag), l;
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
		_supported = true;
		_isPatchFile = false;
		_crcIsXorred = true;
		break;
	}
	case (uint16)ClickteamTagId::FILE_PATCHING_LIST: {
		uint32 stringsOffset = 0x36;
		byte *tag = contentsTag._contents + off;
		uint32 lmax = contentsTag._size - off;
		if (lmax < 33)
			return;
		uint32 ls = READ_LE_UINT32(tag);
		if (ls < 33)
			return;

		byte type = tag[7];
		if (type != 0) {
			_supported = false;
			_fileName = "";
			_fileDataOffset = 0;
			_fileDescriptorOffset = off;
			_compressedSize = 0;
			_uncompressedSize = 0;
			_expectedCRC = 0;
			_isPatchFile = false;
			_crcIsXorred = false;
			return;
		}

		// Layout:
		// 0-3: tag size
		// 4-6: ???
		// 7: operation
		// 8: file type
		// 9: ???
		// a-d: uncompressed size
		// e-11: unxorred uncompressed CRC
		// 12-15: number of original files entries
		// 16-17: pointer to original files entries
		// 18-1d: ???
		// 1e-36: 3 blocks of 8 bytes relating to some timestamps..

		// Original files entries. Array of:
		// 0-3: original CRC
		// 4-7: file size before patching
		// 8-b: patch data offset
		// c-f: patch size

		_expectedCRC = READ_LE_UINT32(tag + 0xe);
		int numPatchEntries = READ_LE_UINT16(tag + 0x12);
		byte *blockb = tag + READ_LE_UINT16(tag + 0x16);

		_uncompressedSize = READ_LE_UINT32(tag + 0xa);
		char *strings = (char *)tag + stringsOffset;
		char *p;
		for (p = strings; p < (char*)tag + lmax && *p; p++);
		_fileName = Common::String(strings, p - strings);
		_fileDescriptorOffset = off;
		_compressedSize = 0;
		_fileDataOffset = 0;
		_supported = true;
		_isPatchFile = !(tag[8] & 2);

		if (!_isPatchFile && numPatchEntries > 0) {
			_compressedSize = READ_LE_UINT32(blockb + 0xc);
			_fileDataOffset = READ_LE_UINT32(blockb + 0x8);
		}

		_patchEntries.resize(numPatchEntries);

		for (int i = 0; i < numPatchEntries; i++) {
			_patchEntries[i]._originalCRC = READ_LE_UINT32(blockb + 0x10 * i);
			_patchEntries[i]._originalSize = READ_LE_UINT32(blockb + 0x10 * i + 4);
			_patchEntries[i]._patchDataOffset = READ_LE_UINT32(blockb + 0x10 * i + 8);
			_patchEntries[i]._patchSize = READ_LE_UINT32(blockb + 0x10 * i + 12);
		}

		_crcIsXorred = false;
		break;
	}
	}
}

ClickteamInstaller::ClickteamTag* ClickteamInstaller::getTag(ClickteamTagId tagId) const {
	return _tags.getValOrDefault((uint16) tagId).get();
}

namespace {
uint32 computeCRC(byte *buf, uint32 sz, uint32 previous) {
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
	static const byte BLOCK1_MAGIC_START[] = { 0x77, 0x77, 0x49, 0x4e, 0x53, 0x53 };
	static const byte BLOCK1_MAGIC_END[] = { 0x77, 0x77, 0x49, 0x4e, 0x53, 0x45 };
	static const byte STUB_SIZE_MAGIC[] = { 0x77, 0x77, 0x67, 0x54, 0x29, 0x48 };
	static const uint32 MAX_SEARCH_RANGE = 0x16000; // So far, if needed increase
	uint32 blockSearchRange = MIN<uint32>(MAX_SEARCH_RANGE, stream->size());

	if (blockSearchRange <= sizeof(STUB_SIZE_MAGIC) + 4 + sizeof(BLOCK1_MAGIC_START) + sizeof(BLOCK1_MAGIC_END)) {
		return false;
	}

	byte *stub = new byte[blockSearchRange];

	stream->seek(0);
	stream->read(stub, blockSearchRange);

	byte *block1start = nullptr;
	byte *block1end = nullptr;
	byte *stubSizePtr = nullptr;
	byte *ptr;

	for (ptr = stub; ptr < stub + blockSearchRange - sizeof(STUB_SIZE_MAGIC) - 3; ptr++) {
		if (memcmp(ptr, STUB_SIZE_MAGIC, sizeof(STUB_SIZE_MAGIC)) == 0)
			stubSizePtr = ptr;
		if (block1start && memcmp(ptr, BLOCK1_MAGIC_END, sizeof(BLOCK1_MAGIC_END)) == 0)
			block1end = ptr;
		if (memcmp(ptr, BLOCK1_MAGIC_START, sizeof(BLOCK1_MAGIC_START)) == 0)
			block1start = ptr;
		if (block1start && block1end && stubSizePtr)
			break;
	}

	if (!block1start || !block1end || !stubSizePtr) {
		delete[] stub;
		return false;
	}

	uint32 stubSize = READ_LE_UINT32(stubSizePtr + sizeof(STUB_SIZE_MAGIC));
	crc = computeCRC(block1start, block1end - block1start, 0);
	delete[] stub;

	stream->seek(stubSize);
	return true;
}

int32 signExtendAndOffset(uint32 val, int bit, uint32 offset) {
	if (val & (1 << bit)) {
		return (val | (0xffffffff << bit)) - offset;
	}
	return val + offset;
}

void applyClickteamPatch(Common::WriteStream *outStream, Common::SeekableReadStream *refStream,
			 Common::SeekableReadStream *patchStream, Common::SeekableReadStream *literalsStream) {
	uint32 referenceBaseOffset = 0;

	while (!patchStream->eos() && !outStream->err()) {
                uint32 referenceReadSize = 0;
		byte patchByte = patchStream->readByte();
                if (patchByte & 0x80) {
			uint32 litteralReadSize = (patchByte >> 5) & 3;
			if (litteralReadSize == 0)
				litteralReadSize = ((patchByte & 0x1f) + 1);
			else
				referenceReadSize = (patchByte & 0x1f) + 2;
			byte *buf = new byte[litteralReadSize]; // optimize this
			literalsStream->read(buf, litteralReadSize);
			outStream->write(buf,litteralReadSize);
			delete[] buf;
                } else if (patchByte == 0)
			referenceReadSize = patchStream->readUint16LE() + 0x81;
                else
			referenceReadSize = patchByte + 1;
                if (referenceReadSize != 0) {
			int referenceOffsetDelta;
			patchByte = patchStream->readByte();
			if (patchByte & 0x80) {
				if ((patchByte & 0x40) == 0)
					referenceOffsetDelta = signExtendAndOffset(patchByte & 0x3f, 5, 0);
				else {
					referenceOffsetDelta = ((patchByte & 0x3f) << 16) | patchStream->readUint16BE();
					if (referenceOffsetDelta == 0x100000)
						referenceOffsetDelta = patchStream->readSint32BE();
					else
						referenceOffsetDelta = signExtendAndOffset(referenceOffsetDelta, 21, 0x4020);
				}
			} else
				referenceOffsetDelta = signExtendAndOffset(((patchByte & 0x7f) << 8) | patchStream->readByte(), 14, 0x20);
			uint32 referenseOffset = referenceOffsetDelta + referenceBaseOffset;
			byte *buf = new byte[referenceReadSize]; // optimize this
			if (referenseOffset < refStream->size()) {
				refStream->seek(referenseOffset);
				refStream->read(buf, referenceReadSize);
			} else {
				memset(buf, 0, referenceReadSize);
			}
			// TODO: Handle zero-out blocks. We never encountered any so far
			outStream->write(buf, referenceReadSize);
			delete[] buf;
			referenceBaseOffset += referenceOffsetDelta + referenceReadSize;
                }
	}
}

bool readBlockHeader(Common::SeekableReadStream *stream, uint32 &compressedSize, uint32 &uncompressedSize, bool &isCompressed) {
	byte codec = stream->readByte();
	if (codec > 7) {
		warning("Unknown block codec %d", codec);
		return false;
	}
	if (codec & 2)
		uncompressedSize = stream->readUint32LE();
	else
		uncompressedSize = stream->readUint16LE();
	switch (codec & 5) {
	case 5:
		compressedSize = stream->readUint32LE();
		break;
	case 1:
		compressedSize = stream->readUint16LE();
		break;
	case 0:
		compressedSize = uncompressedSize;
		break;
	default:
		warning("Unknown block codec %d", codec);
		return false;
	}
	isCompressed = codec & 1;
	return true;
}
}  // end of anonymous namespace

struct TagHead {
	uint16 id;
	uint16 flags;
	uint32 compressedLen;
};

int ClickteamInstaller::findPatchIdx(const ClickteamFileDescriptor &desc, Common::SeekableReadStream *refStream,
				     const Common::String &fileName,
				     uint32 crcXor, bool doWarn) {
	bool hasMatching = refStream->size() == desc._uncompressedSize; // Maybe already patched?
	for (uint i = 0; !hasMatching && i < desc._patchEntries.size(); i++)
		if (desc._patchEntries[i]._originalSize == refStream->size()) {
			hasMatching = true;
			break;
		}
	if (!hasMatching) {
		if (doWarn)
			warning("Couldn't find matching patch entry for file %s size %d", fileName.c_str(), (int)refStream->size());
		return -1;
	}
	uint32 crcOriginal = 0;
	{
		byte buf[0x1000]; // Must be divisible by 4
		while (!refStream->eos()) {
			uint32 actual = refStream->read(buf, sizeof(buf));
			crcOriginal = computeCRC(buf, actual, crcOriginal);
		}
	}
	int patchDescIdx = -1;
	for (uint i = 0; i < desc._patchEntries.size(); i++)
		if (desc._patchEntries[i]._originalSize == refStream->size() && desc._patchEntries[i]._originalCRC == (crcOriginal ^ crcXor)) {
			patchDescIdx = i;
			break;
		}
	 // Maybe already patched if nothing else is found?
	if (patchDescIdx == -1 && refStream->size() == desc._uncompressedSize && crcOriginal == desc._expectedCRC)
		return -2;
	if (patchDescIdx < 0 && doWarn) {
		warning("Couldn't find matching patch entry for file %s size %d and CRC 0x%x", fileName.c_str(), (int)refStream->size(), crcOriginal);
	}
	return patchDescIdx;
}


ClickteamInstaller* ClickteamInstaller::open(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose) {
	return openPatch(stream, false, true, nullptr, dispose);
}

ClickteamInstaller* ClickteamInstaller::openPatch(Common::SeekableReadStream *stream, bool verifyOriginal, bool verifyAllowSkip,
						  Common::Archive *reference, DisposeAfterUse::Flag dispose) {
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
			bool ret = inflateClickteam(uncompressedPayload,
			                             uncompressedPayloadLen,
			                             compressedPayload + 4,
			                             compressedPayloadLen - 4);
			delete[] compressedPayload;
			if (!ret) {
				warning("Decompression error for tag 0x%04x", tagId);
				continue;
			}
			tag = new ClickteamTag(tagId, uncompressedPayload, uncompressedPayloadLen);
		} else {
			tag = new ClickteamTag(tagId, compressedPayload, compressedPayloadLen);
		}
		tags[tagId].reset(tag);
		switch (tag->_tagId) {
		case (uint16) ClickteamTagId::FILE_LIST:
		case (uint16) ClickteamTagId::FILE_PATCHING_LIST: {
			if (tag->_size < 4) {
				return nullptr;
			}
			uint32 count = READ_LE_UINT32(tag->_contents);
			uint32 off = 4;
			for (unsigned i = 0; i < count && off + 0x24 < tag->_size; i++) {
				uint32 l = READ_LE_UINT32(tag->_contents + off);
				if (l < 33)
					break;
				ClickteamFileDescriptor desc(*tag, off);
				if (desc._supported) {
					// Prefer non-patches
					if (!desc._isPatchFile || ! files.contains(desc._fileName))
						files[desc._fileName] = desc;
				}
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

	if (verifyOriginal && reference) {
		for (Common::HashMap<Common::String, ClickteamFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = files.begin(), end = files.end();
		     i != end; ++i) {
			if (i->_value._isPatchFile) {
				Common::ScopedPtr<Common::SeekableReadStream> refStream(reference->createReadStreamForMember(Common::Path(i->_key, '\\')));
				if (!refStream) {
					if (verifyAllowSkip) {
						i->_value._isReferenceMissing = true;
						continue;
					}
					return nullptr;
				}
				if (findPatchIdx(i->_value, refStream.get(), i->_key, crc_xor, false) == -1)
					return nullptr;
			}
		}
	}

	if (!reference) {
		for (Common::HashMap<Common::String, ClickteamFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = files.begin(), end = files.end();
		     i != end; ++i) {
			if (i->_value._isPatchFile) {
				i->_value._isReferenceMissing = true;
			}
		}
	}

	return new ClickteamInstaller(files, tags, crc_xor, block3_offset, block3_len, stream, reference, dispose);
}

bool ClickteamInstaller::hasFile(const Path &path) const {
	return _files.contains(translatePath(path));
}

int ClickteamInstaller::listMembers(ArchiveMemberList &list) const {
	int members = 0;

	for (Common::HashMap<Common::String, ClickteamFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::const_iterator i = _files.begin(), end = _files.end();
	     i != end; ++i) {
		if (!i->_value._isReferenceMissing) {
			list.push_back(ArchiveMemberList::value_type(new GenericArchiveMember(i->_key, *this)));
			++members;
		}
	}

	return members;
}

const ArchiveMemberPtr ClickteamInstaller::getMember(const Path &path) const {
	Common::String translated = translatePath(path);
	ClickteamFileDescriptor el;
	if (!_files.tryGetVal(translated, el))
		return nullptr;

	if (el._isReferenceMissing)
		return nullptr;

	return Common::SharedPtr<Common::ArchiveMember>(new GenericArchiveMember(el._fileName, *this));
}

Common::SharedArchiveContents ClickteamInstaller::readContentsForPath(const Common::String& translated) const {
	ClickteamFileDescriptor desc;
	byte *uncompressedBuffer = nullptr;

	if (!_files.tryGetVal(translated, desc))
		return Common::SharedArchiveContents();
	if (desc._isReferenceMissing)
		return Common::SharedArchiveContents();

	if (desc._isPatchFile) {
		Common::ScopedPtr<Common::SeekableReadStream> refStream(_reference->createReadStreamForMemberNext(Common::Path(translated, '\\'), this));
		if (!refStream) {
			warning("Couldn't open reference file for %s. Skipping", translated.c_str());
			return Common::SharedArchiveContents();
		}
		int patchDescIdx = findPatchIdx(desc, refStream.get(), translated, _crcXor, true);
		if (patchDescIdx == -1 || patchDescIdx < -2)
			return Common::SharedArchiveContents();

		refStream->seek(0);

		 // Already patched
		if (patchDescIdx == -2) {
			return Common::SharedArchiveContents::bypass(refStream.release());
		}

		uint32 patchDataOffset = _block3Offset + desc._patchEntries[patchDescIdx]._patchDataOffset;
		_stream->seek(patchDataOffset);
		uint32 patchCompressedSize, patchUncompressedSize;
		bool patchIsCompressed;
		if (!readBlockHeader(_stream.get(), patchCompressedSize, patchUncompressedSize, patchIsCompressed))
			return Common::SharedArchiveContents();

		uint32 patchStart = _stream->pos();
		_stream->skip(patchCompressedSize);

		uint32 literalsCompressedSize, literalsUncompressedSize;
		bool literalsIsCompressed;
		if (!readBlockHeader(_stream.get(), literalsCompressedSize, literalsUncompressedSize, literalsIsCompressed)) {
			return Common::SharedArchiveContents();
		}
		uint32 literalsStart = _stream->pos();

		Common::ScopedPtr<Common::SeekableReadStream> uncompressedPatchStream, uncompressedLiteralsStream;

		if (patchIsCompressed) {
			Common::SeekableReadStream *compressedPatchStream = new Common::SafeSeekableSubReadStream(
				_stream.get(), patchStart, patchStart + patchCompressedSize);
			if (!compressedPatchStream) {
				warning("Decompression error");
				return Common::SharedArchiveContents();
			}

			uncompressedPatchStream.reset(wrapClickteamReadStream(compressedPatchStream, DisposeAfterUse::YES, patchUncompressedSize));
		} else {
			uncompressedPatchStream.reset(new Common::SafeSeekableSubReadStream(
							      _stream.get(), patchStart, patchStart + patchCompressedSize));

		}
		if (!uncompressedPatchStream) {
			warning("Decompression error");
			return Common::SharedArchiveContents();
		}

		if (literalsIsCompressed) {
			Common::SeekableReadStream *compressedLiteralsStream = new Common::SafeSeekableSubReadStream(
				_stream.get(), literalsStart, literalsStart + literalsCompressedSize);
			if (!compressedLiteralsStream) {
				warning("Decompression error");
				return Common::SharedArchiveContents();
			}

			uncompressedLiteralsStream.reset(wrapClickteamReadStream(compressedLiteralsStream, DisposeAfterUse::YES, literalsUncompressedSize));
		} else {
			uncompressedLiteralsStream.reset(new Common::SafeSeekableSubReadStream(
								 _stream.get(), literalsStart, literalsStart + literalsCompressedSize));
		}
		if (!uncompressedLiteralsStream) {
			warning("Decompression error");
			return Common::SharedArchiveContents();
		}

		uncompressedBuffer = new byte[desc._uncompressedSize];
		Common::MemoryWriteStream outStream(uncompressedBuffer, desc._uncompressedSize);
		applyClickteamPatch(&outStream, refStream.get(), uncompressedPatchStream.get(), uncompressedLiteralsStream.get());
	} else {
		Common::SeekableReadStream *subStream = new Common::SeekableSubReadStream(_stream.get(), _block3Offset + desc._fileDataOffset,
											  _block3Offset + desc._fileDataOffset + desc._compressedSize);
		if (!subStream) {
			warning("Decompression error");
			return Common::SharedArchiveContents();
		}

		Common::ScopedPtr<Common::SeekableReadStream> uncStream(wrapClickteamReadStream(subStream, DisposeAfterUse::YES, desc._uncompressedSize));
		if (!uncStream) {
			warning("Decompression error");
			return Common::SharedArchiveContents();
		}

		uncompressedBuffer = new byte[desc._uncompressedSize];

		int64 ret = uncStream->read(uncompressedBuffer, desc._uncompressedSize);
		if (ret < 0 || ret < desc._uncompressedSize) {
			warning ("Decompression error");
			delete[] uncompressedBuffer;
			return Common::SharedArchiveContents();
		}
	}

	if (desc._expectedCRC != 0 || !desc._fileName.equalsIgnoreCase("Uninstal.exe")) {
		uint32 expectedCrc = desc._crcIsXorred ? desc._expectedCRC ^ _crcXor : desc._expectedCRC;
		uint32 actualCrc = computeCRC(uncompressedBuffer, desc._uncompressedSize, 0);

		if (actualCrc != expectedCrc) {
			warning("CRC mismatch for %s: expected=%08x (obfuscated %08x), actual=%08x (back %08x)", desc._fileName.c_str(), expectedCrc, desc._expectedCRC, actualCrc, actualCrc ^ _crcXor);
			delete[] uncompressedBuffer;
			return Common::SharedArchiveContents();
		}
	}

	// TODO: Make it configurable to use a uncompressing substream instead
	return Common::SharedArchiveContents(uncompressedBuffer, desc._uncompressedSize);
}

}
