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

#include "twp/ggpack.h"
#include "common/debug.h"

namespace Twp {

#define GGP_NULL 1
#define GGP_DICTIONARY 2
#define GGP_ARRAY 3
#define GGP_STRING 4
#define GGP_INTEGER 5
#define GGP_DOUBLE 6
#define GGP_OFFSETS 7
#define GGP_KEYS 8
#define GGP_ENDOFFSETS 0xFFFFFFFF

static bool _readPlo(Common::SeekableReadStream *s, Common::Array<int> &offsets) {
	uint32 ploIndex = s->readUint32LE();
	int64 pos = s->pos();
	s->seek(ploIndex);
	byte c = s->readByte();
	if (c != GGP_OFFSETS)
		return false;
	uint32 offset = 0;
	while (offset != GGP_ENDOFFSETS) {
		offset = s->readUint32LE();
		if (offset != GGP_ENDOFFSETS)
			offsets.push_back(offset);
	}
	s->seek(pos);
	return true;
}

Common::String GGHashMapDecoder::readString(uint32 i) {
	Common::String result;
	int64 pos = _stream->pos();
	_stream->seek(_offsets[i]);
	while (true) {
		byte c = _stream->readByte();
		if (c == 0) {
			_stream->seek(pos);
			return result;
		}
		result += (char)c;
	}
	return result;
}

Common::JSONValue *GGHashMapDecoder::readValue() {
	byte ggType = _stream->readByte();
	switch (ggType) {
	case GGP_NULL: {
		return new Common::JSONValue();
	}
	case GGP_DICTIONARY:
		_stream->seek(_stream->pos() - 1);
		return readHash();
	case GGP_ARRAY:
		_stream->seek(_stream->pos() - 1);
		return readArray();
	case GGP_STRING: {
		uint32 ploIdx = _stream->readUint32LE();
		return new Common::JSONValue(readString(ploIdx));
	}
	case GGP_INTEGER:
	case GGP_DOUBLE: {
		uint32 ploIdx = _stream->readUint32LE();
		Common::String num_str = readString(ploIdx);
		return ggType == GGP_INTEGER ? new Common::JSONValue((long long int)atol(num_str.c_str())) : new Common::JSONValue(atof(num_str.c_str()));
	}
	default:
		error("Not Implemented: value type {ggType}");
		return new Common::JSONValue();
	}
}

Common::JSONValue *GGHashMapDecoder::readArray() {
	byte c = _stream->readByte();
	if (c != GGP_ARRAY)
		error("trying to parse a non-array");
	Common::JSONArray arr;
	uint32 length = _stream->readUint32LE();
	for (int i = 0; i < length; i++) {
		Common::JSONValue *item = readValue();
		arr.push_back(item);
	}
	c = _stream->readByte();
	if (c != GGP_ARRAY)
		error("unterminated array");
	return new Common::JSONValue(arr);
}

Common::JSONValue *GGHashMapDecoder::readHash() {
	Common::JSONObject obj;
	byte c = _stream->readByte();
	if (c != GGP_DICTIONARY) {
		error("trying to parse a non-hash: %d", c);
	}
	uint32 nPairs = _stream->readUint32LE();
	for (int i = 0; i < nPairs; i++) {
		Common::String key = readString(_stream->readUint32LE());
		obj[key] = readValue();
	}
	c = _stream->readByte();
	if (c != GGP_DICTIONARY) {
		error("unterminated hash");
	}
	return new Common::JSONValue(obj);
}

GGHashMapDecoder::GGHashMapDecoder()
	: _stream(nullptr) {
}

Common::JSONValue *GGHashMapDecoder::open(Common::SeekableReadStream *s) {
	uint32 signature = s->readUint32LE();
	if (signature != 0x04030201) {
		error("This version of package is not supported (yet?)");
		return new Common::JSONValue();
	}
	_stream = s;
	uint32 numEntries = s->readUint32LE();
	if (!_readPlo(s, _offsets))
		error("This version of package is not supported (yet?)");
	return readHash();
}

MemStream::MemStream() : _buf(nullptr), _bufSize(0), _pos(0) {
}

bool MemStream::open(const byte *buf, int64 bufSize) {
	_buf = buf;
	_bufSize = bufSize;
	_pos = 0;
	return true;
}

uint32 MemStream::read(void *dataPtr, uint32 dataSize) {
	int64 size = MIN((int64)dataSize, (int64)_bufSize - _pos);
	memcpy(dataPtr, _buf + _pos, size);
	_pos += size;
	return size;
}

bool MemStream::eos() const {
	return _pos >= _bufSize;
}

int64 MemStream::pos() const {
	return _pos;
}

int64 MemStream::size() const {
	return _bufSize;
}

bool MemStream::seek(int64 offset, int whence) {
	if (whence == SEEK_SET) {
		_pos = offset;
		return true;
	}
	_pos = _bufSize + offset;
	return true;
}

RangeStream::RangeStream() : _s(nullptr), _size(0) {
}

bool RangeStream::open(Common::SeekableReadStream *stream, int64 size) {
	_s = stream;
	_start = _s->pos();
	_size = size;
	return true;
}

uint32 RangeStream::read(void *dataPtr, uint32 dataSize) {
	return _s->read(dataPtr, dataSize);
}

bool RangeStream::eos() const {
	return pos() >= _size;
}

int64 RangeStream::pos() const {
	return _s->pos() - _start;
}

int64 RangeStream::size() const {
	return _size;
}

bool RangeStream::seek(int64 offset, int whence) {
	if (whence == SEEK_SET) {
		return _s->seek(_start + offset, SEEK_SET);
	}
	return _s->seek(offset, whence);
}

XorStream::XorStream() : _s(nullptr), _size(0) {
}

bool XorStream::open(Common::SeekableReadStream *stream, int len, const XorKey &key) {
	_s = stream;
	_start = _s->pos();
	_previous = (len & 0xFF);
	_key = key;
	_size = len;
	return true;
}

uint32 XorStream::read(void *dataPtr, uint32 dataSize) {
	int p = (int)pos();
	uint32 result = _s->read(dataPtr, dataSize);
	char *buf = (char *)dataPtr;
	for (int i = 0; i < dataSize; i++) {
		int x = buf[i] ^ _key.magicBytes[p & 0x0F] ^ (i * _key.multiplier);
		buf[i] = (char)(x ^ _previous);
		_previous = x;
		p++;
	}
	return result;
}

bool XorStream::eos() const {
	return pos() >= _size;
}

int64 XorStream::pos() const {
	return _s->pos() - _start;
}

int64 XorStream::size() const {
	return _size;
}

bool XorStream::seek(int64 offset, int whence) {
	if (whence == SEEK_SET) {
		return _s->seek(_start + offset, SEEK_SET);
	}
	return _s->seek(offset, whence);
}

GGPackDecoder::GGPackDecoder() {
}

bool GGPackDecoder::open(Common::SeekableReadStream *s, const XorKey &key) {
	_entries.clear();
	_key = key;
	_s = s;

	uint32 entriesOffset = s->readUint32LE();
	uint32 entriesSize = s->readUint32LE();
	s->seek(entriesOffset);

	// decode entries
	XorStream xor ;
	xor.open(s, entriesSize, key);
	Common::Array<byte> buffer(entriesSize);
	xor.read(buffer.data(), entriesSize);

	// read entries as hash
	MemStream ms;
	ms.open(buffer.data(), entriesSize);
	GGHashMapDecoder tblDecoder;
	Common::JSONValue *value = tblDecoder.open(&ms);
	const Common::JSONObject &obj = value->asObject();
	const Common::JSONArray &files = obj["files"]->asArray();
	for (size_t i = 0; i < files.size(); i++) {
		const Common::JSONObject &file = files[i]->asObject();
		const Common::String &filename = file["filename"]->asString();
		int offset = (int)file["offset"]->asIntegerNumber();
		int size = (int)file["size"]->asIntegerNumber();
		_entries[filename] = GGPackEntry{offset, size};
		debug("filename: %s, off: %d, size: %d", filename.c_str(), offset, size);
	}
	delete value;

	return true;
}

GGPackEntryReader::GGPackEntryReader() {}

bool GGPackEntryReader::open(GGPackDecoder &pack, const Common::String &entry) {
	if(!pack._entries.contains(entry))
		return false;
	GGPackEntry e = pack._entries[entry];
	pack._s->seek(e.offset);

	RangeStream rs;
	rs.open(pack._s, e.size);
	XorStream xs;
	xs.open(&rs, e.size, pack._key);

	_buf.resize(e.size);
	xs.read(&_buf[0], e.size);

	_ms.open(&_buf[0], e.size);
	return true;
}

uint32 GGPackEntryReader::read(void *dataPtr, uint32 dataSize) {
	return _ms.read(dataPtr, dataSize);
}
bool GGPackEntryReader::eos() const {
	return _ms.eos();
}

int64 GGPackEntryReader::pos() const {
	return _ms.pos();
}

int64 GGPackEntryReader::size() const {
	return _ms.size();
}

bool GGPackEntryReader::seek(int64 offset, int whence) {
	return _ms.seek(offset, whence);
}

} // namespace Twp
