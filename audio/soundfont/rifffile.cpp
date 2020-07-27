/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */

#include "rifffile.h"

using namespace std;

uint32 Chunk::GetSize() {
	return 8 + GetPaddedSize(size);
}

void Chunk::SetData(const void *src, uint32 datasize) {
	size = datasize;

	// set the size and copy from the data source
	datasize = GetPaddedSize(size);
	if (data != NULL) {
		delete[] data;
		data = NULL;
	}
	data = new uint8[datasize];
	memcpy(data, src, size);

	// Add pad byte
	uint32 padsize = datasize - size;
	if (padsize != 0) {
		memset(data + size, 0, padsize);
	}
}

void Chunk::Write(uint8 *buffer) {
	uint32 padsize = GetPaddedSize(size) - size;
	memcpy(buffer, id, 4);
	*(uint32 * )(buffer + 4) =
			size + padsize;  // Microsoft says the chunkSize doesn't contain padding size, but many
	// software cannot handle the alignment.
	memcpy(buffer + 8, data, GetPaddedSize(size));
}

Chunk *ListTypeChunk::AddChildChunk(Chunk *ck) {
	childChunks.push_back(ck);
	return ck;
}

uint32 ListTypeChunk::GetSize() {
	uint32 listChunkSize = 12;  // id + size + "LIST"
	for (Common::List<Chunk *>::iterator iter = this->childChunks.begin(); iter != childChunks.end(); iter++)
		listChunkSize += (*iter)->GetSize();
	return GetPaddedSize(listChunkSize);
}

void ListTypeChunk::Write(uint8 *buffer) {
	memcpy(buffer, this->id, 4);
	memcpy(buffer + 8, this->type, 4);

	uint32 bufOffset = 12;
	for (Common::List<Chunk *>::iterator iter = this->childChunks.begin(); iter != childChunks.end(); iter++) {
		(*iter)->Write(buffer + bufOffset);
		bufOffset += (*iter)->GetSize();
	}

	uint32 unpaddedSize = bufOffset;
	uint32 padsize = GetPaddedSize(unpaddedSize) - unpaddedSize;
	*(uint32 *) (buffer + 4) =
			unpaddedSize + padsize - 8;  // Microsoft says the chunkSize doesn't contain padding size, but many
	// software cannot handle the alignment.

	// Add pad byte
	if (padsize != 0) {
		memset(data + unpaddedSize, 0, padsize);
	}
}

RiffFile::RiffFile(const Common::String &file_name, const Common::String &form) : RIFFChunk(form), name(file_name) {}
