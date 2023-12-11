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

#ifndef TWP_GGPACK_H
#define TWP_GGPACK_H

#include "common/hash-str.h"
#include "common/stream.h"
#include "common/list.h"
#include "common/path.h"
#include "common/formats/json.h"

namespace Twp {

struct XorKey {
	Common::Array<int> magicBytes;
    int multiplier;
};

class MemStream: public Common::SeekableReadStream {
public:
	MemStream();

	bool open(const byte* buf, int64 bufSize);
	uint32 read(void *dataPtr, uint32 dataSize);
	bool eos() const;

	int64 pos() const;
	int64 size() const;
	bool seek(int64 offset, int whence = SEEK_SET);

private:
    const byte* _buf;
	int64 _bufSize;
	int64 _pos;
};

class XorStream: public Common::SeekableReadStream {
public:
	XorStream();

	bool open(Common::SeekableReadStream *stream, int len, const XorKey& key);
	uint32 read(void *dataPtr, uint32 dataSize);
	bool eos() const;

	int64 pos() const;
	int64 size() const;
	bool seek(int64 offset, int whence = SEEK_SET);

private:
    Common::SeekableReadStream *_s;
    int _previous;
    int _start;
    int _size;
    XorKey _key;
};

class RangeStream: public Common::SeekableReadStream {
public:
	RangeStream();

	bool open(Common::SeekableReadStream *stream, int64 size);
	uint32 read(void *dataPtr, uint32 dataSize);
	bool eos() const;

	int64 pos() const;
	int64 size() const;
	bool seek(int64 offset, int whence = SEEK_SET);

private:
    Common::SeekableReadStream *_s;
    int64 _start;
    int64 _size;
};

class GGHashMapDecoder {
public:
	GGHashMapDecoder();

	Common::JSONValue* open(Common::SeekableReadStream *stream);

private:
	Common::JSONValue* readHash();
	Common::String readString(uint32 i);
	Common::JSONValue* readValue();
	Common::JSONValue* readArray();

private:
	Common::SeekableReadStream *_stream;
	Common::Array<int> _offsets;
};

struct GGPackEntry {
    int offset, size;
};

typedef Common::HashMap<Common::String, GGPackEntry, Common::IgnoreCase_Hash> GGPackEntries;

class GGPackDecoder {
public:
	friend class GGPackEntryReader;
public:
	GGPackDecoder();

	bool open(Common::SeekableReadStream *s, const XorKey& key);

	bool assetExists(const char* asset) { return _entries.contains(asset); }

private:
	XorKey _key;
	GGPackEntries _entries;
	Common::SeekableReadStream *_s;
};

class GGPackEntryReader: public Common::SeekableReadStream {
public:
	GGPackEntryReader();

	bool open(GGPackDecoder& pack, const Common::String& entry);

	uint32 read(void *dataPtr, uint32 dataSize) override;
	bool eos() const override;

	int64 pos() const override;
	int64 size() const override;
	bool seek(int64 offset, int whence = SEEK_SET) override;

private:
	Common::Array<byte> _buf;
	MemStream _ms;
};

} // namespace Twp

#endif
