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

#ifndef ULTIMA8_FILESYS_IDATASOURCE_H
#define ULTIMA8_FILESYS_IDATASOURCE_H

#include "common/file.h"
#include "common/stream.h"
#include "ultima/shared/std/misc.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class IDataSource : public Common::SeekableReadStream {
public:
	IDataSource() {}
	virtual ~IDataSource() {}

	//  Read a 3-byte value, lsb first.
	virtual uint32 readUint24LE() {
		uint32 val = 0;
		val |= static_cast<uint32>(readByte());
		val |= static_cast<uint32>(readByte() << 8);
		val |= static_cast<uint32>(readByte() << 16);
		return val;
	}

	uint32 readX(uint32 num_bytes) {
		assert(num_bytes > 0 && num_bytes <= 4);
		if (num_bytes == 1) return readByte();
		else if (num_bytes == 2) return readUint16LE();
		else if (num_bytes == 3) return readUint24LE();
		else return readUint32LE();
	}

	int32 readXS(uint32 num_bytes) {
		assert(num_bytes > 0 && num_bytes <= 4);
		if (num_bytes == 1) return static_cast<int8>(readByte());
		else if (num_bytes == 2) return static_cast<int16>(readUint16LE());
		else if (num_bytes == 3) return (((static_cast<int32>(readUint24LE())) << 8) >> 8);
		else return static_cast<int32>(readUint32LE());
	}

	void readline(Std::string &str) {
		str.clear();
		while (!eos()) {
			char character =  static_cast<char>(readByte());

			if (character == '\r') continue;    // Skip cr
			else if (character == '\n') break;  // break on line feed

			str += character;
		}
	}
};


class IFileDataSource: public IDataSource {
private:
	Common::SeekableReadStream *_in;

public:
	IFileDataSource(Common::SeekableReadStream *data_stream) : _in(data_stream) {
	}

	~IFileDataSource() override {
		delete _in;
	}

	uint32 read(void *b, uint32 len) override {
		return _in->read(b, len);
	}

	bool seek(int32 position, int whence = SEEK_SET) override {
		return _in->seek(position, whence);
	}

	int32 size() const override {
		return _in->size();
	}

	int32 pos() const override {
		return _in->pos();
	}

	bool eos() const override {
		return _in->eos();
	}
};

class IBufferDataSource : public IDataSource {
protected:
	const uint8 *_buf;
	const uint8 *_bufPtr;
	bool _freeBuffer;
	uint32 _size;

public:
	IBufferDataSource(const void *data, unsigned int len, bool is_text = false,
	                  bool delete_data = false) : _size(len), _freeBuffer(delete_data) {
		assert(!is_text);
		assert(data != nullptr || len == 0);
		_buf = _bufPtr = static_cast<const uint8 *>(data);
	}

	virtual void load(const void *data, unsigned int len, bool is_text = false,
	                  bool delete_data = false) {
		assert(!is_text);
		if (_freeBuffer && _buf)
			delete[] const_cast<uint8 *>(_buf);
		_freeBuffer = false;
		_buf = _bufPtr = nullptr;

		assert(data != nullptr || len == 0);
		_buf = _bufPtr = static_cast<const uint8 *>(data);
		_size = len;
		_freeBuffer = delete_data;
	}

	~IBufferDataSource() override {
		if (_freeBuffer && _buf)
			delete[] const_cast<uint8 *>(_buf);
		_freeBuffer = false;
		_buf = _bufPtr = nullptr;
	}

	uint32 read(void *str, uint32 num_bytes) override {
		if (_bufPtr >= _buf + _size) return 0;
		uint32 count = num_bytes;
		if (_bufPtr + num_bytes > _buf + _size)
			count = static_cast<int32>(_buf - _bufPtr + _size);
		Std::memcpy(str, _bufPtr, count);
		_bufPtr += count;
		return count;
	}

	bool seek(int32 position, int whence = SEEK_SET) override {
		assert(whence == SEEK_SET || whence == SEEK_CUR);
		if (whence == SEEK_CUR) {
			_bufPtr += position;
		} else if (whence == SEEK_SET) {
			_bufPtr = _buf + position;
		}
		return true;
	}

	int32 size() const override {
		return _size;
	}

	int32 pos() const override {
		return static_cast<int32>(_bufPtr - _buf);
	}

	bool eos() const override {
		return (static_cast<uint32>(_bufPtr - _buf)) >= _size;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
