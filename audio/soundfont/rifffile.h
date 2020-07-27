/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_RIFFFILE_H
#define AUDIO_SOUNDFONT_RIFFFILE_H

#include <cstring>
#include <cassert>
#include "common/scummsys.h"
#include "common/list.h"
#include "common/str.h"
#include "common/array.h"

//////////////////////////////////////////////
// Chunk		- Riff format chunk
//////////////////////////////////////////////
class Chunk {
public:
	char id[4];     //  A chunk ID identifies the type of data within the chunk.
	uint32 size;  //  The size of the chunk data in bytes, excluding any pad byte.
	uint8 *data;  //  The actual data not including a possible pad byte to word align

public:
	Chunk(Common::String theId) : data(NULL), size(0) {
		assert(theId.size() == 4);
		memcpy(id, theId.c_str(), 4);
	}

	virtual ~Chunk() {
		if (data != NULL) {
			delete[] data;
			data = NULL;
		}
	}

	void SetData(const void *src, uint32 datasize);

	virtual uint32 GetSize();  //  Returns the size of the chunk in bytes, including any pad byte.
	virtual void Write(uint8 *buffer);

protected:
	static inline uint32 GetPaddedSize(uint32 originalSize) { return originalSize + (originalSize % 2); }
};

////////////////////////////////////////////////////////////////////////////
// ListTypeChunk	- Riff chunk type where the first 4 data bytes are a sig
//					  and the rest of the data is a collection of child chunks
////////////////////////////////////////////////////////////////////////////
class ListTypeChunk : public Chunk {
public:
	char type[4];  // 4 byte sig that begins the data field, "LIST" or "sfbk" for ex
	Common::List<Chunk *> childChunks;

public:
	ListTypeChunk(Common::String theId, Common::String theType) : Chunk(theId) {
		assert(theType.size() == 4);
		memcpy(type, theType.c_str(), 4);
	}

	virtual ~ListTypeChunk() {
		childChunks.erase(childChunks.begin(), childChunks.end());
	}

	Chunk *AddChildChunk(Chunk *ck);

	virtual uint32 GetSize();  //  Returns the size of the chunk in bytes, including any pad byte.
	virtual void Write(uint8 *buffer);
};

////////////////////////////////////////////////////////////////////////////
// RIFFChunk
////////////////////////////////////////////////////////////////////////////
class RIFFChunk : public ListTypeChunk {
public:
	RIFFChunk(Common::String form) : ListTypeChunk("RIFF", form) {}
};

////////////////////////////////////////////////////////////////////////////
// LISTChunk
////////////////////////////////////////////////////////////////////////////
class LISTChunk : public ListTypeChunk {
public:
	LISTChunk(Common::String theType) : ListTypeChunk("LIST", theType) {}
};

////////////////////////////////////////////////////////////////////////////
// RiffFile		-
////////////////////////////////////////////////////////////////////////////
class RiffFile : public RIFFChunk {
public:
	RiffFile(const Common::String &file_name, const Common::String &form);

	static void WriteLIST(Common::Array<uint8> &buf, uint32 listName, uint32 listSize) {
		//TODO
//        PushTypeOnVectBE<uint32>(buf, 0x4C495354);  // write "LIST"
//        PushTypeOnVect<uint32>(buf, listSize);
//        PushTypeOnVectBE<uint32>(buf, listName);
	}

	// Adds a null byte and ensures 16 bit alignment of a text string
	static void AlignName(Common::String &name) {
		name += (char) 0x00;
		if (name.size() % 2)     // if the size of the name string is odd
			name += (char) 0x00;  // add another null byte
	}

protected:
	Common::String name;
};

#endif // AUDIO_SOUNDFONT_RIFFFILE_H
