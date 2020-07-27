/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */

#ifndef AUDIO_SOUNDFONT_RAWFILE_H
#define AUDIO_SOUNDFONT_RAWFILE_H

#include <climits>
#include <cassert>
#include "common/stream.h"
#include "common/str.h"

class VGMFile;

class RawFile {
public:
	virtual ~RawFile() {};

	virtual size_t size() const = 0;

	bool IsValidOffset(uint32 ofs) { return ofs < size(); }

	const char *begin() const { return data(); }
	const char *end() { return data() + size(); }
	virtual const char *data() const = 0;

	virtual uint8 GetByte(size_t offset) const = 0;
	virtual uint16 GetShort(size_t offset) const = 0;
	virtual uint32 GetWord(size_t offset) const = 0;

	uint32 GetBytes(size_t offset, uint32 nCount, void *pBuffer) const;

private:
};

class MemFile : public RawFile {
private:
	Common::SeekableReadStream *_seekableReadStream;
	const byte *_data;

public:
	MemFile(const byte *data, uint32 size);
	~MemFile() override;

	const char *data() const override;

	uint8 GetByte(size_t offset) const override;
	uint16 GetShort(size_t offset) const override;
	uint32 GetWord(size_t offset) const override;

	size_t size() const override;
};

#endif // AUDIO_SOUNDFONT_RAWFILE_H
