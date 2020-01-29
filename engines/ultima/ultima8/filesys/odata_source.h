/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_FILESYS_ODATASOURCE_H
#define ULTIMA8_FILESYS_ODATASOURCE_H

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/shared/std/containers.h"
#include "common/stream.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima8 {

class ODataSource {
public:
	ODataSource() {}
	virtual ~ODataSource() {}

	virtual void write1(uint32) = 0;
	virtual void write2(uint16) = 0;
	virtual void write2high(uint16) = 0;
	virtual void write3(uint32) = 0;
	virtual void write4(uint32) = 0;
	virtual void write4high(uint32) = 0;
	virtual void write(const void *str, uint32 num_bytes) = 0;

	void writeX(uint32 val, uint32 num_bytes) {
		assert(num_bytes > 0 && num_bytes <= 4);
		if (num_bytes == 1) write1(static_cast<uint8>(val));
		else if (num_bytes == 2) write2(static_cast<uint16>(val));
		else if (num_bytes == 3) write3(val);
		else write4(val);
	}

	void writef(float f) {
		// FIXME: dubious...
		union {
			uint32  i;
			float   f;
		} int_float;
		int_float.f = f;
		write4(int_float.i);
	}


	virtual Common::WriteStream *GetRawStream() {
		return 0;
	}

	virtual void seek(uint32 pos) = 0;
	virtual void skip(int32 delta) = 0;
	virtual uint32 getSize() const = 0;
	virtual uint32 getPos() const = 0;
};


class OFileDataSource : public ODataSource {
private:
	Common::WriteStream *_out;

public:
	OFileDataSource(Common::WriteStream *data_stream) {
		_out = data_stream;
	}

	virtual ~OFileDataSource() {
		FORGET_OBJECT(_out);
	}

	bool good() const {
		return !_out->err();
	}

	virtual void write1(uint32 val) override {
		_out->writeByte(val & 0xff);
	}

	virtual void write2(uint16 val) override {
		_out->writeUint16LE(val);
	}

	virtual void write2high(uint16 val) override {
		_out->writeUint16BE(val);
	}

	virtual void write3(uint32 val) override {
		_out->writeByte(static_cast<byte>(val & 0xff));
		_out->writeByte(static_cast<byte>((val >> 8) & 0xff));
		_out->writeByte(static_cast<byte>((val >> 16) & 0xff));
	}

	virtual void write4(uint32 val) override {
		_out->writeUint32LE(val);
	}

	virtual void write4high(uint32 val) override {
		_out->writeUint32BE(val);
	}

	virtual void write(const void *b, uint32 len) override {
		_out->write(static_cast<const char *>(b), len);
	}

	virtual void seek(uint32 pos) override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		ws->seek(pos);
	}

	virtual void skip(int32 amount) override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		ws->seek(amount, SEEK_CUR);
	}

	virtual uint32 getSize() const override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		return ws->size();
	}

	virtual uint32 getPos() const override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		return _out->pos();
	}

	virtual Common::WriteStream *GetRawStream() {
		return _out;
	}
};

class OBufferDataSource: public ODataSource {
protected:
	uint8 *buf;
	uint8 *buf_ptr;
	uint32 size;
public:
	OBufferDataSource(void *data, uint32 len) {
		assert(data == 0 || len == 0);
		buf = buf_ptr = reinterpret_cast<uint8 *>(data);
		size = len;
	};

	void load(char *data, uint32 len) {
		assert(data == 0 || len == 0);
		buf = buf_ptr = reinterpret_cast<uint8 *>(data);
		size = len;
	};

	virtual ~OBufferDataSource() {};

	virtual void write1(uint32 val) {
		*buf_ptr++ = val & 0xff;
	};

	virtual void write2(uint16 val) {
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
	};

	virtual void write2high(uint16 val) {
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = val & 0xff;
	};

	virtual void write3(uint32 val) {
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = (val >> 16) & 0xff;
	};

	virtual void write4(uint32 val) {
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = (val >> 16) & 0xff;
		*buf_ptr++ = (val >> 24) & 0xff;
	};

	virtual void write4high(uint32 val) {
		*buf_ptr++ = (val >> 24) & 0xff;
		*buf_ptr++ = (val >> 16) & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = val & 0xff;
	};

	virtual void write(const void *b, uint32 len) {
		Common::copy((const byte *)b, (const byte *)b + len, buf_ptr);
		buf_ptr += len;
	};

	virtual void seek(uint32 pos) {
		buf_ptr = const_cast<unsigned char *>(buf) + pos;
	};

	virtual void skip(int32 pos) {
		buf_ptr += pos;
	};

	virtual uint32 getSize() {
		return size;
	};

	virtual uint32 getPos() {
		return static_cast<uint32>(buf_ptr - buf);
	};
};

class ODequeDataSource : public ODataSource {
private:
	Std::deque<byte> _out;

public:
	ODequeDataSource() {}

	virtual ~ODequeDataSource() {}

	const Std::deque<byte> &buf() const {
		return _out;
	}

	virtual void write1(uint32 val) {
		_out.push_back(static_cast<byte>(val & 0xff));
	}

