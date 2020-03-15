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

	virtual Common::WriteStream *GetRawStream() {
		return nullptr;
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

	~OFileDataSource() override {
		FORGET_OBJECT(_out);
	}

	bool good() const {
		return !_out->err();
	}

	void write1(uint32 val) override {
		_out->writeByte(val & 0xff);
	}

	void write2(uint16 val) override {
		_out->writeUint16LE(val);
	}

	void write2high(uint16 val) override {
		_out->writeUint16BE(val);
	}

	void write3(uint32 val) override {
		_out->writeByte(static_cast<byte>(val & 0xff));
		_out->writeByte(static_cast<byte>((val >> 8) & 0xff));
		_out->writeByte(static_cast<byte>((val >> 16) & 0xff));
	}

	void write4(uint32 val) override {
		_out->writeUint32LE(val);
	}

	void write4high(uint32 val) override {
		_out->writeUint32BE(val);
	}

	void write(const void *b, uint32 len) override {
		_out->write(static_cast<const char *>(b), len);
	}

	void seek(uint32 pos) override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		ws->seek(pos);
	}

	void skip(int32 amount) override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		ws->seek(amount, SEEK_CUR);
	}

	uint32 getSize() const override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		return ws->size();
	}

	uint32 getPos() const override {
		Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_out);
		assert(ws);
		return _out->pos();
	}

	Common::WriteStream *GetRawStream() override {
		return _out;
	}
};

class OBufferDataSource: public ODataSource {
protected:
	uint8 *_buf;
	uint8 *_bufPtr;
	uint32 _size;
public:
	OBufferDataSource(void *data, uint32 len) {
		assert(data == 0 || len == 0);
		_buf = _bufPtr = reinterpret_cast<uint8 *>(data);
		_size = len;
	};

	void load(char *data, uint32 len) {
		assert(data == 0 || len == 0);
		_buf = _bufPtr = reinterpret_cast<uint8 *>(data);
		_size = len;
	};

	~OBufferDataSource() override {};

	void write1(uint32 val) override {
		*_bufPtr++ = val & 0xff;
	};

	void write2(uint16 val) override {
		*_bufPtr++ = val & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
	};

	void write2high(uint16 val) override {
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = val & 0xff;
	};

	void write3(uint32 val) override {
		*_bufPtr++ = val & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = (val >> 16) & 0xff;
	};

	void write4(uint32 val) override {
		*_bufPtr++ = val & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = (val >> 16) & 0xff;
		*_bufPtr++ = (val >> 24) & 0xff;
	};

	void write4high(uint32 val) override {
		*_bufPtr++ = (val >> 24) & 0xff;
		*_bufPtr++ = (val >> 16) & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = val & 0xff;
	};

	void write(const void *b, uint32 len) override {
		Common::copy((const byte *)b, (const byte *)b + len, _bufPtr);
		_bufPtr += len;
	};

	void seek(uint32 pos) override {
		_bufPtr = const_cast<unsigned char *>(_buf) + pos;
	};

	void skip(int32 pos) override {
		_bufPtr += pos;
	};

	uint32 getSize() const override {
		return _size;
	};

	uint32 getPos() const override {
		return static_cast<uint32>(_bufPtr - _buf);
	};
};

class ODequeDataSource : public ODataSource {
private:
	Std::deque<byte> _out;

public:
	ODequeDataSource() {}

	~ODequeDataSource() override {}

	const Std::deque<byte> &buf() const {
		return _out;
	}

	void write1(uint32 val) override {
		_out.push_back(static_cast<byte>(val & 0xff));
	}

