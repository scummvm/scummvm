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

#ifndef NUVIE_FILES_NUVIE_IO_H
#define NUVIE_FILES_NUVIE_IO_H

#include "common/stream.h"

namespace Ultima {
namespace Nuvie {

class Configuration;

class NuvieIO {
protected:
	uint32 size;
	uint32 pos;
public:
	NuvieIO();
	virtual ~NuvieIO();

	virtual void close() {
		size = 0;
		pos = 0;
	};

	virtual uint8 read1() {
		return 0;
	};
	virtual uint16 read2() {
		return 0;
	};
	virtual uint32 read4() {
		return 0;
	};

	unsigned char *readAll();
	unsigned char *readBuf(uint32 read_size, uint32 *bytes_read);
	virtual bool readToBuf(unsigned char *buf, uint32 buf_size) {
		return false;
	};


	virtual bool write1(uint8 src) {
		return false;
	};
	virtual bool write2(uint16 src) {
		return false;
	};
	virtual bool write4(uint32 src) {
		return false;
	};
	virtual uint32 writeBuf(const unsigned char *src, uint32 src_size) {
		return 0;
	};
	virtual uint32 write(NuvieIO *src) {
		return 0;
	};

	uint32 get_size() {
		return size;
	};


	inline void seekStart() {
		seek(0);
	};
	inline void seekEnd() {
		seek(size);
	};
	virtual void seek(uint32 new_pos) = 0;

	inline bool is_end() {
		return (pos == size - 1);
	};
	inline bool is_eof() {
		return (size == 0 || pos >= size);
	};
	uint32 position() {
		return pos;
	};
};

#define NUVIE_BUF_COPY   true
#define NUVIE_BUF_NOCOPY false


class NuvieIOBuffer: public NuvieIO {
protected:

	unsigned char *data;
	bool copied_data;

public:
	NuvieIOBuffer();
	~NuvieIOBuffer() override;

	bool open(unsigned char *buf, uint32 buf_size, bool copy_buf = NUVIE_BUF_COPY);

	void close() override;

	unsigned char *get_raw_data() {
		return data;
	}; //hehe evil

	uint8 read1() override;
	uint16 read2() override;
	uint32 read4() override;
	bool readToBuf(unsigned char *buf, uint32 buf_size) override;

	bool write1(uint8 src) override;
	bool write2(uint16 src) override;
	bool write4(uint32 src) override;
	uint32 writeBuf(const unsigned char *src, uint32 src_size) override;
	uint32 write(NuvieIO *src) override;

	void seek(uint32 new_pos) override;
};

extern char *strgets(char *str, int n, Common::ReadStream *stream);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
