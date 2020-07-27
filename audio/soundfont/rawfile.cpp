/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */

#include "common/memstream.h"
#include "rawfile.h"

uint32 RawFile::GetBytes(size_t offset, uint32 nCount, void *pBuffer) const {
	memcpy(pBuffer, data() + offset, nCount);
	return nCount;
}

const char *MemFile::data() const {
	return (const char *) _data;
}

uint8 MemFile::GetByte(size_t offset) const {
	_seekableReadStream->seek(offset);
	return _seekableReadStream->readByte();
}

uint16 MemFile::GetShort(size_t offset) const {
	_seekableReadStream->seek(offset);
	return _seekableReadStream->readUint16LE();
}

uint32 MemFile::GetWord(size_t offset) const {
	_seekableReadStream->seek(offset);
	return _seekableReadStream->readUint32LE();
}

size_t MemFile::size() const {
	return _seekableReadStream->size();
}

MemFile::~MemFile() {
	delete _seekableReadStream;
}

MemFile::MemFile(const byte *data, uint32 size) : _data(data) {
	_seekableReadStream = new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}