	void write2(uint16 val) override {
		_out.push_back(static_cast<byte>(val & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
	}

	void write2high(uint16 val) override {
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>(val & 0xff));
	}

	void write3(uint32 val) override {
		_out.push_back(static_cast<byte>(val & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>((val >> 16) & 0xff));
	}

	void write4(uint32 val) override {
		_out.push_back(static_cast<byte>(val & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>((val >> 16) & 0xff));
		_out.push_back(static_cast<byte>((val >> 24) & 0xff));
	}

	void write4high(uint32 val) override {
		_out.push_back(static_cast<byte>((val >> 24) & 0xff));
		_out.push_back(static_cast<byte>((val >> 16) & 0xff));
		_out.push_back(static_cast<byte>((val >> 8) & 0xff));
		_out.push_back(static_cast<byte>(val & 0xff));
	}

	void write(const void *b, uint32 length) override {
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

	void seek(uint32 /*pos*/) override {
		/*_out->seekp(pos); FIXME: Do something here. */
	}
	void skip(int32 /*pos*/) override {
		/*_out->seekp(pos, Std::ios::cur); FIXME: Do something here. */
	}

	uint32 getSize() const override {
		return static_cast<uint32>(_out.size());
	}

	uint32 getPos() const override {
		return static_cast<uint32>(_out.size()); /*return _out->tellp(); FIXME: Do something here. */
	}
};

class OAutoBufferDataSource: public ODataSource {
protected:
	uint8 *_buf;
	uint8 *_bufPtr;
	uint32 _size;
	uint32 _loc;
	uint32 _allocated;

	void checkResize(uint32 num_bytes) {
		// Increment loc
		_loc += num_bytes;

		// Don't need to resize
		if (_loc <= _size) return;

		// Reallocate the buffer
		if (_loc > _allocated) {
			// The old pointer position
			uint32 pos = static_cast<uint32>(_bufPtr - _buf);

			// The new buffer and size (2 times what is needed)
			_allocated = _loc * 2;
			uint8 *new_buf = new uint8[_allocated];

			memcpy(new_buf, _buf, _size);
			delete [] _buf;

			_buf = new_buf;
			_bufPtr = _buf + pos;
		}

		// Update size
		_size = _loc;
	}

public:
	OAutoBufferDataSource(uint32 initial_len) {
		_allocated = initial_len;
		_size = 0;

		// Make the min allocated size 16 bytes
		if (_allocated < 16)
			_allocated = 16;
		_buf = _bufPtr = new uint8[_allocated];
		_loc = 0;
	};

	//! Get a pointer to the data buffer.
	const uint8 *getBuf() {
		return _buf;
	}

	~OAutoBufferDataSource() override {
		delete [] _buf;
	}

	void write1(uint32 val) override {
		checkResize(1);
		*_bufPtr++ = val & 0xff;
	};

	void write2(uint16 val) override {
		checkResize(2);
		*_bufPtr++ = val & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
	};

	void write2high(uint16 val) override {
		checkResize(2);
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = val & 0xff;
	};

	void write3(uint32 val) override {
		checkResize(3);
		*_bufPtr++ = val & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = (val >> 16) & 0xff;
	};

	void write4(uint32 val) override {
		checkResize(4);
		*_bufPtr++ = val & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = (val >> 16) & 0xff;
		*_bufPtr++ = (val >> 24) & 0xff;
	};

	void write4high(uint32 val) override {
		checkResize(4);
		*_bufPtr++ = (val >> 24) & 0xff;
		*_bufPtr++ = (val >> 16) & 0xff;
		*_bufPtr++ = (val >> 8) & 0xff;
		*_bufPtr++ = val & 0xff;
	};

	void write(const void *b, uint32 len) override {
		checkResize(len);
		Common::copy((const byte *)b, (const byte *)b + len, _bufPtr);
		_bufPtr += len;
	};

	void seek(uint32 pos) override {
		// No seeking past the end of the buffer
		if (pos <= _size) _loc = pos;
		else _loc = _size;

		_bufPtr = const_cast<unsigned char *>(_buf) + _loc;
	};

	void skip(int32 pos) override {
		// No seeking past the end
		if (pos >= 0) {
			_loc += pos;
			if (_loc > _size) _loc = _size;
		}
		// No seeking past the start
		else {
			uint32 invpos = -pos;
			if (invpos > _loc) invpos = _loc;
			_loc -= invpos;
		}
		_bufPtr = const_cast<unsigned char *>(_buf) + _loc;
	};

	uint32 getSize() const override {
		return _size;
	};

	uint32 getPos() const override {
		return static_cast<uint32>(_bufPtr - _buf);
	};

	// Don't actually do anything substantial
	virtual void clear() {
		_bufPtr = _buf;
		_size = 0;
		_loc = 0;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