	virtual void write2(uint16 val) {
		_out.push_back(static_cast<byte>(val & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
	}

	virtual void write2high(uint16 val) {
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>(val & 0xff));
	}

	virtual void write3(uint32 val) {
		_out.push_back(static_cast<byte>(val & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>((val >> 16) & 0xff));
	}

	virtual void write4(uint32 val) {
		_out.push_back(static_cast<byte>(val & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>((val >> 16) & 0xff));
		_out.push_back(static_cast<byte>((val >> 24) & 0xff));
	}

	virtual void write4high(uint32 val) {
		_out.push_back(static_cast<byte>((val >> 24) & 0xff));
		_out.push_back(static_cast<byte>((val >> 16) & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>(val & 0xff));
	}

	virtual void write(const void *b, uint32 length) {
		write(b, length, length);
	}

	virtual void write(const void *b, uint32 length, uint32 pad_length) {
		for (uint32 i = 0; i < length; i++)
			_out.push_back(static_cast<const char *>(b)[i]);
		if (pad_length > length)
			for (pad_length -= length; pad_length > 0; --pad_length)
				_out.push_back(static_cast<byte>(0x00));
	}

	virtual void clear()          {
		_out.clear();
	}

	virtual void seek(uint32 /*pos*/) {
		/*_out->seekp(pos); FIXME: Do something here. */
	}
	virtual void skip(int32 /*pos*/) {
		/*_out->seekp(pos, Std::ios::cur); FIXME: Do something here. */
	}

	virtual uint32 getSize()      {
		return static_cast<uint32>(_out.size());
	}

	virtual uint32 getPos() {
		return static_cast<uint32>(_out.size()); /*return _out->tellp(); FIXME: Do something here. */
	}

};

class OAutoBufferDataSource: public ODataSource {
protected:
	uint8 *buf;
	uint8 *buf_ptr;
	uint32 size;
	uint32 loc;
	uint32 allocated;

	void checkResize(uint32 num_bytes) {
		// Increment loc
		loc += num_bytes;

		// Don't need to resize
		if (loc <= size) return;

		// Reallocate the buffer
		if (loc > allocated) {
			// The old pointer position
			uint32 pos = static_cast<uint32>(buf_ptr - buf);

			// The new buffer and size (2 times what is needed)
			allocated = loc * 2;
			uint8 *new_buf = new uint8[allocated];

			memcpy(new_buf, buf, size);
			delete [] buf;

			buf = new_buf;
			buf_ptr = buf + pos;
		}

		// Update size
		size = loc;
	}

public:
	OAutoBufferDataSource(uint32 initial_len) {
		allocated = initial_len;
		size = 0;

		// Make the min allocated size 16 bytes
		if (allocated < 16) allocated = 16;
		buf = buf_ptr = new uint8[allocated];
		loc = 0;
	};

	//! Get a pointer to the data buffer.
	const uint8 *getBuf() {
		return buf;
	}

	virtual ~OAutoBufferDataSource() override {
		delete [] buf_ptr;
	}

	virtual void write1(uint32 val) override {
		checkResize(1);
		*buf_ptr++ = val & 0xff;
	};

	virtual void write2(uint16 val) override {
		checkResize(2);
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
	};

	virtual void write2high(uint16 val) override {
		checkResize(2);
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = val & 0xff;
	};

	virtual void write3(uint32 val) override {
		checkResize(3);
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = (val >> 16) & 0xff;
	};

	virtual void write4(uint32 val) override {
		checkResize(4);
		*buf_ptr++ = val & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = (val >> 16) & 0xff;
		*buf_ptr++ = (val >> 24) & 0xff;
	};

	virtual void write4high(uint32 val) override {
		checkResize(4);
		*buf_ptr++ = (val >> 24) & 0xff;
		*buf_ptr++ = (val >> 16) & 0xff;
		*buf_ptr++ = (val >> 8) & 0xff;
		*buf_ptr++ = val & 0xff;
	};

	virtual void write(const void *b, uint32 len) override {
		checkResize(len);
		Common::copy((const byte *)b, (const byte *)b + len, buf_ptr);
		buf_ptr += len;
	};

	virtual void seek(uint32 pos) override {
		// No seeking past the end of the buffer
		if (pos <= size) loc = pos;
		else loc = size;

		buf_ptr = const_cast<unsigned char *>(buf) + loc;
	};

	virtual void skip(int32 pos) override {
		// No seeking past the end
		if (pos >= 0) {
			loc += pos;
			if (loc > size) loc = size;
		}
		// No seeking past the start
		else {
			uint32 invpos = -pos;
			if (invpos > loc) invpos = loc;
			loc -= invpos;
		}
		buf_ptr = const_cast<unsigned char *>(buf) + loc;
	};

	virtual uint32 getSize() const override {
		return size;
	};

	virtual uint32 getPos() const override {
		return static_cast<uint32>(buf_ptr - buf);
	};

	// Don't actually do anything substantial
	virtual void clear() {
		buf_ptr = buf;
		size = 0;
		loc = 0;
	}

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
